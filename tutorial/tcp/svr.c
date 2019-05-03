#include <stdio.h>		/* for printf()*/
#include <sys/socket.h> /* for socket(), bind(), connection(), send() and recv() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <stdlib.h>		/* for atoi() and exit() */
#include <string.h>		/* for memset(), stramp(), and  strtok()*/
#include <unistd.h>		/* for close() */
#include <sys/types.h>
#include <fcntl.h>
#define MAX 255 /* Longest string */
int main(int argc, char *argv[])
{
	int sock; /* Socket descriptor */
	int sock1;
	struct sockaddr_in ServAddr; /* Local address */
	struct sockaddr_in ClntAddr; /* Client address */
	unsigned short ServPort;	 /* Server port */
	char filename[MAX];
	int sendStringLen; /* Length of string to send */
	int recvStringLen; /* Length of received response */
	int fd;
	int fd1;
	char *ClntIP = "127.0.0.1"; /* IP address of server */
	unsigned int framesize;
	if (argc != 2)
	{
		printf("Usage: %s <port number> \n", argv[0]);
		exit(1);
	}

	ServPort = atoi(argv[1]); /* local port */
	if ((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("socket() failed.\n");
	}
	printf("1");
	memset(&ServAddr, 0, sizeof(ServAddr));
	ServAddr.sin_family = AF_INET; /* Internet addr family */
	ServAddr.sin_addr.s_addr = inet_addr(INADDR_ANY);
	ServAddr.sin_port = htons(ServPort); /* Server port */
	if ((bind(sock, (struct sockaddr *)&ServAddr, sizeof(ServAddr))) < 0)
	{
		printf("bind() failed.\n");
	}
	printf("2");
	if ((listen(sock, MAX)) < 0)
	{
		printf("listen() failed.\n");
	}
	printf("3");
	memset(&ClntAddr, 0, sizeof(ClntAddr));
	ClntAddr.sin_family = AF_INET; /* Internet addr family */
	ClntAddr.sin_addr.s_addr = inet_addr(ClntIP);
	ClntAddr.sin_port = htons(ServPort); /* Server port */
	framesize = sizeof(ClntAddr);
	if (sock1 = (accept(sock, (struct sockaddr *)&ClntAddr, &framesize)) < 0)
	{
		printf("accept() failed");
	}

	if ((recvStringLen = recv(sock, filename, MAX, 0)) < 0)
	{
		printf("recvfrom() failed\n");
	}
	filename[recvStringLen] = '\0';

	if ((fd = open(filename, O_RDWR)) == -1)
	{
		char error[12];
		strcpy(error, "No such file");
		printf("No such file\n");

		if ((send(sock, error, 12, 0)) != 12)
		{
			printf("sendto() sent a different number of bytes than expected.\n");
		}
		exit(1);
	}
	else
	{
		printf("File is successfully opened\n");
	}
	int fileLen = (lseek(fd, 0, SEEK_END));
	if (fileLen > MAX)
	{
		printf("File too large\n");
		exit(1);
	}
#define filelength fileLen
	lseek(fd, 0, SEEK_SET);
	char file[filelength];
	if (read(fd, file, fileLen) != fileLen)
	{
		printf("Read error\n");
		exit(1);
	}
	sendStringLen = fileLen;
	if ((send(sock, filename, sendStringLen, 0)) != sendStringLen)
	{
		printf("sendto() sent a different number of bytes than expected.\n");
	}
	close(fd);
	close(sock1);
	exit(0);
}
