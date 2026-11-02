#include "protocol.h"


int recvn(int sockfd, void *buff, int len)//解决粘包问题
{
    int total_recv = 0;
    char *ptr = (char *)buff;
    while (total_recv < len) {
        int ret = recv(sockfd, ptr + total_recv, len - total_recv, 0);
        if (ret == -1) {
            if (errno == EINTR) {
                continue;
            }
            perror("recv failed");
            return -1;
        }
        if (ret == 0) {
            printf("客户端断开连接了\n");
            return total_recv;
        }
        total_recv += ret;
    }
    return total_recv;
}

int sendn(int sockfd, const void *buff, int len)//解决半包问题
{
    int total_send = 0;
    const char *ptr = (const char *)buff;
    while (total_send < len) {
        int ret = send(sockfd, ptr + total_send, len - total_send, 0);
        if (ret == -1) {
            if (errno == EINTR) {
                continue;
            }
            perror("send failed");
            return -1;
        }
        total_send += ret;
    }
    return total_send;
}
