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
int client_sock; 		/*global client socket*/

int 
main(void){

	struct sockaddr_in server_addr;//server address
	
	/*create socket of TCP connection*/
	if((client_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		printf("error create socket\n");
		exit(-1);
	}	

	/*fill in the server address*/
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERV_PORT);//the weather server port
	inet_pton(AF_INET, SERV_IP, &server_addr.sin_addr.s_addr);//weather server ip address

	/*try to connect*/
	if(connect(client_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1){
		printf("error connect\n");
		exit(-1);
	}
	
	
	login(client_sock);
	//printf("login success\n");
	for( ; ; ){
		//to be completed	
	}

	/*close the tcp connection before exit*/
	close(client_sock);
	return 0;
}


/*send a login request to server*/
void 
login(int client_sock){

	char username[20];

	if(system("reset"));//unused warnning

	printf(	"Hello, this is an IM program.\n" 
			"You can pick a nickname and enter it below to login.\n"
			"======================================================================\n");
	while(true){
		printf("Your name (no more than 15 characters):");
		get_keyboard_input(username, 20);


		/*construct a login request packet and send it*/
		memset(sendbuf, 0, sizeof(sendbuf));
		struct im_pkt *login_pkt = (struct im_pkt *)sendbuf;
		login_pkt -> service = SERVICE_LOGIN;
		struct login_request_data *req = (struct login_request_data *)login_pkt -> data; 
		strncpy(req -> username, username, 20);
		send(client_sock, login_pkt, IM_PKT_SIZE, 0);

		/*parse the response packet*/
		memset(recvbuf, 0 ,sizeof(recvbuf));
		readn(client_sock, recvbuf, IM_PKT_SIZE);
		login_pkt = (struct im_pkt *)recvbuf;
		if(login_pkt -> service == SERVICE_LOGIN){
			struct login_response_data *resp = (struct login_response_data *)login_pkt -> data;
			if(resp -> login_success)
				break;
		}
		printf(	"Sorry, the name %s seems to have been taken, you can pick another one.\n\n", username);
	}
}
