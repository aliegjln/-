#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

int num = 0;

void *sum(void * arg)
{
    for(int i=0; i<10000; i++) {
        num++;
    }
}

int main(int argc, char **argv)
{
    pthread_t tid;
    if(pthread_create(&tid, NULL, sum, NULL)) {
        printf("create thread error\n");
        exit(1);
    }
    for(int i=0; i<10000; i++) {
        num++;
    }
    pthread_join(tid, NULL);
    printf("%d\n", num);
    return 0;
}
