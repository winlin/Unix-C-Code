/*父进程先创建一个共享内存文件，然后mmap该文件，等待子进程向
 *共享内存中写入数据，等待子进程退出，读取数据
 *POSIX 共享内存例程
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/mman.h>
#include <sys/wait.h>

void error_out(const char* msg)
{
	perror(msg);
	exit(EXIT_FAILURE);
}

int main()
{   
	int r;
	const char* memname="/mymem";
	const size_t region_size=1024;
	/*打开共享内存文件*/
	int fd=shm_open(memname,O_CREAT|O_TRUNC|O_RDWR,0666);   
	if(fd==-1)
		error_out("shm_open");
	/*将文件截至指定大小*/
	r=ftruncate(fd,region_size);
	if(r!=0)
		error_out("ftruncate");
	/*将文件映射为内存*/
	void* ptr=mmap(0,region_size,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0);
	if(ptr==MAP_FAILED)
		error_out("mmap");
	/*关闭文件*/
	close(fd); 

	/*产生新进程*/
	pid_t pid=fork();
	if(pid==0){
		/*子进程向共享内存中写入数据*/
		unsigned long *d=(unsigned long *)ptr;
		*d=0xdeadbeef;
		exit(0);
	}
	else{
		/*父进程等待子进程结束并读取共享内存*/
		int status;
		waitpid(pid,&status,0);
		printf("The data child wrote is %#lx\n",*(unsigned long*)ptr);
	}
	/*取消内存映射*/
	r=munmap(ptr,region_size);
	if(r!=0)
		error_out("munmap");

	/*删除共享内存文件*/
	r=shm_unlink(memname);
	if(r!=0)
		error_out("shm_unlink");
	
	return 0;
}

/*
  由于父进程等待子进程退出，所以，不会初现共同访问同步的问题，如果是
  多个进程进行读写的话，要仔细的考虑同步问题，可以使用信号灯进行同步。
 */
