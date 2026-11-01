#ifndef  __HASH_TABLE_H__
#define  __HASH_TABLE_H__
#include<my_header.h>

#define MAX_SIZE 100
#define EMPTY NULL

typedef struct {
    char key[50];
    void *value;//存储任意类型
}KeyValue;//键值对结构体

typedef struct {
    KeyValue table[MAX_SIZE];
    int size;
}HashTable;//哈希表结构体

unsigned int hash(const char *key);//哈希函数
void initHashtable(HashTable *ht);//初始化哈希表
void insert(HashTable *ht, const char *key, void *value);//插入键值对
void *find(HashTable *ht, const char *key);//查找键值对 
void erase(HashTable *ht, const char *key);//删除键值对
void printHashtable(HashTable *ht);//打印哈希表
void destroyHashtable(HashTable *ht);//销毁哈希表

#endif
