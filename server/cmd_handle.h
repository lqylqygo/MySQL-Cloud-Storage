#ifndef MY__CMD_HANDLE_H__
#define MY__CMD_HANDLE_H__
#include <my_header.h>
#include"threadpool.h"
#include"protocol.h"
void doTask(task_t *ptask);
void pwdCommand(task_t *task);
void putsCommand(task_t *task);
void getsCommand(task_t *task);
void lsCommand(task_t *task);
void cdCommand(task_t *task);
void mkdirCommand(task_t *task);
void removeCommand(task_t *task);


#endif