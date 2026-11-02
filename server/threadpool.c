#include"threadpool.h"

void queueInit(Queue *q) {
    q->head = NULL;
    q->tail = NULL;
    q->size = 0;
    q->shutdown = 0;
    pthread_mutex_init(&q->lock, NULL);
    pthread_cond_init(&q->cond, NULL);
}

void queuePush(Queue *q, task_t *ptask) {
   if(ptask == NULL)return;
   pthread_mutex_lock(&q->lock);
   ptask->pnext = NULL;
   if (q->tail == NULL) {
       q->head = ptask;
       q->tail = ptask;
   } else {
       q->tail->pnext = ptask;
       q->tail = ptask; 
   }
   q->size++;
    pthread_cond_signal(&q->cond); 
    pthread_mutex_unlock(&q->lock);
}
task_t *queuePop(Queue *q) {
    pthread_mutex_lock(&q->lock);
    while (q->size == 0 && !q->shutdown) {
        pthread_cond_wait(&q->cond, &q->lock);
    }
    if (q->shutdown&&q->size) {
        pthread_mutex_unlock(&q->lock);
        return NULL;
    }
    task_t *ptask = q->head;
    if (ptask != NULL) {
        q->head = ptask->pnext;
        if (q->head == NULL) {
            q->tail = NULL; 
        }
        q->size--;
    }
    pthread_mutex_unlock(&q->lock);
    return ptask;
}

task_t* queueFront(Queue *q) {
    pthread_mutex_lock(&q->lock);
    task_t *ptask = q->head; // 只是读取指针，没有改变链表结构
    pthread_mutex_unlock(&q->lock);
    return ptask;
}

void *threadFunc(void *arg) {
    ThreadPool *pool = (ThreadPool *)arg;
    
    while (1) {
        // 1. 尝试从队列拿任务（没任务就卡在这里等）
        task_t *ptask = queuePop(&pool->queue);
        
        // 2. 如果拿到 NULL，说明线程池要被销毁了，退出循环
        if (ptask == NULL) {
            break;
        }

        // 3. 执行业务逻辑（去 doTask 里 switch-case）
        doTask(ptask);
        
        // 4. 任务执行完毕，释放节点！（对应 PPT Page 7 第 14 行）
        free(ptask); 
    }
    
    printf("sub thread %ld is exiting.\n", pthread_self());
    return NULL;
}
ThreadPool* threadpoolCreate(int thread_count) {
    if (thread_count <= 0) return NULL;

    ThreadPool *pool = (ThreadPool *)malloc(sizeof(ThreadPool));
    queueInit(&pool->queue); // 初始化里面的队列
    pool->thread_count = thread_count;
    pool->threads = (pthread_t *)malloc(sizeof(pthread_t) * thread_count);

    for (int i = 0; i < thread_count; i++) {
        // 创建线程，执行 threadFunc，把 pool 传进去
        pthread_create(&pool->threads[i], NULL, threadFunc, pool);
        pthread_detach(pool->threads[i]); // 线程分离，自动回收
    }
    return pool;
}
void queueDestroy(Queue *q) {
    pthread_mutex_lock(&q->lock);
    q->shutdown = 1;
    pthread_cond_broadcast(&q->cond); // 唤醒所有睡眠的子线程
    pthread_mutex_unlock(&q->lock);

    // 清理队列中残留的任务
    task_t *cur = q->head;
    while (cur != NULL) {
        task_t *next = cur->pnext;
        free(cur);
        cur = next;
    }
    pthread_mutex_destroy(&q->lock);
    pthread_cond_destroy(&q->cond);
}
void threadpoolDestroy(ThreadPool *pool) {
    if (pool == NULL) return;
    queueDestroy(&pool->queue);
    free(pool->threads);
    free(pool);
}
