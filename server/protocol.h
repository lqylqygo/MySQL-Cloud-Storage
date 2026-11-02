#ifndef __MY_PROTOCOL_H__
#define __MY_PROTOCOL_H__
#include<my_header.h>
typedef struct train_s{
    int len;
    int type;
    char buff[4096];
}train_t;//TLV协议火车头

typedef enum{
    CMD_TYPE_PWD = 1,
    CMD_TYPE_LS,
    CMD_TYPE_CD,
    CMD_TYPE_MKDIR,
    CMD_TYPE_RMDIR,//删除文件夹（目录）
    CMD_TYPE_PUTS,
    CMD_TYPE_GETS,
    CMD_TYPE_RM,//删除文件
    CMD_TYPE_NOTCMD,//非法命令

     /* ---- 二期登录（PPT Page 23） ---- */
    TASK_LOGIN_SECTION1              = 100,
    TASK_LOGIN_SECTION1_RESP_OK,
    TASK_LOGIN_SECTION1_RESP_ERROR,
    TASK_LOGIN_SECTION2,
    TASK_LOGIN_SECTION2_RESP_OK,
    TASK_LOGIN_SECTION2_RESP_ERROR,

    /* ---- 三期注册（PPT Page 40） ---- */
    CMD_TYPE_USERNAME                = 200,  /* 发用户名 */
    MSG_TYPE_SALT,                           /* 回 salt */
    CMD_TYPE_ENCRYPTEDCODE,                  /* 发密文 */
    MSG_TYPE_REGISTER_OK_CODE,               /* 注册成功 */
    MSG_TYPE_REGISTER_ERROR_CODE             /* 注册失败 */
} CmdType;


int recvn(int sockfd, void *buff, int len);//解决粘包问题
int sendn(int sockfd, const void *buff, int len);//解决半包问题
#endif