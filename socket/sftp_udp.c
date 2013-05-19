/*
 *使用UDP实现简单的TFTP服务器例程
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/*TFTP协议操作码定义*/
#define RRQ      1  /*读数据操作*/
#define WRQ      2  /*写数据操作*/
#define DATA     3  /*数据块*/
#define ACK      4  /*确认*/
#define ERROR    5  /*错误*/

/*TFTP协议错误码定义*/
#define FILE_NO_FOUND  1 /*文件未找到*/
#define BAD_OP         2 /*非法操作*/

/*TFTP协议数据包格式定义*/
struct tftp_packet{
	short opcode;  /*2B操作码*/
	union{
		char bytes[514];   /*最多514B，2B块号+512B数据,此句用于确定枚举的长度*/
		struct{
			short code;  /*错误码*/
			char message[200];  /*错误信息*/
		}error;  /*错误包*/
		struct{
			short block;  /*块号*/
			char bytes[512]; /*文件数据*/
		}data; /*数据包*/
		struct{
			short block; /*块号*/
		}ack; /*确认包*/
	}u;
};

void die(char* msg)
{
	perror(msg);
	exit(-1);
}

/*发送错误包*/
void send_error(int s,struct sockaddr_in* addr,socklen_t addr_len,int code)
{
	struct tftp_packet err;
	int size;
	int bytes;
	err.opcode=htons(ERROR);
	err.u.error.code=htons(code);
	switch(code){
	case FILE_NO_FOUND:
		strcpy(err.u.error.message,"File not found.");
		break;
	case BAD_OP:
		strcpy(err.u.error.message,"Bad operation.");
		break;
	default:
		strcpy(err.u.error.message,"Undefined error.");
	}
	/*2B操作码+2B错误码+错误信息+字符串结束符*/
	size=2+2+strlen(err.u.error.message)+1;
	/*发送*/
	bytes=sendto(s,&err,size,0,(struct sockaddr*)addr,addr_len);
}

/*处理请求*/
void handle_request(int sock,struct sockaddr_in* from,socklen_t from_len,struct tftp_packet* request)
{
	char* file_name;
	char* mode;
	int fd;
	int size;
	int bytes;
	struct tftp_packet data,response;
	int block;
	struct sockaddr_in new_from;
	socklen_t new_from_len;
	/*分析操作码*/
	request->opcode=ntohs(request->opcode);
	if(request->opcode!=RRQ)
		return;   /*如果不是读请求则不处理*/
	file_name=request->u.bytes; /*得到文件名*/
	mode=file_name+strlen(file_name)+1; /*得到传输模式*/
	printf("Request file name is %s,mode is %s\n",file_name,mode);
	/*我们只支持octet模式*/
	if(strncmp(mode,"octet",4)){
		send_error(sock,from,from_len,BAD_OP);
		return;
	}
	/*打开文件*/
	fd=open(file_name,O_RDONLY);
	if(fd<0){
		send_error(sock,from,from_len,FILE_NO_FOUND);
		return;
	}
	/*准备发送数据*/
	printf("Send start. \n");
	data.opcode=htons(DATA);  /*操作码为数据块*/
	for(block=1;(size=read(fd,data.u.data.bytes,512))>0;block++){
		data.u.data.block=htons(block);
		/*加上2B操作码，2B块号长度*/
		size+=4;
		/*发送*/
		bytes=sendto(sock,&data,size,0,(struct sockaddr*)from,from_len);
		if(bytes!=size){
			perror("handle_request:sendto");
			close(fd);
			return;
		}
		/*接收确认*/
		new_from_len=sizeof(new_from);
		bytes=recvfrom(sock,&response,sizeof(response),0,(struct sockaddr*)&new_from,&new_from_len);
		if(bytes<0){
			perror("handle_request:recvfrom");
			close(fd);
			return;
		}
		/*如果数据包来自非原来的客户端，说明受到干扰*/
		if(new_from.sin_addr.s_addr!=from->sin_addr.s_addr){
			fprintf(stderr,"handle_request: bad_address\n");
			close(fd);
			return;
		}
		/*判断是否为确认*/
		response.opcode=ntohs(response.opcode);
		if(response.opcode!=ACK){
			fprintf(stderr,"handle_request: not ACK\n");
			close(fd);
			return;
		}
		/*判断确认的块号是否正确*/
		response.u.ack.block=ntohs(response.u.ack.block);
		if(response.u.ack.block!=block){
			fprintf(stderr,"handle_request: wrong ACK\n");
			close(fd);
			return;
		}
		printf(".");  /*输出信息表示一块数据传输成功*/
	}	
	printf("\nSend out.\n");
}

int main()
{   
	int s;
	int bytes;
	struct sockaddr_in addr,from;
	socklen_t from_len;
	struct tftp_packet packet;
	if((s=socket(AF_INET,SOCK_DGRAM,0))<0)
		die("socket");
	/*绑定*/
	memset(&addr,0,sizeof(addr));
	addr.sin_family=AF_INET;
	addr.sin_addr.s_addr=htonl(INADDR_ANY);
	addr.sin_port=htons(69); /*TFTP服务默认端口号*/
	if(bind(s,(struct sockaddr*)&addr,sizeof(addr)))
		die("bind");
	/*主循环*/
	while(1){
		/*接收*/
		from_len=sizeof(from);
		bytes=recvfrom(s,&packet,sizeof(packet),0,(struct sockaddr*)&from,&from_len);
		if(bytes<0)
			die("recvfrom");
		printf("Request from %s:%d\n",inet_ntoa(from.sin_addr),htons(from.sin_port));
		/*处理请求*/
		handle_request(s,&from,from_len,&packet);
	}
		
	return 0;
}

/*
  本例中最终要的是学会，怎样构建一个传输控制协议包的格式，以及怎样解析这个包中的短整数/整数数据(ntohs/nthol)
  该例子采用简单的方式，同时只允许一个客户端进行文件传输，如果有其他客户请求传输文件，则正长的传输将会终止。

  后期改进的话，应当注意发送包失败时进行重发的功能。
 */
