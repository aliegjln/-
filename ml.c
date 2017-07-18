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
int main(int argc,char **argv)
{


    if(chdir("../")==-1) //切换工作目录，(接下来进行的一切操作都会之间影响这个目录)
    {
        perror("dfgdfgd");
        exit(1);
    }
    DIR *odir;//打开目录
    struct dirent *rdir;//获取目录下的目录
    struct stat dirbuf;//获取文件属性
    struct passwd *pad;//保存得到的用户名
    struct group *grp;//保存得到的组名
    char buftime[30];//储存时间字符串

    odir=opendir("./");//已经进入到这个目录了，就不用再使用相对路径了
    if(odir==NULL)
    {
        perror("/home/gjln/gjl");
        exit(1);
    }

    while (rdir=readdir(odir),rdir!=NULL)
    {


        if(stat(rdir->d_name,&dirbuf)==-1)//获得文件属性结构体后，里面的都是模式值，还要一些奇怪的操作把模式值转化为字符串；
        {
            perror("/hogjl");//这里会出错应为之前第一次做没有切换目录，opendir正常打开后获取了相对路径下的文件名称，而自己目路下没有这个文件。
            exit(1);//所以如果不切换目录stat时候就要讲rdir->name,保存在字符串里，然后处理成相对路径就好了。
        }

        if(S_ISLNK(dirbuf.st_mode))//st_mode中获取文件类型（通过使用ＰＯＳＩＸ定义的一系列宏）
            printf("|");//符号链接文件
        if(S_ISREG(dirbuf.st_mode))
            printf("-");//普通文件
        if(S_ISDIR(dirbuf.st_mode))
            printf("d");//目录文件
        if(S_ISCHR(dirbuf.st_mode))
            printf("c");//字符设备文件
        if(S_ISBLK(dirbuf.st_mode))
            printf("b");//设备文件
        if(S_ISFIFO(dirbuf.st_mode))
            printf("p");//管道文件
        if(S_ISSOCK(dirbuf.st_mode))
            printf("s");//套接字文件

        if(dirbuf.st_mode& S_IRUSR)//文件所有者(宏定义)
            printf("r");
        else
            printf("-");
        if(dirbuf.st_mode& S_IWUSR)
            printf("w");
        else
            printf("-");
        if(dirbuf.st_mode& S_IXUSR)
            printf("x");
        else
            printf("-");

        if(dirbuf.st_mode& S_IRGRP)//文件所有者同组
            printf("r");
        else
            printf("-");
        if(dirbuf.st_mode& S_IWGRP)
            printf("w");
        else
            printf("-");
        if(dirbuf.st_mode& S_IXGRP)
            printf("x");
        else
            printf("-");

        if(dirbuf.st_mode& S_IROTH)//文件其他人权限
            printf("r");
        else
            printf("-");
        if(dirbuf.st_mode& S_IWOTH)
            printf("w");
        else
            printf("-");
        if(dirbuf.st_mode& S_IXOTH)
            printf("x");
        else
            printf("-");


        printf(" %-5d",dirbuf.st_nlink);//链接数

        pad=getpwuid(dirbuf.st_uid);//这个函数通过用户ＩＤ找到用户信息赋给结构体
        printf(" %-5s",pad->pw_name);//名字在结构体里面
        grp=getgrgid(dirbuf.st_gid);//这个函数通过用户组ＩＤ找到用户组信息赋给结构体
        printf(" %-5s",grp->gr_name);//名字在结构体里面

        printf("%-6d",(int )dirbuf.st_size);

        strcpy(buftime,ctime(&dirbuf.st_mtime));//ctime()可以把时间转成字符串,返回值为字符串
        buftime[strlen(ctime(&dirbuf.st_mtime))-1]=0;//返回的字符串末尾是一个回车，这个操作就是为了取掉回车；
        printf(" %s",buftime);

        printf(" %s\n",rdir->d_name);

    }
    closedir(odir);
    return 0;
}
