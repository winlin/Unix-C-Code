/*
 *POSIX 消息队列
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <mqueue.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <sys/types.h>

#define MSGSIZE 128

/*包装消息*/
struct message{
	char mtext[MSGSIZE];
};

/*发送消息*/
static int send_msg(mqd_t qid,int pri,const char text[])
{
	int r=mq_send(qid,text,strlen(text)+1,pri);
	if(r==-1){
		perror("mq_send");
	}
	return r;
}

/*消息发送者*/
static void producer(mqd_t qid)
{
	/*发送低优先级消息*/
	send_msg(qid,1,"This is my first message.");
	send_msg(qid,1,"This is my second message.");
	/*发送高优先级消息*/
	send_msg(qid,3,"No more message.");
}

/*消息接收者*/
static void consumer(mqd_t qid)
{
	struct mq_attr mattr;
	do{
		unsigned int pri;
		struct message msg;
		ssize_t len;
		len=mq_receive(qid,(char*)&msg,sizeof(struct message),&pri);
		if(len==-1){
			perror("mq_receive");
			break;
		}
		printf("Get priority %u '%s' len=%d\n",pri,msg.mtext,(int)len);

                /*检查是否有更多的消息*/
		int r=mq_getattr(qid,&mattr);
		if(r==-1){
			perror("mq_getattr");
			break;
		}
	}while(mattr.mq_curmsgs);   /*如果没有消息则退出循环*/	
}

int main()
{   
	mqd_t qid;
	struct mq_attr attr;
	char* qname="/msgq";   /*队列的名字*/
	memset(&attr,0,sizeof(attr));
	attr.mq_maxmsg=5;
	attr.mq_msgsize=MSGSIZE;
	qid=mq_open(qname,O_CREAT|O_RDWR,0666,&attr);  /*创建队列*/
	if(qid<0){
		perror("mq_open");
		return -1;
	}
	if(!fork()){
		producer(qid);
		exit(0);
	}
	
	wait(NULL);
	
	consumer(qid);
	
	mq_unlink(qname);
	
	return 0;
}


/*
Get priority 3 'No more message.' len=17
Get priority 1 'This is my first message.' len=26
Get priority 1 'This is my second message.' len=27

从结果中可以看到优先级的作用，优先级高的先被接收，同级的则fifo；
另外，消息队列能够容纳消息，甚至在发送者进程退出以后也保留它所发送的消息。
 */
