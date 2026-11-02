#include "user_op.h"
#include "mysql_ops.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/* crypt() 在 glibc 里声明在 <crypt.h>，但少数环境没有，按需 include */
#if __has_include(<crypt.h>)
#include <crypt.h>
#else
#include <unistd.h>   /* 某些系统上 crypt() 也在这里 */
#endif

static const char kSaltChars[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789./";

/* 构造一个 $6$XXXXXXXX$ 形式的 salt 字符串（malloc 返回） */
char *generateSalt(void)
{
    char *salt = (char *)malloc(6 + 8 + 2 + 1); /* "$6$" + 8 chars + "$" + '\0' */
    if (!salt) return NULL;
    /* 种子：用 /dev/urandom 最好，没有就用 time+pid 退化 */
    FILE *fp = fopen("/dev/urandom", "rb");
    unsigned char seed[8];
    if (fp) {
        fread(seed, 1, sizeof(seed), fp);
        fclose(fp);
    } else {
        unsigned int s = (unsigned int)time(NULL) ^ (unsigned int)getpid();
        seed[0] = (unsigned char)(s & 0xFF);
        seed[1] = (unsigned char)((s >> 8) & 0xFF);
        seed[2] = (unsigned char)((s >> 16) & 0xFF);
        seed[3] = (unsigned char)((s >> 24) & 0xFF);
        for (int i = 4; i < 8; ++i) seed[i] = (unsigned char)(s >> (i * 3));
    }

    /* $6$ = SHA512，4 个字节固定，8 字节随机 */
    memcpy(salt, "$6$", 3);
    for (int i = 0; i < 8; ++i) {
        salt[3 + i] = kSaltChars[seed[i] % (sizeof(kSaltChars) - 1)];
    }
    salt[11] = '$';
    salt[12] = '\0';
    return salt;
}

const char *getSetting(const char *cryptpasswd)
{
    /* glibc crypt 输出的完整串就是 setting，直接返回 */
    return cryptpasswd;
}

/* ==================== 发送辅助 ==================== */
static void sendResp(int peerfd, int type, const char *payload /* may be NULL */)
{
    train_t t;
    memset(&t, 0, sizeof(t));
    t.type = type;
    if (payload) {
        t.len = (int)strlen(payload);
        memcpy(t.buff, payload, t.len);
    }
    sendTrain(peerfd, &t);
}

/* ==================== 注册两阶段 ==================== */
void regPhase1(dp_pool_t *pool, int peerfd, const char *username)
{
    /* 1) 拼 SQL，再取连接 —— 这是之前经验里强调的正确顺序 */
    char sql[512];
    snprintf(sql, sizeof(sql),
             "SELECT id FROM user WHERE username='%s'",
             username);

    MYSQL *mysql = getConnection(pool);
    if (!mysql) {
        LOG_ERROR("regPhase1: 连接池取不到连接");
        sendResp(peerfd, MSG_TYPE_REGISTER_ERROR_CODE, "server busy");
        return;
    }

    MYSQL_RES *res = db_query(mysql, sql);
    int exist = (res && mysql_num_rows(res) > 0);
    if (res) mysql_free_result(res);

    releaseConnection(pool, mysql);

    if (exist) {
        LOG_WARN("regPhase1: 用户 %s 已存在", username);
        sendResp(peerfd, MSG_TYPE_REGISTER_ERROR_CODE, "username exists");
        return;
    }

    char *salt = generateSalt();
    LOG_INFO("regPhase1: 新用户 %s, 生成 salt=%s", username, salt);
    sendResp(peerfd, MSG_TYPE_SALT, salt);
    free(salt);
}

void regPhase2(dp_pool_t *pool, int peerfd, const char *username,
               const char *cryptpasswd, session_t **sessions)
{
    char sql[1024];
    snprintf(sql, sizeof(sql),
             "INSERT INTO user(username, cryptpasswd) VALUES('%s','%s')",
             username, cryptpasswd);

    MYSQL *mysql = getConnection(pool);
    if (!mysql) {
        sendResp(peerfd, MSG_TYPE_REGISTER_ERROR_CODE, "server busy");
        return;
    }
    int rc = db_execute(mysql, sql);
    releaseConnection(pool, mysql);

    if (rc < 0) {
        LOG_ERROR("regPhase2: INSERT 失败, user=%s", username);
        sendResp(peerfd, MSG_TYPE_REGISTER_ERROR_CODE, "insert error");
        return;
    }
    LOG_INFO("regPhase2: 用户 %s 注册成功", username);
    sendResp(peerfd, MSG_TYPE_REGISTER_OK_CODE, NULL);

    /* 顺手把新用户顶层目录建出来：virtual_file(owner_id=self, parent_id=0, type='d') */
    /* 先取刚插入的 id：用 LAST_INSERT_ID() */
    MYSQL *mysql2 = getConnection(pool);
    if (mysql2) {
        MYSQL_RES *r = db_query(mysql2, "SELECT LAST_INSERT_ID()");
        int uid = 0;
        if (r) {
            MYSQL_ROW row = mysql_fetch_row(r);
            if (row) uid = atoi(row[0]);
            mysql_free_result(r);
        }
        if (uid > 0) {
            char sql2[512];
            snprintf(sql2, sizeof(sql2),
                     "INSERT INTO virtual_file(parent_id, filename, owner_id, sha256, filesize, type)"
                     " VALUES(0,'/',%d,'',0,0)",
                     uid);
            db_execute(mysql2, sql2);

            /* 把 user.pwd 更新成这个顶层目录的 id */
            /* 先查刚插入的顶层目录 id */
            MYSQL_RES *r2 = db_query(mysql2, "SELECT LAST_INSERT_ID()");
            if (r2) {
                MYSQL_ROW row = mysql_fetch_row(r2);
                if (row) {
                    int root_id = atoi(row[0]);
                    char sql3[256];
                    snprintf(sql3, sizeof(sql3),
                             "UPDATE user SET pwd=%d WHERE id=%d", root_id, uid);
                    db_execute(mysql2, sql3);
                }
                mysql_free_result(r2);
            }
        }
        releaseConnection(pool, mysql2);

        /* 如果调用者给了 sessions，注册完直接登录也行；这里先留空 */
        (void)sessions;
    }
}

/* ==================== 登录两阶段 ==================== */
void loginPhase1(dp_pool_t *pool, int peerfd, const char *username)
{
    char sql[512];
    snprintf(sql, sizeof(sql),
             "SELECT cryptpasswd FROM user WHERE username='%s'",
             username);

    MYSQL *mysql = getConnection(pool);
    if (!mysql) {
        sendResp(peerfd, MSG_TYPE_LOGIN_ERROR_CODE, "server busy");
        return;
    }
    MYSQL_RES *res = db_query(mysql, sql);
    MYSQL_ROW row = res ? mysql_fetch_row(res) : NULL;
    char *setting = row ? strdup(row[0]) : NULL;
    if (res) mysql_free_result(res);
    releaseConnection(pool, mysql);

    if (!setting) {
        LOG_WARN("loginPhase1: 用户 %s 不存在", username);
        sendResp(peerfd, MSG_TYPE_LOGIN_ERROR_CODE, "user not found");
        return;
    }
    LOG_INFO("loginPhase1: 用户 %s 存在, 返回 setting", username);
    sendResp(peerfd, MSG_TYPE_SALT, setting);
    free(setting);
}

void loginPhase2(dp_pool_t *pool, int peerfd, const char *username,
                 const char *encrypted, session_t **sessions)
{
    char sql[512];
    snprintf(sql, sizeof(sql),
             "SELECT id, cryptpasswd FROM user WHERE username='%s'",
             username);

    MYSQL *mysql = getConnection(pool);
    if (!mysql) {
        sendResp(peerfd, MSG_TYPE_LOGIN_ERROR_CODE, "server busy");
        return;
    }
    MYSQL_RES *res = db_query(mysql, sql);
    MYSQL_ROW row = res ? mysql_fetch_row(res) : NULL;
    int found_userid = 0;
    char *found_crypt = NULL;
    if (row) {
        found_userid = atoi(row[0]);
        found_crypt = strdup(row[1]);
    }
    if (res) mysql_free_result(res);
    releaseConnection(pool, mysql);

    if (!found_crypt) {
        sendResp(peerfd, MSG_TYPE_LOGIN_ERROR_CODE, "user not found");
        return;
    }

    /* PPT 说 client 做 crypt(pwd, setting)，setting 就是完整 cryptpasswd */
    /* 所以 client 发来的 encrypted 应该 = 存库的 found_crypt */
    if (strcmp(encrypted, found_crypt) != 0) {
        LOG_WARN("loginPhase2: 用户 %s 密码错误", username);
        free(found_crypt);
        sendResp(peerfd, MSG_TYPE_LOGIN_ERROR_CODE, "password error");
        return;
    }
    free(found_crypt);

    LOG_INFO("loginPhase2: 用户 %s 登录成功, uid=%d", username, found_userid);

    /* 会话：取 user.pwd（顶层目录 id）作为 cwd_id */
    int cwd_id = 0;
    MYSQL *mysql2 = getConnection(pool);
    if (mysql2) {
        char sql2[256];
        snprintf(sql2, sizeof(sql2), "SELECT pwd FROM user WHERE id=%d", found_userid);
        MYSQL_RES *r = db_query(mysql2, sql2);
        if (r) {
            MYSQL_ROW ro = mysql_fetch_row(r);
            if (ro) cwd_id = atoi(ro[0]);
            mysql_free_result(r);
        }
        releaseConnection(pool, mysql2);
    }

    if (sessions) {
        session_t *s = sessionAdd(sessions, peerfd, found_userid, username);
        if (s) s->cwd_id = cwd_id;
    }
    sendResp(peerfd, MSG_TYPE_LOGIN_OK_CODE, NULL);
}
