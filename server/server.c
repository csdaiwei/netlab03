/*reference : lecture P152, TCP concurrent echo server 
 **/

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>	//readn errno
#include <time.h>	//system time

#include "server.h"
#include "bool.h"

#define SERV_PORT 6666
#define LISTENQ 8	/*maximum number of client connections*/


char recvbuf[BUFFER_SIZE];
char sendbuf[BUFFER_SIZE];

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
			while( (n = readn(connfd, recvbuf, 33)) == 33){
				
				
			}
			if( n < 0)
				printf("Read error\n");
			printf("Client from %s quit...A child process end\n", client_ip);
			exit(0);
		}

		close(connfd);
	}

}


/* readn - read exactly n bytes */
int readn( int sock_fd, char *bp, size_t len)
{
	int cnt;
	int rc;

	cnt = len;
	while ( cnt > 0 )
	{
		rc = recv( sock_fd, bp, cnt, 0 );
		if ( rc < 0 )				/* read error? */
		{
			if ( errno == EINTR )	/* interrupted? */
				continue;			/* restart the read */
			return -1;				/* return error */
		}
		if ( rc == 0 )				/* EOF? */
			return len - cnt;		/* return short count */
		bp += rc;
		cnt -= rc;
	}
	return len;
}

