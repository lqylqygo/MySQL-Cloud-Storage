#include"config.h"

void readConfig(const char *filename,HashTable *ht) 
{
    FILE *fp=fopen(filename,"r");
    if(fp==NULL)
    {
        printf("open file %s error.\n",filename);
        return;
    }
    char buf[128]={0};
    while(fgets(buf,sizeof(buf),fp)!=NULL)
    {
        //如果是空行或者注释行，直接跳过
        if(buf[0]=='\n'||buf[0]=='#'||buf[0]=='/')continue;
        //先去掉末尾的零，用strcspn
        buf[strcspn(buf, "\r\n")] = 0; // 去掉换行符
        //用strtok分割字符串，获取key和value
        char *key=strtok(buf,"=");
        char *value=strtok(NULL,"=");
        if(key==NULL||value==NULL)continue;
        //去掉key和value的前面空格
        while(*key==' ')key++;
        while(*value==' ')value++;
        //去掉key和value的后面空格
        char *end;
        end=key+strlen(key)-1;
        while(end>key&&*end==' ')end--;
        *(end+1)='\0';
        end=value+strlen(value)-1;
        while(end>value&&*end==' ')end--;
        *(end+1)='\0';
        //因为哈希表的结构，需要给value分配内存
        char *value_copy = (char *)malloc(strlen(value) + 1);
        if (value_copy == NULL) {
            printf("Memory allocation failed for value_copy.\n");
            continue; // 跳过当前循环，继续处理下一行
        }
        strcpy(value_copy, value);
        //insert into 哈希表
        insert(ht,key,value_copy);
        
    }
    //关闭文件
    fclose(fp);
    
}