#ifndef _PTHREADPOOL_H_
#define _PTHREADPOOL_H_
#include <stdio.h>
#include <stdlib.h>
#include <windef.h>

typedef struct worker {
    void *(*process) (void *arg);
    void *arg;
    struct worker *next;
} CThread_worker;

typedef struct pool {
	HANDLE queue_lock;
	HANDLE queue_ready;
	HANDLE barrier;
    HANDLE *threadid;
    CThread_worker *queue_head;
    int shutdown;
    int max_thread_num;
    int task_num;
    int cur_queue_size;
} CThread_pool;

static CThread_pool *_pool = NULL;
extern int pool_add_worker(void *(*process) (void *arg), void *arg);
extern unsigned int __stdcall thread_routine(void *arg);
extern void pool_init(int max_thread_num, int task_num);
extern int pool_barrier();
extern int pool_destroy();

#endif
