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

#define PARAM_NONE 0     //无参数
#define PARAM_a    1     //all
#define PARAM_l    2     //详细信息
#define PARAM_R    4     //递归
#define PARAM_r    8     //倒序
#define PARAM_t    16    //时间
#define PARAM_MORE 32    //多文件
#define MAXLINE    80    //一行最多长度

char root_pathname[100];//获得初始目录
int my_err(char *err,int line)
{
    printf("line:%d ",line);
    perror(err);
    exit(1);
}
int Gjldw()
{
    printf("真是明智的想法，这里才是真正的说明\n");
    exit(0);
}
int Help()
{
    printf("这里应该是说明，但是我懒得写。。。\n");
    printf("tip:大喊”高嘉两大王“他会告诉你怎么用，记住是高嘉两大王\n");
    exit(0);
}
int play_file(char *name,int flag)//处理文件打开方式．．．
{
    printf("文件%s %d\n",name,flag);
    return 0;
}
int play_dir(char *pathname,int flag)//处理目录下的文件们．．．
{
    char time_pathname[100];
    if(getcwd(time_pathname,100)<0)
    {
        my_err("getcwd",__LINE__);
    }
    if(chdir(pathname)==-1) //切换工作目录，(接下来进行的一切操作都会之间影响这个目录),千万要记得返回时
        perror("chdir");

    char filename[256][100];
    int book[256];
    int sum,r_start,r_end;
    int i,j,k;

    DIR *odir;//打开目录
    struct dirent *rdir;//获取目录下的文件信息

    if(odir=opendir("./"),odir==NULL)
        my_err("opendir",__LINE__);

    sum=0;
    while(rdir=readdir(odir),rdir!=NULL)//循环读文件名保存数组
    {
        strcpy(filename[sum],rdir->d_name);
        book[sum]=sum;
        sum++;
    }
    if(sum>256)
        my_err("有这么多文件的吗",__LINE__);

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

    if(flag&PARAM_MORE)//如果有多个且为文件输出一下文件名
        printf("%s:\n",pathname);
    for(i=r_start;i<r_end;i++)
    {
        play_file(filename[book[abs(i)]],flag);
    }
    printf("\n");
    if(flag&PARAM_R)
    {
        struct stat buf;//保存文件信息
        for(i=r_start;i<r_end;i++)
        {
            if(!strcmp(".",filename[book[abs(i)]])||!strcmp("./",filename[book[abs(i)]])||!strcmp("..",filename[book[abs(i)]])||!strcmp("../",filename[book[abs(i)]]))
                continue;
            if(stat(filename[book[abs(i)]],&buf)==-1)
                my_err("stst",__LINE__);

            if(S_ISDIR(buf.st_mode))
            {
                printf("\n%s\\\%s:\n",pathname,filename[book[abs(i)]]);
                play_dir(filename[book[abs(i)]],flag);
            }
        }
    }




/*for(i=r_start;i<r_end;i++)
{
    printf("%s\n",filename[book[abs(i)]]);
}
    printf("目录%s %d\n",pathname,flag);

*/



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
    if(getcwd(root_pathname,100)<0)
    {
        my_err("getcwd",__LINE__);
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
 printf("%d %d\n",flag_param,i);
    if(i==argc)//只有一个文件名
        play_dir("./",flag_param);
    if(i+1<argc)       //两个以上
        flag_param|=PARAM_MORE;
    for(i;i<argc;i++)//遍历一遍文件名
    {
        if(stat(argv[i],&buf)==-1)//文件是否存在是否是
            my_err(argv[i],__LINE__);
        if(S_ISDIR(buf.st_mode))
            play_dir(argv[i],flag_param);
        else
            play_file(argv[i],flag_param);

    }

}






