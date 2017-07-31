#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
int add(int a,int b)//两个进程都调用了一个函数怎么办，会不会有影响，如果不会是为什么？
{
    a++;
    sleep(2);
    return a+b;
}
void thread1(pthread_t *id2)
{
    int i;
    for(i=0;i<3;i++)
        printf("This is a pthread1111111.\n");
    printf("add1 %d\n",add(1,3));
    //printf("fanhui111:%d\n",pthread_join(*id2,NULL));
   // exit(0);//线程退出三种方式；
    pthread_exit(0);
}
void thread2(void)
{
    int i;
    for(i=0;i<3;i++)
        printf("This is a pthread2222222.\n");
    printf("add2 %d\n",add(4,5));
}
void thread3(void)
{
    int i;
    for(i=0;i<3;i++)
        printf("This is a pthread3333333.\n");
    pthread_t id3;
    id3=pthread_self();
    printf("fanhui33333:%d\n",pthread_join(id3,NULL));
}
int main(void)
{
    pthread_t id1,id2;
    int i,ret;

    ret=pthread_create(&id2,NULL,(void *) thread2,NULL);
    ret=pthread_create(&id1,NULL,(void *) thread1,&id2);
    if(ret!=0)
    {
        printf ("Create pthread error!\n");
        exit (1);
    }
    for(i=0;i<3;i++)
        printf("This is the main process.\n");
    printf("fanhui2:%d\n",pthread_join(id2,NULL));
    printf("fanhui1:%d\n",pthread_join(id1,NULL));
    /*
    线程有一个分离和结合状态：可结合的（joinable），或者是分离的（detached）即脱离同步；
    如果是joinable：线程在结束后会保留自己占用的系统资源，
    等待pthread_join函数调用。如果不调用就会一直占着资源，有一点像僵尸进程；
    如果是detached；线程结束就会归还自己占用的资源，不用等待pthread_join函数调用；
    1.自己
    */

    pthread_t id3;
    ret=pthread_create(&id3,NULL,(void *) thread3,NULL);
    printf("fanhui3:%d\n",pthread_join(id3,NULL));
    //pthread_exit(0);
    return 0;
    /*
    线程退出三种方式；
    1，return 0 ， 主线程退出，其他线程也停止，进程结束；其他线程退出，就只是他自己退出；
    2.exit 0 ：所有线程都退出，进程结束
    3.pthread_exit(0);所有线程自己退出，不影响其他线程；
    */
}
