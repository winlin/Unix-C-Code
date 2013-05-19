/*两个进程输出一句话，一个输出前半句，一个后半句
 *演示进程间同步问题
 *由于进程运行次序的不定性，所以，会乱序输出
 */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

int main()
{   
	int i,j;
	pid_t pid;
	const char* message="hello world!\n";
	int n=strlen(message);
	pid=fork();
	j=(pid==0)?0:n;   /*子进程从头开始，父进程从中间开始*/
	for(i=0;i<n;++i){
		write(STDOUT_FILENO,message+j+i,1);
		sleep(1); /*睡眠，以调度其他进程*/
	}
	
	return 0;
}
