#include"master.h"

void send_fd(int socket, int fd_to_send)//发送fd
{
    struct msghdr msg;
    struct iovec iov;
    char buf[1] = {'x'}; // 发送一个字节的数据
    iov.iov_base = buf;
    iov.iov_len = sizeof(buf);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    
    char cmsgbuf[CMSG_SPACE(sizeof(int))];
    msg.msg_control = cmsgbuf;
    msg.msg_controllen = sizeof(cmsgbuf);

    struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_RIGHTS;
    cmsg->cmsg_len = CMSG_LEN(sizeof(int));
    *((int *)CMSG_DATA(cmsg)) = fd_to_send;//把fd塞进去
    int sendmsg_ret = sendmsg(socket, &msg, 0);
    ERROR_CHECK(sendmsg_ret, -1, "sendmsg failed");
}

int recv_fd(int socket)
{
    struct msghdr msg;
    struct iovec iov;
    char buf[1];
    iov.iov_base = buf;
    iov.iov_len = sizeof(buf);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    char cmsgbuf[CMSG_SPACE(sizeof(int))];
    msg.msg_control = cmsgbuf;
    msg.msg_controllen = sizeof(cmsgbuf);

    int recvmsg_ret = recvmsg(socket, &msg, 0);
    ERROR_CHECK(recvmsg_ret, -1, "recvmsg failed");
    struct cmsghdr *cmsg = CMSG_FIRSTHDR(&msg);
    if(cmsg==NULL){
        fprintf(stderr, "No cmsg found.\n");
        return -1;
    }
    int recv_fd = *((int *)CMSG_DATA(cmsg));//把fd取出来
    return recv_fd;
}

void masterLoop(int listenfd, int *workers_fd, int worker_num)
{
    int idx =0;//轮询worker
    while(1)
    {
        int connectfd=accept(listenfd, NULL, NULL);
        ERROR_CHECK(connectfd, -1, "accept failed");
          printf("[Master] 收到新连接 (connfd=%d)，准备发给 Worker %d\n", connectfd, idx);
          send_fd(workers_fd[idx], connectfd);
          close(connectfd);
          idx=(idx+1)%worker_num;
    }
}