/*
 * user_op.h — 注册 / 登录
 *
 * 所有函数都接收 dp_pool_t*（业务层自己 getConnection / releaseConnection，
 * 保持和之前经验里"拼好 SQL 再取连接"的顺序一致）。
 *
 * 登录成功时，还需要 sessionAdd 把该 peerfd 加入会话表；
 * session 参数是可选的，调用者如果不想在 user_op 里碰会话，可以传 NULL。
 */
#ifndef __USER_OP_H__
#define __USER_OP_H__

#include "../common/protocol.h"
#include "dp_pool.h"
#include "session.h"

/* 生成随机 salt：$6$ + 8 位随机字符 + $，返回 malloc 出来的字符串；调用者 free */
char *generateSalt(void);

/* 从已有的 cryptpasswd 里提取 setting 部分（整个 $id$salt$hash 串就是 setting） */
const char *getSetting(const char *cryptpasswd);

/* ===== 注册两阶段（CMD_TYPE_USERNAME → MSG_TYPE_SALT → CMD_TYPE_ENCRYPTEDCODE → OK/ERROR） =====
 * regPhase1：查重，存在 → 回 MSG_TYPE_REGISTER_ERROR_CODE
 *           不存在 → generateSalt，回 MSG_TYPE_SALT
 * regPhase2：INSERT user，成功回 MSG_TYPE_REGISTER_OK_CODE */
void regPhase1(dp_pool_t *pool, int peerfd, const char *username);
void regPhase2(dp_pool_t *pool, int peerfd, const char *username,
               const char *cryptpasswd, session_t **sessions /* may be NULL */);

/* ===== 登录两阶段 =====
 * loginPhase1：用户名存在 → 回 MSG_TYPE_SALT（存的是完整 cryptpasswd，客户端用它做 crypt）
 *           不存在 → 回 MSG_TYPE_LOGIN_ERROR_CODE
 * loginPhase2：crypt 产物比对 → 成功 sessionAdd + 回 MSG_TYPE_LOGIN_OK_CODE
 *           失败 → 回 MSG_TYPE_LOGIN_ERROR_CODE */
void loginPhase1(dp_pool_t *pool, int peerfd, const char *username);
void loginPhase2(dp_pool_t *pool, int peerfd, const char *username,
                 const char *encrypted, session_t **sessions);

#endif
