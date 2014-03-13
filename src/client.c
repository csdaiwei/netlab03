#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <errno.h>	//readn errno
#include <arpa/inet.h>	//inet_addr()
#include <netinet/in.h>	//sockaddr_in
#include <unistd.h>	//close()
#include <sys/socket.h>    
#include <sys/types.h>

#include "protocol.h"
#include "client.h"
#include "bool.h"

#define SERV_IP "127.0.0.1"
#define SERV_PORT 6666

#define BUF_SIZE 256

char sendbuf[BUF_SIZE];
char recvbuf[BUF_SIZE];

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
	
	/*send a packet to server to tell it my name*/
	memset(sendbuf, 0, BUF_SIZE);
	struct request_packet *request = (struct request_packet *)sendbuf;		
	request -> request_type = 0x00; //login request
	strncpy(request -> sender_name, "David", 19);
	request -> sender_name[19] = '\0';	
	send(client_sock, request, REQUEST_PACKET_SIZE, 0);
	printf("successful login\n");	
	while(1){
		//waiting	
	}

	/*close the tcp connection before exit*/
	close(client_sock);
	return 0;
}


