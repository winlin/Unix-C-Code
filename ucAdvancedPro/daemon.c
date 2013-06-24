#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <pwd.h>
#include <signal.h>
#include <stdlib.h>

int term=0;
void func(int sig)
{
	term = 1;
}
int main()
{
	//修改从父进程继承来的权限屏蔽字
	umask(0000);
	//fork让父进程结束
	if(fork()!=0) return 0;
	signal(SIGTERM, func);
	printf("cid=%d\n", getpid());
	int i;
	//关闭从父进程继承来的文件描述符
	for(i=0; i<256; i++)
		close(i);
	//自己成立进程组（会话）
	setsid();
	//设置当前工作目录
	chdir(getpwuid(getuid())->pw_dir);//getenv("HOME"))|getpwnam
	//执行真正的服务工作
	char c='A';
	for(;term==0;){
		int fd = open("abc", O_WRONLY|O_CREAT, 0644);
		if(fd<0) continue;
		write(fd, &c, 1);
		close(fd);
		if(++c>'Z') c='A';
		sleep(1);
	}
	unlink("abc");
	return 0;
}

//Another way to realize deamon
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
int main(int argc, char* argv[])
{
FILE *fp= NULL;
pid_t process_id = 0;
pid_t sid = 0;
// Create child process
process_id = fork();
// Indication of fork() failure
if (process_id < 0)
{
printf("fork failed!\n");
// Return failure in exit status
exit(1);
}
// PARENT PROCESS. Need to kill it.
if (process_id > 0)
{
printf("process_id of child process %d \n", process_id);
// return success in exit status
exit(0);
}
//unmask the file mode
umask(0);
//set new session
sid = setsid();
if(sid < 0)
{
// Return failure
exit(1);
}
// Change the current working directory to root.
chdir("/");
// Close stdin. stdout and stderr
close(STDIN_FILENO);
close(STDOUT_FILENO);
close(STDERR_FILENO);
// Open a log file in write mode.
fp = fopen ("Log.txt", "w+");
while (1)
{
//Dont block context switches, let the process sleep for some time
sleep(1);
fprintf(fp, "Logging info...\n");
fflush(fp);
// Implement and call some function that does core work for this daemon.
}
fclose(fp);
return (0);
}

