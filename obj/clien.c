#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include <termios.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>

#include<arpa/inet.h>
#include<unistd.h>
#include "../cJSON.h"
#define SBUF 256

#include <mysql/mysql.h>
#define HOST    "158.46.288.78"
#define USER    "root"
#define PASSWD  "*********"
#define DB_NAME "my"
MYSQL *mysqll; // 创建一个MYSQL句柄并
struct friends
{
    int friend_id;              //id
    char friend_fname[22];      //备注
    int friend_state;           //状态
    int friend_nots;            //未读消息数
    struct friends *friend_next;
};
struct groups
{
    char group_id[10];
    char group_fname[40];
    int group_sum;
    int group_nots;
    struct groups *group_next;
};
struct groups *head_group=NULL;
struct friends *head_friend=NULL;
int client_fd;
int myid;
int my_error(char *err,int line)
{

    perror(err);
    printf("%d\n",line);
    exit(1);
}
void client_send(int function,char *buf1,char *buf2,char *buf3,char*buf4)
{

    cJSON *root=cJSON_CreateObject();//创建一个对象
printf("进入发送\n");
    cJSON_AddNumberToObject(root,"function",function);
    cJSON_AddStringToObject(root, "f1", buf1);//添加一条信息（键值对）
    cJSON_AddStringToObject(root, "f2", buf2);//添加一条信息（键值对）
    cJSON_AddStringToObject(root, "f3", buf3);//添加一条信息（键值对）
    cJSON_AddStringToObject(root, "f4", buf4);//添加一条信息（键值对）
    char *buf=cJSON_Print(root);
printf("%d %s\n",strlen(buf),buf);
    if(send(client_fd,buf,sizeof(char )*1000,0)<0)
        my_error("send",__LINE__);
    cJSON_Delete(root);
}
void client_recv()
{
    cJSON *root =cJSON_CreateObject();
    char *buf1;
    char *buf2;
    char *buf3;
    char *buf4;
    int function=0;
    char recv_buf[1000];
    if(recv(client_fd,recv_buf,sizeof(char )*1000,0)<0)
        my_error("recv",__LINE__);
    root = cJSON_Parse(recv_buf);
    function=(cJSON_GetObjectItem(root,"function")->valueint);
    buf1=(char*)malloc(sizeof(char)*1000);
    buf2=(char*)malloc(sizeof(char)*1000);
    buf3=(char*)malloc(sizeof(char)*1000);
    buf4=(char*)malloc(sizeof(char)*1000);
    strcpy(buf1,(cJSON_GetObjectItem(root,"f1")->valuestring));
    strcpy(buf2,(cJSON_GetObjectItem(root,"f2")->valuestring));
    strcpy(buf3,(cJSON_GetObjectItem(root,"f3")->valuestring));
    strcpy(buf4,(cJSON_GetObjectItem(root,"f4")->valuestring));
    switch(function)
    {
        case 7: printf("创建成功,群号为:%s",buf1);getchar();break;
    }


}
void mysql_connect()
{
    mysqll=mysql_init(NULL);//初始化
    if (!mysqll)
        my_error("mysql_init", __LINE__);
    if(!mysql_real_connect(mysqll, HOST, USER, PASSWD, DB_NAME, 0, NULL, 0))//链接数据库
        my_error("mysql_real_connect", __LINE__);
    if (!mysql_set_character_set(mysqll, "utf8"))
    {
        printf("New connection character set: %s\n",
        mysql_character_set_name(mysqll));
    }
printf("Connect database sucessfully!\n\n");
}
int connect_server()
{
    struct sockaddr_in client_add;                           //初始化本地网络信息
    memset(&client_add,0,sizeof(struct sockaddr));
    client_add.sin_family=AF_INET;
    client_add.sin_port=htons(8080);
    if(inet_aton("118.89.245.72",&client_add.sin_addr)<0)    //wocaO!!这个ip每一次登录获取的都不一样
        my_error("inet_aton",__LINE__);
    client_fd=socket(AF_INET,SOCK_STREAM,0);              //创建一个套接字
    if(client_fd<0)
        my_error("socket",__LINE__);
    if(connect(client_fd,(struct sockaddr *)&client_add,sizeof(struct sockaddr_in))<0)//建立连接
        my_error("connect",__LINE__);
    return 1;
}
int input_user_password(char password[SBUF])
{
    struct termios stored_settings;
    struct termios new_settings;
    tcgetattr (0, &stored_settings);// 得到原来的终端属性
    new_settings = stored_settings;
    new_settings.c_lflag &= (~ICANON);//设置非正规模式，程序每次要从终端读取一个字符的
    new_settings.c_cc[VTIME] = 0;//设置非正规模式下的读延时
    new_settings.c_cc[VMIN] = 1;// 设置非正规模式下的最小字符数
    tcsetattr (0, TCSANOW, &new_settings);// 设置新的终端属性
    char c;
    int i=0;
    while(c=getchar(),c!='\n')
    {
        password[i]=c;
        putchar('\b');
        i++;
    }
    tcsetattr (0, TCSANOW, &stored_settings);// 恢复默认设置
    if(i>10)
        password[10]=0;
    else
        password[i]=0;
    return 1;
}
void create_friend()        //获取好友转成链表
{
    char query[100];        //给数据库发的命令
    memset(query, '\0', sizeof(query));
    MYSQL_RES *res;         // 返回查询结果
    MYSQL_ROW row;          // 返回行数据
    sprintf(query,"select * from f%d",myid);
printf("%s\n",query);
    if(mysql_real_query(mysqll, query, strlen(query)))
    {
        printf("获取好友失败 %s\n",mysql_error(mysqll));
        exit(1);
    }
    res = mysql_store_result(mysqll);
    if(res==NULL)
        my_error("获取好友失败",__LINE__);
    row = mysql_fetch_row(res);
    if(row==NULL)
        return ;
    head_friend=(struct friends*)malloc(sizeof(struct friends));
    struct friends *timp_head_friend=head_friend;
    timp_head_friend->friend_next=NULL;
    while(1)
    {

printf("%s\n",row[0]);
        sscanf(row[0],"%d",&timp_head_friend->friend_id);
        row = mysql_fetch_row(res);
        if(row==NULL)
            break;
        timp_head_friend->friend_next=(struct friends*)malloc(sizeof(struct friends));
        timp_head_friend=timp_head_friend->friend_next;
        timp_head_friend->friend_next=NULL;
    }
    timp_head_friend=head_friend;
while(timp_head_friend!=NULL)
{
    printf("%d\n",timp_head_friend->friend_id);
    timp_head_friend=timp_head_friend->friend_next;
}
}
void friend_note()           //获取好友在线状态,和离线消息数字
{
    char query[100];        //给数据库发的命令
    memset(query, '\0', sizeof(query));
    MYSQL_RES *res;         // 返回查询结果
    MYSQL_ROW row;          // 返回行数据
    struct friends *timp_head=head_friend;
    while(timp_head!=NULL)
    {
        sprintf(query,"select state from information where id=%d",timp_head->friend_id);
printf("%s\n",query);
        if(mysql_real_query(mysqll, query, strlen(query)))
        {
            printf("获取好友失败 %s\n",mysql_error(mysqll));
            exit(1);
        }
        res = mysql_store_result(mysqll);
        row = mysql_fetch_row(res);
        if(res==NULL||row==NULL)
            my_error("获取好友状态失败\n",__LINE__);
        sscanf(row[0],"%d",&timp_head->friend_state);
        mysql_free_result(res);

        sprintf(query,"select qname from information where id=%d",timp_head->friend_id);
printf("%s\n",query);
        if(mysql_real_query(mysqll, query, strlen(query)))
        {
            printf("获取好友失败 %s\n",mysql_error(mysqll));
            exit(1);
        }
        res = mysql_store_result(mysqll);
        row = mysql_fetch_row(res);
        if(res==NULL||row==NULL)
            my_error("获取好友状态失败\n",__LINE__);
        sscanf(row[0],"%s",timp_head->friend_fname);
        mysql_free_result(res);

        timp_head->friend_nots=0;
        sprintf(query,"select readed from conversation where toid=%d and formid=%d and readed=0",myid,timp_head->friend_id);
printf("%s\n",query);
        if(mysql_real_query(mysqll, query, strlen(query)))
        {
            printf("获取消息失败 %s\n",mysql_error(mysqll));
            exit(1);
        }
        res = mysql_store_result(mysqll);
        while (row = mysql_fetch_row(res))
            timp_head->friend_nots++;
        mysql_free_result(res);
        timp_head=timp_head->friend_next;
    }

}
void talk_friend(char talk_id[10]) //聊天发消息
{

    int i;
    printf("会话  %s\n",talk_id);
    printf("输入消息\n");
    fflush(stdin);
    char note[800];
    for(i=0;i<800;i++)
    {
        note[i]=getchar();
        if(note[i]=='\n')
            break;
    }
    if(i>=800)
    {
        printf("说好的善待本程序的呢?\n");
        printf("一次输这么多你是故意的吗?\n");
        return ;
    }
    note[i]=0;
    if(strlen(note)==0)
    {
        printf("没有内容\n");
        return ;
    }
    char *buf;
    cJSON *root=cJSON_CreateObject();//创建一个对象
    if(strlen(talk_id)<6)
        cJSON_AddNumberToObject(root,"function",9);
    else
        cJSON_AddNumberToObject(root,"function",5);
    char timp_myid[10];
    sprintf(timp_myid,"%d",myid);
    cJSON_AddStringToObject(root, "f1", timp_myid);//添加一条信息（键值对）
    cJSON_AddStringToObject(root, "f2", talk_id);//添加一条信息（键值对）
    cJSON_AddStringToObject(root, "f3",note);//添加一条信息（键值对）
    cJSON_AddStringToObject(root, "f4","NULL");//添加一条信息（键值对）
    buf=cJSON_Print(root);
    cJSON_Delete(root);
printf("%d %s\n",strlen(buf),buf);
    if(send(client_fd,buf,sizeof(char )*1000,0)<0)
        my_error("send",__LINE__);
    fflush(stdin);
}
void show_history(char talk_id[10])//好友历史记录
{
    printf("会话记录\n");
    char query1[200];
    char query2[200];
    char id2[10];
    sprintf(id2,"%d",myid);
    if(strlen(talk_id)>=6)
    {
        sprintf(query1,"select time,formid,new from conversation where (formid=%s and toid=%d) or (toid=%s and formid=%d) order by time desc limit 20",talk_id,myid,talk_id,myid);
        sprintf(query2,"update conversation set readed=1 where (formid=%s and toid=%d) or (toid=%s and formid=%d) order by time desc limit 20",talk_id,myid,talk_id,myid);
    }
    else
    {
        sprintf(query1,"select time,gfid,new from conversation where formid=%s and toid=%d order by time desc limit 20",talk_id,myid);
        sprintf(query2,"update conversation set readed=1 where formid=%s and toid=%d order by time desc limit 20",talk_id,myid);
    }
    client_send(6,query1,query2,"NULL","NULL");
    char buf[1000];
    if(recv(client_fd,buf,sizeof(char)*1000,0)<0)
    {
        perror("6");
        exit (1);
    }
    cJSON *root =cJSON_CreateObject();
    root = cJSON_Parse(buf);
    printf("%s",(cJSON_GetObjectItem(root,"f1")->valuestring));
    getchar();
}
void show_friend(char talk_id[10])//显示详细信息
{
    getchar();
    char query[100];        //给数据库发的命令
    memset(query, '\0', sizeof(query));
    MYSQL_RES *res;         // 返回查询结果
    MYSQL_ROW row;          // 返回行数据
    sprintf(query,"select * from information where id=%s",talk_id);
printf("%s\n",query);
    if(mysql_real_query(mysqll, query, strlen(query)))
    {
        printf("获取好友失败 %s\n",mysql_error(mysqll));
        exit(1);
    }
    res = mysql_store_result(mysqll);
    row = mysql_fetch_row(res);
    if(res==NULL&&row==NULL)
        my_error("获取好友详细信息失败",__LINE__);
    char choose='y';
    while(choose!='q')
    {
        printf("账号:");
        printf("%s\n",row[0]);
        printf("昵称:");
        printf("%s\n",row[1]);
        printf("姓名:");
        if(row[2]!=NULL)
            printf("%s\n",row[2]);
        else
            printf("NULL\n");
        printf("年龄:");
        if(row[3]!=NULL)
            printf("%s\n",row[3]);
        else
            printf("NULL\n");
        printf("性别:");
        if(row[4]!=NULL)
            printf("%s\n",row[4]);
        else
            printf("NULL\n");
        printf("电话:");
        if(row[5]!=NULL)
            printf("%s\n",row[5]);
        else
            printf("NULL\n");
        printf("地址:");
        if(row[6]!=NULL)
            printf("%s\n",row[6]);
        else
            printf("NULL\n");
        printf("请输入选项\n");
        printf("退出:q  聊天:t  查看记录:s  传文件:f");
        scanf("%c",&choose);
        getchar();
        if(choose=='t')
        {
            talk_friend(talk_id);
        }
        if(choose=='s')
        {
            show_history(talk_id);
        }
    }
}
void friend_list()          //展示好友链表
{
    getchar();
    char choose_talk='y';
    char id[10];
    struct friends *timp_head=head_friend;
    while(choose_talk!='q')
    {
        create_friend();
        friend_note();
        timp_head=head_friend;

        printf("好友列表\n");
        printf("账号号     昵称     状态    消息\n");
        printf("--------------------------------\n");
        if(timp_head==NULL)
            printf("您当前没有好友,赶快去搜索添加好友吧\n");
        else
        {
            while(timp_head!=NULL)
            {
                printf("%d %s %d %d\n",timp_head->friend_id,timp_head->friend_fname,timp_head->friend_state,timp_head->friend_nots);
                timp_head=timp_head->friend_next;
            }
        }
        printf("输入选项\n");
        printf("退出:q    选择好友:s\n");
        scanf("%c",&choose_talk);
        getchar();
        if(choose_talk=='s')
        {
            printf("输入选择的好友账号:");
            scanf("%s",id);
            getchar();
            show_friend(id);
        }
    }
}
void find_friend()          //搜索添加好友
{
    printf("查找好友\n");
    int find_id;
    printf("查找\n");
    printf("请输入查找账号\n");
    scanf("%d",&find_id);
    char query[100];        //给数据库发的命令
    memset(query, '\0', sizeof(query));
    MYSQL_RES *res;         // 返回查询结果
    MYSQL_ROW row;          // 返回行数据
    sprintf(query,"select * from information where id=%d",find_id);
printf("%s\n",query);
    if(mysql_real_query(mysqll, query, strlen(query)))
    {
        printf("获取好友失败 %s\n",mysql_error(mysqll));
        exit(1);
    }
    res = mysql_store_result(mysqll);
    row = mysql_fetch_row(res);
    if(res==NULL||row==NULL)
    {
        printf("查无此人\n");
        return ;
    }
    for(int i=0;row[i]!=NULL;i++)
        printf("%s    ",row[i]);
    printf("\n");
    printf("是否添加他or她为好友？[y/n]\n");
    char ansswer;
    getchar();
    scanf("%c",&ansswer);
    getchar();
    mysql_free_result(res);
    if(ansswer=='y')
    {
        char *buf;
        cJSON *root=cJSON_CreateObject();//创建一个对象
        cJSON_AddNumberToObject(root,"function",3);
        cJSON_AddNumberToObject(root, "id",find_id);//添加一条信息（键值对）
        buf=cJSON_Print(root);
        cJSON_Delete(root);
printf("%d %s\n",strlen(buf),buf);
        if(send(client_fd,buf,sizeof(char )*1000,0)<0)
            my_error("send",__LINE__);
        printf("好友申请已发送\n");
    }
}
void accept_friend()        //查看好友申请并同意
{
    getchar();
    printf("好友请求\n");
    printf("账号 \n");
    char ansser='y';
    char *buf;
    char query[100];        //给数据库发的命令
    memset(query, '\0', sizeof(query));
    MYSQL_RES *res;         // 返回查询结果
    MYSQL_ROW row;          // 返回行数据
    sprintf(query,"select formid from invitation where toid=%d",myid);
printf("%s\n",query);
    if(mysql_real_query(mysqll, query, strlen(query)))
    {
        printf("获取好友失败 %s\n",mysql_error(mysqll));
        exit(1);
    }
    res = mysql_store_result(mysqll);
    row = mysql_fetch_row(res);
    if(res==NULL||row==NULL)
    {
        printf("当前无好友申请\n");
        return ;
    }
    do
    {
        printf("%s\n",row[0]);
        printf("是否接受他or她[y/n]:");
        scanf("%c",&ansser);
        getchar();
        cJSON *root=cJSON_CreateObject();//创建一个对象
        cJSON_AddNumberToObject(root,"function",4);
        cJSON_AddStringToObject(root, "fromid",row[0]);//添加一条信息（键值对）
        cJSON_AddNumberToObject(root, "toid",myid);//添加一条信息（键值对）*/
        if(ansser=='y')
            cJSON_AddNumberToObject(root,"ansswer",1);
        else
            cJSON_AddNumberToObject(root,"ansswer",0);
        buf=cJSON_Print(root);
        cJSON_Delete(root);
printf("%d %s\n",strlen(buf),buf);
        if(send(client_fd,buf,sizeof(char )*1000,0)<0)
            my_error("send",__LINE__);
    }while(row = mysql_fetch_row(res));

}
void add_friend(char group_id[10])//群里加好友
{
    struct friends *timp_head=head_friend;
    printf("账号号     昵称     \n");
    printf("----------------\n");
    if(timp_head==NULL)
        printf("您当前没有好友,赶快去搜索添加好友吧\n");
    else
    {
        while(timp_head!=NULL)
        {
            printf("%d %s\n",timp_head->friend_id,timp_head->friend_fname);
            timp_head=timp_head->friend_next;
        }
    }
    char timp[10];
    printf("请选择要发展的好友:");
    scanf("%s",timp);
    client_send(10,group_id,timp,"NULL","NULL");
}
void show_vip(char group_id[10])
{
    printf("群成员\n");
    char query1[200];
    sprintf(query1,"select vip from groups where id=%s",group_id);
    client_send(11,query1,"NULL","NULL","NULL");
    char buf[1000];
    if(recv(client_fd,buf,sizeof(char)*1000,0)<0)
    {
        perror("6");
        exit (1);
    }
    cJSON *root =cJSON_CreateObject();
    root = cJSON_Parse(buf);
    printf("%s",(cJSON_GetObjectItem(root,"f1")->valuestring));
    getchar();
}
void show_group(char group_id[10])//显示详细信息
{
    char choose='y';
    while(choose!='q')
    {
        getchar();
        printf("--------------------\n");
        struct groups *timp_head=head_group;
        while(timp_head!=NULL)
        {
            if(strcmp(timp_head->group_id,group_id)==0)
            {
                printf("群号: %s\n",timp_head->group_id);
                printf("群名: %s\n",timp_head->group_fname);
                break;
            }
            timp_head=timp_head->group_next;
        }
        printf("请输入选项\n");
        printf("退出:q  聊天:t  查看记录:s  查看群成员:f 发展下线:i \n");
        scanf("%c",&choose);
        getchar();
        if(choose=='t')
        {
            talk_friend(group_id);
        }
        if(choose=='i')
        {
            add_friend(group_id);
        }
        if(choose=='s')
        {
            show_history(group_id);
        }
        if(choose=='f')
        {
            show_vip(group_id);
        }
    }
}
void create_group_queue()        //获取好友转成链表
{
    char query[100];        //给数据库发的命令
    memset(query, '\0', sizeof(query));
    sprintf(query,"select groups from information where id=%d",myid);
printf("%s\n",query);
    client_send(8,query,"NULL","NULL","NULL");

    cJSON *root =cJSON_CreateObject();
    char *buf1;
    char *buf2;
    char *buf3;
    char recv_buf[1000];
    if(recv(client_fd,recv_buf,sizeof(char )*1000,0)<0)
        my_error("recv",__LINE__);
    root = cJSON_Parse(recv_buf);
    buf1=(char*)malloc(sizeof(char)*1000);
    buf2=(char*)malloc(sizeof(char)*1000);
    buf3=(char*)malloc(sizeof(char)*1000);
    strcpy(buf1,(cJSON_GetObjectItem(root,"f1")->valuestring));
    strcpy(buf2,(cJSON_GetObjectItem(root,"f2")->valuestring));
    strcpy(buf3,(cJSON_GetObjectItem(root,"f3")->valuestring));

    head_group=(struct groups*)malloc(sizeof(struct groups));
    struct groups *timp_head_group=head_group;
    timp_head_group->group_next=NULL;
printf("%s\n%s\n",buf1,buf2);
    int len_buf=strlen(buf1)-1;
    *(buf1+len_buf)=0;
    len_buf--;
    while(len_buf>=0)
    {
//printf("buf1 %s\n",buf1);
        if(*(buf1+len_buf)=='/')
        {
            sprintf(timp_head_group->group_id,"%s",(buf1+len_buf+1));
            if(len_buf==0)
                break;
            timp_head_group->group_next=(struct groups*)malloc(sizeof(struct groups));
            timp_head_group=timp_head_group->group_next;
            timp_head_group->group_next=NULL;
            *(buf1+len_buf)=0;
        }
        else
        {
            len_buf--;
        }
    }

    timp_head_group=head_group;
    len_buf=strlen(buf3);
    int i;
    for(i=0;i<len_buf;i++)
        if(buf3[i]=='/')
            buf3[i]=0;
    for(i=0;i<len_buf-1;i++)
    {
        if(*(buf3+i)==0)
        {
            sprintf(timp_head_group->group_fname,"%s",(buf3+i+1));
printf("buf3 %s  ##%s\n",buf3,timp_head_group->group_fname);
            timp_head_group=timp_head_group->group_next;
        }

    }
    timp_head_group=head_group;
    len_buf=strlen(buf2);
    for(i=0;i<len_buf;i++)
        if(buf2[i]=='/')
            buf2[i]=0;
    for(i=0;i<len_buf-1;i++)
    {
        if(*(buf2+i)==0)
        {
//printf("buf2 %s  ##%d\n",buf2,timp_head_group->group_nots);
            sscanf((buf2+i+1),"%d",&timp_head_group->group_nots);
            timp_head_group=timp_head_group->group_next;
        }

    }
/*
    len_buf=strlen(buf2)-1;
    *(buf2+len_buf)=0;
    len_buf--;
    while(len_buf>=0)
    {
        if(*(buf2+len_buf)=='/')
        {
            sscanf((buf2+len_buf+1),"%d",&timp_head_group->group_nots);
//printf("buf2 %s  ##%d\n",buf2,timp_head_group->group_nots);
            if(len_buf==0)
                break;
            timp_head_group=timp_head_group->group_next;
            *(buf2+len_buf)=0;
        }
        else
        {
            len_buf--;
        }
    }
*/
timp_head_group=head_group;
while(timp_head_group!=NULL)
{
    printf("%s %d\n",timp_head_group->group_id,timp_head_group->group_nots);
    timp_head_group=timp_head_group->group_next;
}
}
void create_group()                //创建组
{
    printf("建群\n");
    printf("请输入群名称:");
    char group_namme[40];
    scanf("%s",group_namme);
    char query1[200];
    char query2[200];
    sprintf(query1,"insert into groups (name,vip)values(\"%s\",\"/%d/\")",group_namme,myid);
    client_send(7,query1,"NULL","NULL","NULL");
    client_recv();
}
void group_list()          //展示好友链表
{
    getchar();
    char choose_talk='y';
    char id[10];
    struct groups *timp_head=head_group;
    while(choose_talk!='q')
    {
        create_group_queue();
        timp_head=head_group;

        printf("小组列表\n");
        printf("小组号     小组名      消息\n");
        printf("--------------------------------\n");
        if(timp_head==NULL)
            printf("您当前没有加入小组,赶快去搜索添加吧\n");
        else
        {
            while(timp_head!=NULL)
            {
                printf("%s %s %d\n",timp_head->group_id,timp_head->group_fname,timp_head->group_nots);
                timp_head=timp_head->group_next;
            }
        }
        printf("输入选项\n");
        printf("退出:q    选择群:s  新建群组:c\n");
        scanf("%c",&choose_talk);
        getchar();
        if(choose_talk=='s')
        {
            printf("输入选择的组账号:");
            scanf("%s",id);
            getchar();
            show_group(id);
        }
        if(choose_talk=='c')
        {
            getchar();
            create_group(id);
        }
    }
}
void send_file()
{
    ;
}
int infunction()            //登录成功二级菜单
{

    int choose_infun=10;
    while(choose_infun)
    {
        printf("选择菜单\n");
        printf("1.好友列表\n");
        printf("2.群列表\n");
        printf("3.用户查询\n");
        printf("4.文件传输\n");
        printf("5.好友请求\n");
        printf("0.退出\n");
        scanf("%d",&choose_infun);
        switch(choose_infun)
        {
            case 1:friend_list();getchar();break;
            case 3:find_friend();getchar();break;
            case 2:group_list();getchar();break;
            case 4:send_file();getchar();break;
            case 5:accept_friend();getchar();break;
            case 0:break;
        }
    }
}
int login()
{
    printf("登录\n");
    int user_id;
    char user_password[SBUF];
    printf("输入ｉｄ:");
    scanf("%d",&user_id);
    if(user_id<100000&&user_id>999999)
        user_id=0;
    getchar();
    printf("输入密码： ");
    input_user_password(user_password);
//printf("%d %s\n",user_id,user_password);
    char *buf;
    cJSON *root=cJSON_CreateObject();//创建一个对象
    cJSON_AddNumberToObject(root,"function",1);
    cJSON_AddNumberToObject(root, "id", user_id);//添加一条信息（键值对）
    cJSON_AddStringToObject(root, "password", user_password);//添加一条信息（键值对）*/
    buf=cJSON_Print(root);
    cJSON_Delete(root);
printf("%d %s\n",strlen(buf),buf);
    if(send(client_fd,buf,sizeof(char )*1000,0)<0)
        my_error("send",__LINE__);

    int flag;
    char buf1[1000];
    root=cJSON_CreateObject();

    if(recv(client_fd,buf1,sizeof(char )*1000,0)<0)
        my_error("recv",__LINE__);
printf("OK...,%s\n",buf1);
    root = cJSON_Parse(buf1);
    flag=(cJSON_GetObjectItem(root,"answer")->valueint);
    if(flag==0)
        printf("账号或密码错误\n");
    else
    {
        printf("登陆成功\n");
        myid=user_id;
        getchar();
        infunction();
    }
    free(buf);
}
void logon()
{
    char head_query[100];
    char end_query[100];
    sprintf(head_query,"insert into information (");
    sprintf(end_query,") values(");
    getchar();
    char answer='n';
    char qname[22];
    char name[10];
    int age;
    int sex;
    char phone[12];
    char addsser[100];
    char password[11];
    printf("注册\n");
    printf("注册条款\n");
    printf("请注册本账号的用户友好的操作本软件\n");
    printf("我已仔细阅读并接受注册条款[y/n];");
    scanf("%c",&answer);
    getchar();
//printf("%c\n",answer);
    if(answer=='n')
        return;
    printf("输入昵称:");
    scanf("%s",qname);
    sprintf(head_query,"%sqname,",head_query);
    sprintf(end_query,"%s\"%s\",",end_query,qname);
//sprintf(query,"insert into information(qname) values(\"%s\")",);
    getchar();
    printf("是否输入真实姓名[y/n]");
    scanf("%c",&answer);
    getchar();
    if(answer=='y')
    {
        scanf("%s",name);
        getchar();
        sprintf(head_query,"%sname,",head_query);
        sprintf(end_query,"%s\"%s\",",end_query,name);
    }
    printf("是否输入年龄[y/n]");
    scanf("%c",&answer);
    getchar();
    if(answer=='y')
    {
        scanf("%d",&age);
        getchar();
        sprintf(head_query,"%sage,",head_query);
        sprintf(end_query,"%s%d,",end_query,age);
    }
    printf("是否输入性别[y/n]");
    scanf("%c",&answer);
    getchar();
    if(answer=='y')
    {
        printf("1.boy  2.gril");
        scanf("%d",&sex);
        getchar();
        sprintf(head_query,"%ssex,",head_query);
        sprintf(end_query,"%s%d,",end_query,sex);
    }
    printf("是否输入电话[y/n]");
    scanf("%c",&answer);
    getchar();
    if(answer=='y')
    {
        scanf("%s",phone);
        getchar();
        sprintf(head_query,"%sphone,",head_query);
        sprintf(end_query,"%s\"%s\",",end_query,phone);
    }
    printf("是否输入地址[y/n]");
    scanf("%c",&answer);
    getchar();
    if(answer=='y')
    {
        scanf("%s",addsser);
        getchar();
        sprintf(head_query,"%saddsser,",head_query);
        sprintf(end_query,"%s\"%s\",",end_query,addsser);
    }
    printf("输入密码:");
    scanf("%s",password);
    sprintf(head_query,"%sstate,password,groups",head_query);
    sprintf(end_query,"%s0,\"%s\",\"/\")",end_query,password);
    sprintf(head_query,"%s%s",head_query,end_query);
    char *buf;
    cJSON *root =cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "function",2);
    cJSON_AddStringToObject(root, "note", head_query);//添加一条信息（键值对）
    buf=cJSON_Print(root);
    cJSON_Delete(root);
printf("%d %s\n",strlen(buf),buf);
    if(send(client_fd,buf,sizeof(char )*1000,0)<0)
        my_error("send",__LINE__);
    buf=(char *)malloc(sizeof(char)*1000);
    if(recv(client_fd,buf,sizeof(char)*1000,0)<0)
    {
        perror("6");
        exit (1);
    }
    root =cJSON_CreateObject();
    root = cJSON_Parse(buf);
    strcpy(buf,cJSON_GetObjectItem(root,"answer")->valuestring);
    printf("注册成功!您的账号为:%s\n请返回登录\n",buf);
}
int main()
{
    connect_server();
    mysql_connect();

    int choose_main=10;
    while(choose_main)
    {
        printf("欢迎\n");
        printf("1.登录\n");
        printf("2.注册\n");
        //printf("3.找回\n");
        printf("0.退出\n");
        printf("请选择：\n");
        scanf("%d",&choose_main);
        switch(choose_main)
        {
            case 1:login();getchar();myid=0;break;
            case 2:logon();getchar();break;
        }

    }
}
