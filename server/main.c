#include"config.h"
#include"tcpinit.h"
#include"worker.h"
#include"master.h"
int main(){
    HashTable ht;
    initHashtable(&ht);
    readConfig("../conf/server.conf",&ht);
    char *ip=(char *)find(&ht,IP);
    char *port_str=(char *)find(&ht,PORT);
    char *worknum_str=(char *)find(&ht,WORKER_NUM);
    char *threadnum_str=(char *)find(&ht,THREAD_NUM);
    printf("ip=%s\n",ip);//打印
    printf("port=%s\n",port_str);
    printf("worknum=%s\n",worknum_str);
    printf("threadnum=%s\n",threadnum_str);
    int port=atoi(port_str);
    int worknum=atoi(worknum_str);
    int threadnum=atoi(threadnum_str);
    int listenfd=tcp_init(ip,port);
    int workers_fd[worknum];//存放master端的socketpair的fd
    for(int i=0;i<worknum;i++)
    {
        int sv[2];//socketpair的两个fd
        int socketpair_ret=socketpair(AF_UNIX,SOCK_STREAM,0,sv);
        ERROR_CHECK(socketpair_ret, -1, "socketpair failed");
        pid_t pid=fork();
        if(pid<0)
        {
            perror("fork failed");
            exit(1);
        }
        else if(pid==0)
        {
            close(sv[0]);
            //子进程
            close(listenfd);
            workerLoop(sv[1],threadnum);
        }
        else
        {
            //父进程
            close(sv[1]);
            workers_fd[i]=sv[0];
            
        }
    }
    masterLoop(listenfd,workers_fd,worknum);
    destroyHashtable(&ht);
    return 0;
}