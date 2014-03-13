#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "client.h"
#include "bool.h"

#include <errno.h>	//readn errno
#include <arpa/inet.h>	//inet_addr()
#include <netinet/in.h>	//sockaddr_in
#include <unistd.h>	//close()
#include <sys/socket.h>    
#include <sys/types.h>

#define SERV_IP "127.0.0.1"
#define SERV_PORT 6666


int 
main(void){

	int client_sock;
	struct sockaddr_in server_addr;//server address
	
	/*create socket of TCP connection*/
	if((client_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		printf("error create socket");
		exit(-1);
	}	

	/*fill in the server address*/
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERV_PORT);//the weather server port
	inet_pton(AF_INET, SERV_IP, &server_addr.sin_addr.s_addr);//weather server ip address

	/*try to connect*/
	if(connect(client_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1){
		printf("error connect");
		exit(-1);
	}
	
	/*code here*/

	/*close the tcp connection before exit*/
	close(client_sock);
	return 0;
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

