#include "client.h"

int main(int argc, char *argv[])
{
    if (argc != 3) {
        printf("usage: %s <ip> <port>\n", argv[0]);
        return -1;
    }

    int sockfd = tcpConnect(argv[1], (unsigned short)atoi(argv[2]));
    printf("已连接 %s:%s\n", argv[1], argv[2]);

    char line[4096] = {0};
    while (1) {
        printf("> ");
        fflush(stdout);

        if (fgets(line, sizeof(line), stdin) == NULL) {
            break;//Ctrl+D 退出
        }

        train_t t;
        if (parseCommand(line, (int)strlen(line), &t) != 0) {
            continue;//空行
        }

        if (sendTrain(sockfd, &t) != 0) {
            printf("发送失败\n");
            break;
        }

        /* 目前只有 pwd 有响应，其余命令服务端只打印日志 */
        if (t.type == CMD_TYPE_PWD) {
            train_t resp;
            if (recvTrain(sockfd, &resp) != 0) {
                printf("接收失败\n");
                break;
            }
            printf("%s\n", resp.buff);
        } else if (t.type == CMD_TYPE_NOTCMD) {
            printf("未知命令\n");
        } else {
            printf("该命令尚未实现\n");
        }
    }

    close(sockfd);
    return 0;
}
