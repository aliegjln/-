#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<string.h>
#include<stdlib.h>
#include<arpa/inet.h>
#include<pthread.h>
#include<unistd.h>
#include "cJSON.h"

#define MAX_FD_SIZE 1024
struct server_context_st
{
    int cli_cnt;            //当前有的描述符个数
    int clifds[MAX_FD_SIZE];//client fd
    int maxFd;              //最大的文件描述符
    fd_set allfds;          //存放所有描述符的集合(include client and server)
};
struct server_context_st * s_srv_ctx = NULL;
void *run(void *client_fd)
{
    char buf[100];
    if(recv(*(int *)client_fd,buf,sizeof(char )*100,0)<0)
    {
        perror("6");
        exit (1);
    }
    printf("%s###\n",buf);
    close(*(int *)client_fd);
    return 0;
}
int main()
{
    int i;
//初始化本地网络信息
    struct sockaddr_in serv_add;
    memset(&serv_add,0,sizeof(struct sockaddr));
    serv_add.sin_family=AF_INET;
    serv_add.sin_port=htons(8080);
    serv_add.sin_addr.s_addr = htonl(INADDR_ANY);
   /* if(inet_aton("192.168.30.172",&serv_add.sin_addr)<0)
    {
        perror("1");
        exit (1);
    }*/
//创建一个套接字
    int sock_fd=socket(AF_INET,SOCK_STREAM,0);
    if(sock_fd<0)
    {
        perror("2");
        exit (1);
    }
//绑定到本地端口
    if(bind(sock_fd,(struct sockaddr *)&serv_add,sizeof(struct sockaddr_in))<0)
    {
        perror("3");
        exit (1);
    }
//转为倾听套接字
    if(listen(sock_fd,5)<0)
    {
        perror("4");
        exit (1);
    }

//设置io时间
    struct timeval tv;
//开始ＩＯ复路操作
    s_srv_ctx = (struct server_context_st*)malloc(sizeof(struct server_context_st));//给IO复路控制结构体开空间
    memset(s_srv_ctx, 0, sizeof(s_srv_ctx));
    for (i = 0; i < MAX_FD_SIZE; i++)//给每一个描述赋初值
    {
        s_srv_ctx->clifds[i] = -1;
    }
    fd_set *readfds = &s_srv_ctx->allfds;//单独找了一个来管理描述符集合
//开始操作了


    while (1)
    {
        tv.tv_sec = 5;
        tv.tv_usec = 0;
        FD_ZERO(readfds);//清空集合
        FD_SET(sock_fd, readfds);//把服务端描述符添加集合里
        s_srv_ctx->maxFd =sock_fd;//更新最大描述符
        for (i = 0; i < MAX_FD_SIZE; i++)
        {
            int clifd = s_srv_ctx->clifds[i];
            FD_SET(clifd, readfds);//添加
            s_srv_ctx->maxFd = (clifd > s_srv_ctx->maxFd ? clifd : s_srv_ctx->maxFd);//更新
        }

        int retval = select(s_srv_ctx->maxFd + 1, readfds, NULL, NULL, &tv);//开始监控
        if (retval == -1)//失败
        {
            perror("select");
            return 0;
        }
        if (retval == 0)//监控时间没有人要读写
        {
            printf("这个好像是没有人发消息\n");
            continue ;
        }
        if (FD_ISSET(sock_fd, readfds))//是不是服务端来信号了（他来信号说明有新的链接请求）
        {
//接受连接
            int client_fd;                  //存放连接来的客户端信息；
            struct sockaddr_in client_add;
            int client_len=sizeof(struct sockaddr_in);
            client_fd=accept(sock_fd,(struct sockaddr *)&client_add,&client_len);
            if(client_fd<0)
            {
                perror("5");
                exit (1);
            }
            for (i = 0; i < MAX_FD_SIZE; i++)
            {
                if (s_srv_ctx->clifds[i] < 0)
                {
                    s_srv_ctx->clifds[i] =client_fd;
                    s_srv_ctx->cli_cnt++;
                    FD_SET(client_fd, readfds);
                    break;
                }
            }
        }
//服务端处理完了，看有没有客户端要链接
        char buf[100];
        char buf2[100];
        //cJSON *root=(cJSON*)malloc(sizeof(char)*100);//创建一个对象;//声明一个结
        for (int i = 0; i < s_srv_ctx->cli_cnt; i++)
        {
            int cl = s_srv_ctx->clifds[i];//如果有小零的就跳过（第一次肯定没有，应为他是一个一个赋上来的，但当有客户退出，就有－１存在）
            if (cl < 0)
            {
                continue;
            }
            if (FD_ISSET(cl, readfds))//如果有消息
            {
                if(recv(cl,buf,sizeof(char)*100,0)<0)
                {

                    perror("6");
                    exit (1);
                }
                //buf=cJSON_Print(root);
                printf("%s###\n",buf);
                strcpy(buf2,"1223");
                send(cl,buf2,sizeof(char)*100,0);
                strcpy(buf2,"456");
                send(cl,buf2,sizeof(char)*100,0);
            }
        }

       /* else
        {
           ;printf("OK\n")
            pthread_t tipm;
            pthread_create (&tipm, NULL,run,(void *)&client_fd);
            pthread_join(tipm,NULL);
        }*/
    }


}
