#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<string.h>
#include<stdlib.h>
#include<arpa/inet.h>
#include<unistd.h>
#include "cJSON.h"
int main()
{
    //初始化本地网络信息
    struct sockaddr_in serv_add;
    memset(&serv_add,0,sizeof(struct sockaddr));
    serv_add.sin_family=AF_INET;
    serv_add.sin_port=htons(8080);

    if(inet_aton("192.168.30.181",&serv_add.sin_addr)<0)//wocaO!!这个ip每一次登录获取的都不一样
    {
        perror("1");
        exit (1);
    }
    //创建一个套接字
    int sock_fd=socket(AF_INET,SOCK_STREAM,0);
    if(sock_fd<0)
    {
        perror("2");
        exit (1);
    }
    //建立连接
    if(connect(sock_fd,(struct sockaddr *)&serv_add,sizeof(struct sockaddr_in))<0)
    {
        perror("3");
        exit (1);
    }
    printf("connect OK\n");
    char *buf1;
    cJSON *root=NULL;//声明一个结构
    root=cJSON_CreateObject();//创建一个对象
    cJSON_AddNumberToObject(root, "fromid", 666666);//添加一条信息（键值对）
    cJSON_AddNumberToObject(root, "tomid", 123456);//添加一条信息（键值对）
//buf1=cJSON_Print(root);
//printf("%s\n",buf1);

    while(1)
    {
        buf1=(char *)malloc(sizeof(char )*100);
        printf("input:");
        scanf("%s",buf1);
        //cJSON_AddItemToObject(root, "note", cJSON_CreateString(buf1));//添加一条信息（键值对）
//buf1=cJSON_Print(root);
//printf("%s\n",buf1);
        send(sock_fd,buf1,sizeof(char)*100,0);
        printf("send OK\n");
        if(recv(sock_fd,buf1,sizeof(char)*100,0)<0)
                {

                    perror("6");
                    exit (1);
                }
        printf("%s\n",buf1);
        if(recv(sock_fd,buf1,sizeof(char)*100,0)<0)
                {

                    perror("6");
                    exit (1);
                }
        printf("%s\n",buf1);
    }
close(sock_fd);
}
