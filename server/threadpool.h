#ifndef MY__THREADPOOL_H__
#define MY__THREADPOOL_H__
#include <my_header.h>
#include"protocol.h"

typedef struct task_s {
    int peerfd;          // 客户端 socket
    int epfd;            // Worker 进程的 epoll 句柄（为了 PUTS 结束后重新注册）
    CmdType type;        // 命令类型
    char data[1000];     // 消息内容（如密码密文、文件名）
    struct task_s *pnext; // 队列的下一个节点
} task_t;

typedef struct Queue {
    task_t *head;            // 队头指针，用于出队
    task_t *tail;            // 队尾指针，用于入队
    int size;                // 队列中任务的数量
    int shutdown;            // 销毁标志（0:运行中, 1:准备销毁）
    
    pthread_mutex_t lock;    // 互斥锁，保护队列
    pthread_cond_t cond;     // 条件变量，唤醒阻塞的子线程
} Queue;
typedef struct ThreadPool {
    Queue queue;             // 每个线程池内部包含一个任务队列
    pthread_t *threads;      // 线程数组
    int thread_count;        // 线程数量
} ThreadPool;
void queueInit(Queue *q);
void queuePush(Queue *q, task_t *ptask); // 入队
task_t* queuePop(Queue *q);              // 出队
void printfqueue(Queue *q); // 打印队列内容
task_t* queueFront(Queue *q);// 获取队头任务
void queueDestroy(Queue *q);

ThreadPool* threadpoolCreate(int thread_count);
void threadpoolDestroy(ThreadPool *pool);

#endif