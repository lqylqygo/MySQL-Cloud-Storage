#include"hashtable.h"

unsigned int hash(const char *key)
{
    //采用BKDR哈希算法，用131质数减少冲突
    if(key==NULL)return 0;
    unsigned int seed=131;//比较好的质数，如1331，131等
    unsigned int hash_val=0;
    while(*key){
        hash_val=hash_val*seed+(*key);//算出每轮的哈希值
        key++;//指针后移
    }
    return hash_val%MAX_SIZE;
}
void initHashtable(HashTable *ht)
{
    if(ht==NULL)return;
    for(int i =0;i<MAX_SIZE;i++)
    {
        ht->table[i].value=NULL;
        ht->table[i].key[0]='\0';
    }
    ht->size=0;
}
void insert(HashTable *ht, const char *key, void *value)
{
    if(ht==NULL||key==NULL)return;
    if(ht->size>=MAX_SIZE)
    {
        printf("HashTable is full, cannot insert new key-value pair.\n");
        return;
    }
    unsigned int index=hash(key);
    //处理冲突，采用线性探测法
    while(ht->table[index].key[0]!='\0')
    {
        if(strcmp(ht->table[index].key,key)==0)
        {
            //如果key已经存在，更新value
            ht->table[index].value=value;
            return;
        }
        index=(index+1)%MAX_SIZE;
    }
    //找到空位了 ，把key和value存进去
    //注意：key是char【50】数组，用strcpy复制
    strncpy(ht->table[index].key,key,sizeof(ht->table[index].key)-1);
    ht->table[index].key[sizeof(ht->table[index].key)-1]='\0';//确保字符串结尾
    ht->table[index].value=value;
    ht->size++;   
}
void *find(HashTable *ht, const char *key)
{
    if(ht==NULL||key==NULL)return NULL;
    unsigned int index=hash(key);
    unsigned int start_index=index;//记录起始位置，防止死循环
    //处理冲突，采用线性探测法
    while(ht->table[index].key[0]!='\0')
    {
        if(strcmp(ht->table[index].key,key)==0)
        {
            return ht->table[index].value;
        }
        index=(index+1)%MAX_SIZE;
    }
    return NULL;//没找到
}
void erase(HashTable *ht, const char *key)
{
    if(ht==NULL||key==NULL)return;
    unsigned int index=hash(key);
    unsigned int start_index=index;//记录起始位置，防止死循环
    //处理冲突，采用线性探测法
    while(ht->table[index].key[0]!='\0')
    {
        if(strcmp(ht->table[index].key,key)==0)
        {
            //找到要删除的键值对
            ht->table[index].key[0]='\0';//标记为空
            ht->table[index].value=NULL;
            ht->size--;
            return;
        }
        index=(index+1)%MAX_SIZE;
        if(index==start_index)break;//回到起始位置，说明没找到
    }
}
void printHashtable(HashTable *ht)//用于调试
{
    if(ht==NULL)return;
    printf("------哈希表HashTable size: %d------\n",ht->size);
    for(int i=0;i<MAX_SIZE;i++)
    {
        if(ht->table[i].key[0]!='\0')
        {
            printf("索引Index %d: Key: %s, Value: %p\n",i,ht->table[i].key,ht->table[i].value);
        }
    }
    printf("------哈希表HashTable end------\n");
}
void destroyHashtable(HashTable *ht)//销毁哈希表
{
    if(ht==NULL)return;
    for(int i=0;i<MAX_SIZE;i++)
    {
        if(ht->table[i].key[0]!='\0')
        {
            //如果value是动态分配的内存，需要释放
            if(ht->table[i].value!=NULL)
            {
                free(ht->table[i].value);
                ht->table[i].value=NULL;
            }
            ht->table[i].key[0]='\0';
        }
    }
    ht->size=0;
}