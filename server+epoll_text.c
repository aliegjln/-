#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<string.h>
#include<stdlib.h>
#include<arpa/inet.h>
#include<pthread.h>
#include<unistd.h>
#include<sys/select.h>
#include<sys/time.h>
#include<sys/param.h>   //包含NOFILE

void * run_pthread();
struct Works_fd
{
    int work_fd;
    int work_id;
    char work_buf[1000];
    struct Works_fd * work_next;
};
struct Clients_fd
{
    int client_fd;
    int client_id;
    struct Clients_fd * client_next;
};
struct Clients_fd * head_clients;   //声明在线客户队列；
int sum_clients;                    //在线客户数量；

struct pthread_pool
{
    pthread_mutex_t queue_lock;         //声明一个锁；
    pthread_cond_t queue_cond;          //声明一个条件变量；
    pthread_t *queue_pthread;           //声明一个线程指针，将来线程声明是连续的空间（动态数组），所以一个指针就可以用了；
    int max_pthread;                    //最大线程数量；
    int destroy;                        //是否销毁池子，是1，否0；

    struct Works_fd * head_works;     //有需求的用户队列
    int sum_work;                       //有需求用户数量

};
struct pthread_pool* mpool=NULL;        //声明一个池子
int my_error(char *err,int line)
{
    perror(err);
    printf("%d\n",line);
    exit(1);
}
int init_pool(void)                                //初始化池子；
{
    mpool=(struct pthread_pool *)malloc(sizeof(struct pthread_pool));         //开一个池子；
    pthread_mutex_init(&mpool->queue_lock,NULL);                //初始化锁；
    pthread_cond_init(&mpool->queue_cond,NULL);                 //初始化信号量‘
    mpool->max_pthread=5;                                       //最大线程量为5
    mpool->destroy=0;                                           //不销毁队列

    mpool->head_works=NULL;                                     //任务队列头
    mpool->sum_work=0;                                          //有需求用户数量

    mpool->queue_pthread=(pthread_t *)malloc(sizeof(pthread_t)*5);//开放线程的数组N个
    int i;
    for(i=0;i<5;i++)                                            //循环开线程，到线程工作函数；
    {
        pthread_create(&mpool->queue_pthread[i],NULL,run_pthread,NULL);
    }
}
void * run_pthread()                                //线程工作函数
{
    char recv_buf[1000];
    struct Works_fd *timp_work=NULL;
    //cJSON *root =cJSON_CreateObject();
printf("线程:%lu 已建立\n",pthread_self());

    while(1)
    {
        pthread_mutex_lock(&mpool->queue_lock);             //上锁
        while(mpool->sum_work==0&&mpool->destroy!=1)  //1.如果没任务，且不摧毁池子，就让线程等待
        {
            printf("线程%lu 正在等待任务\n",pthread_self());
            pthread_cond_wait(&mpool->queue_cond,&mpool->queue_lock);
        }
        if(mpool->destroy==1)
        {
            pthread_mutex_unlock (&mpool->queue_lock);  //遇到break,continue,return等跳转语句，千万不要忘记先解锁,这里千万要注意，要有这种意识，我自己写的时候就忘了
            printf("线程%lu 将被摧毁\n",pthread_self());
            pthread_exit (NULL);
        }
        timp_work=mpool->head_works;
        mpool->head_works=timp_work->work_next;
        mpool->sum_work--;
        pthread_mutex_unlock(&mpool->queue_lock);           //操作完了解锁

        //cJSON *root = cJSON_Parse(recv_buf);
        printf("RECV: %s\n",timp_work->work_buf);
        //printf("%s",)*/
        free(timp_work);                                //释放结点
        timp_work=NULL;
    }
}
void *run(void *client_fd)
{
    char buf[100];
    while(1)
    {
        if(recv(*(int *)client_fd,buf,sizeof(char )*100,0)<0)
            my_error("recv",__LINE__);
        printf("%s###\n",buf);
    }
  //  close(*(int *)client_fd);
    return 0;
}
int main()
{
    //初始化本地网络信息
    struct sockaddr_in serv_add;
    memset(&serv_add,0,sizeof(struct sockaddr));
    serv_add.sin_family=AF_INET;
    serv_add.sin_port=htons(8080);
    serv_add.sin_addr.s_addr = htonl(INADDR_ANY);//any 任何地方都能连得到
    int server_fd;                               //创建一个套接字
    server_fd=socket(AF_INET,SOCK_STREAM,0);
    if(server_fd<0)
        my_error("socket",__LINE__);
//绑定到本地端口
    if(bind(server_fd,(struct sockaddr *)&serv_add,sizeof(struct sockaddr_in))<0)
        my_error("bind",__LINE__);
//转为倾听套接字
    if(listen(server_fd,5)<0)
        my_error("listen",__LINE__);

    init_pool();

    struct timeval tv;
    int max_Fd;                         //最大的文件描述符
    fd_set allfds;
    fd_set * readfds=&allfds;
    struct Clients_fd * timp_head=NULL;
    while(1)
    {
printf("我循环了几次\n");
        tv.tv_sec = 5;
        tv.tv_usec = 0;
        FD_ZERO(readfds);               //清空集合
        FD_SET(server_fd, readfds);     //把服务端描述符添加集合里
        max_Fd =server_fd;              //更新最大描述符
        timp_head=head_clients;
        while(timp_head!=NULL)
        {
//printf("%d\n",timp_head->client_fd);
            FD_SET(timp_head->client_fd, readfds);//添加
            if(timp_head->client_fd>max_Fd)
                max_Fd=timp_head->client_fd;
            timp_head=timp_head->client_next;
        }
        int IO_retval = select(max_Fd + 1, readfds, NULL, NULL, &tv);//开始监控
        if(IO_retval==-1)
            my_error("selsct",__LINE__);
printf("我循环了...\n");
        if(IO_retval==0)
        {
            printf("好像没有任务\n");
            continue;
        }
        if (FD_ISSET(server_fd, readfds))               //是不是服务端来信号了（他来信号说明有新的链接请求）
        {
printf("进来一个连接\n");
            struct Clients_fd *time_client=(struct Clients_fd *)malloc(sizeof(struct Clients_fd));//存放用户信息
            time_client->client_next=NULL;
            time_client->client_id=0;
            struct sockaddr_in client_add;
            int client_len=sizeof(struct sockaddr_in);
            time_client->client_fd=accept(server_fd,(struct sockaddr *)&client_add,&client_len);

            if(time_client->client_fd<0)
                my_error("accept",__LINE__);
            timp_head=head_clients;                     //将连接来的用户添加到连接对列
            if(timp_head==NULL)
            {
                head_clients=time_client;
                head_clients->client_next=NULL;
printf("开始加第一个认了\n");
            }
            else
            {
                while(timp_head->client_next!=NULL)
                    timp_head=timp_head->client_next;
                timp_head->client_next=time_client;
            }
            continue;
        }
        timp_head=head_clients;

        while(timp_head!=NULL)                          //轮寻客户描述符
        {
            if (FD_ISSET(timp_head->client_fd, readfds))//如果有事件
            {
printf("他是一个有问题的人%d\n",timp_head->client_fd);
                struct Works_fd *timp_work=(struct Works_fd*)malloc(sizeof(struct Works_fd));//赋给结点
                timp_work->work_fd=timp_head->client_fd;
                timp_work->work_id=timp_head->client_id;
                timp_work->work_next=NULL;//这里也没有给出一个具体的时间来
                if(recv(timp_work->work_fd,timp_work->work_buf,sizeof(char )*1000,0)<0)
                    my_error("recv",__LINE__);
                pthread_mutex_lock(&mpool->queue_lock); //上锁添加
                struct Works_fd* timp_work_head=mpool->head_works;
                if(timp_work_head==NULL)
                    mpool->head_works=timp_work;
                else
                {
                    while(timp_work_head->work_next!=NULL)
                        timp_work_head=timp_work_head->work_next;
                    timp_work_head->work_next=timp_work;
                }
                mpool->sum_work++;
printf("添加一个任务\n");
                pthread_mutex_unlock(&mpool->queue_lock);
                pthread_cond_signal(&mpool->queue_cond); //这里是如果有一个线程是在等待，则那个线程会被打开；如果线程都是在忙碌，那这个信号就没有意义；
            }
            timp_head=timp_head->client_next;
        }

    }
}


