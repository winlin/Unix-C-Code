/*
 *使用两个管道进行进程间的相互通讯     
 */
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>

#define MSGSIZE 30
char* msg0 = "hello child process";
char* msg1 = "hello parent process";

int main()
{   
	char inbuf[MSGSIZE];
	int p0[2];  //父写子读
	int p1[2];  //子写父读
	
	pid_t pid;
	if(pipe(p0) == -1||pipe(p1)==-1){
		perror("pipe");
		exit(-1);
	}
	
	switch(pid = fork()){
	case -1:
		perror("fork");
		exit(2);
	case 0:
		close(p0[1]);
		close(p1[0]);
		
		read(p0[0],inbuf,MSGSIZE);
		printf("Parent send: %s\n",inbuf);
		
		write(p1[1],msg1,strlen(msg1));
		write(p1[1],"",1);

		close(p0[0]);
		close(p1[1]);
		
		break;
	default:  
		close(p0[0]);
		close(p1[1]);

		write(p0[1],msg0,strlen(msg0));
		write(p0[1],"",1);

		read(p1[0],inbuf,MSGSIZE);
		printf("Child send: %s\n",inbuf);

		close(p0[1]);
		close(p1[0]);
		wait(NULL);    /*等待子进程结束*/
	}
	return 0;
}

/*
  管道可以交互通信吗？  理论上是不可以的。
  Pipes  and  FIFOs (also known as named pipes) provide a unidirectional inter
  process communication channel.
  管道和命名管道提供了一种“单向” unidirectional 的通讯方式。
  当使用 read 读取一个没有数据的管道时，默认会阻塞 read 直到有数据，除非使用 fcntl 设置 O_NONBLOCK；
  当时用 write 写一个满管道(PIPE_BUF)时, 默认也会阻塞 write 直到有空闲空间。

  如果一个管道的所有读端都关闭时，再写入会得到 SIGPIPE 的消息；
  如果一个管道的所有写端都关闭时，再读取会得到一个 EOF 标记，read 返回0；

  所以，应该先运行读管道的程序。

   PIPE_BUF
       POSIX.1-2001 says that write(2)s of less than PIPE_BUF bytes must be atomic: the output data is written
       to the pipe as a contiguous sequence.  Writes of more than PIPE_BUF bytes may be non-atomic: the  kernel
       may interleave(交错，插入空白页)the  data with data written by other processes.  POSIX.1-2001 requires
       PIPE_BUF to be at least 512 bytes.  (On Linux, PIPE_BUF is 4096 bytes.)  The precise semantics depend on
       whether the file descrip-tor  is non-blocking (O_NONBLOCK), whether there are multiple writers to the
       pipe, and on n, the number of bytes to be written:

       man 7 pipe
 */
