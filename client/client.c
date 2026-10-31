#include"client.h"


int tcpConnect(const char *ip,unsigned short port)
{
    int client_socketfd=socket(AF_INET,SOCK_STREAM,0);
    ERROR_CHECK(client_socketfd,-1,"client socket failed");
    
    struct sockaddr_in address;
    address.sin_family=AF_INET;
    address.sin_port = htons(port);
    address.sin_addr.s_addr=inet_addr(ip);
    int connect_ret=connect(client_socketfd,(struct sockaddr *)&address,sizeof(address));
    ERROR_CHECK(connect_ret,-1,"connect failed");

    return client_socketfd;

}

int recvn(int sockfd, void *buff,int len)//解决粘包问
{
    int total_recv=0;
    char *ptr=(char *)buff;
    while(total_recv<len){
        int ret=recv(sockfd,ptr+total_recv,len-total_recv,0);
        ERROR_CHECK(ret,-1,"recv failed");
        if(ret == 0){
            printf("服务器和客户端断开连接了\n");
            return total_recv;
        }
        total_recv+=ret;
    }
    return total_recv;
}
int sendn(int sockfd,const void *buff,int len)//解决半包问题
{
    int total_send=0;
    char *ptr=(char *)buff;
    while(total_send<len){
        int ret=send(sockfd,ptr+total_send,len-total_send,0);
        ERROR_CHECK(ret,-1,"send failed");
        if(ret == 0){
            printf("服务器和客户端断开连接了\n");
            return total_send;
        }
        total_send+=ret;
    }
    return total_send;

}
int parseCommand(const char *input,int len ,train_t*pt)//解析命令
{

}

int getCommandType(const char *str)
{

}
