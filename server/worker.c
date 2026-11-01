#include"worker.h"

void handleMessage(int connfd, int epfd,ThreadPool *pool)
{
     train_t t;
    memset(&t, 0, sizeof(t));
    
    // 1. 接收 8 字节协议头 (4字节 len + 4字节 type)
    int ret = recvn(connfd, &t, 8); 
    if (ret <= 0) {
        // 客户端断开或出错
        printf("[Worker] 客户端 connfd=%d 断开连接\n", connfd);
        epoll_ctl(epfd, EPOLL_CTL_DEL, connfd, NULL);
        close(connfd);
        return;
    }

    // 2. 接收包体（如果有内容）
    if (t.len > 0) {
        recvn(connfd, t.buff, t.len);
    }

    // 3. 封装 Task
    task_t *ptask = (task_t *)calloc(1, sizeof(task_t));
    if (ptask == NULL) {
        perror("calloc task failed");
        return;
    }
    
    ptask->peerfd = connfd;
    ptask->epfd = epfd; // 把 epfd 交给子线程，方便以后增删监听
    ptask->type = t.type;
    if (t.len > 0) {
        memcpy(ptask->data, t.buff, t.len);
    }

    // 4. 长命令处理：暂时从 epoll 移除
    if (ptask->type == CMD_TYPE_PUTS || ptask->type == CMD_TYPE_GETS) {
        epoll_ctl(epfd, EPOLL_CTL_DEL, connfd, NULL);
        printf("[主线程] 长命令触发，connfd=%d 移出 epoll，专享给子线程\n", connfd);
    }

    // 5. 扔进队列
    // 这里直接用传进来的 pool，不需要 extern 全局变量了！
    queuePush(&pool->queue, ptask);
}

void workerLoop(int socketfd,int threadnum)
{
    ThreadPool *pool = threadpoolCreate(threadnum); 
    int epfd=epoll_create(1);
    ERROR_CHECK(epfd,-1,"epoll_create failed");
    struct epoll_event events[1024];
    struct epoll_event ev;
    ev.events=EPOLLIN;
    ev.data.fd=socketfd;
    int ctl_ret=epoll_ctl(epfd,EPOLL_CTL_ADD,socketfd,&ev);
    ERROR_CHECK(ctl_ret,-1,"epoll_ctl failed");
    printf("[Worker %d] 启动完毕，进入 epoll 循环，等待任务...\n", getpid());
    while(1){
        int nfds=epoll_wait(epfd,events,1024,-1);
        ERROR_CHECK(nfds,-1,"epoll_wait failed");
        for(int i=0;i<nfds;i++){
            int current_fd=events[i].data.fd;
            if(current_fd==socketfd){
                int connfd=recv_fd(socketfd);
                ERROR_CHECK(connfd,-1,"recv_fd failed");
            
                printf("[Worker %d] 收到新连接 (connfd=%d)，开始处理请求\n", getpid(), connfd);
            ev.events=EPOLLIN;
            ev.data.fd=connfd;

            int ctl2_ret=epoll_ctl(epfd,EPOLL_CTL_ADD,connfd,&ev);
            ERROR_CHECK(ctl2_ret,-1,"epoll_ctl failed");
            printf("[Worker %d] 已将 connfd=%d 添加到 epoll 监听中\n", getpid(), connfd);
            }
        
            else{
                if(events[i].events&EPOLLIN){
                    char buf[1];
                    int n=recv(current_fd,buf,sizeof(buf),MSG_PEEK);
                    if(n==0)
                    {
                        printf("[Worker %d] 客户端断开连接 (connfd=%d)\n", getpid(), current_fd);
                        int ctl3_ret=epoll_ctl(epfd,EPOLL_CTL_DEL,current_fd,NULL);
                        ERROR_CHECK(ctl3_ret,-1,"epoll_ctl failed");
                        close(current_fd);
                    }
                    else if(n>0){
                        printf("[Worker %d] 检测客户端有数据 (connfd=%d)，准备交给handle message\n", getpid(), current_fd);
                        handleMessage(current_fd, epfd, pool);
                    }else{
                        perror("recv failed");
                        epoll_ctl(epfd,EPOLL_CTL_DEL,current_fd,NULL);
                        close(current_fd);
                    }
                }
            }
        }
         threadpoolDestroy(pool);
    }
}