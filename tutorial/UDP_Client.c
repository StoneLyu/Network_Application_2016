#include <stdio.h>      /* for printf() and fprintf() */
#include <sys/socket.h> /* for socket(), sendto() and recvfrom() */
#include <arpa/inet.h>  /* for sockaddr_in and inet_addr() */
#include <stdlib.h>     /* for atoi() and exit() */
#include <string.h>     /* for memset() */
#include <unistd.h>     /* for close() */
#define ECHOMAX 255     /* Longest string to echo */

int main(int argc, char *argv[])
{
    int sock, i = 2;                 /* Socket descriptor */
    struct sockaddr_in echoServAddr; /* Echo server address */
//    struct sockaddr_in fromAddr; /* Source address of echo */
    unsigned short echoServPort;     /* Echo server port */
    unsigned int fromSize;           /* In-out of address size for recvfrom() */
    char *servIP;                    /* IP address of server */
    char *echoString;                /* String to send to echo server */
//   char echoBuffer[ECHOMAX+1]; /* Buffer for receiving echoed string */
    int echoStringLen;               /* Length of string to echo */
//    int respStringLen; /* Length of received response */
    if (argc < 3)
    { /* Test for correct number ofarguments */
        printf("Usage: %s <Server IP> <Echo Word> [<Echo Port>]\n", argv[0]);
        exit(1);
    }
    servIP = argv[1];
    echoServPort = 1500;
    memset(&echoServAddr, 0, sizeof(echoServAddr));   /*Zero out structure*/
    echoServAddr.sin_family = AF_INET;                /* Internet addr family */
    echoServAddr.sin_addr.s_addr = inet_addr(servIP); /*Server IP address*/
    echoServAddr.sin_port = htons(echoServPort);      /* Server port */
    if ((sock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        printf("socket() failed.\n");
    while (i < argc)
    {
        echoString = argv[i];                               /* String to echo */
        if ((echoStringLen = strlen(echoString)) > ECHOMAX) /* Check input length */
            printf("Echo word too long.\n");
        if ((sendto(sock, echoString, echoStringLen, 0, (struct sockaddr *)&echoServAddr, sizeof(echoServAddr))) != echoStringLen)
            printf("sendto() sent a different number of bytes than expected.\n");
//       fromSize = sizeof(fromAddr);
        i++;
    }
    close(sock);
    exit(0);
}