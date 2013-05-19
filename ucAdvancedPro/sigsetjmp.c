/*
 *演示在信号处理程序被调用时，系统所设置的信号屏蔽字如何自动地包括刚被捕捉到的信号
 *同时，也演示了如何使用sigsetjmp/siglongjmp()
 */
#include <stdio.h>
#include <setjmp.h>
#include <time.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>

static void sig_usr1(int);
static void sig_alrm(int);
static sigjmp_buf jmpbuf;
static volatile sig_atomic_t  canjump;

int sigsetjmp(sigjmp_buf env, int savesigs);
void siglongjmp(sigjmp_buf env, int val);

int main()
{   
	if(signal(SIGUSR1,sig_usr1)==SIG_ERR)
		perror("signal(SIGUSR1)");
	if(signal(SIGALRM,sig_alrm)==SIG_ERR)
		perror("signal(SIGALRM)");

	printf("main starting...");
	if(sigsetjmp(jmpbuf,1)){
		printf("ending main...");
		exit(0);
	}
	canjump=1;
	while(1){
		pause();
	}
	return 0;
}

static void sig_usr1(int signo)
{
	time_t  starttime;
	if(canjump==0)
		return;

	printf("starting sig_user1...");
	alarm(3);
	starttime=time(NULL);
	while(1){
		if(time(NULL)>starttime+5)
			break;
	}
	printf("finishing sig_usr1...");
	canjump=0;
	siglongjmp(jmpbufm,1);
}

static void sig_alrm(int signo)
{
	printf("in sig_alrm:");
}
