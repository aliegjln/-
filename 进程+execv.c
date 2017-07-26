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
#include<sys/param.h>
int main(int argc,char *argv[])
{
    int fp;
    fp=open(argv[0],O_RDWR|O_CREAT|O_APPEND,0777);
    write(fp,"ghjd",sizeof(char)*5);
    close(fp);
    sleep(5);
    exit (0);
}
