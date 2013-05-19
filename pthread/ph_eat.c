/*
 *使用线程模拟哲学家吃饭问题
 *利用信号灯模拟
 */
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#define NUM   5  /*人数*/
#define FOOD  5  /*食物数量*/

#define LEFT(x)  x   /*得到左边的筷子编号*/
#define RIGHT(x) (((x)+1)%NUM)    /*得到右边的筷子编号*/

static sem_t sem[NUM];   /*表示筷子*/
static int food[NUM];    /*剩余食物数量*/
static int stick[NUM];   /*记录筷子被那个哲学家拿走*/

/*输出剩余食物的情况*/
void print_food()
{
	int i;
	printf("\033[1;31m Food:");
	for(i=0;i<NUM;++i)
		printf(" %1d ",food[i]);
	printf("\033[m\n");
}
/*输出筷子的使用情况*/
void print_stick()
{
	int i;
	printf("Stick:");
	for(i=0;i<NUM;++i){
		if(stick[i]<0)
			printf(" - ");
		else
			printf(" %1d ",stick[i]);
	}
	printf("\n");	
}
/*用于模拟一个哲学家的线程工作函数，arg参数代表哲学家的编号*/
void* th_eat(void* arg)
{
	int id=*((int*)arg);
	printf("th_eat started! ID= %d.\n",id);
	while(food[id]>0){
		sleep(1);
		sem_wait(&sem[LEFT(id)]); /*等待拿左边的筷子*/
		stick[LEFT(id)]=id;
		print_stick();
		#if 0
		sleep(1);
		#endif
		sem_wait(&sem[RIGHT(id)]);  /*等待拿右边的筷子*/
		stick[RIGHT(id)]=id;
		print_stick();

		food[id]--;       /*食物减1*/
		print_food();

		sleep(1);
		/*准备释放筷子*/
		stick[LEFT(id)]=-1;
		print_stick();
		sem_post(&sem[LEFT(id)]);  /*放下左边的筷子*/
		stick[RIGHT(id)]=-1;
		print_stick();
		sem_post(&sem[RIGHT(id)]);
	}
	return NULL;
}

int main()
{   
	int i;
	pthread_t th[NUM];
	/*初始化信号灯、食物数量和筷子的使用情况*/
	for(i=0;i<NUM;++i){
		sem_init(&sem[i],0,1);
		food[i]=1;
		stick[i]=-1;
	}
	/*生成NUM个线程，代表NUM个哲学家*/
	for(i=0;i<NUM;++i){
		if(pthread_create(&th[i],NULL,th_eat,(void*)&i)){
			perror("main:pthrea_create");
			return -1;
		}
	}
	/*等待所有线程结束*/
	for(i=0;i<NUM;++i)
		pthread_join(th[i],NULL);
	/*销毁信号灯*/
	for(i=0;i<NUM;++i)
		sem_destroy(&sem[i]);
	
	return 0;
}

/*
  最多两个不相临的人可以同时进餐。
  该程序可能会出现死锁，当这五个人同时拿起右边的筷子时，出现死锁
 */
