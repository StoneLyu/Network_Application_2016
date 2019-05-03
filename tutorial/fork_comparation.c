#include <sys/typeds.h>
#include <unistd.h>
#include <stdio.h>

int main(void)
{
	pid_t pid;
	printf("Original program,pid=%d\n", getpid());
	pid = fork();
	if (pid == 0)
	{
		printf("In child process,pid=%d,ppid=%d\n", getpid(), getppid());
	}
	else
	{
		printf("In parent, pid=%d, fork returned=%d\n", getpid(), pid);
	}
}