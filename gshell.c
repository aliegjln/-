#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/types.h>
#include<fcntl.h>
#include<unistd.h>
#include<signal.h>        //sigal
#include<sys/stat.h>
#include<time.h>
#include<syslog.h>
#include<sys/param.h>   //包含NOFILE
#include<sys/wait.h>
#include <errno.h>
#include<dirent.h>

#define PARAM_NONE 0     //无参数
#define PARAM_out1 1     //all
#define PARAM_out2 2     //递归
#define PARAM_beh  4     //倒序
#define PARAM_pip  8    //时间
#define PARAM_in   16
char in_file[100];
char out_file[100];
char time_filein[100]="/home/gjln/timein.txt";
char time_fileout[100]="/home/gjln/timeout.txt";
int flag_param;
int my_error(char *err,int line)
{
    printf("Line:%d \n",line);
    perror(err);
    exit (1);
}
int printf_tip()//输出
{
    int n,i;
    char path[200];
    char *home,*pa=path;
    if(getcwd(path,200)<0)
        my_error("getcwd",__LINE__);
    home=getenv("HOME");
    n=strlen(home);
//printf("%d %s %s\n",n,home,path);
    for(i=0;i<n;i++)
    {
        if(*(pa++)!=*(home++))
        {
            break;
        }
    }
//printf("%d %c\n",i,*(pa));
    if(i==n&&*(pa)=='/')
    {
        printf("~%s",pa);
    }
    else
    {
        printf("%s",path);
    }
    printf("# ");
}
int in_commands(char cmd_long[256])//输入
{
    int len=0;
    char timec;

    timec=getchar();
    while(len<256&&timec!='\n')
    {
        cmd_long[len]=timec;
        len++;
        timec=getchar();
    }
    if(len==256)
    {
        my_error("输入命令过长",__LINE__);
    }
    cmd_long[len]=0;
}
int apart_commands(char **cmd_s,char cmd_long[256])//book看之前是不是空的，如果不是就说名有冲突，语法错误
{
    int n=strlen(cmd_long);
    int i,j,k;
    j=0;
    for(i=0;i<n;i++)
    {
        if(cmd_long[i]=='&')//处理&（必须是最后一个）不然语法错误；
        {
            if(i!=n-1)
            {
                printf("&附近语法错误\n");
                return 0;
            }
            else
                flag_param|=PARAM_beh;
        }
        else if(cmd_long[i]=='>')//处理> >>
        {

            if((flag_param&PARAM_out1)||(flag_param&PARAM_out2))//有且只有一个
            {
                printf("语法错误:含有多个定向输出\n");
                return 0;
            }
            else if(cmd_long[i+1]=='>')//>>
            {
                 flag_param|=PARAM_out2;
                 i++;
            }
            else//>
            {
                flag_param|=PARAM_out1;
            }
            i++;
            for(k=0;cmd_long[i]!=' '&&i<n;k++,i++)//>后面紧跟的东西作为文件名，如果是空格就gg以后就会出错，有提醒
            {
                out_file[k]=cmd_long[i];
            }
            out_file[k]==0;
        }
        else if(cmd_long[i]=='<')
        {
            if(flag_param&PARAM_in)//有且只有一个
            {
                printf("语法错误:含有多个定向输入\n");
                return 0;
            }
            flag_param|=PARAM_in;
            i++;
            for(k=0;cmd_long[i]!=' '&&i<n;k++,i++)
            {
                in_file[k]=cmd_long[i];
            }
            in_file[k]==0;
        }
        else if(cmd_long[i]!='>'&&cmd_long[i]!='<'&&cmd_long[i]!='|'&&cmd_long[i]!='&')
        {
            for(k=0;i<n&&cmd_long[i]!='>'&&cmd_long[i]!='<'&&cmd_long[i]!='|'&&cmd_long[i]!='&';k++,i++)
            {
                cmd_s[j][k]=cmd_long[i];
            }
            j++;
            i--;
        }
    }
    cmd_s[j]=NULL;
    return 1;
}
int apart_command(char *argv[256],char cmd_time[100])
{
//printf("传入222apart_cmd:%s\n",cmd_time);
    int i,j=0,k;
    int flag=0;
    int n=strlen(cmd_time);
    for(i=0;i<n;i++)
    {
        for(k=0;cmd_time[i]!=' '&&i<n;k++,i++)
        {
            argv[j][k]=cmd_time[i];
            flag=1;
        }
        if(flag)
        {
            argv[j][k]=0;
            j++;
            flag=0;
        }
    }
    argv[j]=NULL;
/*for(i=0;argv[i]!=NULL;i++)
{
    printf("^^^^%s\n",argv[i]);
}*/
    return 0;
}
int find_command(char *cmd_short)
{
    DIR *dp;
    struct dirent* dirp;
    char *path[]={"./","/bin","/usr/bin",NULL};
    if(*(cmd_short)=='.'&&*(cmd_short+1)=='/')
    {
        cmd_short+=2;
    }
    int i=0;
    while(path[i]!=NULL)
    {
        if((dp=opendir(path[i]))==NULL)
        {
            printf("bin打不开\n");
        }
        while((dirp=readdir(dp))!=NULL)
        {
            if(strcmp(dirp->d_name,cmd_short)==0)
            {
                closedir(dp);
                return 1;
            }
        }
        closedir(dp);
        i++;
    }
    return 0;
}
int do_commands(char **cmd_s)
{
    int pid1;
    int pid2;

    int i,j;
    char *argv[256];

    pid1=fork();
    if(pid1<0)//失败
    {
        my_error("fock",__LINE__);
    }
    if(pid1==0)//子
    {

        for(i=0;cmd_s[i]!=NULL;i++)
        {

            for(j=0;j<256;j++)
            {
                argv[j]=(char *)malloc(sizeof(char)*100);
            }
            apart_command(argv,cmd_s[i]);//进一步解析命令
            if(find_command(argv[0])==0)//找命令
            {
                printf("我找呀找不到%s这个命令\n",argv[0]);
                break;
            }
            pid2=fork();
            if(pid2<0)//失败
            {
                my_error("fock",__LINE__);
            }
            if(pid2>0)//子
            {

                int stsus;//if(&)不等，检测
                if(wait(&stsus)==-1)
                    printf("等子进程失败\n");;//等
            }
            if(pid2==0)//孙
            {
                int fp_in;
                int fp_out;
                if(i==0&&(flag_param&PARAM_in))//输入重定向第一次有文件
                {
                    if((fp_in=open(in_file,O_RDONLY))==-1)
                        my_error("qwer.txt",__LINE__);
                    dup2(fp_in,0);
                }
                else if(i!=0)//除了第一次都要从time文件里读
                {
                    char timeb;
                    char *b=&timeb;
                    int fp1=open(time_filein,O_WRONLY|O_CREAT|O_TRUNC,0777);
                    int fp2=open(time_fileout,O_RDONLY);
                    while(read(fp2,&b,1))
                    {
                        write(fp1,&b,sizeof(char));
                    }
                    close(fp1);
                    close(fp2);

                    if((fp_in=open(time_filein,O_RDONLY))==-1)
                        my_error("qwer.txt",__LINE__);
                    dup2(fp_in,0);
                }//都不满足就之间从接盘读入
                if(cmd_s[i+1]==NULL&&((flag_param&PARAM_out1)||(flag_param&PARAM_out2)))//最后一次，并且有定向输出
                {
                    if(flag_param&PARAM_out1)//>
                    {
                        if((fp_out=open(out_file,O_WRONLY|O_CREAT|O_TRUNC,0777))==-1)
                            my_error("qwer.txt",__LINE__);
                        dup2(fp_out,1);
                    }
                    else
                    {
                         if((fp_out=open(out_file,O_WRONLY|O_CREAT|O_APPEND ,0777))==-1)
                            my_error("qwer.txt",__LINE__);
                        dup2(fp_out,1);
                    }
                }
                else if(cmd_s[i+1]!=NULL)
                {
                    if((fp_out=open(time_fileout,O_WRONLY|O_CREAT|O_TRUNC,0777))==-1)
                            my_error("qwer.txt",__LINE__);
                    dup2(fp_out,1);
                }
                printf("%s 找到啦！！！！！\n",argv[0]);//换程序
                execvp(argv[0],argv);
                exit(0);
            }
        }
        exit (0);
    }
    if(pid1>0)//父
    {
        int stsus;//if(&)不等，检测
        if(wait(&stsus)==-1)
            printf("等子进程失败\n");
        //if（！&）等
    }

}
int main()
{
    char cmd_long[256];
    while(1)
    {
        flag_param=PARAM_NONE;//初始化参数
        printf_tip();
        in_commands(cmd_long);
//printf("%s\n",cmd_s);
//退出
       // char cmd_s[25 6][100];
        char **cmd_s=(char **)malloc(sizeof(char *)*256);
        int i;
        for(i=0;i<256;i++)
        {
            cmd_s[i]=(char*)malloc(sizeof(char)*100);
        }
        if(!apart_commands(cmd_s,cmd_long))
            continue;

for(i=0;cmd_s[i]!=NULL;i++)
{
    printf("****%s\n",cmd_s[i]);
}
        do_commands(cmd_s);

    }

}
