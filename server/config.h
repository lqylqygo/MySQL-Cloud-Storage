#ifndef __CONFIG_H__
#define __CONFIG_H__
#include "hashtable.h"

#define IP "ip"
#define PORT "port"
#define THREAD_NUM "thread_num"
#define WORKER_NUM "worker_num"
#define DB_IP "db_ip"
#define DB_USER "db_user"
#define DB_PASSWD "db_passwd"
#define DB_NAME "db_name"

void readConfig(const char *filename,HashTable *ht);



#endif