#include "cmd_handle.h"


/* 以下命令函数目前只是占位，业务留到各自模块里实现 */
static void pwdCommand(task_t *task)
{
    /* 三期接入虚拟文件系统后，这里改成查 session 的 cwd */
    const char *cwd = "/server/home/";
    printf("[Worker %d] 处理 pwd，返回 %s\n", getpid(), cwd);

    train_t t;
    memset(&t, 0, sizeof(t));
    t.type = CMD_TYPE_PWD;
    t.len = (int)strlen(cwd);
    memcpy(t.buff, cwd, t.len);

    int header[2];
    header[0] = t.len;
    header[1] = t.type;
    sendn(task->peerfd, header, sizeof(header));
    if (t.len > 0) {
        sendn(task->peerfd, t.buff, t.len);
    }
}

static void lsCommand(task_t *task)
{
    (void)task;
    printf("[task] ls\n");
}

static void cdCommand(task_t *task)
{
    (void)task;
    printf("[task] cd\n");
}

static void mkdirCommand(task_t *task)
{
    (void)task;
    printf("[task] mkdir\n");
}

static void rmdirCommand(task_t *task)
{
    (void)task;
    printf("[task] rmdir\n");
}

static void rmCommand(task_t *task)
{
    (void)task;
    printf("[task] rm\n");
}

static void putsCommand(task_t *task)
{
    (void)task;
    printf("[task] puts (长命令，等待接收文件)\n");
}

static void getsCommand(task_t *task)
{
    (void)task;
    printf("[task] gets (长命令，等待发送文件)\n");
}

static void notCommand(task_t *task)
{
    (void)task;
    printf("[task] 非法命令\n");
}

static void userLoginCheck1(task_t *task)
{
    (void)task;
    printf("[task] 登录第一阶段\n");
}

static void userLoginCheck2(task_t *task)
{
    (void)task;
    printf("[task] 登录第二阶段\n");
}

void doTask(task_t *task)
{
    assert(task);
    switch (task->type) {
    case CMD_TYPE_PWD:
        pwdCommand(task);
        break;
    case CMD_TYPE_LS:
        lsCommand(task);
        break;
    case CMD_TYPE_CD:
        cdCommand(task);
        break;
    case CMD_TYPE_MKDIR:
        mkdirCommand(task);
        break;
    case CMD_TYPE_RMDIR:
        rmdirCommand(task);
        break;
    case CMD_TYPE_RM:
        rmCommand(task);
        break;
    case CMD_TYPE_PUTS:
        putsCommand(task);
        /* 长命令执行完毕，把 connfd 重新交给主线程监听 */
        addEpollReadfd(task->epfd, task->peerfd);
        break;
    case CMD_TYPE_GETS:
        getsCommand(task);
        addEpollReadfd(task->epfd, task->peerfd);
        break;
    case CMD_TYPE_NOTCMD:
        notCommand(task);
        break;
    case TASK_LOGIN_SECTION1:
        userLoginCheck1(task);
        break;
    case TASK_LOGIN_SECTION2:
        userLoginCheck2(task);
        break;
    default:
        notCommand(task);
        break;
    }
}
