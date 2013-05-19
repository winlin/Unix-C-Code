/*
 *TCP通信客户端
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int inet_aton(const char *cp, struct in_addr *inp);
void die(const char* msg)
{
	perror(msg);
	exit(EXIT_FAILURE);
}
/*从文件 from 向 to 复制数据*/
void copy_data(int from,int to)
{
	char buf[1024];
	int amount;
	while((amount=read(from,buf,sizeof(buf)))>0){
		if(write(to,buf,amount)!=amount){
			die("write");
		}
	}
	if(amount<0)
		die("read");
}

int main(int argc,char** argv)
{
	if(argc<2){
		printf("Usage: %s ip address\n",argv[0]);
		exit(0);
	}
	
	struct sockaddr_in addr;
	memset(&addr,0,sizeof(addr));
	addr.sin_family=AF_INET;
	addr.sin_port=htons(8888);
	inet_aton(argv[1],&addr.sin_addr);
	
	int sock;
	if((sock=socket(AF_INET,SOCK_STREAM,0))<0){
		die("socket");
	}
	if(connect(sock,(struct sockaddr*)&addr,sizeof(addr))){
			die("connect");
	}
	printf("Connect!\n");

	copy_data(STDIN_FILENO,sock);
	close(sock);
		
	return 0;
}
