/*
 *线程池应用
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#define NAME_SIZE 10  /*任务名长度*/

/*任务队列节点，任务队列用链表来实现*/
typedef struct task{
	void (*handler)(void*); /*任务处理函数*/
	void *arg;  /*传给处理函数的参数*/
	struct task*next;
}task_t;

/*线程信息*/
typedef struct thread_info{
	pthread_t id;  /*线程句柄*/
	int index;     /*线程编号*/
}thread_info_t;

/*线程池数据*/
typedef struct pool_struct{
	int num_threads;   /*线程池的大小*/
	int max_task;     /*任务队列大小*/
	sem_t empty;
	sem_t full;
	pthread_mutex_t lock;
	thread_info_t* threads;  /*线程池中的线程*/
	task_t* head;      /*队列头*/
	task_t* tail;      /*队列尾*/
}pool_struct_t;

static int pool_add_task(pool_struct_t* pool,task_t* task);
static void* process_task(void* arg);
static void task_handler(void* arg);
static void task_exit(void* arg);

/*创建任务并初始化*/
static task_t* create_task(void (*routine)(void*),char name[])
{
	task_t* task=NULL;
	/*分配空间以存储数据*/
	task=(task_t*)malloc(sizeof(task_t));
	if(!task)
		return NULL;
	/*分配空间以存储任务名*/
	task->arg=(char*)malloc(NAME_SIZE);
	if(!task->arg){
		free(task);
		return NULL;
	}
	task->handler=routine;
	strncpy(task->arg,name,NAME_SIZE);
	task->next=NULL;
	return task;
}

/*销毁任务*/
static void destory_task(task_t* task)
{
	free(task->arg);
	free(task);
}

/*创建线程池并初始化*/
static pool_struct_t* create_pool(int num_threads,int max_task)
{
	int i;
	pool_struct_t* pool=NULL;
	printf("Init thread pool...\n");
	/*分配线程池数据空间*/
	pool=(pool_struct_t*)malloc(sizeof(pool_struct_t));
	if(!pool){
		printf("Unable to malloc the thread pool!\n");
		return NULL;
	}
	/*分配空间以存放线程信息*/
	pool->threads=(thread_info_t*)malloc(sizeof(thread_info_t)*num_threads);
	if(!pool->threads){
		printf("Unable to malloc thread info array!\n");
		free(pool);
		return NULL;
	}
	pool->max_task=max_task;
	pool->head=pool->tail=NULL;
	sem_init(&pool->empty,0,max_task);
	sem_init(&pool->full,0,0);
	pthread_mutex_init(&pool->lock,NULL);
	/*创建线程*/
	for(i=0;i<num_threads;++i){
		pool->threads[i].index=i;
		if(pthread_create(&(pool->threads[i].id),NULL,process_task,(void*)pool)){
			perror("pthreat_create");
			free(pool->threads);
			free(pool);
			return NULL;
		}
	}
	return pool;
}

/*销毁线程池*/
static void destory_pool(pool_struct_t* pool)
{
	task_t* task;
	/*清空任务队列中的任务*/
	pthread_mutex_lock(&pool->lock);
	do{
		task=pool->head;
		pool->head=task->next;
		destory_task(task);
	}while(!pool->head);
	pthread_mutex_unlock(&pool->lock);

	/*向任务队列开头加上退出任务以使工作线程退出*/
	int i;
	for(i=0;i<pool->num_threads;++i){
		task=create_task(task_exit,"EXIT");
		pool_add_task(pool,task);
	}
	for(i=0;i<pool->num_threads;++i){
		pthread_join(pool->threads[i].id,NULL);
	}
	/*销毁信号灯和互斥体*/
	sem_destroy(&pool->empty);
	sem_destroy(&pool->full);
	pthread_mutex_destroy(&pool->lock);
	/*释放内存*/
	free(pool->threads);
	free(pool);
}

/*向任务队列中添加任务*/
int pool_add_task(pool_struct_t* pool,task_t* task)
{
	/*获取表示队列空闲空间的信号*/
	sem_wait(&pool->empty);
	/*添加到链表*/
	pthread_mutex_lock(&pool->lock);
	if(!pool->head){
		pool->head=pool->tail=task;
	}
	else{
		pool->tail->next=task;
		pool->tail=task;
	}
	pthread_mutex_unlock(&pool->lock);

	/*释放表示队列中任务数的信号*/
	sem_post(&pool->full);
	return 0;
}

/*线程工作函数，处理任务*/
void* process_task(void* arg)
{
	int i,index=-1;
	task_t* task;
	pthread_t myid;
	pool_struct_t* pool=(pool_struct_t*)arg;

        /*查找线程信息数组以得到自己的编号，仅是为了输出一下提示*/
	myid=pthread_self();
	for(i=0;i<pool->num_threads;++i){
		if(myid==pool->threads[i].id){
			index=pool->threads[i].index;
			break;
		}
	}
	printf("Thread %d ready.\n",index);
	/*取任务并执行*/
	while(1){
		/*获取表示队列中任务数的信号灯*/
		sem_wait(&pool->full);
		pthread_mutex_lock(&pool->lock);
		/*从链表中取任务*/
		task=pool->head;
		pool->head=task->next;
		pthread_mutex_unlock(&pool->lock);
		/*释放表示队列空闲的信号灯*/
		sem_post(&pool->empty);

		/*执行任务，这是线程的关键，显然有点重点不突出了*/
		printf("Thread %d take task %s ...\n",index,(char*)task->arg);
		(*(task->handler))(task->arg);
		/*销毁任务*/
		destory_task(task);
	}
	return NULL;
}

/*任务处理函数*/
void task_handler(void* arg)
{
	sleep(1);
	printf("Task %s DONE!\n",(char*)arg);
}

/*特殊任务，退出线程*/
void task_exit(void* arg)
{
	printf("Task %s DONE!\n",(char*)arg);
	pthread_exit(NULL);
}


int main()
{   
	int i;
	char name[NAME_SIZE];
	task_t* task=NULL;
	pool_struct_t* pool=NULL;
	pool=create_pool(3,5);

	/*向任务队列增加任务，这里主线程就是生产者角色*/
	for(i=0;i<10;++i){
		sprintf(name,"TASK %d",i);
		task=create_task(task_handler,name);
		if(!task){
			printf("Uable to create task \n");
			destory_pool(pool);
			return -1;
		}
		pool_add_task(pool,task);
	}
	destory_pool(pool);

	return 0;
}

/*
 *有一个任务队列来存放任务，任务就是用相应的参数运行相应的任务函数，每个任务里面存放有任务函数指针和参数；
 *有多个线程，它们不停的去任务队列里取任务；
 *使用信号量表示任务数，互斥体来控制临界资源的访问。
 *我们需要自定义每个任务函数，和使一个线程退出的函数。
 */
