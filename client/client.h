#ifndef MY__CLIENT_H
#define MY__CLIENT_H
#include<my_header.h>
typedef enum{
    CMD_TYPE_PWD=1,
    CMD_TYPE_LS,
    CMD_TYPE_CD,
    CMD_TYPE_MKDIR,
    CMD_TYPE_RMDIR,
    CMD_TYPE_NOTCMD,
}CmdType;

typedef struct
{
    int len;//L
    CmdType type;//T
    char buff[1000];//V
}train_t;

int tcpConnect(const char *ip,unsigned short port);
int recvn(int sockfd, void *buff,int len);//解决粘包问题
int sendn(int sockfd,const void *buff,int len);//解决半包问题

int parseCommand(const char *input,int len ,train_t*pt);

int getCommandType(const char *str);
#endif
