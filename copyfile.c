#include "copyfile.h"

//判断是文件还是目录
int file_or_dir(struct stat *p, const char *src, const char *dest)
{
	if (S_ISDIR(p->st_mode))
	{
		if (access(dest, 0) == -1) //access函数是查看文件是不是存在
		{
			if (mkdir(dest, 0777)) //如果不存在就用mkdir函数来创建
			{
				perror("mkir dest dir failed");
			}
		}
		return 0;
	}
	return -1;
}

//拷贝文件夹
int files2files(threadpool_p threadpool, const char *argv1, const char *argv2)
{
	//获取目录信息
	int res;
	struct stat buf;
	struct dirent * dr;
	DIR *dirp = opendir(argv1);
	//读取文件名
	while ((dr = readdir(dirp)) != NULL)
	{

		if (strncmp(dr->d_name, ".", 1) == 0)
		{
			if (strncmp(dr->d_name, "..", 2) == 0)
				continue;
			if (strlen(dr->d_name) == 1)
				continue;
		}
		char chdir[maxname];
		sprintf(chdir, "%s/%s", argv1, dr->d_name);
		res = stat(chdir, &buf);
		if (res != 0)
		{
			puts(chdir);
			perror("line 400 stat failed");
		}

		if (S_ISDIR(buf.st_mode))
		{
			bzero(chdir, maxname);
			if (argv1[strlen(argv1) - 1] == '/')
				sprintf(chdir, "%s/%s", argv1, dr->d_name);

			if (argv1[strlen(argv1) - 1] != '/')
				sprintf(chdir, "%s/%s", argv1, dr->d_name);
			char changdir[maxname];
			if (argv2[strlen(argv2) - 1] != '/')
				sprintf(changdir, "%s/%s", argv2, dr->d_name);
			if (argv2[strlen(argv2) - 1] == '/')
				sprintf(changdir, "%s%s", argv2, dr->d_name);

			if (access(changdir, 0) == -1) //access函数是查看文件是不是存在
			{

				if (mkdir(changdir, 0777)) //如果不存在就用mkdir函数来创建
				{
					printf("[LINE %d] \n", __LINE__);
					perror("mkir dest dir failed");
				}

			}

			sprintf(changdir, "%s/", changdir);
			files2files(threadpool, chdir, changdir);

			continue;
		}

		cpyfile_p cpy = calloc(1, sizeof(cpyfile_t));
		cpy->srcfile = calloc(1, maxname);
		cpy->destfile = calloc(1, maxname);

		//拼接文件名+路径
		if (argv1[strlen(argv1) - 1] == '/')
			sprintf(cpy->srcfile, "%s%s", argv1, dr->d_name);
		if (argv1[strlen(argv1) - 1] != '/')
			sprintf(cpy->srcfile, "%s/%s", argv1, dr->d_name);

		if (argv2[strlen(argv1) - 1] == '/')
			sprintf(cpy->destfile, "%s%s", argv2, dr->d_name);
		if (argv2[strlen(argv1) - 1] != '/')
			sprintf(cpy->destfile, "%s/%s", argv2, dr->d_name);

		add_job_queue(threadpool, threadpool->head, copy_file, (void *) cpy);
		//添加拷贝文件任务节点
		AutoCreatThread(threadpool);
	}

	closedir(dirp);
	return 0;
}

int file2file(threadpool_p threadpool, const char *argv1, char *argv2)
{

	int i, j; //循环数
	char file[maxname], change[maxname] =
	{ 0 };
	if (argv2 == NULL)
	{
		printf("请输入目标文件，或者路径 \n");
		exit(0);
	}
	if (argv2[strlen(argv2) - 1] == '/')
	{
		if (access(argv2, 0) == -1) //access函数是查看文件是不是存在
		{
			//添加相对路径转绝对路径
			if (mkdir(argv2, 0777)) //如果不存在就用mkdir函数来创建
			{
				printf("[LINE %d] arg : %s \n", __LINE__, argv2);
				perror("mkdir dest dir failed");
			}
		}

		for (i = 0, j = strlen(argv1);; j--, i++) //把文件名扣下
		{
			if (argv1[j - 1] == '/' || j == 0)
				break;
			file[i] = argv1[j - 1];
		}
		sprintf(change, "%s", argv2);
		for (j = strlen(change); i >= 0; i--, j++)
		{
			change[j] = file[i - 1];
		}
		sprintf(argv2, "%s", change);

	}
	puts(argv2);
	cpyfile_p cpy = calloc(1, sizeof(cpyfile_t));
	cpy->srcfile = calloc(1, maxname);
	cpy->destfile = calloc(1, maxname);
	sprintf(cpy->srcfile, "%s", argv1);
	sprintf(cpy->destfile, "%s", argv2);

	add_job_queue(threadpool, threadpool->head, copy_file, (void *) cpy);
	return 0;
}

