#include "ThreadPool.h"

namespace CCPool {
	void* Worker(void* _pool) {
		ThreadPool* Pool = (ThreadPool*)_pool;
		while (1) {
			pthread_mutex_lock(&Pool->TPoolIOLock);
			while (Pool->firstTask == NULL && 0 == Pool->PoolShutdown)
			{
				pthread_cond_wait(&Pool->TPoolNoEmpty, &Pool->TPoolIOLock);//If Pool cache is NULL Then Wait for taskadd
			}
			if (1 == Pool->PoolShutdown) {
				pthread_mutex_unlock(&Pool->TPoolIOLock);//If Pool Shutdown Exit The Task
				pthread_exit(NULL);
			}
			Task* work = Pool->firstTask;//Take out the Task From the
			Pool->firstTask = work->nextWork;
			Pool->pretaskNum -= 1;
			Pool->ontaskNum += 1;
			pthread_mutex_unlock(&Pool->TPoolIOLock);
			work->Fuc(work->args);
			delete work;
			pthread_mutex_lock(&Pool->TPoolIOLock);
			Pool->ontaskNum -= 1;
			if (Pool->ontaskNum + Pool->pretaskNum == 0) {
				pthread_cond_signal(&Pool->PoolSync);
			}
			pthread_mutex_unlock(&Pool->TPoolIOLock);
		}
		return NULL;
	}
	ThreadPool* PoolInit(int Num) {
		if (Num > 16384) {
			return NULL;
		}
		ThreadPool* Pool = new ThreadPool;
		
		if (pthread_mutex_init(&Pool->TPoolIOLock, NULL) != 0) {
			delete Pool;
			return NULL;
		}
		if (pthread_cond_init(&Pool->TPoolNoEmpty, NULL) != 0) {
			delete Pool;
			pthread_mutex_destroy(&Pool->TPoolIOLock);
			return NULL;
		}
		if (pthread_cond_init(&Pool->PoolSync, NULL) != 0) {
			delete Pool;
			pthread_mutex_destroy(&Pool->TPoolIOLock);
			pthread_cond_destroy(&Pool->TPoolNoEmpty);
			return NULL;
		}
		Pool->ThreadNum = Num;
		Pool->ontaskNum = 0;
		Pool->pretaskNum = 0;
		for (int i = 0; i < Num; i++) {
			pthread_t tid;
			if (pthread_create(&tid, NULL, Worker, Pool) != 0) {
				PoolDestory(Pool);
				return NULL;
			}
		}
		return Pool;
	}
	int PoolSync(ThreadPool* Pool) {
		if (Pool == NULL) {
			return -1;
		}
		pthread_mutex_lock(&Pool->TPoolIOLock);
		while (Pool->ontaskNum + Pool->pretaskNum != 0) {
			pthread_cond_wait(&Pool->PoolSync, &Pool->TPoolIOLock);
		}
		pthread_mutex_unlock(&Pool->TPoolIOLock);
		return 0;
	}
	int PoolDestory(ThreadPool* Pool) {
		if (Pool == NULL) {
			return -1;
		}
		Pool->PoolShutdown = 1;
		pthread_cond_broadcast(&Pool->TPoolNoEmpty);
		pthread_cond_destroy(&Pool->PoolSync);
		pthread_cond_destroy(&Pool->TPoolNoEmpty);
		pthread_mutex_destroy(&Pool->TPoolIOLock);
		delete Pool;
		return 1;
	}
	int PoolAdd(ThreadPool* Pool, void (*Fuc)(void*), void* args) {
		if (Pool == NULL) {
			return -1;
		}
		Task* work = new Task;
		if (work == NULL) {
			return -1;
		}
		work->args = args;
		work->Fuc = Fuc;
		work->nextWork = NULL;
		pthread_mutex_lock(&Pool->TPoolIOLock);
		if (Pool->pretaskNum == 0) {
			Pool->firstTask = Pool->lastTask = work;
		}
		else
		{
			Pool->lastTask->nextWork = work;
			Pool->lastTask = work;
		}
		Pool->pretaskNum += 1;
		pthread_mutex_unlock(&Pool->TPoolIOLock);
		pthread_cond_signal(&Pool->TPoolNoEmpty);
		return 1;
	}

}

