/*
 *此文件用于展示多个线程同时访问临界资源，产生的意想不到的结果
 */
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>

void* pthread_proc(void* arg)
{
	int id=*(int*)arg;
	printf("Thread %d Running...\n",id);
	sleep(1);
	printf("Thread %d End\n",id);
	return NULL;
}


int main()
{   
	int i;
	pthread_t pid[3];
	for(i=0;i<3;++i){
		/*此处 i 为临界资源*/
		if(pthread_create(&pid[i],NULL,pthread_proc,(void*)&i)){
			perror("pthread_create");
			return -1;
		}
		//pthread_join(pid[i]);  加上这一句就没有问题，一个结束再创建另一个，不会互斥
	}
	sleep(4);
	return 0;
}
