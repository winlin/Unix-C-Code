/*
 *写FIFO
 */
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>

int main(int argc,char** argv)
{   
	if(argc<2){
		printf("Usage: %s fifo_path\n",argv[0]);
		exit(0);
	}
	int count=0;
	int fd;
	int retry=0;
	char buf[128]="hello world!";
	/*以写方式打开FIFO*/
	if((fd=open(argv[1],O_WRONLY))<0){
		perror("open");
		exit(-1);
	}
	printf("Open fifo file success to write.\n");
	while(1){
		/*写FIFO*/
		count=write(fd,buf,sizeof(buf));
		++retry;
		printf("Write %d bytes, %d times.\n",count,retry);
	}
	close(fd);
	return 0;
}

/*如果在运行读取管道程序前，先运行写管道的程序，则会出现没有读端的情况，这时会发送 SIGPIPE 消息
  如果一个管道的所有读端都关闭时，再写入会得到 SIGPIPE 的消息；这时进程会挂起，直到有读取端加入，write才会写入数据，然后返回；
  如果write写满了FIFO文件，也会导致write阻塞，整个进程挂起。
  
 */
