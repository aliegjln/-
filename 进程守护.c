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
int main()
{
    //0.屏蔽一些有关控制终端操作的信号。这是为了防止在守护进程没有正常运转起来时，控制终端受到干扰退出或挂起。
    signal(SIGTTOU,SIG_IGN);
    signal(SIGTTIN,SIG_IGN);
    signal(SIGTSTP,SIG_IGN);
    signal(SIGHUP ,SIG_IGN);
    pid_t pid;
    int i;
    pid=fork();//1,产生一个子进程，让父进程退出
    /*
    创建一个进程后，
        父进程没有等待(调用wait / waitpid)子，
            如果子结束了，那子就会变成僵尸进程
    （僵尸进程虽然释放了内存，但是保留有一定的信息
    （包括进程号，退出状态，退出时间等，）
    其进程号就会一直被占用，但是系统所能使用的进程号是有限的，如果大量的产生僵死进程，将因为没有可用的进程号而导致系统不能产生新的进程.。）
        但如果父进程等待了，父进程就会变为停止状态，影响进程并发性；
        所以就会两难，解决办法在下面（处理SIGCHLD信号，通知内核，父不接收对子进程的结束信号）
            但如果父进程提前结束那么子进程就会变成孤儿进程，他会被init进程收养，（这里有个坑，在命令行下是init,但是在图形界面模式是upstart(upstart是Ubuntu使用的用来代替init的东西,它的优点在于更快的启动系统,以及在硬件热拔插的时候启动或者停止相关服务.)）
    */
    if(pid==-1)
    {
        perror("process creation failed\n");
        exit (0);
    }
    if(pid>0)
    {
        exit(0);
    }
    setsid();//2.创建一个新的会话，并且担任该会话组的组长（会话组长调用这个函数会失败，但是这时进程是刚刚建立的，不可能是会话组长）
    /*
    子继承了父所有的会话信息，进程组信息，和终端还有牵扯
    要让子进程成为新的会话组长，进程组长，
    摆脱原会话的控制，让进程摆脱原进程的控制，
    会话对终端的独占性，进程摆脱原控制终端的控制。
    */
    pid=fork();//3.再产生一个子进程
    /*
    这时子进程脱离终端了，是无终端的会话组长了
    但是组长还可以重新申请打开一个终端；
    所以再fock一个子进程，这个子进程一定不是会话组长
    关闭父进程，则这个进程一定不会被终端控制
    */
    if(pid==-1)
    {
        perror("process creation failed\n");
        exit (0);
    }
    if(pid>0)
        exit(0);
    for(i=0;i<NOFILE;close(i++));//4.进程从创建它的父进程那里继承了打开的文件描述符。如不关闭，将会浪费系统资源，造成进程所在的文件系统无法卸下以及引起无法预料的错误。按如下方法关闭它们(NOFILE允许的最大文件描述符)：
    chdir("./");//5.进程活动时，其工作目录所在的文件系统不能卸下。一般需要将工作目录改变到根目录。对于需要转储核心，写运行日志的进程将工作目录改变到特定目录如
    umask(0);//6.进程从创建它的父进程那里继承了文件创建屏蔽字。它可能修改守护进程所创建的文件的存取位。为防止这一点，将文件屏蔽字清楚：
    signal(SIGCHLD,SIG_IGN);//7.这里就是上面讲的（处理SIGCHLD信号，通知内核，父不接收对子进程的结束信号）
    //------------------守护进程-------OVER------------------------//


    char *argv[2];
    argv[0]="qwer.txt";
    argv[1]=NULL;
    while(1)//8.守护进程检测（我在这里是将一个字符串不断地追加在一个文件里，如果成功就可以通过文件来看到）
    {
        pid=fork();
        if(pid==-1)
        {
            exit(1);
        }
        if(pid==0)
        {

            signal(SIGCHLD,SIG_IGN);//实验一：让父进程不管子，一直疯狂的造孩子。。。。
            execv("asd",argv);//执行新程序，将之前的那块内存完全换成我调用的新的程序（记住，是是直接换程序，如果那个程序结束了，那就直接退出了不会返回来），但是ID什么的不变；
            /*
            execv(可执行文件名，传给下一个程序的argv字符串数组）（注意必须要以NULL结束）；
            */
        }
        /*if(pid>0)
        {
            int stat_val;//子进程结束的状态码；
            pid_t child_pid;//
            child_pid=wait(&stat_val);//让父进程等待子进程（不然的话我这里是死循环，一直在造子进程，多个子进程对同一个文件进行写，会不会出现奇奇怪怪的问题呢？）
            if(WIFEXITED(stat_val));//获取子进程返回停止状态宏函数；
        }//实验一：让父进程不管子，一直疯狂的造孩子。。。。
        */
        sleep(10);
    }
    return 0;
}
