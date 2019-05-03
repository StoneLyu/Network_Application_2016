#include<arpa/inet.h>
#include<stdio.h>
#include<stdlib.h>
#include<bits/socket.h>
#include <netdb.h>
#include <sys/socket.h>



int main(int argc, char **argv)
{
    char *ptr,**pptr;
    struct hostent *hptr;
    char str[32];
   
    /* get domain name*/
    ptr = argv[1];

    /* use gethostbyname(),store it in hptr */
    if((hptr = gethostbyname(ptr)) == NULL)
    {
        printf("gethostbyname error for host:%s\n", ptr);
        return 0; /* gethostbyname£¬1 for wrong */
    }
    /* print official name */
    printf("official hostname:%s\n",hptr->h_name);
    /* print aliases name */
    for(pptr = hptr->h_aliases; *pptr != NULL; pptr++)
        printf("  alias:%s\n",*pptr);
    /* print address type */
    switch(hptr->h_addrtype)
    {
        case AF_INET:
        case AF_INET6:
            pptr=hptr->h_addr_list;
            /* inet_ntop(),2 to dot */
            for(; *pptr !=NULL;pptr++)
            printf("  address:%s\n", inet_ntop(hptr->h_addrtype, *pptr, str, sizeof(str)));
        break;
        default:
            printf("unknown address type\n");
        break;
    }
    return 0;
}