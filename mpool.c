#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>//锁
#include <unistd.h>
#define N 3
void * run_pthread();
typedef struct worker           //任务通用结构体（所谓通用是因为每个任务可能不同）
{
    void *(*work)(void *argv);  //线程执行的函数通用格式；
    void *argv;                 //传入的参数；
    struct worker * next;         //执行任务链表；
}pthread_worker;
typedef struct
{
    pthread_mutex_t queue_lock; //声明一个锁；
    pthread_cond_t queue_cond;  //声明一个条件变量；
    pthread_worker * head;      //声明一个任务链表头，看出任务是用链表实现；
    pthread_t *queue_pthread;   //声明一个线程指针，将来线程声明是连续的空间（动态数组），所以一个指针就可以用了；
    int destroy;                //是否销毁池子标记
    int max_pthread;            //最大线程数量；（也就是开的线程数量）；
    int sum_queue_work;         //任务对列中任务个数；
}pthread_pool;
pthread_pool *mpool=NULL;       //声明一个池子
int init_pool(void)                                //初始化池子；
{
    mpool=(pthread_pool *)malloc(sizeof(pthread_pool));         //开一个池子；
    pthread_mutex_init(&mpool->queue_lock,NULL);                //初始化锁；
    pthread_cond_init(&mpool->queue_cond,NULL);                 //初始化信号量‘
    mpool->head=NULL;                                           //任务队列头
    mpool->destroy=0;                                           //不销毁队列
    mpool->max_pthread=N;                                       //最大线程量为N
    mpool->sum_queue_work=0;                                    //任务数为零；
    mpool->queue_pthread=(pthread_t*)malloc(sizeof(pthread_t)*N);//开放线程的数组N个

    int i;
    for(i=0;i<N;i++)                                            //循环开线程，到线程工作函数；
    {
        pthread_create(&mpool->queue_pthread[i],NULL,run_pthread,NULL);
    }
}
void * run_pthread(void)                           //线程工作函数
{
    printf("线程:%lu 已建立\n",pthread_self());
    while(1)
    {
        pthread_mutex_lock(&mpool->queue_lock);
//1.如果没任务，且不摧毁池子，就让线程等待
        while(mpool->sum_queue_work==0&&mpool->destroy!=1)
        {
            printf("线程%lu 正在等待任务\n",pthread_self());
            pthread_cond_wait(&mpool->queue_cond,&mpool->queue_lock);
        }
//2。如果没任务（任务已经完了），要结束了要摧毁池子，就让线程退出
        if(mpool->destroy==1)
        {
            pthread_mutex_unlock (&mpool->queue_lock);  //遇到break,continue,return等跳转语句，千万不要忘记先解锁,这里千万要注意，要有这种意识，我自己写的时候就忘了
            printf("线程%lu 将被摧毁\n",pthread_self());
            pthread_exit (NULL);
        }
//3.以上条件都不满足，说明有任务可以执行就开始执行任务，执行任务首先需要把池子数据改一下；
        pthread_worker *timp_work=mpool->head;
        mpool->head=timp_work->next;
        mpool->sum_queue_work--;
//4.池子修改完了就可以解锁，然后执行刚刚那出来的任务了；
        pthread_mutex_unlock(&mpool->queue_lock);
        (*(timp_work->work))(timp_work->argv);          //执行读取的任务函数，注意这个指针形式
        free(timp_work);                                //释放掉之前的那个任务链表结点，timp指针指向空；这两步感觉特别体现细节，我自己只想到要释放，没想到制空，更加严谨吧；
        timp_work=NULL;
    }
    pthread_exit (NULL);                                //这一句应该是不可达的
}
void*add_work(void*(*job)(void *argv),void* arg)   //添加任务，即给任务链表加结点(注意参数，传入函数指针，给函数的参数)
{
//1.把传进来的任务赋给临时结点
    pthread_worker *timp_work=(pthread_worker *)malloc(sizeof(pthread_worker));
    timp_work->work=job;
    timp_work->argv=arg;
    timp_work->next=NULL;
//2.开始对池子进行操作，要锁上
    pthread_mutex_lock(&mpool->queue_lock);
    pthread_worker * timp_head=mpool->head;
    if(timp_head==NULL)
        mpool->head=timp_work;
    else
    {
        while(timp_head->next!=NULL)
            timp_head=timp_head->next;
        timp_head->next=timp_work;
    }
    mpool->sum_queue_work++;
printf("添加一个任务\n");
//3.对池子操作完啦，开锁，并通知子线程们活来了
    pthread_mutex_unlock(&mpool->queue_lock);
    pthread_cond_signal(&mpool->queue_cond);            //这里是如果有一个线程是在等待，则那个线程会被打开；如果线程都是在忙碌，那这个信号就没有意义；
    return 0;
}
void *destroy_pthread(void)                        //销毁池子
{
    if(mpool->destroy==1)//防止再次进入该函数，重复销毁；
        return 0;
    mpool->destroy=1;
    pthread_cond_broadcast(&mpool->queue_cond);         //前面置1后就可以唤醒所有线程，
    pthread_t *timp_queue_pthread=mpool->queue_pthread; //循环给线程等待结束；
    int i;
    for(i=0;i<N;i++)
        pthread_join(timp_queue_pthread[i],NULL);
    free(timp_queue_pthread);
    pthread_worker *timp_work=mpool->head;              //销毁任务队列，这里我感觉任务对列应该是空的
    while(mpool->head!=NULL)
    {
        timp_work=mpool->head;
        mpool->head=mpool->head->next;
        free(timp_work);
    }
    pthread_mutex_destroy(&(mpool->queue_lock));        //销毁锁，条件变量
    pthread_cond_destroy(&(mpool->queue_cond));
    free(mpool);                                        //销毁池子；
    mpool=NULL;
    return 0;
}
////////以上就是一个池子////////
////////下面我们实验一下////////
void * thisiswork()
{
    printf("这是线程%lu 在工作了\n",pthread_self());
    sleep(3);
    return NULL;                                        //返回值是一个指针，所以这里是NULL；
}
int main()
{
    init_pool();
    sleep(3);
    int i;
    for(i=0;i<10;i++)
    {
        add_work(thisiswork,NULL);
    }
    sleep(40);
    destroy_pthread();
    return 0;
}
