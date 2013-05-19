#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc,char* argv[])
{
	printf("main() start \n");
	int bb=1234;
	pid_t cid;
	if((cid=fork())==0){
		printf("child process %d start \n",getpid());
		printf("cid=%d\n",cid);
		++bb;
		sleep(2);
	}
	printf("main():child has run\n");
	printf("main():%d\n",getpid());
	printf("cid=%d\n",cid);
	printf("bb=%d\n",bb);
	return 0;
}

/*
main() start 
main():child has run
main():4988
cid=4989
bb=1234
child process 4989 start 
cid=0
~$ main():child has run
main():4989
cid=0
bb=1235

可以看出子进程复制了父进程所有的代码，和父进程的SIP指针，
子进程会在调用fork执行点开始执行，fork之前的语句不再执行，
并且fork在子进程中返回0，在父进程中返回子进程的PID。
其余的变量的值，都保持在调用fork时的状态。
 */
