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

    if(inet_aton("192.168.30.172",&serv_add.sin_addr)<0)
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
    printf("OK\n");
}
