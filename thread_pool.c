#include "thread_pool.h"
#include <pthread.h>
thread_pool *pool = NULL;
void * thread_pool_entrance(void *arg)
{
	int thread_id = (int)arg;
	printf("thread %d is created\n",thread_id);

	while(1)
	{
		pthread_mutex_lock(&(pool->queue_mutex));
		while(pool->task_queue_size == 0 && !pool->is_pool_destroyed)//必须用while，防止假唤醒
		{
			pthread_cond_wait(&(pool->queue_cond),&(pool->queue_mutex));//等待的时候会解锁，唤醒后加锁
		}

		if(pool->is_pool_destroyed)
		{
			printf("thread %d exit!!!\n",thread_id);
			pthread_mutex_unlock(&(pool->queue_mutex));//中途退出最容易出错，注意要解锁
			pthread_exit(NULL);
		}

		pool->idle_thread_num--;//线程进入忙碌状态
		//从任务队列中取出任务
		task *work;
		work = pool->task_queue_head;
		pool->task_queue_head = pool->task_queue_head->next;
		if(pool->task_queue_head == NULL)
			pool->task_queue_end = NULL;

		pool->task_queue_size--;

		pthread_mutex_unlock(&(pool->queue_mutex));

		//回调函数
		(*(work->taskfunc))(work->arg);
		pool->idle_thread_num++;//线程空闲
	}
	return NULL;
}


int thread_pool_init(int thread_pool_size)
{
	pool = (thread_pool *)malloc(sizeof(thread_pool));//不要最先给线程池分配空间

	pool->is_pool_destroyed = 0;

	pool->task_queue_head = NULL;
	pool->task_queue_end = NULL;
	pool->task_queue_size = 0;

	pool->thread_num = thread_pool_size;
	pool->thread_queue = (pthread_t *)malloc(thread_pool_size * sizeof(pthread_t));
	pool->idle_thread_num = thread_pool_size;

	//创建线程
	int i, ret;
	for(i=0; i<thread_pool_size; i++)
	{
		ret = pthread_create(&(pool->thread_queue[i]), NULL, thread_pool_entrance, (void *)i);
		if(ret < 0)
		{
			printf("thread create error!!!\n");
			thread_pool_destroy();//注意销毁，避免内存泄漏
			return -1;
		}
	}

	pthread_mutex_init(&(pool->queue_mutex), NULL);
	pthread_cond_init(&(pool->queue_cond), NULL);

	return 0;
}


typedef void *(*taskfunc)(void *arg);
int thread_pool_add_task(taskfunc func, void *arg)
{
	task *newtask;
	newtask = (task *)malloc(sizeof(task));
	newtask->taskfunc = func;
	newtask->arg = arg;
	newtask->next = NULL;

	pthread_mutex_lock(&(pool->queue_mutex));

	if(pool->task_queue_head == NULL)
	{
		pool->task_queue_head = pool->task_queue_end = newtask;
	}
	else
	{
		pool->task_queue_end = pool->task_queue_end->next = newtask;
	}
	pool->task_queue_size++;

	pthread_cond_signal(&(pool->queue_cond));
	pthread_mutex_unlock(&(pool->queue_mutex));

	return 0;
}


int thread_pool_destroy()
{
	if(pool->is_pool_destroyed)//防止多次销毁
		return -1;

	pool->is_pool_destroyed = 1;

	pthread_cond_broadcast(&(pool->queue_cond));//通知所有线程线程池销毁了
	int i;
	for(i=0; i<pool->thread_num; i++)//等待线程全部执行完
		pthread_join(pool->thread_queue[i], NULL);

	//销毁任务队列
	task *temp = NULL;
	while(pool->task_queue_head)
	{
		temp = pool->task_queue_head;
		pool->task_queue_head = pool->task_queue_head->next;
		free(temp);
	}
	//pool->task_queue_head = NULL；
	//pool->task_queue_end = NULL；

	//销毁线程队列
	free(pool->thread_queue);
	pool->thread_queue = NULL;

	pthread_mutex_destroy(&(pool->queue_mutex));
	pthread_cond_destroy(&(pool->queue_cond));

	free(pool);
	pool = NULL;

	return 0;
}

