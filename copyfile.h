#ifndef _COPYFILE_H
#define _COPYFILE_H

#include "head.h"
#include "threadpool.h"
#define maxname 1024

typedef struct copyfile
{
	char *srcfile; //源文件
	char *destfile; //目标文件

}* cpyfile_p, cpyfile_t;

int file_or_dir(struct stat *p, const char *src, const char *dest);
int files2files(threadpool_p threadpool, const char *argv1, const char *argv2);
int file2file(threadpool_p threadpool, const char *argv1,char *argv2);
void *copy_file(void *arg);


#endif