//拷贝文件
void *copy_file(void *arg)
{
	cpyfile_p cpyfile = (cpyfile_t *) arg;
	const char *srcf = cpyfile->srcfile;
	const char *destf = cpyfile->destfile;
	int i, j;
	int src, dest;
	char* buf = calloc(1, maxname);
	char *file = calloc(1, maxname);
	if (!access(srcf, F_OK))
	{

		src = open(srcf, O_RDWR);
		if (src == -1)
		{
			printf("[LINE %d] : %s\n", __LINE__, srcf);
			perror("open src file failed ");
			exit(0);
		}

	}
	if (destf != NULL)
	{
		struct stat fbuf;
		stat(destf, &fbuf);
		if (S_ISDIR(fbuf.st_mode))
		{
			if (access(destf, 0) == -1) //access函数是查看文件是不是存在
			{
				if (mkdir(destf, 0777)) //如果不存在就用mkdir函数来创建
				{
					perror("mkir dest dir failed");
					exit(0);
				}
			}

		}
		else
			sprintf(file, "%s", destf);

		for (i = 0; !access(file, F_OK); i++)
		{

			for (j = 0; j < strlen(destf); j++)
			{
				if (destf[j] == '.' && destf[j + 1] != '.')
					if (destf[j] == '.' && destf[j + 1] != '/')
						break;

				file[j] = destf[j];
			}
			file[j] = i + 48;
			for (; j < strlen(destf); j++)
			{
				file[j + 1] = destf[j];
			}
			if (access(file, F_OK))
				break;
		}

	}
	else if (destf == NULL)
	{
		for (i = 0;; i++)
		{
			for (j = 0; j < strlen(srcf); j++)
			{
				if (srcf[j] == '.' && srcf[j + 1] != '/')
					break;
				file[j] = srcf[j];
			}
			file[j] = i + 48;
			for (; j < strlen(srcf); j++)
			{
				file[j + 1] = srcf[j];
			}
			if (access(file, F_OK))
				break;
		}
	}
	//清空掩码
	umask(0);
	struct stat fbuf;
	int res = stat(srcf, &fbuf);
	if (res != 0)
	{
		perror("stat src file failed");
	}
	if (S_ISLNK(fbuf.st_mode))
	{
		char link[maxname];
		readlink(srcf, link, maxname);
		symlink(link, file);

		cpyfile_p p = (cpyfile_p) arg;
		free(p->srcfile);
		free(p->destfile);
		free(p);
		close(src);

		return (void*) 0;

	}

	//判断目标文件权限
	if ((!access(srcf, X_OK)) && (!S_ISDIR(fbuf.st_mode)))
	{
		dest = open(file, O_CREAT | O_RDWR, 0766);
	}
	else
		dest = open(file, O_CREAT | O_RDWR, 0666);
	//是否成功创建目标文件
	if (dest == -1)
	{
		puts(file);
		printf("[LINE %d] : %s\n", __LINE__, file);
		perror("open dest file failed");
		exit(0);
	}
	//判断是否是链接文件

	while ((i = read(src, buf, maxname)) > 0) //n正确读取到的字节数
	{
		if (i == 0)
			perror("read src file failed");
		char *tmp = buf;
		while (i > 0)
		{
			j = write(dest, tmp, i);
			if (j == 0)
				perror("write dest file failed");
			i -= j;
			tmp += j;
		}
	}
	//printf("[LINE %d] \n" ,__LINE__);
	cpyfile_p p = (cpyfile_p) arg;
	free(p->srcfile);
	free(p->destfile);
	free(p);
	close(src);
	close(dest);

	return (void*) 0;
}
