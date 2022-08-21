#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdatomic.h>
#include <signal.h>
#include <unistd.h>
#include <sched.h>

atomic_int flag [2] = {0,0};
atomic_int turn = 0;
int p0_in_cs = 0;
int p1_in_cs = 0;
int in_cs = 0;
int cpu_p0, cpu_p1;

void per_second(int signum)
{
    static int p0_pre, p1_pre;
    printf("進入次數（每秒）p0: %5d, p1: %5d", p0_in_cs - p0_pre, p1_in_cs - p1_pre);
    printf("，分別執行於 core#%d 及 core#%d\n", cpu_p0, cpu_p1);   
    p0_pre = p0_in_cs;
    p1_pre = p1_in_cs;

    alarm(1);
}

void proc0()
{
    printf("start p0\n");
    while(1)
    {
        atomic_store(&flag[0],1);

        atomic_thread_fence(memory_order_seq_cst);

        atomic_store_explicit(&turn,1,memory_order_release);

        while(atomic_load(&flag[1]) && atomic_load(&turn)==1);;

        //底下程式碼用於模擬在critical section
        cpu_p0 = sched_getcpu();
        in_cs++;	//計算有多少人在CS中
        //nanosleep(&ts, NULL);
        if (in_cs == 2) fprintf(stderr, "p0及p1都在critical section\n");
        p0_in_cs++;	//P0在CS幾次
        //nanosleep(&ts, NULL);
        in_cs--;	//計算有多少人在CS中

        atomic_store(&flag[0],0);
    }
}

void proc1()
{
    printf("start p1\n");
    while(1)
    {
        atomic_store(&flag[1],1);

        atomic_thread_fence(memory_order_seq_cst);

        atomic_store_explicit(&turn,0,memory_order_release);

        while(atomic_load(&flag[0]) && atomic_load(&turn)==0);;

        //底下程式碼用於模擬在critical section
        cpu_p1 = sched_getcpu();
        in_cs++;	//計算有多少人在CS中
        //nanosleep(&ts, NULL);
        if (in_cs == 2) fprintf(stderr, "p0及p1都在critical section\n");
        p1_in_cs++;	//P0在CS幾次
        //nanosleep(&ts, NULL);
        in_cs--;	//計算有多少人在CS中

        atomic_store(&flag[1],0);
    }
}

int main()
{
    pthread_t p0, p1;
    alarm(1);
    signal(SIGALRM,per_second);
    pthread_create(&p0,NULL,(void*)proc0,NULL);
    pthread_create(&p1,NULL,(void*)proc1,NULL);

    pthread_join(p0,NULL);
    pthread_join(p1,NULL);
    return 0;
}