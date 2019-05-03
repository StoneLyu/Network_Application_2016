#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

int main(void)
{
	pid_t pid;
	pid = fork(); //create a same process with different pid
	printf("fork returned %d\n", pid);
	exit(0);
}