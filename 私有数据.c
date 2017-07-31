#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
int change0;
int change1;

pthread_key_t key;
int change2;
int chan()
{
    change2+=10;
}
void *thread1 ()
{
    int chang3=30;
    chang3+=2;
/*
    int change0=0;
    pthread_setspecific(key,(void *)change0);
    change0+=2;
*/
    pthread_setspecific(key,(void *)change2);
    chan();
    change2+=2;

    change1+=2;

    printf("1111111111 %d %d %d\n",change1,change0,chang3);

}
void *thread2(void)
{
    int change3=30;
    change3+=5;
/*
    int change0=0;
    pthread_setspecific(key,(void *)change0);
    change0+=5;
*/
    pthread_setspecific(key,(void *)change2);
    chan();
    change2+=5;

    change1+=5;

    printf("2222222222 %d %d %d\n",change1,change0,change3);
}
int main()
{
    change1=10;
    pthread_key_create(&key,NULL);

    pthread_t id1,id2;
    pthread_create(&id1,NULL,(void *)thread1,NULL);
    pthread_create(&id2,NULL,(void *)thread2,NULL);

    pthread_join(id1,NULL);
    pthread_join(id2,NULL);

}
