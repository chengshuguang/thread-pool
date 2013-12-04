#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

typedef struct task
{
	void *(*taskfunc)(void *arg);//声明一个函数指针
	void *arg;//函数的参数
	struct task *next;
}task;

typedef struct thread_pool
{
	task *task_queue_head;//任务队列
	task *task_queue_end;//指向任务队列结尾
	int task_queue_size;

	pthread_t *thread_queue;//线程队列
	int thread_num;
	int idle_thread_num;//空闲线程数

	int is_pool_destroyed;

	pthread_mutex_t queue_mutex;//用来互斥访问任务队列
	pthread_cond_t queue_cond;
}thread_pool;


#ifdef __cplusplus
extern "C"{
#endif

extern thread_pool *pool;
extern int thread_pool_init(int thread_pool_size);
//extern void * thread_pool_entrance(void *arg);
extern int thread_pool_add_task(void *(*taskfunc)(void *arg), void *arg);
extern int thread_pool_destroy();

#ifdef __cplusplus
}
#endif

#endif //THREAD_POOL_H
