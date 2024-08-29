#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<pthread.h>

namespace CCPool {
	typedef struct _Task
	{
		void* args;
		void (*Fuc)(void*);
		struct _Task* nextWork;
	}Task;
	typedef struct _ThreadPool
	{
		int ThreadNum;
		pthread_mutex_t TPoolIOLock;
		pthread_cond_t TPoolNoEmpty;
		pthread_cond_t PoolSync;
		Task* firstTask;
		Task* lastTask;
		int pretaskNum;
		int ontaskNum;
		int PoolShutdown;
		
	}ThreadPool;
	ThreadPool* PoolInit(int);// init pool
	int PoolDestory(ThreadPool*); //Destory thread pool
	int PoolAdd(ThreadPool*,void (void*),void *args); //add a task to Thread pool
	int PoolSync(ThreadPool*);//wait until the all add task is done
}

