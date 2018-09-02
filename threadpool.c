#include "threadpool.h"

//线程任务
void *thread_fun(void *arg)
{
	threadpool_p tpool = (threadpool_p) arg;
	if (tpool->pthread_num < 3)
		tpool->pthread_num++;

	//获取所资源（因为需要访问任务队列，任务队列属于共享资源）
	//判断任务队列是否有任务
	while (1)
	{
		pthread_mutex_lock(&(tpool->pool_mutex));
		while (tpool->job_num == 0 && tpool->destroy_flag == false)
		{
			//等待条件变量获取任务
			pthread_cond_wait(&tpool->pool_cond, &tpool->pool_mutex);

		}
		//如果任务队列发来条件变量
		//获取任务队列
		if (tpool->destroy_flag == true && tpool->job_num == 0)
		{
			pthread_mutex_unlock(&(tpool->pool_mutex));
			//tpool->pthread_num--;
			pthread_exit(NULL);
		}
		job_p job = tpool->head->next;
		tpool->head->next = tpool->head->next->next;
		tpool->job_num--;
		//解锁互斥量
		pthread_mutex_unlock(&(tpool->pool_mutex));
		//执行任务
		(*job->work)(job->arg);

		free(job);
		//返回上面继续判断任务队列是否有任务
	}
	return (void*) 0;

}

int signal2t(threadpool_p pool)
{
	int err;
	err = pthread_cond_signal(&pool->pool_cond);
	if (err != 0)
	{
		perror("signal failed");
		return -1;
	}
	return 0;
}

int add_job_queue(threadpool_p p, job_p head, void *(*job)(void *arg),
		void *arg)
{
	int err;
	job_p mession = calloc(1, sizeof(job_t));
	if (mession == NULL)
	{
		perror("add job failed");
		return -1;
	}
	mession->work = job;
	mession->arg = arg;
	mession->next = NULL;
	pthread_mutex_lock(&p->pool_mutex);
	//任务队列没有节点
	if (head->next == NULL)
	{
		head->next = mession;
	}
	else if (head->next != NULL) //任务队列中存在节点
	{
		job_p loop = head;
		while (loop->next != NULL)
		{
			loop = loop->next;
		}
		loop->next = mession;
	}
	p->job_num++;
	pthread_mutex_unlock(&p->pool_mutex);
	err = pthread_cond_signal(&p->pool_cond);
	if (err != 0)
	{
		perror("job queue signal failed  ");
		return -1;
	}

	return 0;
}
int thread_pool_init(threadpool_p pool, int threadnums)
{
	int err;

	err = pthread_cond_init(&pool->pool_cond, NULL);
	if (err != 0)
	{
		perror("init cond failed ");
		return -1;
	}

	err = pthread_mutex_init(&pool->pool_mutex, NULL);
	if (err != 0)
	{
		perror("init mutex failed ");
		return -1;
	}
	pool->pthread_num = 0;
	pool->job_num = 0;
	pool->destroy_flag = false;
	pool->thread_flag = true;
	//线程id队列
	pool->p_ids = malloc(sizeof(pthread_t) * threadnums);
	//任务队列头节点初始化
	pool->head = calloc(1, sizeof(job_t));
	pool->head->work = NULL;
	pool->head->arg = NULL;
	pool->head->next = NULL;

	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	int i;
	for (i = 0; i < threadnums; i++)
	{
		err = pthread_create(pool->p_ids + i, NULL, thread_fun, pool);
		if (err != 0)
		{
			perror("create pthread failed");
			return -1;
		}
	}
	pthread_attr_destroy(&attr);

	return 0;
}

//销毁线程池
int thread_pool_destroy(threadpool_p pool)
{
	int err;
	int i;
	for (i = 0; i < pool->pthread_num; i++)
	{
		err = pthread_cancel(pool->p_ids[i]);
		if (err != 0)
		{
			perror("cacel failed");
			return -1;
		}
		pthread_join(pool->p_ids[i], NULL);
	}
	free(pool->head);
	free(pool->p_ids);
	free(pool);
	printf("system clean up ! \n");
	return 0;
}

//等待线程创建成功并处于wait条件变量的状态
int process_start(threadpool_p p, int num)
{
	while (1)
	{
		pthread_mutex_lock(&p->pool_mutex);
		if (p->pthread_num == num)
		{
			pthread_mutex_unlock(&p->pool_mutex);
			break;
		}
		pthread_mutex_unlock(&p->pool_mutex);
	}
	return 0;
}

int process_end(threadpool_p p)
{
	while (1)
	{
		pthread_mutex_lock(&p->pool_mutex);
		if (p->job_num == 0)
		{
			p->destroy_flag = true;
			pthread_cond_broadcast(&p->pool_cond);
			pthread_mutex_unlock(&p->pool_mutex);
			break;
		}
		pthread_mutex_unlock(&p->pool_mutex);
	}
	int i;
	for (i = 0; i < p->pthread_num; i++)
	{
		pthread_join(p->p_ids[i], NULL);
	}
	printf("thread nums is %d \n", p->pthread_num);
	free(p->p_ids);
	free(p->head);

	free(p);
	printf("process clean up ! \n");
	return 0;
}

int AutoCreatThread(threadpool_p p) //动态产生线程
{
	if (p->job_num > p->pthread_num)
	{
		if (p->pthread_num < MAXTHREADNUM)
		{
			if (p->thread_flag == true)
			{
				p->p_ids = realloc(p->p_ids,
						sizeof(pthread_t) * (p->pthread_num + 2));

				int err = pthread_create(p->p_ids + (p->pthread_num + 1), NULL,
						thread_fun, p);
				if (err != 0)
				{
					p->p_ids = realloc(p->p_ids,
							sizeof(pthread_t) * (p->pthread_num));
					p->thread_flag = false;
					perror("create pthread failed");
					return -1;
				}
				p->pthread_num++;
			}
		}
	}
	return 0;
}
