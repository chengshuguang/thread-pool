#include "thread_pool.h"
#include <stdio.h>

void *taskprocess(void *arg)
{
	printf("aaaaaaaaaaaaaaaaaaaaaaaaaa\n");
	usleep(1000);
	return NULL;
}


int main()
{
	thread_pool_init(5);
	int i;
	for(i=1; i<=10; i++)
	{
		thread_pool_add_task(taskprocess,(void *)i);
		usleep(1000);
	}
	sleep(1);
	thread_pool_destroy();
	return 0;
}
