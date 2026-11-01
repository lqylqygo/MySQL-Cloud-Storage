#include"worker.h"
#include"master.h"
void workerLoop(int socketfd)
{
    while(1){
        int connfd=recv_fd(socketfd);
        printf("[Worker %d] 收到新连接 (connfd=%d)，开始处理请求\n", getpid(), connfd);
    }
}