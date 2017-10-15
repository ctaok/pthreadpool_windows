#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <windows.h>
#include <process.h>
#include "pthreadpool.h"

int pool_add_worker(void *(*process) (void *arg), void *arg)
{
    CThread_worker *newworker = (CThread_worker *)malloc(sizeof (CThread_worker));
    newworker->process = process;
    newworker->arg = arg;
    newworker->next = NULL;
	WaitForSingleObject(_pool->queue_lock, INFINITE);
    CThread_worker *member = _pool->queue_head;
    if (member != NULL) {
        while (member->next != NULL)
            member = member->next;
        member->next = newworker;
    } else {
        _pool->queue_head = newworker;
    }
    assert (_pool->queue_head != NULL);
    _pool->cur_queue_size++;
	ReleaseMutex(_pool->queue_lock);
	ReleaseSemaphore(_pool->queue_ready, 1, NULL);
    return 0;
}

unsigned int __stdcall thread_routine(void *arg)
{
    printf("starting thread\n");
    fflush(stdout);
    while (1) {
        int set_barrier = 0;
		WaitForSingleObject(_pool->queue_lock, INFINITE);
        while (_pool->cur_queue_size == 0 && !_pool->shutdown) {
            printf("thread is waiting\n");
            fflush(stdout);
			ReleaseMutex(_pool->queue_lock);
			WaitForSingleObject(_pool->queue_ready, INFINITE);
			WaitForSingleObject(_pool->queue_lock, INFINITE);
        }
		printf("xx2\n");
		fflush(stdout);
        if (_pool->shutdown) {
			ReleaseMutex(_pool->queue_lock);
            printf("thread will exit\n");
            fflush(stdout);
			return 0;
        }
        printf("thread is starting to work\n");
        fflush(stdout);
        assert(_pool->cur_queue_size != 0);
        assert(_pool->queue_head != NULL);
        if (_pool->task_num == 1) {
            printf("%dvs%d\n", _pool->task_num, _pool->max_thread_num);
            fflush(stdout);
            set_barrier = 1;
        }
        _pool->cur_queue_size--;
        _pool->task_num--;
        CThread_worker *worker = _pool->queue_head;
        _pool->queue_head = worker->next;
		ReleaseMutex(_pool->queue_lock);
        (*(worker->process)) (worker->arg);
        free (worker);
        worker = NULL;
        if (set_barrier) {
            set_barrier = 0;
            printf("thread wait for barrier\n");
			SetEvent(_pool->barrier);
        }
    }
}

void pool_init(int max_thread_num, int task_num)
{
    int i = 0;
    _pool = (CThread_pool *)malloc(sizeof (CThread_pool));
	_pool->queue_lock = CreateMutex(nullptr, FALSE, nullptr);
	_pool->queue_ready = CreateSemaphore(NULL, 1, 1, NULL);
    _pool->queue_head = NULL;
	_pool->barrier = CreateEvent(NULL, FALSE, FALSE, NULL);
    _pool->max_thread_num = max_thread_num;
    _pool->task_num = task_num;
    _pool->cur_queue_size = 0;
    _pool->shutdown = 0;
    _pool->threadid = (HANDLE *)malloc(max_thread_num * sizeof (HANDLE));
    for (i = 0; i < max_thread_num; i++) {
		_pool->threadid[i] = (HANDLE)_beginthreadex(NULL, 0, thread_routine, NULL, 0, NULL);
    }
}

int pool_barrier()
{
	WaitForSingleObject(_pool->barrier, INFINITE);
	return 0;
}

int pool_destroy()
{
    int i;
    if (_pool->shutdown)
        return -1;
    _pool->shutdown = 1;
    Sleep(1000);
	for (i = 0; i < _pool->max_thread_num; i++) {
		ReleaseSemaphore(_pool->queue_ready, 1, NULL);
	}

	WaitForMultipleObjects(_pool->max_thread_num, _pool->threadid, TRUE, INFINITE);
    free(_pool->threadid);
    CThread_worker *head = NULL;
    while (_pool->queue_head != NULL) {
        head = _pool->queue_head;
        _pool->queue_head = _pool->queue_head->next;
        free(head);
    }
	CloseHandle(_pool->queue_lock);
	CloseHandle(_pool->queue_ready);
    free(_pool);
    _pool = NULL;
    return 0;
}
