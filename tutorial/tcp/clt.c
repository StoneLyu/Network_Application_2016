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
	int sock;					 /* Socket descriptor */
	struct sockaddr_in ServAddr; /* Server address */
	struct sockaddr_in fromAddr; /* Source address of response */
	unsigned short ServPort;	 /* Server port */
	unsigned int fromSize;		 /* In-out of address size for recv() */
	char *servIP;				 /* IP address of server */
	char *filename;
	char Buffer[MAX + 1]; /* Buffer for receiving string */
	int sendStringLen;	/* Length of string to send */
	int recvStringLen;	/* Length of received response */
	int fd;

	if (argc != 4) /* Test for correct number ofarguments */
	{
		printf("Usage: %s <Server IP> <port number> <file name>\n", argv[0]);
		exit(1);
	}
	ServPort = atoi(argv[2]); /* local port */

	if ((strcmp("127.0.0.1", argv[1]) != 0) && (strcmp("localhost", argv[1]) != 0))
	{
		printf("Address error\n");
		exit(1);
	}
	servIP = "127.0.0.1";
	printf("Connecting to the server...\n");

	/* Create a TCP socket */
	if ((sock = socket(PF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("socket() failed.\n");
	}
	memset(&ServAddr, 0, sizeof(ServAddr));
	ServAddr.sin_family = AF_INET;				  /* Internet addr family */
	ServAddr.sin_addr.s_addr = inet_addr(servIP); /*Server IP address*/
	ServAddr.sin_port = htons(ServPort);		  /* Server port */
	if ((connect(sock, (struct sockaddr *)&ServAddr, sizeof(ServAddr))) < 0)
	{
		printf("Connection denied\n");
	}
	filename = argv[3];
	sendStringLen = strlen(filename);

	if ((send(sock, filename, sendStringLen, 0)) != sendStringLen)
	{
		printf("sendto() sent a different number of bytes than expected.\n");
	}

	if ((recvStringLen = recv(sock, Buffer, MAX, 0)) < 0)
	{
		printf("recvfrom() failed\n");
	}
	Buffer[recvStringLen] = '\0';

	if (strcmp(Buffer, "No such file") == 0)
	{
		printf("No such file\n");
		exit(1);
	}
	if ((fd = open("new file.txt", O_RDWR | O_CREAT | O_APPEND, 0)) < 0)
	{
		printf("Creat error\n");
	}
	if (write(fd, Buffer, recvStringLen) != recvStringLen)
	{
		printf("Buffer write error\n");
	}

	printf("The connection is closed. Exiting now ...");
	close(fd);
	close(sock);
	exit(0);
}
