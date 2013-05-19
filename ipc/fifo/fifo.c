/*
 *用于创建 FIFO
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>

int main(int argc,char* argv[])
{   
	/*命令行输入FIFO文件名*/
	if(argc<2){
		printf("Usage: %s fifo_name\n",argv[0]);
		exit(0);
	}
	/*清楚文件标志掩码*/
	umask(0);
	/*创建FIFO*/
	if(mkfifo(argv[1],0777)){
		perror("mkfifo");
		exit(-1);
	}
	printf("Sucess to create the fifo %s\n",argv[1]);
	return 0;
}
