#include "head.h"
#include "threadpool.h"
#include "copyfile.h"

int main(int argc, char *argv[])
{
	int res;
	struct stat fbuf;

	threadpool_p threadpool = calloc(1, sizeof(threadpool_t));
	if (threadpool == NULL)
	{
		perror("threadpool calloc failed ");
		return -1;
	}

	//stat看参数一
	res = stat(argv[1], &fbuf);
	if (res != 0)
	{
		perror("stat argv failed");
		exit(0);
	}

	//初始化线程池
	res = thread_pool_init(threadpool, 1);
	if (res != 0)
	{
		perror("thread pool init failed");
		return -1;
	}

	//等待线程创建完毕，进入挂起状态
	process_start(threadpool, 1);

	//判断是拷贝目录还是拷贝文件
	if (!file_or_dir(&fbuf, argv[1], argv[2]))
	{
		files2files(threadpool, argv[1], argv[2]); //拷贝文件夹to文件夹
	}
	else
	{
		file2file(threadpool, argv[1], argv[2]); //拷贝文件to文件
		//等待线程结束
		process_end(threadpool);
		return 0;
	}

	process_end(threadpool);
	//等待线程执行

	return 0;
}
