#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <windows.h>
#include <process.h>
#include "pthreadpool.h"

void *myprocess(void *arg)
{
    Sleep(1000);
    printf("threadid is working on task %d\n",*(int *) arg);
    fflush(stdout);
    Sleep(1000);
    return NULL;
}

int main(int argc, char **argv)
{
    int thread_num = 3;
    int task_num = 10;
    pool_init(thread_num, task_num);

	Sleep(1000);

    int *workingnum = (int*)malloc(sizeof (int) * task_num);
    int i;
    for (i = 0; i < task_num; i++) {
        workingnum[i] = i;
        pool_add_worker(myprocess, &workingnum[i]);
    }
	pool_barrier();
    printf("work over\n");
    pool_destroy();
    free(workingnum);
    return 0;
}
