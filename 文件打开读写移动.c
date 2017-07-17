#include<stdio.h>
#include<sys/types.h>//read open ...等
#include<sys/stat.h>
#include<fcntl.h>
#include<unistd.h>//lseek
#include<errno.h>//errno全局变量
int main()
{
    int len,set;
    char b[100];
    //int a=100;
    char a[]="gjldw";
    int p1=open("sdfd.txt",O_RDWR|O_CREAT|O_EXCL,511);//ｏｐｅｎ打开方式
    if(p1==-1)
    {
        perror("sdfd.txt");//这个函数可以自动找出错误提示，此错误原因依照全局变量errno 的值来决定要输出的字符串。在库函数中有个errno变量，每个errno值对应着以字符串表示的错误类型。
        printf("%s ",strerror(errno));//这两句和上面那个效果一样,errno错误的编号
        printf("%d ",errno);
        fprintf(stderr,"%d ",__LINE__);//__LINE__这个是内置宏，表示在源文件的行数行数(记得是两个下划线)
        exit(1);
/*此处退出结果
sdfd.txt: File exists
File exists 17 24
*/
    }

    write(p1,&a,sizeof(a));//写函数，返回值为写入的字节数
    len=lseek(p1,0,SEEK_END);//lseek,返回值为移动后指针距离文件开头的字节数
    printf("len:%d ",len);//（把文件已到尾返回值赋给len就可以知道文件的长度）
    printf("%d ",lseek(p1,0,SEEK_SET));//将指针移动到开头
    printf("%d ",read(p1,b,len));//返回值为读取的字节数（已经知道前面文件长度了，用个了就好）
    /*if(read(p1,b,len)!=len)//如果读的长度和返回值不一样就说名读错了
    {
        perror("read");
    }*/
    printf("%s ",b);//输出一次文件里读的东西
    lseek(p1,10,SEEK_END);//这里就有点奇怪了　文件指针移动到结尾的后面十个字节，这时相当与给文件变长了，末尾加了十个‘\0’,
    write(p1,&a,sizeof(a));//又写入了了一次（这是在在文件末尾添加了一次，即十个‘\0'之后）
    len=lseek(p1,0,SEEK_END);//再次看一下长度，以及下面的对比一下
    printf("%d ",len);
    printf("%d ",lseek(p1,0,SEEK_SET));
    printf("%d ",read(p1,b,len));
    printf("%s ",b);
    close(p1);
    return 0;
}
/*len:6 0 6 gjldw 22 0 22 gjldw
gjln@gjldw:~/gjl/练习/练习$ od -c sdfd.txt
0000000   g   j   l   d   w  \0  \0  \0  \0  \0  \0  \0  \0  \0  \0  \0
0000020   g   j   l   d   w  \0
0000026
参数表
*/
/*宏跟踪调试
ANSI标准说明了五个预定义的宏名。它们是：
__LINE__
__FILE__
__DATE__
__TIME__
__STDC__

如果编译不是标准的，则可能仅支持以上宏名中的几个，或根本不支持。记住编译程序 也许还提供其它预定义的宏名。
是行连接符，会将下一行和前一行连接成为一行，即将物理上的两行连接成逻辑上的一行
__FILE__ 是内置宏 代表源文件的文件名
__LINE__ 是内置宏，代表该行代码的所在行号
__DATE__宏指令含有形式为月/日/年的串，表示源文件被翻译到代码时的日期。
源代码翻译到目标代码的时间作为串包含在__TIME__ 中。串形式为时：分：秒。
如果实现是标准的，则宏__STDC__含有十进制常量1。如果它含有任何其它数，则实现是非标准的。
*/


