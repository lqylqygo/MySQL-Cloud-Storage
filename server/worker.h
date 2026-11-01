#ifndef MY__WORKER_H__
#define MY__WORKER_H__
#include <my_header.h>
#include"master.h"
#include "protocol.h"   // 包含 train_t, recvn, sendn 等
#include "threadpool.h" // 包含 Queue, task_t 等
#include "cmd_handle.h" // 包含 CmdType 等
void handleMessage(int connfd, int epfd,ThreadPool *pool); 
void workerLoop(int socket,int threadnum);



#endif