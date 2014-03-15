/*reference : lecture P152, TCP concurrent echo server 
 **/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>	//pton
#include <unistd.h>	//close
#include <time.h>	//system time

#include "protocol.h"
#include "server.h"
#include "bool.h"

#define SERV_PORT 6666
#define LISTENQ 8	/*maximum number of client connections*/

#define BUF_SIZE 256

char sendbuf[BUF_SIZE];
char recvbuf[BUF_SIZE];


int main(void){
	int listenfd, connfd, n;
	pid_t childpid;
	socklen_t clilen;
	struct sockaddr_in cliaddr, servaddr;

	/*create a socket*/
	if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		perror("problem in creating socket");
		exit(2);
	}

	/*preparation of the socket address*/
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(SERV_PORT);

	/*bind the socket*/
	bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

	/*listen to the socket by creating a connection queue , then wait for clients*/
	listen(listenfd, LISTENQ);

	printf("Server running... waiting for connections.\n");

	while(true){
	
		clilen = sizeof(cliaddr);
		//accept a connection
		connfd = accept(listenfd, (struct sockaddr *) &cliaddr, &clilen);
			
		char client_ip[20];
		inet_ntop(AF_INET, &cliaddr.sin_addr.s_addr, client_ip, 16);		
	
		printf("Received request from %s ...\n", client_ip);

		if( (childpid = fork()) == 0 ){ //if it's 0, it's child process
	
			printf("Child created for dealing with client requests\n");
			
			//close listening socket
			close(listenfd);
			while( (n = readn(connfd, recvbuf, IM_PKT_SIZE)) == IM_PKT_SIZE){
				
				/*handle the request packet (usually by a response packet)*/
				struct im_pkt *request_pkt = (struct im_pkt *)recvbuf;
				struct im_pkt *response_pkt = (struct im_pkt *)sendbuf;
				memset(sendbuf, 0, sizeof(sendbuf));

				switch(request_pkt -> service){
					case SERVICE_LOGIN: ;
						/*check the username repeat or not, then response*/
						struct login_request_data *req = (struct login_request_data *)request_pkt -> data;
						struct login_response_data *resp = (struct login_response_data *)response_pkt -> data;
						if(strcmp(req -> username, "david") == 0)
							resp -> login_success = true;
						else
							resp -> login_success = false;
						send(connfd, response_pkt, IM_PKT_SIZE, 0);
						break;
					default:
						printf("Received a error packet, drop it.\n");break;
				}
				memset(recvbuf, 0, sizeof(recvbuf));
			}
			if( n < 0)
				printf("Read error\n");
			printf("Client from %s quit...A child process end\n", client_ip);
			exit(0);
		}

		close(connfd);
	}

}




