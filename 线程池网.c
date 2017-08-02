#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>
#include <assert.h>
/*
*线程池里所有运行和等待的任务都是一个CThread_worker
*由于所有任务都在链表里，所以是一个链表结构
*/
typedef struct worker
{
    void *(*process) (void *arg);/*回调函数，任务运行时会调用此函数，注意也可声明成其它形式*/
    void *arg;/*回调函数的参数*/
    struct worker *next;
} CThread_worker;

/*线程池结构*/
typedef struct
{
    pthread_mutex_t queue_lock;
    pthread_cond_t queue_ready;
    CThread_worker *queue_head;/*链表结构，线程池中所有等待任务*/
    int shutdown;/*是否销毁线程池*/
    pthread_t *threadid;            //×线程指针，数组结构；
    int max_thread_num;/*线程池中允许的活动线程数目*/
    int cur_queue_size;/*当前等待队列的任务数目*/
} CThread_pool;

int pool_add_worker (void *(*process) (void *arg), void *arg);
void *thread_routine (void *arg);

static CThread_pool *pool = NULL;//×声明一个池吧

void pool_init (int max_thread_num)//×池的初始化+造线程；
{
    pool = (CThread_pool *) malloc (sizeof (CThread_pool));
    pthread_mutex_init (&(pool->queue_lock), NULL);//×初始化锁
    pthread_cond_init (&(pool->queue_ready), NULL);//×初始化型号量
    pool->queue_head = NULL;                        //×消息对列链表头
    pool->max_thread_num = max_thread_num;          //×线程最大数目
    pool->cur_queue_size = 0;                       //×当前等待队列的任务数目
    pool->shutdown = 0;
    pool->threadid =(pthread_t *) malloc (max_thread_num * sizeof (pthread_t));//×开线程最大数目的线程数组，用指针指向；

    int i = 0;
    for (i = 0; i < max_thread_num; i++)
    {
        pthread_create (&(pool->threadid[i]), NULL, thread_routine,NULL);//×开线程
    }
}
int pool_add_worker (void *(*process) (void *arg), void *arg)/*向线程池中加入任务*/
{
    CThread_worker *newworker=(CThread_worker *) malloc (sizeof (CThread_worker));/*构造一个新任务*/
    newworker->process = process;           //×任务具体函数
    newworker->arg = arg;                   //×那个函数的参数
    newworker->next = NULL;/*别忘置空*/
    pthread_mutex_lock (&(pool->queue_lock));//*锁住，不知道为什么还是那把锁

    CThread_worker *member = pool->queue_head;//×把消息队列头弄过来，给他加消息
    if (member != NULL)/*将任务加入到等待队列中，尾插*/
    {
        while (member->next != NULL)
            member = member->next;
        member->next = newworker;
    }
    else
    {
        pool->queue_head = newworker;
    }
    assert (pool->queue_head != NULL);
    pool->cur_queue_size++;                  //任务数加一
    pthread_mutex_unlock (&(pool->queue_lock));//解锁
    /*好了，等待队列中有任务了，唤醒一个等待线程；
    注意如果所有线程都在忙碌，这句没有任何作用*/
    pthread_cond_signal (&(pool->queue_ready));
    return 0;
}
/*销毁线程池，等待队列中的任务不会再被执行，但是正在运行的线程会一直
把任务运行完后再退出*/
int pool_destroy ()
{
    if (pool->shutdown)
        return -1;/*防止两次调用*/
    pool->shutdown = 1;
    /*唤醒所有等待线程，线程池要销毁了*/
    pthread_cond_broadcast (&(pool->queue_ready));
    /*阻塞等待线程退出，否则就成僵尸了*/
    int i;
    for (i = 0; i < pool->max_thread_num; i++)
        pthread_join (pool->threadid[i], NULL);//×通知他们退出，我感觉这里不用给所有线程通知，因为只有一个线程被卡在接收信号了，其他是卡在了锁哪里，锁打开后销毁标记变为1了，就不会进去，不会卡在接收信号那句，就直接进行消亡提示；
    free (pool->threadid);
    /*销毁等待队列*/
    CThread_worker *head = NULL;
    while (pool->queue_head != NULL)
    {
        head = pool->queue_head;
        pool->queue_head = pool->queue_head->next;
        free (head);
    }
    /*条件变量和互斥量也别忘了销毁*/
    pthread_mutex_destroy(&(pool->queue_lock));
    pthread_cond_destroy(&(pool->queue_ready));

    free (pool);
    /*销毁后指针置空是个好习惯*/
    pool=NULL;
    return 0;
}
void * thread_routine (void *arg)          //×线程函数
{
    printf ("starting thread %lu/n", pthread_self ());
    while (1)
    {
        pthread_mutex_lock (&(pool->queue_lock));//×这里是上锁了，问题是他是给一池子都上了吗？或者是给每个线程都上锁

        while (pool->cur_queue_size == 0 && !pool->shutdown)
        {
/*如果等待队列为0并且不销毁线程池，则处于阻塞状态; 注意
pthread_cond_wait是一个原子操作，等待前会解锁，唤醒后会加锁*/
            printf ("thread %lu is waiting/n", pthread_self ());
            pthread_cond_wait (&(pool->queue_ready), &(pool->queue_lock));
        }
        if (pool->shutdown)/*线程池要销毁了*/
        {
            /*遇到break,continue,return等跳转语句，千万不要忘记先解锁*/
            pthread_mutex_unlock (&(pool->queue_lock));
            printf ("thread %lu will exit/n", pthread_self ());
            pthread_exit (NULL);
        }
        printf ("thread %lu is starting to work/n", pthread_self ());
        /*assert是调试的好帮手*/
        assert (pool->cur_queue_size != 0);
        assert (pool->queue_head != NULL);

        /*等待队列长度减去1，并取出链表中的头元素*/
        pool->cur_queue_size--;                          //×长度减去一
        CThread_worker *worker = pool->queue_head;       //头赋给新声明的临时的
        pool->queue_head = worker->next;                 //头向后移一个；
        pthread_mutex_unlock (&(pool->queue_lock));      //解锁
        /*调用回调函数，执行任务*/
        (*(worker->process)) (worker->arg);
        free (worker);
        worker = NULL;
    }
    /*这一句应该是不可达的*/
    pthread_exit (NULL);
}

//下面是测试代码

void *myprocess (void *arg)
{
    printf ("threadid is %lu, working on task %d/n", pthread_self (),*(int *) arg);
    sleep (1);/*休息一秒，延长任务的执行时间*/
    return NULL;
}
int main (int argc, char **argv)
{
    pool_init (3);/*线程池中最多三个活动线程*/

    int *workingnum = (int *) malloc (sizeof (int) * 10);/*连续向池中投入10个任务*/
    int i;
    for (i = 0; i < 10; i++)
    {
        workingnum[i] = i;
        pool_add_worker (myprocess, &workingnum[i]);
    }
    /*等待所有任务完成*/
    sleep (5);
    /*销毁线程池*/
    pool_destroy ();
    free (workingnum);
    return 0;
}
