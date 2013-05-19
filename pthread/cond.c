/*
 *条件变量使用例程
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

static pthread_mutex_t lock=PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond=PTHREAD_COND_INITIALIZER;

static int x=0;

/*消费者线程工作函数*/
static void* thread_eat(void* arg)
{
	int id=(int)arg;
	printf("thread %d run\n",id);
	while(1){
		pthread_mutex_lock(&lock);
		/*判断条件是否满足，如果 x<=0，则等待条件变量cond*/
		while(x<=0){
			printf("thread %d will wait on cond\n",id);
			pthread_cond_wait(&cond,&lock);
			printf("thread %d wakeup \n",id);
		}
		x--;
		printf("thread %d eat one,now x=%d\n",id,x);
		pthread_mutex_unlock(&lock);
		sleep(1);
	}
	return NULL;
}

/*生产者*/
static void* thread_feed(void* arg)
{
	while(1){
		pthread_mutex_lock(&lock);
		x+=4;
		/*触发条件变量*/
		pthread_cond_broadcast(&cond);
		printf("thread feed set x=%d\n",x);
		pthread_mutex_unlock(&lock);
		sleep(2);
	}
	return NULL;
}

int i=0;
int main()
{
	pthread_t tid[3];
	for(i=0;i<3;++i){
		/*注意，没有使用 (void*)&i 是因为创建的多个进程都会去访问i,造成资源互斥*/
		if(pthread_create(&tid[i],NULL,thread_eat,(void*)i)){   
			perror("pthread_create");
			return -1;
		}
	}
	sleep(3);  /*保证消费者先运行*/
	pthread_t fid;
	if(pthread_create(&fid,NULL,thread_feed,NULL)){
		perror("pthread_create");
		return -1;
	}
	pthread_join(fid,NULL);
	
	return 0;
}


/*
    eat1.lock
          |
       cond_wait--unlock--> eat2.lock
                     |             |
	     |-->>--lock       cond_wait--unlock--> eat3.lock
	     |	     |                        |            |
	     |	   unlock	     -->>-- lock      cond_wait--unlock--> feed4.lock
	     |				     |	                  |            |
	     |				    unlock	  -->>--lock	  cond_broadcast---------->->-
	     |							  |               |                  |
	     |							unlock	      unlock                 |
             |                                                                                       |
             -<<-------------------<<-----------------<<--------------------------------------<--<---|

上面这幅图显示了条件变量使用过程中，整个程序互斥体的加锁和解锁情况，注意pthread_cond_wait()会先解锁，等待消息，然后再加锁.
		
 */
