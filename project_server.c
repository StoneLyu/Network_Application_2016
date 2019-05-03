#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <fcntl.h>

#define MAXSIZE 1024

int main(int argc, const char *argv[])
{

	char *host = "192.168.56.1";
	char *wellKnownPort = "21";

	int proxy_cmd_socket = 0;	//proxy listen socket
	int accept_cmd_socket = 0;   //proxy accept socket
	int connect_cmd_socket = 0;  //proxy connect socket
	int proxy_data_socket = 0;   //proxy listen socket
	int accept_data_socket = 0;  //proxy accept socket
	int connect_data_socket = 0; //proxy connect socket

	int selectResult = 0; //return value of select
	int select_sd = 10;   //range of file description in select()

	char filename[MAXSIZE];
	int choiceFlag; //choice flag of 0 or 1, 0 is upload
	int sendcache = 0;
	;				   //send cache or not, 0 is not send
	int cachemode = 0; //cached or not, 0 is not cached
	int filefd;		   //fd for file operation
	int modeflag;	  //0 is active, 1 is passive
	int dataport;	  //port number of opposite

	fd_set master_set, working_set; //set of file description
	FD_ZERO(&master_set);			//clear master_set

	struct timeval timeout;			  //structure of time in select()
	bzero(&timeout, sizeof(timeout)); //set timeout printer to all 0

	//bind() and listen() for proxy_cmd_socket
	if ((proxy_cmd_socket = socket(PF_INET, SOCK_STREAM, 0)) < 0)
	{
		printf("socket() failed.\n");
		exit(0);
	}

	struct sockaddr_in IPAddr;
	memset(&IPAddr, 0, sizeof(IPAddr));
	IPAddr.sin_family = AF_INET;
	IPAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	IPAddr.sin_port = htons(21);

	if ((bind(proxy_cmd_socket, (struct sockaddr *)&IPAddr, sizeof(IPAddr))) < 0)
	{
		printf("bind() failed.\n");
		exit(0);
	}

	if (listen(proxy_cmd_socket, 10) < 0)
	{
		printf("listen error.\n");
		exit(0);
	}

	FD_SET(proxy_cmd_socket, &master_set);

	timeout.tv_sec = 120; //timeput time of select()
	timeout.tv_usec = 0;  //ms

	while (1)
	{
		int i;
		FD_ZERO(&working_set); //clear master_set
		memcpy(&working_set, &master_set, sizeof(master_set));

		selectResult = select(select_sd, &working_set, NULL, NULL, &timeout);

		// fail
		if (selectResult < 0)
		{
			printf("select() failed\n");
			exit(0);
		}

		// timeout
		if (selectResult == 0)
		{
			printf("select() timed out.\n");
			continue;
		}

		for (i = 0; i < select_sd; i++)
		{
			if (FD_ISSET(i, &working_set))
			{
				//decide whether the changing socket is in the working_set
				if (i == proxy_cmd_socket)
				{

					struct sockaddr_in addr_pcmd;
					int addr_pcmd_len;
					addr_pcmd_len = sizeof(addr_pcmd);
					accept_cmd_socket = accept(proxy_cmd_socket, (struct sockaddr *)&addr_pcmd, &addr_pcmd_len);

					if (accept_cmd_socket < 0)
					{
						printf("accept() failed.\n");
						exit(0);
					}

					if ((connect_cmd_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
					{
						printf("socket() failed.\n");
						exit(0);
					}

					struct sockaddr_in addr_ccmd;
					memset(&addr_ccmd, 0, sizeof(addr_ccmd));
					addr_ccmd.sin_family = AF_INET;
					addr_ccmd.sin_addr.s_addr = inet_addr(host);
					addr_ccmd.sin_port = htons(atoi(wellKnownPort));

					if (connect(connect_cmd_socket, (struct sockaddr *)&addr_ccmd, sizeof(addr_ccmd)) < 0)
					{
						printf("connect() failed.\n");
					}

					printf("connect to server successfully\n");

					FD_SET(accept_cmd_socket, &master_set);
					FD_SET(connect_cmd_socket, &master_set);
				}

				if (i == accept_cmd_socket)
				{
					char buff[MAXSIZE];
					int nread;
					sleep(5);
					if ((nread = read(i, buff, MAXSIZE - 1)) == 0)
					{
						close(i); //if can't get any infomation, then shut down Socket
						close(connect_cmd_socket);
						//after socket closed, FD_CLR will remove socket out from master_set, so that select() will not monitor the closed socket
						FD_CLR(i, &master_set);
						FD_CLR(connect_cmd_socket, &master_set);
					}
					else
					{
						nread = read(i, buff, MAXSIZE - 1);
						buff[nread] = '\0';
						printf("accept_cmd_socket() successfully.\n");

						//PORT
						if (strncmp(buff, "PORT", 4) == 0)
						{
							char *j;
							int p1, p2;

							j = strrchr(buff, ',');
							p1 = atoi(j + 1);
							*j = '\0';
							j = strrchr(buff, ',');
							p2 = atoi(j + 1);
							sprintf(buff, "PORT 192,168,56,101,%d,%d\r\n", p1, p2);
							nread = strlen(buff);

							modeflag = 0; //active

							dataport = p1 * 256 + p2;

							if (FD_ISSET(proxy_data_socket, &master_set))
							{
								close(proxy_data_socket);
								FD_CLR(proxy_data_socket, &master_set);
							}

							if ((proxy_data_socket = socket(PF_INET, SOCK_STREAM, 0)) < 0)
							{
								printf("p_d_s socket() failed.\n");
								exit(0);
							}

							//bind() and listen() for proxy_data_socket
							struct sockaddr_in addr_pdata;
							memset(&addr_pdata, 0, sizeof(addr_pdata));
							addr_pdata.sin_family = AF_INET;
							addr_pdata.sin_addr.s_addr = htonl(INADDR_ANY);
							addr_pdata.sin_port = htons(dataport);

							if ((bind(proxy_data_socket, (struct sockaddr *)&addr_pdata, sizeof(addr_pdata))) < 0)
							{
								printf("p_d_s bind() failed.\n");
								exit(0);
							}
							if (listen(proxy_data_socket, 10) < 0)
							{
								printf("listen error.\n");
								exit(0);
							}

							FD_SET(proxy_data_socket, &master_set);
						}

						//PASV
						if (strncmp(buff, "PASV", 4) == 0)
						{
							modeflag = 1;
						}

						//RETR
						if (strncmp(buff, "RETR", 4) == 0)
						{
							choiceFlag = 1;
							buff[nread] = '\0';
							strcpy(filename, strtok(buff, "\r"));

							int openfd, j;
							openfd = open(filename, O_RDONLY);
							if (openfd < 0)
							{
								close(openfd);
								j = 0;
							}
							else
							{
								close(openfd);
								j = 1;
							}

							if (j == 1)
							{
								printf("file cached\n");
								if (modeflag == 1)
								{				   //passive
									cachemode = 1; //do not need to cache
								}
								if (modeflag == 0)
								{								  //active
									char openDataConnection[50];  //info of 150 response
									char closeDataConnection[50]; //info of 226 response

									sprintf(openDataConnection, "150 Opening data channel for \"/%s\"\r\n", filename);
									write(accept_cmd_socket, openDataConnection, strlen(openDataConnection));

									if ((connect_data_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
									{
										printf("c_d_s socket() failed\n");
										exit(0);
									}

									struct sockaddr_in addr_cdata;
									memset(&addr_cdata, 0, sizeof(addr_cdata));
									addr_cdata.sin_family = AF_INET;
									addr_cdata.sin_addr.s_addr = inet_addr(host);
									addr_cdata.sin_port = htons(dataport);

									if (connect(connect_data_socket, (struct sockaddr *)&addr_cdata, sizeof(addr_cdata)) < 0)
									{
										printf("c_d_s connect() failed.\n");
									}
									printf("c_d_s connect() successfully.\n");

									int sendfd;
									sendfd = open(filename, O_RDONLY);
									if (sendfd < 0)
									{
										printf("open() faild.\n");
										exit(0);
									}
									else
									{
										char content[MAXSIZE];
										int nread1;
										while ((nread1 = read(sendfd, content, MAXSIZE - 1)) > 0)
										{
											write(connect_data_socket, content, nread1);
										}
										close(sendfd);
									}

									sprintf(closeDataConnection, "226 Successfully transferred \"/%s\"\r\n", filename);
									write(accept_cmd_socket, closeDataConnection, strlen(closeDataConnection));
									close(connect_data_socket);
								}
								continue;
							}
							else
							{
								cachemode = 0;
							}
						}

						//STOR
						if (strncmp(buff, "STOR", 4) == 0)
						{
							choiceFlag = 0; //upload
						}

						write(connect_cmd_socket, buff, nread);
						buff[nread] = '\0';
						printf("handle client CMD successfully.\n");
					}

					if (i == connect_cmd_socket)
					{
						char buff[MAXSIZE];
						int nread2;
						nread2 = read(i, buff, MAXSIZE - 1);
						if (nread2 == 0)
						{
							close(i);
							close(accept_cmd_socket);

							FD_CLR(i, &master_set);
							FD_CLR(accept_cmd_socket, &master_set);
						}
						else
						{
							buff[nread2] = '\0';
							printf("get server CMD successfully.\n");
							//227
							if (strncmp(buff, "227", 3) == 0)
							{
								char *j;
								int p1, p2;

								j = strrchr(buff, ',');
								p1 = atoi(j + 1);
								*j = '\0';
								j = strrchr(buff, ',');
								p2 = atoi(j + 1);

								char pasvResponse[100];
								sprintf(pasvResponse, "227 Entering Passive Mode (192,168,56,101,%d,%d)\r\n", p1, p2);

								int responseLen;
								responseLen = strlen(buff);
								modeflag = 1;
								dataport = p1 * 256 + p2;
								if (FD_ISSET(proxy_data_socket, &master_set))
								{
									close(proxy_data_socket);
									FD_CLR(proxy_data_socket, &master_set);
								}

								if ((proxy_data_socket = socket(PF_INET, SOCK_STREAM, 0)) < 0)
								{
									printf("p_d_s socket() failed.\n");
									exit(0);
								}

								struct sockaddr_in addr_pdata;
								memset(&addr_pdata, 0, sizeof(addr_pdata));
								addr_pdata.sin_family = AF_INET;
								addr_pdata.sin_addr.s_addr = htonl(INADDR_ANY);
								addr_pdata.sin_port = htons(dataport);

								if ((bind(proxy_data_socket, (struct sockaddr *)&addr_pdata, sizeof(addr_pdata))) < 0)
								{
									printf("p_d_s bind() failed\n");
									exit(0);
								}

								if (listen(proxy_data_socket, 10) < 0)
								{
									printf("listen error.\n");
									exit(0);
								}

								FD_SET(proxy_data_socket, &master_set);
							}
						}
						write(accept_cmd_socket, buff, nread2);

						printf("handle server cmd successfully.\n");
					}
				}

				if (i == proxy_data_socket)
				{
					if (modeflag == 0)
					{ //active
						struct sockaddr_in addr_adata;
						int addr_adata_len;
						addr_adata_len = sizeof(addr_adata);
						accept_data_socket = accept(proxy_data_socket, (struct sockaddr *)&addr_adata, &addr_adata_len);
						if (accept_data_socket < 0)
						{
							printf("a_d_s accept() failed.\n");
							exit(0);
						}

						if ((connect_data_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
						{
							printf("c_d_s socket() failed.\n");
							exit(0);
						}

						struct sockaddr_in addr_cdata2;
						memset(&addr_cdata2, 0, sizeof(addr_cdata2));
						addr_cdata2.sin_family = AF_INET;
						addr_cdata2.sin_addr.s_addr = inet_addr(host);
						addr_cdata2.sin_port = htons(dataport);

						if (connect(connect_data_socket, (struct sockaddr *)&addr_cdata2, sizeof(addr_cdata2)) < 0)
						{
							printf("connect() failed\n");
						}

						printf("connect to c_d_s successfully.\n");

						FD_SET(accept_data_socket, &master_set);
						FD_SET(connect_data_socket, &master_set);

						//cache
						char *extension;
						int k, openfd, j;
						if (strstr(filename, ".") == NULL)
							k = 0;
						if (choiceFlag == 0) //
							k = 0;

						strtok(filename, ".");
						extension = strtok(NULL, ".");

						if (strcmp(extension, "jpg") == 0 || strcmp(extension, "pdf") == 0)
						{
							openfd = open(filename, O_RDONLY);

							if (openfd < 0)
							{
								close(openfd);
								j = 0;
							}
							else
							{
								close(openfd);
								j = 1;
							}
							if (j == 1)
							{
								k = 1;
							}
						}

						if (k == 1)
						{ //do cache
							sendcache = 1;
							filefd = creat(filename, 0644);
							if (filefd < 0)
							{
								printf("create() faild.\n");
								exit(0);
							}
							printf("Create cache file successfully.\n");
						}
						else
						{
							sendcache = 0;
						}
					}

					if (modeflag == 1)
					{
						if (choiceFlag == 0)
						{ //upload
							struct sockaddr_in addr_adata2;
							int addr_data_len2;
							addr_data_len2 = sizeof(addr_adata2);
							accept_data_socket = accept(proxy_data_socket, (struct sockaddr *)&addr_adata2, &addr_data_len2);

							if (accept_data_socket < 0)
							{
								printf("a_d_s accept() failed\n");
								exit(0);
							}

							if ((connect_data_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
							{
								printf("c_d_s socket() failed\n");
								exit(0);
							}

							struct sockaddr_in addr_cdata3;
							memset(&addr_cdata3, 0, sizeof(addr_cdata3));
							addr_cdata3.sin_family = AF_INET;
							addr_cdata3.sin_addr.s_addr = inet_addr(host);
							addr_cdata3.sin_port = htons(dataport);

							if (connect(connect_data_socket, (struct sockaddr *)&addr_cdata3, sizeof(addr_cdata3)) < 0)
							{
								printf("connect() failed\n");
							}

							printf("connect to c_d_s.\n");

							FD_SET(accept_data_socket, &master_set);
							FD_SET(connect_data_socket, &master_set);
						}

						if (choiceFlag == 1)
						{ //download
							if (cachemode == 1)
							{ //cached
								cachemode = 0;
								char openDataConnection2[50];
								int sendfd2;

								sprintf(openDataConnection2, "150 Opening data channel for \"/%s\"\r\n", filename);

								struct sockaddr_in addr_adata3;
								int addr_adata_len2;
								char content2[MAXSIZE];

								addr_adata_len2 = sizeof(addr_adata3);
								accept_data_socket = accept(proxy_data_socket, (struct sockaddr *)&addr_adata3, &addr_adata_len2);
								if (accept_data_socket < 0)
								{
									printf("a_d_s accept() failed\n");
									exit(0);
								}

								write(accept_cmd_socket, openDataConnection2, strlen(openDataConnection2));

								sendfd2 = open(filename, O_RDONLY);
								if (sendfd2 < 0)
								{
									printf("open() faild.\n");
									exit(0);
								}
								else
								{
									int nread3;
									while ((nread3 = read(sendfd2, content2, MAXSIZE - 1)) > 0)
									{
										write(accept_data_socket, content2, nread3);
									}
									close(sendfd2);
								}

								char successesResponse[50];
								sprintf(successesResponse, "226 Successfully transferred \"/%s\"\r\n", filename);
								write(accept_cmd_socket, successesResponse, strlen(successesResponse));
								close(accept_data_socket);
							}
							else
							{ //not cached
								struct sockaddr_in addr_adata4;
								int addr_adata_len4, j, k, openfd2;
								char *extension2;

								addr_adata_len4 = sizeof(addr_adata4);
								accept_data_socket = accept(proxy_data_socket, (struct sockaddr *)&addr_adata4, &addr_adata_len4);

								if (accept_data_socket < 0)
								{
									printf("a_d_s accept() failed\n");
									exit(0);
								}

								if ((connect_data_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
								{
									printf("c_d_s socket() failed\n");
									exit(0);
								}

								struct sockaddr_in addr_cdata4;
								memset(&addr_cdata4, 0, sizeof(addr_cdata4));
								addr_cdata4.sin_family = AF_INET;
								addr_cdata4.sin_addr.s_addr = inet_addr(host);
								addr_cdata4.sin_port = htons(dataport);

								if (connect(connect_data_socket, (struct sockaddr *)&addr_cdata4, sizeof(addr_cdata4)) < 0)
								{
									printf("c_d_s connect() failed\n");
								}

								printf("connect to c_d_s.\n");

								FD_SET(accept_data_socket, &master_set);
								FD_SET(connect_data_socket, &master_set);

								if (strstr(filename, ".") == NULL)
									k = 0;
								if (choiceFlag == 0)
									k = 0;

								strtok(filename, ".");
								extension2 = strtok(NULL, ".");

								if (strcmp(extension2, "jpg") == 0 || strcmp(extension2, "pdf") == 0)
								{
									openfd2 = open(filename, O_RDONLY);
									if (openfd2 < 0)
									{
										close(openfd2);
										j = 0;
									}
									else
									{
										close(openfd2);
										j = 1;
									}
									if (j = 1)
									{
										k = 1;
									}
								}

								if (k = 1)
								{
									sendcache = 1; //send cache
									filefd = creat(filename, 0644);
									if (filefd < 0)
									{
										printf("create() faild.\n");
										exit(0);
									}
									printf("Create file for cache.\n");
								}
								else
								{
									sendcache = 0;
								}
							}
						}
					}
				}

				if (i == accept_data_socket)
				{
					char buff[MAXSIZE];
					int nread4;
					nread4 = read(i, buff, MAXSIZE - 1);

					if (nread4 == 1)
					{
						write(connect_data_socket, buff, nread4);
						buff[nread4] = '\0';
						if (sendcache)
						{
							write(filefd, buff, nread4);
						}
					}
					else
					{
						close(i);
						close(connect_data_socket);
						FD_CLR(accept_data_socket, &master_set);
						FD_CLR(connect_data_socket, &master_set);
						if (sendcache)
						{
							close(filefd);
						}
					}
				}

				if (i == connect_data_socket)
				{
					char buff[MAXSIZE];
					int nread5;
					nread5 = read(i, buff, MAXSIZE - 1);

					if (nread5 == 1)
					{
						write(accept_data_socket, buff, nread5);
						buff[nread5] = '\0';
						if (sendcache)
						{
							write(filefd, buff, nread5);
						}
					}
					else
					{
						close(i);
						close(accept_data_socket);
						FD_CLR(accept_data_socket, &master_set);
						FD_CLR(connect_data_socket, &master_set);
						if (sendcache)
						{
							close(filefd);
						}
					}
				}
			}
		}
	}
	return 0;
}
