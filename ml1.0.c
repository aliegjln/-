#include<stdio.h>
#include<stdio.h>
#include<sys/types.h>
#include<dirent.h>//目录文件
#include<sys/stat.h>//获取属性命令
#include <unistd.h>
#include<stdlib.h>
#include <string.h>
#include <pwd.h>//包含getpwuid
#include <grp.h>//getgrgid
#include<fcntl.h>
#include <time.h> //ctime();
#include <math.h>
#include <errno.h>

#define PARAM_NONE 0     //无参数
#define PARAM_a    1     //all
#define PARAM_l    2     //详细信息
#define PARAM_R    4     //递归
#define PARAM_r    8     //倒序
#define PARAM_t    16    //时间
#define PARAM_MORE 32    //多文件
#define MAXLINE    80    //一行最多长度

char R_name[200];//-R 相对路径
int line_long;//最长文件
int line_rest;//一行剩余长度


int printf_strlen(char a[])//字符串在屏幕上占的长度
{
    int i,sum=0;
    for(i=0;i<strlen(a);i++)
    {
        if(a[i]<0)
            sum++;
    }
    sum/=3;
    return strlen(a)-sum;
}
int my_err(char *err,int line)
{
    printf("line:%d \n",line);
    perror(err);
    exit(1);
}
int Gjldw()
{
    printf("真是明智的想法，这里才是真正的说明\n\n");
    printf("用法：ml[选项]…[文件]…\n");
    printf("多文件名纵向并列输出。\n目录下文件横向并列输出。\n");
    printf("默认按照文件名排序。\n\n");
    printf("选择参数对长短选项同时适用。\n");
    printf("  -a :       不隐藏任何以. 开始的项目\n");
    printf("  -l :       使用较长格式列出信息\n");
    printf("  -R :       递归显示子目录\n");
    printf("  -r :       逆序排列\n");
    printf("  -t :       按照文件修改时间排序\n");
    printf("  -help:     显示假的帮助信息并退出\n");
    printf("  -gjldw:    显示此帮助信息并退出\n\n");
    printf("退出状态:\n");
    printf("  0:  正常\n");
    printf("  1:  问题退出（例如：无法访问文件）\n");
    exit(0);
}
int Help()
{
    printf("这里应该是说明，但是我懒得写。。。\n");
    printf("tip:大喊”高嘉两大王“他会告诉你怎么用，\n    记住是高嘉两大王\n\n");
    exit(0);
}
int printf_colour(char name[])
{
    struct stat buf;
    if(lstat(name,&buf)==-1)
    {
        if(errno==2||errno==13)
        {
            perror(name);
            return 0 ;
        }
        else
            my_err(name,__LINE__);
    }
    if(S_ISLNK(buf.st_mode))//st_mode中获取文件类型（通过使用ＰＯＳＩＸ定义的一系列宏）
    {
        printf("\033[36m%s\033[0m",name);//|符号链接文件
        return 0;
    }

    if(S_ISDIR(buf.st_mode))
    {
        printf("\033[34m%s\033[0m",name);//d目录文件
        return 0;
    }
    if(S_ISCHR(buf.st_mode)||S_ISBLK(buf.st_mode))
    {
         printf("\033[33m%s\033[0m",name);// 字符设备文件//设备文件
         return 0;
    }
    if(buf.st_mode& S_IXUSR)
    {
         printf("\033[32m%s\033[0m",name);//x 可执行文件
         return 0;
    }
    printf("\033[37m%s\033[0m",name);
    return 0;


}
int show_name(char *name)
{
    int i,len;
    len=printf_strlen(name);
    if(len>line_rest)
    {
        printf("789\n");
        line_rest=80;
    }
    printf_colour(name);//printf("%s",name);//
    line_rest=line_rest-line_long-1;
    for(i=0;i<=line_long-len&&i<=line_rest;i++)
        printf(" ");
//printf("%d %d",line_long,line_rest);
    return 0;
}
int show_property(char name[])
{
    struct stat buf;
    struct passwd *pad;//保存得到的用户名
    struct group *grp;//保存得到的组名
    char buftime[30];//储存时间字符串
    if(lstat(name,&buf)==-1)
    {
        if(errno==2||errno==13)
        {
            perror(name);
            return 0 ;
        }
        else
            my_err(name,__LINE__);
    }
    if(S_ISLNK(buf.st_mode))//st_mode中获取文件类型（通过使用ＰＯＳＩＸ定义的一系列宏）
        printf("|");//符号链接文件
    if(S_ISREG(buf.st_mode))
        printf("-");//普通文件
    if(S_ISDIR(buf.st_mode))
        printf("d");//目录文件
    if(S_ISCHR(buf.st_mode))
        printf("c");//字符设备文件
    if(S_ISBLK(buf.st_mode))
        printf("b");//设备文件
    if(S_ISFIFO(buf.st_mode))
        printf("p");//管道文件
    if(S_ISSOCK(buf.st_mode))
        printf("s");//套接字文件

    if(buf.st_mode& S_IRUSR)//文件所有者(宏定义)
        printf("r");
    else
        printf("-");
    if(buf.st_mode& S_IWUSR)
        printf("w");
    else
        printf("-");
    if(buf.st_mode& S_IXUSR)
        printf("x");
    else
        printf("-");

    if(buf.st_mode& S_IRGRP)//文件所有者同组
        printf("r");
    else
        printf("-");
    if(buf.st_mode& S_IWGRP)
        printf("w");
    else
        printf("-");
    if(buf.st_mode& S_IXGRP)
        printf("x");
    else
        printf("-");

    if(buf.st_mode& S_IROTH)//文件其他人权限
        printf("r");
    else
        printf("-");
    if(buf.st_mode& S_IWOTH)
        printf("w");
    else
        printf("-");
    if(buf.st_mode& S_IXOTH)
        printf("x");
    else
        printf("-");

    printf(" %-5d",buf.st_nlink);//链接数

    pad=getpwuid(buf.st_uid);//这个函数通过用户ＩＤ找到用户信息赋给结构体
    printf(" %-5s",pad->pw_name);//名字在结构体里面
    grp=getgrgid(buf.st_gid);//这个函数通过用户组ＩＤ找到用户组信息赋给结构体
    printf(" %-5s",grp->gr_name);//名字在结构体里面

    printf("%-6d",(int )buf.st_size);

    strcpy(buftime,ctime(&buf.st_mtime));//ctime()可以把时间转成字符串,返回值为字符串(莫名其妙，dirbuf结构体里面是st_mtim,但是那里面要再填个e);
    buftime[strlen(ctime(&buf.st_mtime))-1]=0;//返回的字符串末尾是一个回车，这个操作就是为了取掉回车；
    printf(" %s",buftime);

    printf_colour(name);
    printf("\n");

    return 0;
}
int play_file(char *name,int flag)//处理文件打开方式．．．
{
    if(flag&PARAM_l)
        show_property(name);
    else
        show_name(name);

    //printf("文件%s %d\n ",name,flag);
    return 0;
}
int shot_time(char filename[][100],int book[],int n)//文件名，记录数组，文件个数
{
    char (*filetime)[100]=(char (*)[100])malloc(sizeof(char (*)[100])*n*100);
    struct stat file_buf;
    int i,j,k;
    for(i=0;i<n;i++)
    {

        if(lstat(filename[book[i]],&file_buf)==-1)
           my_err("lstat",__LINE__);
        strcpy(filetime[book[i]],ctime(&file_buf.st_mtime));//ctime()可以把时间转成字符串,返回值为字符串
        for(j=0;j<4;j++)//转换年份
        {
            filetime[book[i]][j]=filetime[book[i]][j+20];
            filetime[book[i]][j+20]=0;
        }
        if(filetime[book[i]][j]=='J'&&filetime[book[i]][j+1]=='a')//转换月份
           filetime[book[i]][j+2]=filetime[book[i]][j+1]=filetime[book[i]][j]='1';
        if(filetime[book[i]][j]=='F')
           filetime[book[i]][j+2]=filetime[book[i]][j+1]=filetime[book[i]][j]='2';
        if(filetime[book[i]][j]=='M'&&filetime[book[i]][j+2]=='r')
           filetime[book[i]][j+2]=filetime[book[i]][j+1]=filetime[book[i]][j]='3';
        if(filetime[book[i]][j]=='A'&&filetime[book[i]][j+1]=='p')
           filetime[book[i]][j+2]=filetime[book[i]][j+1]=filetime[book[i]][j]='4';
        if(filetime[book[i]][j]=='M'&&filetime[book[i]][j+2]=='y')
           filetime[book[i]][j+2]=filetime[book[i]][j+1]=filetime[book[i]][j]='5';
        if(filetime[book[i]][j]=='J'&&filetime[book[i]][j+2]=='n')
           filetime[book[i]][j+2]=filetime[book[i]][j+1]=filetime[book[i]][j]='6';
        if(filetime[book[i]][j]=='J'&&filetime[book[i]][j+2]=='l')
           filetime[book[i]][j+2]=filetime[book[i]][j+1]=filetime[book[i]][j]='7';
        if(filetime[book[i]][j]=='A'&&filetime[book[i]][j+1]=='u')
           filetime[book[i]][j+2]=filetime[book[i]][j+1]=filetime[book[i]][j]='8';
        if(filetime[book[i]][j]=='S')
           filetime[book[i]][j+2]=filetime[book[i]][j+1]=filetime[book[i]][j]='9';
        if(filetime[book[i]][j]=='O')
           filetime[book[i]][j+2]=filetime[book[i]][j+1]=filetime[book[i]][j]=':';
        if(filetime[book[i]][j]=='N')
           filetime[book[i]][j+2]=filetime[book[i]][j+1]=filetime[book[i]][j]=':';
        if(filetime[book[i]][j]=='D')
           filetime[book[i]][j+2]=filetime[book[i]][j+1]=filetime[book[i]][j]='<';
    }
    for(i=0;i<n;i++)//按名字排序
        for(j=0;j<n-1-i;j++)
        {
            if(strcmp(filetime[book[j]],filetime[book[j+1]])<0)
            {
                k=book[j];
                book[j]=book[j+1];
                book[j+1]=k;
            }
        }
}
int play_dir(char *pathname,int flag)//处理目录下的文件们．．．
{

    strcat(R_name,pathname);//，当前目录添加到递归目录
    if(R_name[strlen(R_name)-1]!='/')
    {
        R_name[strlen(R_name)+1]=0;
        R_name[strlen(R_name)]='/';
    }
    char time_pathname[100];//或取之前的目录
    if(getcwd(time_pathname,100)<0)
    {
        my_err("getcwd",__LINE__);
    }

    if(chdir(pathname)==-1) //切换工作目录，(接下来进行的一切操作都会之间影响这个目录),千万要记得返回时
    {
        if(errno==13)
        {
            perror(pathname);
            if(chdir(time_pathname)==-1) //切换回来工作目录
            perror("chdir");
            return 0 ;
        }
        else
            my_err("chird",__LINE__);
    }


    int sum,r_start,r_end;
    int i,j,k;
    DIR *odir;//打开目录
    struct dirent *rdir;//获取目录下的文件信息

    if(odir=opendir("./"),odir==NULL)
    {
        if(errno==13)
        {

            perror(R_name);
            if(chdir(time_pathname)==-1) //切换回来工作目录
                perror("chdir");
            return 0 ;
        }
        else
            my_err("opendir",__LINE__);
    }
    sum=0;
    while(rdir=readdir(odir),rdir!=NULL)//循环读文件名找到文件个数
    {
        sum++;
    }
    closedir(odir);

    if(odir=opendir("./"),odir==NULL)
    {
        if(errno==13)
        {
            perror(R_name);
            if(chdir(time_pathname)==-1) //切换回来工作目录
                perror("chdir");
            return 0 ;
        }
        else
            my_err("opendir",__LINE__);
    }
    char (*filename)[100]=(char (*)[100])malloc(sizeof(char (*)[100])*sum*100);
    int *book=(int *)malloc(sizeof(int )*sum);

    sum=0;
    while(rdir=readdir(odir),rdir!=NULL)//循环读文件名保存数组
    {
        strcpy(filename[sum],rdir->d_name);
        book[sum]=sum;
        sum++;
    }
    closedir(odir);

    //if(sum>10000)
     //   my_err("有这么多文件的吗",__LINE__);


    if(!(flag&PARAM_a))                  //处理a
        for(i=0;i<sum;i++)
            if(filename[book[i]][0]=='.')
            {
                for(j=i;j<sum-1;j++)
                    book[j]=book[j+1];
                sum--;
                i--;
            }

    for(i=0;i<sum;i++)//按名字排序
        for(j=0;j<sum-1-i;j++)
        {
            if(strcmp(filename[book[j]],filename[book[j+1]])>0)
            {
                k=book[j];
                book[j]=book[j+1];
                book[j+1]=k;
            }
        }

    if(flag&PARAM_t)//按时间排序
        shot_time(filename,book,sum);

    if(flag&PARAM_r)//倒序处理
    {
        r_start=-sum+1;
        r_end=1;
    }
    else
    {
        r_start=0;
        r_end=sum;
    }

    line_long=0;
    line_rest=80;
    for(i=0;i<sum;i++)//找出最大宽度
    {
        if(printf_strlen(filename[book[i]])>line_long)
            line_long=printf_strlen(filename[book[i]]);
    }

    if(flag&PARAM_MORE && !(flag&PARAM_R))//如果有多个且为文件输出一下文件名
        printf("%s:\n",pathname);

    for(i=r_start;i<r_end;i++)//输出一遍
    {
        play_file(filename[book[abs(i)]],flag);
    }
    if(flag&PARAM_l)//对a ; 不同输出的格式控制
        printf("\n");
    else
        printf("\n123\n");

    if(flag&PARAM_R)
    {
        struct stat buf;//保存文件信息
        for(i=r_start;i<r_end;i++)
        {
            if(!strcmp(".",filename[book[abs(i)]])||!strcmp("./",filename[book[abs(i)]])||!strcmp("..",filename[book[abs(i)]])||!strcmp("../",filename[book[abs(i)]]))
                continue;
//printf("%s 这里 ",filename[book[abs(i)]]);
//printf("time-lstat:%d \n",lstat(filename[book[abs(i)]],&buf));
            if(lstat(filename[book[abs(i)]],&buf)==-1)
            {
                if(errno==2)
                {
                    perror(filename[book[abs(i)]]);
                    if(chdir(time_pathname)==-1) //切换回来工作目录
                        perror("chdir");
                    //getchar();
                    return 0 ;
                }
                if(errno=13)
                {
                     perror(filename[book[abs(i)]]);
                    if(chdir(time_pathname)==-1) //切换回来工作目录
                        perror("");
                    return 0 ;
                }
                else
                {
                    my_err(filename[book[abs(i)]],__LINE__);
                }
            }
            if(S_ISDIR(buf.st_mode))
            {

                printf("%s%s:456\n",R_name,filename[book[abs(i)]]);

                play_dir(filename[book[abs(i)]],flag);

                for(k=strlen(R_name)-2;k>=0;k--)
                {
                    if(R_name[k]!='/')
                        R_name[k]=0;
                    else
                        break;
                }
            }

        }
    }
    if(chdir(time_pathname)==-1) //切换回来工作目录
        perror("chdir");
    return 0;
}
int main(int argc,char **argv)
{
    if(argc>256)
    {
        printf("ml:一次性写这么多你是故意的吗?\n");
        exit(1);
    }
    int i,k;
    int flag_param=PARAM_NONE;
    struct stat buf;//保存文件信息
    for(i=1;i<argc;i++)//多个参数可能分开的就要遍历一遍
    {
        if(argv[i][0]!='-')
            break;
        else
        {
            for(k=1;k<strlen(argv[i]);k++)//一个－后面可能含有多个参数，所以也要遍历一遍
            {
                switch(argv[i][k])
                {
                    case 'a':flag_param|=PARAM_a;break;
                    case 'l':flag_param|=PARAM_l;break;
                    case 'R':flag_param|=PARAM_R;break;
                    case 'r':flag_param|=PARAM_r;break;
                    case 't':flag_param|=PARAM_t;break;
                    case 'h':
                        if(strlen(argv[i])>=5&&argv[i][k+1]=='e'&&argv[i][k+2]=='l'&&argv[i][k+3]=='p')
                        Help();
                    case 'g':
                        if(strlen(argv[i])>=6&&argv[i][k+1]=='j'&&argv[i][k+2]=='l'&&argv[i][k+3]=='d'&&argv[i][k+4]=='w')
                        Gjldw();
                    default:
                        printf("ml:没有 %s 这种操作\n你可以用""ml -hplp""查看一下说明\n",argv[i]);
                        exit(1);
                }
            }

        }
    }
    if(i==argc)//只有一个文件名
        play_dir("./",flag_param);
    if(i+1<argc)       //两个以上
        flag_param|=PARAM_MORE;
    for(i;i<argc;i++)//遍历一遍文件名
    {
        line_rest=80;
        if(lstat(argv[i],&buf)==-1)//文件是否存在是否是//全局使用lstat,f防止有链接文件；
            my_err(argv[i],__LINE__);
        if(S_ISDIR(buf.st_mode))
        {
            play_dir(argv[i],flag_param);

        }
        else
        {
            play_file(argv[i],flag_param);
            printf("\n");
        }
    }
    return 0;
}






