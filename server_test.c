#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<string.h>
#include<stdlib.h>
#include<arpa/inet.h>
int main()
{
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
    //接受连接
    int client_fd;                  //存放连接来的客户端信息；
    struct sockaddr_in client_add;
    int client_len=sizeof(struct sockaddr_in);

    while (1)
    {
        client_fd=accept(sock_fd,(struct sockaddr *)&client_add,&client_len);
        if(client_fd<0)
        {
            perror("5");
            exit (1);
        }
        else
        {
            printf("OK\n");
            pthread_t tipm;
            //timp=
            pthread_create (&(pool->threadid[i]), NULL, thread_routine,NULL);
        }
    }


}
