#include"cmd_handle.h"

void doTask(task_t *ptask)
{
    if(ptask==NULL)return;
    switch(ptask->type)
    {
        case CMD_TYPE_PWD:
            pwdCommand(ptask);
            break;
        case CMD_TYPE_PUTS:
            putsCommand(ptask);
            break;
        case CMD_TYPE_GETS:
            getsCommand(ptask);
            break;
        case CMD_TYPE_LS:
            lsCommand(ptask);
            break;
        case CMD_TYPE_CD:
            cdCommand(ptask);
            break;
        case CMD_TYPE_MKDIR:
            mkdirCommand(ptask);
            break;
        case CMD_TYPE_RM:
            removeCommand(ptask);
            break;
        default:
            printf("[Worker] 未知命令类型: %d\n", ptask->type);
            break;
    }
}
void pwdCommand(task_t *task)
{
    //准备一个数据包
    train_t t;
    bzero(&t,sizeof(t));
    const char *pwd="/server/home/";//假设当前路径
    t.len=strlen(pwd);
    t.type=CMD_TYPE_PWD;
    strcpy(t.buff,pwd);
    sendn(task->peerfd,&t,8+t.len);
}
void putsCommand(task_t *task)
{
    printf("[Worker] 处理 PUTS 命令，connfd=%d, 文件名=%s\n", task->peerfd, task->data);
}