/*
 *管道使用说明      
 */
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>

#define MSGSIZE 8
char* msg = "hello";

int main()
{   
	char inbuf[MSGSIZE];
	int p[2];
	pid_t pid;
	/*打开管道*/
	if(pipe(p) == -1){
		perror("pipe");
		exit(-1);
	}
	/*创建子进程*/
	switch(pid = fork()){
	case -1:
		perror("fork");
		exit(2);
	case 0:    /*子进程*/
		close(p[0]);  /*关闭读端*/
		/*向写端写入*/
		write(p[1],msg,strlen(msg));
		write(p[1],"",1);    /*写入一个字符串结束符*/
		break;
	default:   /*父进程*/
		close(p[1]);  /*关闭写端*/
		/*从读端读数据*/
		read(p[0],inbuf,MSGSIZE);
		printf("Parent Process: %s\n",inbuf);
		wait(NULL);    /*等待子进程结束*/
	}
	return 0;
}

/*
  注意管道是一种数据流的方式进行传输的，并且是先进先出
  之所以在子进程中关闭读端，在父进程中关闭写端，是因为者两个进程都不在需要这两种
  文件描述符，应当关闭不使用的文件描述符。
  0端代表读端；1端代表写端。
 */
