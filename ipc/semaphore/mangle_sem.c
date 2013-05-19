/*
 *使用POSIX解决进程同步问题
 */
#include <stdio.h>
#include <string.h>
#include <semaphore.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int main()
{   
	int i,j;
	pid_t pid;
	const char* message="hello world!\n";
	int n=strlen(message)/2;
	/*打开命名信号灯，初始个数为0*/
	sem_t* sem=sem_open("/mysem",O_CREAT,S_IRUSR|S_IWUSR,0);
	assert(sem!=NULL);
	pid=fork();
	j=(pid==0)?0:n;
	if(pid)
		sem_wait(sem);   /*由于初始化为0，所以父进程会等待信号灯*/
	for(i=0;i<n;++i){
		write(STDOUT_FILENO,message+i+j,1);
		sleep(1);       /*以调度其他进程*/
	}
	if(pid==0)
		sem_post(sem);  /*子进程释放信号灯*/
	sem_close(sem);         /*关闭命名信号灯*/
	return 0;
}

/*
  本程序使用的是POSIX标准的信号等函数。POSIX信号灯分为两类：命名信号灯和无名信号灯。命名信号灯有一个全局的名字，因而可以在两个没有亲源关系
  的进程间使用。无名信号灯是基于内存变量的一种信号灯，如果要在两个进程中使用它，则必须把它放到两者的共享内存中。
  命名信号灯函数：sem_open,sem_close,sem_unlink
  无名信号灯函数:sem_init,sem_destroy
  共用函数:sem_post(信号灯加1),sem_wait(信号灯减1),sem_trywait(不阻塞)

  需要注意，因为POSIX IPC 接口并非C标准库的组成部分，因此编译时必须与 librt 或 libpthread 共享库链接。

  还有一组SYS V 和 POSIX 都支持的接口:
  semget,semctl,semop
 */
