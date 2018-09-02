#ifndef _THREADPOOL_H
#define _THREADPOOL_H

#include "head.h"
#define MINTHREADNUM 5
#define MAXTHREADNUM 1000
//线程池
typedef struct thread_pool
{
	//管理整片线程池的同步，互斥量，条件变量
	pthread_mutex_t pool_mutex;
	pthread_cond_t pool_cond;
	//管理线程池中的线程
	pthread_t *p_ids; //动态分配一片内存，存放线程id
	//线程数量
	int pthread_num;
	//管理任务队列（任务队列的头节点）
	struct job *head;
	//任务队列中的任务数量
	int job_num;
	bool thread_flag;
	bool destroy_flag; //销毁标志位
}* threadpool_p, threadpool_t;

//任务节点
typedef struct job
{
	//函数指针
	void *(*work)(void *arg);
	void * arg;
	//struct job *prev; //简单点先来个单向链表
	struct job *next;
}*job_p, job_t;

void *thread_fun(void *arg);
int signal2t(threadpool_p pool);
int thread_pool_init(threadpool_p pool, int threadnums);
int thread_pool_destroy(threadpool_p pool);
int process_start(threadpool_p p, int num);
int process_end(threadpool_p p);
int add_job_queue(threadpool_p p, job_p head, void *(*job)(void *arg),
		void *arg);
int AutoCreatThread(threadpool_p p);

#endif
