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
#include<sys/param.h>                           //包含NOFILE
#include "cJSON.h"

#include <mysql/mysql.h>
#define HOST    "118.89.245.72"
#define USER    "root"
#define PASSWD  "Gjl1141411.1"
#define DB_NAME "myqq"
MYSQL *mysqll;                                  // 创建一个MYSQL句柄并

void * run_pthread();
struct Works_fd                                 //工作任务
{
    int work_fd;
    int work_id;
    char work_buf[1000];
    struct Works_fd * work_next;
};
struct Clients_fd                               //在线
{
    int client_fd;
    int client_id;
    struct Clients_fd * client_next;
};
struct Clients_fd * head_clients;               //声明在线客户队列；
int sum_clients;                                //在线客户数量；
pthread_mutex_t client_lock;         //声明一个锁；
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
struct pthread_pool* mpool=NULL;                //声明一个池子
int my_error(char *err,int line)
{
    perror(err);
    printf("%d\n",line);
    exit(1);
}
void client_send(struct Works_fd *work,int function,char *buf1,char *buf2,char *buf3,char*buf4)
{
    cJSON *root=cJSON_CreateObject();//创建一个对象
    cJSON_AddNumberToObject(root,"function",function);
    cJSON_AddStringToObject(root, "f1", buf1);//添加一条信息（键值对）
    cJSON_AddStringToObject(root, "f2", buf2);//添加一条信息（键值对）
    cJSON_AddStringToObject(root, "f3", buf3);//添加一条信息（键值对）
    cJSON_AddStringToObject(root, "f4", buf4);//添加一条信息（键值对）
    char *buf=cJSON_Print(root);
    cJSON_Delete(root);
printf("%d %s\n",strlen(buf),buf);
    if(send(work->work_fd,buf,sizeof(char )*1000,0)<0)
        my_error("send",__LINE__);
}
void mysql_connect()
{
    mysqll=mysql_init(NULL);//初始化
    if (!mysqll)
        my_error("mysql_init", __LINE__);
    if(!mysql_real_connect(mysqll, HOST, USER, PASSWD, DB_NAME, 0, NULL,0))//链接数据库
        my_error("mysql_real_connect", __LINE__);
    if (!mysql_set_character_set(mysqll, "utf8"))
    {
        printf("New connection character set: %s\n",
        mysql_character_set_name(mysqll));
    }
printf("Connect database sucessfully!\n\n");
}
int init_pool(void)                             //初始化池子；
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
void login (struct Works_fd *work)              //登录
{
    cJSON *root =cJSON_CreateObject();
    int flag=0;
    int user_id;
    char user_password[11];
    char query[100];        //给数据库发的命令
    memset(query, '\0', sizeof(query));
    MYSQL_RES *res;         // 返回查询结果
    MYSQL_ROW row;          // 返回行数据

    root = cJSON_Parse(work->work_buf);
    user_id=(cJSON_GetObjectItem(root,"id")->valueint);
    strcpy(user_password,cJSON_GetObjectItem(root,"password")->valuestring);
    cJSON_Delete(root);

    sprintf(query,"select * from information where id=%d and password=\"%s\"",user_id,user_password);
printf("%s\n",query);
    if(mysql_real_query(mysqll, query, strlen(query)))
    {
        printf("登录失败 %s\n",mysql_error(mysqll));
        exit(1);
    }
    res = mysql_store_result(mysqll);
    row = mysql_fetch_row(res);
    if(res!=0&&row!=NULL)
        flag=1;
    if(flag)
    {
        struct Clients_fd *timp_head=head_clients;//跟新在线链表id
        while(timp_head!=NULL)
        {
            if(timp_head->client_fd==work->work_fd)
            {
                timp_head->client_id=user_id;

                break;
            }
            timp_head=timp_head->client_next;
        }
        memset(query, '\0', sizeof(query));
        sprintf(query,"update information set state=1 where id=%d",user_id);    //库
printf("%s\n",query);
        if(mysql_real_query(mysqll, query, strlen(query)))
        {
            printf("登录失败 %s\n",mysql_error(mysqll));
            exit(1);
        }
    }
    root =cJSON_CreateObject();
    cJSON_AddNumberToObject(root,"function",1);
    cJSON_AddNumberToObject(root, "answer", flag);//添加一条信息（键值对）
    strcpy(work->work_buf,cJSON_Print(root));
    cJSON_Delete(root);
    send(work->work_fd,work->work_buf,sizeof(char )*1000,0);

    return ;
}
void logon(struct Works_fd *work)
{
    cJSON *root =cJSON_CreateObject();
    root = cJSON_Parse(work->work_buf);
    char query[200];        //给数据库发的命令
    memset(query, '\0', sizeof(query));
    MYSQL_RES *res;         // 返回查询结果
    MYSQL_ROW row;          // 返回行数据
    strcpy(query,cJSON_GetObjectItem(root,"note")->valuestring);
    cJSON_Delete(root);
printf("%s\n",query);
    if(mysql_real_query(mysqll, query, strlen(query)))
    {
        printf("注册失败 %s\n",mysql_error(mysqll));
        exit(1);
    }
    res = mysql_store_result(mysqll);
    mysql_free_result(res);
    sprintf(query,"select id from information order by id desc limit 1");
printf("%s\n",query);
    if(mysql_real_query(mysqll, query, strlen(query)))
    {
        printf("注册失败 %s\n",mysql_error(mysqll));
        exit(1);
    }
    res = mysql_store_result(mysqll);
    row = mysql_fetch_row(res);
    char timp_id[10];
    strcpy(timp_id,row[0]);
    mysql_free_result(res);
printf("%s\n",timp_id);
    sprintf(query,"create table f%s (id mediumint primary key,fname char(22),constraint for_%s foreign key (id) references information (id))",timp_id,timp_id);
printf("%s\n",query);
    if(mysql_real_query(mysqll, query, strlen(query)))
    {
        printf("注册失败 %s\n",mysql_error(mysqll));
        exit(1);
    }
    root =cJSON_CreateObject();
    cJSON_AddNumberToObject(root,"function",2);
    cJSON_AddStringToObject(root, "answer",timp_id);//添加一条信息（键值对）
    strcpy(work->work_buf,cJSON_Print(root));
    cJSON_Delete(root);
    send(work->work_fd,work->work_buf,sizeof(char )*1000,0);
    return ;
}
void Exit(struct Works_fd *work)
{
    struct Clients_fd *timp_head=head_clients;//跟新在线链表id
    struct Clients_fd *timp=NULL;//跟新在线链表id
    pthread_mutex_lock(&client_lock);
    if(timp_head->client_fd==work->work_fd)
    {
        head_clients=head_clients->client_next;
        free(timp_head);
    }
    else
        while(timp_head->client_next!=NULL)
        {
            if(timp_head->client_next->client_fd==work->work_fd)
            {
                timp=timp_head->client_next;
                timp_head->client_next=timp->client_next;
                free(timp);
                break;
            }
            timp_head=timp_head->client_next;
        }
    pthread_mutex_unlock(&client_lock);
    char query[100];        //给数据库发的命令
    memset(query, '\0', sizeof(query));
MYSQL_RES *res;         // 返回查询结果
MYSQL_ROW row;          // 返回行数据
    sprintf(query,"update information set state=0 where id=%d",work->work_id);    //库
printf("%s\n",query);
    if(mysql_query(mysqll, query))
    {
        printf("失败 %s\n",mysql_error(mysqll));
        exit(1);
    }
printf("你真的进函数了?\n");
//printf("%s %s\n",query,row[0]);
    res = mysql_store_result(mysqll);
    if(res<=0)
    {
        printf("whywhy?\n");
    }
    //row = mysql_fetch_row(res);
    close(work->work_fd);
    free(work);
    return ;
}
void find_friend(struct Works_fd *work)
{
    cJSON *root =cJSON_CreateObject();
    root = cJSON_Parse(work->work_buf);
    int find_id=(cJSON_GetObjectItem(root,"id")->valueint);
    cJSON_Delete(root);

    char query[100];        //给数据库发的命令
    memset(query, '\0', sizeof(query));
    MYSQL_RES *res;         // 返回查询结果
    MYSQL_ROW row;          // 返回行数据
    sprintf(query,"insert into  invitation (formid,toid,readded,accept) values (%d,%d,0,0)",work->work_id,find_id);
printf("%s\n",query);
    if(mysql_real_query(mysqll, query, strlen(query)))
    {
        printf("申请好友失败 %s\n",mysql_error(mysqll));
        exit(1);
    }
}
void accept_friend(struct Works_fd *work)
{
    cJSON *root =cJSON_CreateObject();
    root = cJSON_Parse(work->work_buf);
    char from_id[11];
    int ansser=(cJSON_GetObjectItem(root,"ansswer")->valueint);
    strcpy(from_id,(cJSON_GetObjectItem(root,"fromid")->valuestring));
    int to_id=(cJSON_GetObjectItem(root,"toid")->valueint);
    cJSON_Delete(root);

    char query[100];        //给数据库发的命令
    memset(query, '\0', sizeof(query));
    sprintf(query,"delete from invitation where formid=%s and toid=%d",from_id,to_id);
printf("%s\n",query);
    if(mysql_real_query(mysqll, query, strlen(query)))
    {
        printf("申请好友失败 %s\n",mysql_error(mysqll));
        exit(1);
    }
    if(ansser==1)
    {
        sprintf(query,"insert into f%s (id,fname)values(%d,\"beizhu\")",from_id,to_id);
printf("%s\n",query);
        if(mysql_real_query(mysqll, query, strlen(query)))
        {
            printf("申请好友失败 %s\n",mysql_error(mysqll));
            exit(1);
        }
        sprintf(query,"insert into f%d (id,fname)values(%s,\"beizhu\")",to_id,from_id);
printf("%s\n",query);
        if(mysql_real_query(mysqll, query, strlen(query)))
        {
            printf("申请好友失败 %s\n",mysql_error(mysqll));
            exit(1);
        }
    }
}
void talk_friend(struct Works_fd *work)
{
    cJSON *root =cJSON_CreateObject();
    root = cJSON_Parse(work->work_buf);
    char note[800];
    strcpy(note,(cJSON_GetObjectItem(root,"f3")->valuestring));
    char to_id[10];
    strcpy(to_id,(cJSON_GetObjectItem(root,"f2")->valuestring));
    char from_id[10];
    strcpy(from_id,(cJSON_GetObjectItem(root,"f1")->valuestring));
    cJSON_Delete(root);

    char query[100];        //给数据库发的命令
    memset(query, '\0', sizeof(query));
    sprintf(query,"insert into conversation (formid,toid,time,readed,new)values(%s,%s,now(),0,\"%s\")",from_id,to_id,note);
printf("%s\n",query);
    if(mysql_real_query(mysqll, query, strlen(query)))
    {
        printf("会话失败 %s\n",mysql_error(mysqll));
        exit(1);
    }
}
void show_history(struct Works_fd *work,char *buf1,char *buf2)
{
printf("%s\n%s\n",buf1,buf2);
    MYSQL_RES *res;         // 返回查询结果
    MYSQL_ROW row;          // 返回行数据
    char buf_send[800];
    memset(buf_send, '\0', sizeof(buf_send));
    if(mysql_real_query(mysqll,buf1, strlen(buf1)))
    {
        printf("获取好友失败 %s\n",mysql_error(mysqll));
        exit(1);
    }
    res = mysql_store_result(mysqll);
    while(row = mysql_fetch_row(res))
    {
        sprintf(buf_send,"%s%s\n%s:%s\n",buf_send,row[0],row[1],row[2]);
    }
    client_send(work,6,buf_send,"NULL","NULL","NULL");
    if(mysql_real_query(mysqll,buf2, strlen(buf2)))
    {
        printf("获取好友失败 %s\n",mysql_error(mysqll));
        exit(1);
    }

}
void create_group(struct Works_fd *timp_work,char *buf1)
{
printf("%s\n",buf1);
    char timp[1000];
    MYSQL_RES *res;         // 返回查询结果
    MYSQL_ROW row;          // 返回行数据
    if(mysql_real_query(mysqll,buf1, strlen(buf1)))//插入群组
    {
        printf("建群好友失败 %s\n",mysql_error(mysqll));
        exit(1);
    }
    sprintf(buf1,"select id from groups order by id desc limit 1");//获取刚刚的组号
printf("%s\n",buf1);
    if(mysql_real_query(mysqll,buf1, strlen(buf1)))
    {
        printf("建群好友失败%s\n",mysql_error(mysqll));
        exit(1);
    }
    res = mysql_store_result(mysqll);
    row = mysql_fetch_row(res);
    strcpy(timp,row[0]);
    mysql_free_result(res);

    sprintf(buf1,"select groups from information where id=%d",timp_work->work_id);//读出建群的人的组
printf("%s\n",buf1);
    if(mysql_real_query(mysqll,buf1, strlen(buf1)))
    {
        printf("建群好友失败%s\n",mysql_error(mysqll));
        exit(1);
    }
    res = mysql_store_result(mysqll);
    row = mysql_fetch_row(res);
    char timp2[1000];
    sprintf(timp2,"%s%s/",row[0],timp);
    mysql_free_result(res);
    sprintf(buf1,"update information set groups=\"%s\" where id=%d",timp2,timp_work->work_id);//把群加入个人信息
printf("%s\n",buf1);
    if(mysql_real_query(mysqll,buf1, strlen(buf1)))
    {
        printf("建群好友失败%s\n",mysql_error(mysqll));
        exit(1);
    }
    client_send(timp_work,7,timp,"NULL","NULL","NULL");
}
void create_group_queue (struct Works_fd*timp_work,char *buf1)
{
printf("%s\n",buf1);
    char timp1[800];
    char timp2[800];
    char timp3[800];
    char buf[800];
    char *bufp=buf;

    MYSQL_RES *res;         // 返回查询结果
    MYSQL_ROW row;          // 返回行数据
    if(mysql_real_query(mysqll,buf1, strlen(buf1)))//插入群组
    {
        printf("获取群失败 %s\n",mysql_error(mysqll));
        exit(1);
    }
    res = mysql_store_result(mysqll);
    row = mysql_fetch_row(res);
    sprintf(timp1,"%s",row[0]);
    sprintf(buf,"%s",row[0]);
    mysql_free_result(res);
printf("%s\n",timp1);
    int len_buf=strlen(buf)-1;
    *(bufp+len_buf)=0;
    len_buf--;

    timp2[0]='/';
    timp2[1]=0;
    timp3[0]='/';
    timp3[1]=0;
    int sum=0;
    while(len_buf>=0)
    {
printf("输出buf%s\n",(bufp+len_buf));
        if(*(bufp+len_buf)=='/')
        {
            sprintf(buf1,"select new from conversation where formid=%s and toid=%d and readed=0",(bufp+len_buf+1),timp_work->work_id);
printf("%s\n",buf1);
            if(mysql_real_query(mysqll,buf1, strlen(buf1)))//插入群组
            {
                printf("获取群失败 %s\n",mysql_error(mysqll));
                exit(1);
            }
            res = mysql_store_result(mysqll);
            sum=0;
            while (row = mysql_fetch_row(res))
                sum++;
            mysql_free_result(res);
            sprintf(timp2,"%s%d/",timp2,sum);

            /*if(res==NULL||row==NULL)
                sprintf(timp2,"%s%d/",timp2,0);
            else
                sprintf(timp2,"%s%d/",timp2,mysql_num_fields(res));*/
printf("每一次的返回值%s  %d\n",timp2,sum);
           // mysql_free_result(res);


            sprintf(buf1,"select name from groups where id=%s",(bufp+len_buf+1));
printf("%s\n",buf1);
            if(mysql_real_query(mysqll,buf1, strlen(buf1)))//插入群组
            {
                printf("获取群失败 %s\n",mysql_error(mysqll));
                exit(1);
            }
            res = mysql_store_result(mysqll);
            row = mysql_fetch_row(res);
            if(res==NULL||row==NULL)
            {
                printf("获取群失败 %s\n",mysql_error(mysqll));
                exit(1);
            }
            sprintf(timp3,"%s%s/",timp3,row[0]);
printf("每一次的返回值%s  \n",timp3);
            mysql_free_result(res);


            if(len_buf==0)
                break;
            *(bufp+len_buf)=0;
        }
        else
            len_buf--;
    }
    client_send(timp_work,8,timp1,timp2,timp3,"NULL");
}
void talk_groups(struct Works_fd* timp_work,char *buf1,char *buf2,char *buf3)
{
    char buf[800];
    char query[200];
    sprintf(buf,"select vip from groups where id=%s",buf2);
printf("21323213213213\n");
    char *bufp=buf;
printf("%s\n",buf);
    MYSQL_RES *res;         // 返回查询结果
    MYSQL_ROW row;          // 返回行数据
    if(mysql_real_query(mysqll,buf, strlen(buf)))//插入群组
    {
        printf("获取群失败 %s\n",mysql_error(mysqll));
        exit(1);
    }
    res = mysql_store_result(mysqll);
    row = mysql_fetch_row(res);
    sprintf(buf,"%s",row[0]);
    mysql_free_result(res);

    int len_buf=strlen(buf)-1;
    *(bufp+len_buf)=0;
    len_buf--;

    while(len_buf>=0)
    {
printf("输出buf%s\n",(bufp+len_buf));
        if(*(bufp+len_buf)=='/')
        {
            sprintf(query,"insert into conversation (formid,toid,time,readed,new,gfid)values(%s,%s,now(),0,\"%s\",%s)",buf2,(bufp+len_buf+1),buf3,buf1);
printf("%s\n",buf1);
            if(mysql_real_query(mysqll,query, strlen(query)))//插入群组
            {
                printf("获取群失败 %s\n",mysql_error(mysqll));
                exit(1);
            }
            if(len_buf==0)
                break;
            *(bufp+len_buf)=0;
        }
        else
            len_buf--;
    }

}
void add_groups_friend(struct Works_fd *timp_work,char *buf1,char *buf2)
{
    char query[100];        //给数据库发的命令
    memset(query, '\0', sizeof(query));
    char timp_buf[800];
    MYSQL_RES *res;         // 返回查询结果
    MYSQL_ROW row;          // 返回行数据
    sprintf(query,"select groups from information where id=%s",buf2);
printf("%s\n",query);
    if(mysql_real_query(mysqll,query, strlen(query)))//插入群组
    {
        printf("获取群失败 %s\n",mysql_error(mysqll));
        exit(1);
    }
    res = mysql_store_result(mysqll);
    row = mysql_fetch_row(res);
    sprintf(timp_buf,"%s%s/",row[0],buf1);
    mysql_free_result(res);
    sprintf(query,"update information set groups=\"%s\"",timp_buf);
printf("%s\n",query);
    if(mysql_real_query(mysqll,query, strlen(query)))//插入群组
    {
        printf("获取群失败 %s\n",mysql_error(mysqll));
        exit(1);
    }

    sprintf(query,"select vip from groups where id=%s",buf1);
printf("%s\n",query);
    if(mysql_real_query(mysqll,query, strlen(query)))//插入群组
    {
        printf("获取群失败 %s\n",mysql_error(mysqll));
        exit(1);
    }
    res = mysql_store_result(mysqll);
    row = mysql_fetch_row(res);
    sprintf(timp_buf,"%s%s/",row[0],buf2);
    mysql_free_result(res);
    sprintf(query,"update groups set vip=\"%s\"",timp_buf);
printf("%s\n",query);
    if(mysql_real_query(mysqll,query, strlen(query)))//插入群组
    {
        printf("获取群失败 %s\n",mysql_error(mysqll));
        exit(1);
    }
}
void show_vip(struct Works_fd *timp_work,char *buf1)
{
printf("%s\n",buf1);
    MYSQL_RES *res;         // 返回查询结果
    MYSQL_ROW row;          // 返回行数据
    char buf_send[800];
    memset(buf_send, '\0', sizeof(buf_send));
    if(mysql_real_query(mysqll,buf1, strlen(buf1)))
    {
        printf("获取好友失败 %s\n",mysql_error(mysqll));
        exit(1);
    }
    res = mysql_store_result(mysqll);
    row = mysql_fetch_row(res);
    sprintf(buf_send,"%s",row[0]);
    int i;
    for(i=0;i<strlen(buf_send);i++)
        if(buf_send[i]=='/')
            buf_send[i]='\n';
    client_send(timp_work,11,buf_send,"NULL","NULL","NULL");
}
void * run_pthread()                            //线程工作函数
{
    char *buf1;
    char *buf2;
    char *buf3;
    char *buf4;
    char recv_buf[1000];
    struct Works_fd *timp_work=NULL;
    cJSON *root =cJSON_CreateObject();
    int function=0;
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
        root = cJSON_Parse(timp_work->work_buf);
        printf("RECV: %s\n",cJSON_Print(root));
        function=(cJSON_GetObjectItem(root,"function")->valueint);
printf("funtion %d\n",function);
        buf1=(char*)malloc(sizeof(char)*1000);
        buf2=(char*)malloc(sizeof(char)*1000);
        buf3=(char*)malloc(sizeof(char)*1000);
        buf4=(char*)malloc(sizeof(char)*1000);
        if(function>=6)
        {
            strcpy(buf1,(cJSON_GetObjectItem(root,"f1")->valuestring));
            strcpy(buf2,(cJSON_GetObjectItem(root,"f2")->valuestring));
            strcpy(buf3,(cJSON_GetObjectItem(root,"f3")->valuestring));
            strcpy(buf4,(cJSON_GetObjectItem(root,"f4")->valuestring));
        }
        cJSON_Delete(root);                             //回收root指针指向的内存
        switch(function)
        {
            case 1: login(timp_work);break;
            case 2: logon(timp_work);break;
            case 3: find_friend(timp_work);break;
            case 4: accept_friend(timp_work);break;
            case 5: talk_friend(timp_work);break;
            case 6: show_history(timp_work,buf1,buf2);break;
            case 7: create_group(timp_work,buf1);break;
            case 8: create_group_queue(timp_work,buf1);break;
            case 9: talk_groups(timp_work,buf1,buf2,buf3);break;
            case 10: add_groups_friend(timp_work,buf1,buf2);break;
            case 11: show_vip(timp_work,buf1);break;
            default:;
        }
        printf("areyou/n");
        free(timp_work);
        free(buf1);
        free(buf2);
        free(buf3);
        free(buf4);//释放结点
        timp_work=NULL;
        buf1=buf2=buf3=buf4=NULL;
    }
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

    mysql_connect();
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
        int resize_recv;                                //接收
        while(timp_head!=NULL)                          //轮寻客户描述符
        {
            if (FD_ISSET(timp_head->client_fd, readfds))//如果有事件
            {
printf("他是一个有问题的人%d\n",timp_head->client_fd);
                struct Works_fd *timp_work=(struct Works_fd*)malloc(sizeof(struct Works_fd));//赋给结点
                timp_work->work_fd=timp_head->client_fd;
                timp_work->work_id=timp_head->client_id;
                timp_work->work_next=NULL;//这里也没有给出一个具体的时间来
                resize_recv=recv(timp_work->work_fd,timp_work->work_buf,sizeof(char )*1000,0);
                if(resize_recv<0)
                    my_error("recv",__LINE__);
                else if(resize_recv==0)
                {
printf("我要退出,进来没?\n");
                    Exit(timp_work);
                    break;
                }
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
                pthread_mutex_unlock(&mpool->queue_lock);//解锁
                pthread_cond_signal(&mpool->queue_cond); //这里是如果有一个线程是在等待，则那个线程会被打开；如果线程都是在忙碌，那这个信号就没有意义；
            }
            timp_head=timp_head->client_next;
        }

    }
}


