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
int my_error(char *err,int line)
{
    printf("Line:%d \n",line);
    perror(err);
    exit (1);
}
int do_command()
{
   ;
}
int main()
{
    printf("#:");
    char long_command[256];
    scanf("%s",long_command);



    char *argv[3];
    argv[0]=long_command;
    //strcat(argv[0],"\n");
    argv[1]="m";
    argv[2]=NULL;
    int in_fd;
    int out_fd;
    if((in_fd=open("qwer.txt",O_RDONLY))==-1)
        my_error("qwer.txt",__LINE__);
    if((out_fd=open("qwout.txt",O_WRONLY|O_CREAT|O_TRUNC,0777))==-1)
       my_error("qwer.txt",__LINE__);
    pid_t pid;
    if((pid=fork())<0)
        my_error("fork",__LINE__);
    if(pid==0)
    {
        //dup2(in_fd,0);
       // dup2(out_fd,1);
        execvp(long_command,argv);
    }
    int blbl;
    wait(&blbl);
}
