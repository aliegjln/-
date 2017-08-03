#include<stdio.h>
#include<pthread.h>
#include<unistd.h>

int sum=0;
pthread_cond_t mcond;
pthread_mutex_t mlock;
void * thread1()
{
    pthread_mutex_lock(&mlock);
    printf("111 %lu %d\n",pthread_self(),sum);
    pthread_cond_wait(&mcond,&mlock);
    sum+=10;
    printf("111 %lu %d\n",pthread_self(),sum);
    pthread_mutex_unlock(&mlock);
}
void * thread2()
{

    pthread_mutex_lock(&mlock);
    sum+=20;
    printf("222 %lu %d\n",pthread_self(),sum);
    pthread_cond_signal(&mcond);
    pthread_mutex_unlock(&mlock);
}
int main()
{
    pthread_t pid1,pid2;
    pthread_mutex_init(&mlock,NULL);
    pthread_cond_init(&mcond,NULL);

    pthread_create(&pid1,NULL,thread1,NULL);
    pthread_create(&pid2,NULL,thread2,NULL);


    pthread_join(pid1,NULL);
    pthread_join(pid2,NULL);


}
