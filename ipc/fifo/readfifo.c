/*
 *读取FIFO
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

char work_area[32];

int main(int argc,char** argv)
{
	if(argc<2){
		printf("Usage: %s fifo_path\n",argv[0]);
		exit(0);
	}
	
	int count=0;
	char buf[128];
	int fd;
	/*以读方式打开fifo文件*/
	if((fd=open(argv[1],O_RDONLY))<0){
		perror("open");
		exit(-1);
	}
	printf("open fifo success to read \n");
	/*循环读取*/
	while(1){
		/*需要用户输入一个c才开始读*/
		while(strncmp("c",work_area,1)!=0){
			fgets(work_area,32,stdin);
		}
		printf("Ready to read fifo......\n");
		count=read(fd,buf,128);
		printf("Recv mas: %s\n",buf);
		work_area[0]='\0';
	}
	close(fd);
	return 0;
}

