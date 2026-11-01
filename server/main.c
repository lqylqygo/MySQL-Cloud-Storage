#include"config.h"

int main(){
    HashTable ht;
    initHashtable(&ht);
    readConfig("../conf/server.conf",&ht);
    printf("ip=%s\n",(char*)find(&ht,IP));
    printf("port=%s\n",(char*)find(&ht,PORT));
    printf("thread_num=%s\n",(char*)find(&ht,THREAD_NUM));
    printf("worker_num=%s\n",(char*)find(&ht,WORKER_NUM));
    printf("db_ip=%s\n",(char*)find(&ht,DB_IP));
    printf("db_user=%s\n",(char*)find(&ht,DB_USER));
    printf("db_passwd=%s\n",(char*)find(&ht,DB_PASSWD));
    printf("db_name=%s\n",(char*)find(&ht,DB_NAME));
    destroyHashtable(&ht);
    return 0;
}