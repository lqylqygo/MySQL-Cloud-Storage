#include "client.h"

int tcpConnect(const char *ip, unsigned short port)
{
    int client_socketfd = socket(AF_INET, SOCK_STREAM, 0);
    ERROR_CHECK(client_socketfd, -1, "client socket failed");

    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr = inet_addr(ip);
    int connect_ret = connect(client_socketfd, (struct sockaddr *)&address, sizeof(address));
    ERROR_CHECK(connect_ret, -1, "connect failed");

    return client_socketfd;
}

int recvn(int sockfd, void *buff, int len)//解决粘包问题
{
    int total_recv = 0;
    char *ptr = (char *)buff;
    while (total_recv < len) {
        int ret = recv(sockfd, ptr + total_recv, len - total_recv, 0);
        ERROR_CHECK(ret, -1, "recv failed");
        if (ret == 0) {
            printf("服务器和客户端断开连接了\n");
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
        ERROR_CHECK(ret, -1, "send failed");
        if (ret == 0) {
            printf("服务器和客户端断开连接了\n");
            return total_send;
        }
        total_send += ret;
    }
    return total_send;
}

/* 发送一包：先发 len+type 共 8 字节，再按 len 发数据体 */
int sendTrain(int sockfd, const train_t *pt)
{
    if (pt == NULL) {
        return -1;
    }
    int header[2];
    header[0] = pt->len;
    header[1] = pt->type;

    if (sendn(sockfd, header, sizeof(header)) != (int)sizeof(header)) {
        return -1;
    }
    if (pt->len > 0 && sendn(sockfd, pt->buff, pt->len) != pt->len) {
        return -1;
    }
    return 0;
}

/* 接收一包：先收 8 字节报头，再按 len 收数据体 */
int recvTrain(int sockfd, train_t *pt)
{
    if (pt == NULL) {
        return -1;
    }
    memset(pt, 0, sizeof(train_t));

    if (recvn(sockfd, pt, sizeof(int) * 2) != (int)(sizeof(int) * 2)) {
        return -1;
    }
    if (pt->len < 0 || pt->len > (int)sizeof(pt->buff)) {
        printf("非法的数据长度: %d\n", pt->len);
        return -1;
    }
    if (pt->len > 0 && recvn(sockfd, pt->buff, pt->len) != pt->len) {
        return -1;
    }
    return 0;
}

int getCommandType(const char *str)
{
    if (str == NULL) {
        return CMD_TYPE_NOTCMD;
    }
    if (strcmp(str, "pwd") == 0) {
        return CMD_TYPE_PWD;
    }
    if (strcmp(str, "ls") == 0) {
        return CMD_TYPE_LS;
    }
    if (strcmp(str, "cd") == 0) {
        return CMD_TYPE_CD;
    }
    if (strcmp(str, "mkdir") == 0) {
        return CMD_TYPE_MKDIR;
    }
    if (strcmp(str, "rmdir") == 0) {
        return CMD_TYPE_RMDIR;
    }
    if (strcmp(str, "rm") == 0) {
        return CMD_TYPE_RM;
    }
    if (strcmp(str, "puts") == 0) {
        return CMD_TYPE_PUTS;
    }
    if (strcmp(str, "gets") == 0) {
        return CMD_TYPE_GETS;
    }
    return CMD_TYPE_NOTCMD;
}

/* 解析一行输入：命令字 + 参数，参数放进 buff，只做拆词不做业务校验 */
int parseCommand(const char *input, int len, train_t *pt)
{
    if (input == NULL || pt == NULL || len <= 0) {
        return -1;
    }

    char buf[4096];
    if (len > (int)sizeof(buf) - 1) {
        len = (int)sizeof(buf) - 1;
    }
    memcpy(buf, input, len);
    buf[len] = '\0';
    buf[strcspn(buf, "\r\n")] = '\0';//去掉换行

    char *cmd = buf;
    while (*cmd == ' ' || *cmd == '\t') {
        ++cmd;
    }
    if (*cmd == '\0') {
        return -1;//空行，不发送
    }

    char *args = cmd;
    while (*args != '\0' && *args != ' ' && *args != '\t') {
        ++args;
    }
    if (*args == '\0') {
        args = NULL;
    } else {
        *args = '\0';
        ++args;
        while (*args == ' ' || *args == '\t') {
            ++args;
        }
        if (*args == '\0') {
            args = NULL;
        }
    }

    memset(pt, 0, sizeof(train_t));
    pt->type = getCommandType(cmd);

    if (args != NULL) {
        pt->len = (int)strlen(args);
        if (pt->len > (int)sizeof(pt->buff)) {
            pt->len = (int)sizeof(pt->buff);
        }
        memcpy(pt->buff, args, pt->len);
    } else {
        pt->len = 0;
    }
    return 0;
}
