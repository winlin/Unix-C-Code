/*
 *TCP通信例程服务器端程序
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

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

int main()
{   
	int sock,conn,i;
	struct sockaddr_in addr;
	socklen_t addr_len=sizeof(addr);
	if((sock=socket(AF_INET,SOCK_STREAM,0))<0){
		die("socket");
	}
	/*设置地址可重用*/
	i=1;
	setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&i,sizeof(i));
	/*绑定到任意地址*/
	memset(&addr,0,sizeof(addr));
	addr.sin_family=AF_INET;
	addr.sin_addr.s_addr=htonl(INADDR_ANY);
	addr.sin_port=htons(8888);

	if(bind(sock,(struct sockaddr*)&addr,sizeof(addr))){
		die("bind");
	}
	if(listen(sock,5)){
		die("listen");
	}
	while((conn=accept(sock,(struct sockaddr*)&addr,&addr_len))>0){
		printf("Accept %s:%d\n",inet_ntoa(addr.sin_addr),addr.sin_port);
		/*使用新接收的套机子通信*/
		copy_data(conn,STDOUT_FILENO);
		printf("Done!\n");
		close(conn);
	}
	if(conn<0)
		die("accept");
	close(sock);
	return 0;
}
