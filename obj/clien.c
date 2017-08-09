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
#define HOST    "118.89.245.72"
#define USER    "root"
#define PASSWD  "Gjl1141411.1"
#define DB_NAME "myqq"
MYSQL *mysqll; // 创建一个MYSQL句柄并


int my_error(char *err,int line)
{
    perror(err);
    printf("%d\n",line);
    exit(1);
}
void mysql_connect()
{

    mysqll=mysql_init(NULL);//初始化
    if (!mysqll)
    {
        my_error("mysql_init", __LINE__);
    }
    if(!mysql_real_connect(mysqll, HOST, USER, PASSWD, DB_NAME, 0, NULL, 0))
    {
        my_error("mysql_real_connect", __LINE__);
    }
printf("Connect database sucessfully!\n\n");
}
int connect_server()
{
    struct sockaddr_in client_add;                           //初始化本地网络信息
    memset(&client_add,0,sizeof(struct sockaddr));
    client_add.sin_family=AF_INET;
    client_add.sin_port=htons(8080);
    if(inet_aton("192.168.30.181",&client_add.sin_addr)<0)    //wocaO!!这个ip每一次登录获取的都不一样
        my_error("inet_aton",__LINE__);
    int client_fd=socket(AF_INET,SOCK_STREAM,0);              //创建一个套接字
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
int login()
{
    printf("登录\n");
    int user_id;
    char user_password[SBUF];
    char query[100];
    MYSQL_RES *res;         // 返回查询结果
    MYSQL_ROW row;          // 返回行数据
    memset(query, '\0', sizeof(query));
    printf("输入ｉｄ:");
    scanf("%d",&user_id);
    if(user_id<100000&&user_id>999999)
        user_id=0;
    getchar();
    printf("输入密码： ");
    input_user_password(user_password);
printf("%d %s\n",user_id,user_password);
    sprintf(query,"select password from passworld where id=%d",user_id);
printf("%s\n",query);
    if(mysql_real_query(mysqll, query, strlen(query)))
    {
        printf("登录失败 %s\n",mysql_error(mysqll));
        return 0;
    }
    res = mysql_store_result(mysqll);
    row = mysql_fetch_row(res);
    if(strcmp(row[0],user_password)!=0)
    {
        printf("密码错误\n");
        return 0;
    }
    mysql_free_result(res);
    printf("登录成功\n");
}
int logon()
{
    printf("注册\n");

}
int main()
{
    //connect_server();
    mysql_connect();
    int choose_main=10;
    while(choose_main)
    {
        printf("欢迎\n");
        printf("1.登录\n");
        printf("2.注册\n");
        printf("3.找回\n");
        printf("0.退出\n");
        printf("请选择：\n");
        scanf("%d",&choose_main);
        switch(choose_main)
        {
            case 1:login();getchar();break;
            case 2:logon();getchar();break;
        }

    }
}
