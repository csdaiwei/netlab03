#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

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
char username[20];		/*global username, never change after login*/

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
	
	//load login page
	login();
	/*after login, the username will be settled down
	 *then load next page*/
	if(system("clear")) ;
	print_prompt_words(username);
	while(true){
		char command[20];
		get_keyboard_input(command, 20);
		//printf("debug:%s\n", command);
		if(command[0] != '-'){
			printf("input error, pls re-enter:");
			continue;
		}
		if(strcmp(&command[1], "list") == 0) {
			query_online();
			continue;

		} else if(strcmp(&command[1], "clear") == 0){
			if(system("clear")) ;
			print_prompt_words(username);
			continue;

		} else if(strcmp(&command[1], "logout") == 0){
			logout();
			break;
		} else{
			printf("input error, pls re-enter:");
			continue;	
		}
	}

	/*close the tcp connection before exit*/
	close(client_sock);
	return 0;
}



/*send a login request to server*/
void 
login(){

	if(system("clear"));//unused warnning

	printf(	"Hello, this is an IM program.\n" 
			"You can pick a nickname and enter it below to login.\n"
			"======================================================================\n");
	while(true){
		printf("Your name (no more than 15 characters):");
		get_keyboard_input(username, 20);


		/*construct a login request packet and send it*/
		memset(sendbuf, 0, sizeof(sendbuf));
		unsigned short data_size = 20;	//request im packet data size
		construct_im_pkt_head((struct im_pkt_head *)sendbuf, TYPE_REQUEST, SERVICE_LOGIN, data_size);
		concat_im_pkt_data((struct im_pkt_head *)sendbuf, username);
		send(client_sock, sendbuf, IM_PKT_HEAD_SIZE + data_size, 0);

		/*get and parse the response packet*/
		memset(recvbuf, 0 ,sizeof(recvbuf));
		readvrec(client_sock, recvbuf, BUF_SIZE);
		struct im_pkt_head *response_head = (struct im_pkt_head *)recvbuf;
		if(response_head -> service == SERVICE_LOGIN){
			char *response_data = (char *)(response_head + 1);//right after the head
			bool login_success = (bool) response_data[0];
			if(login_success)
				break;
			else{	//login falied. name repeat
				printf(	"Sorry, the name %s seems to have been taken, you can pick another one.\n\n", username);
				continue;
			}
		}
		printf("Sorry ,the server seems not work properly, you can try again\n");	
	}
}


void
logout(){

	/*build a logout request packet and send it*/
	memset(sendbuf, 0, sizeof(sendbuf));
	int data_size = 0;
	construct_im_pkt_head((struct im_pkt_head *)sendbuf, TYPE_REQUEST, SERVICE_LOGOUT, data_size);
	concat_im_pkt_data((struct im_pkt_head *)sendbuf, NULL);
	send(client_sock, sendbuf, IM_PKT_HEAD_SIZE + data_size, 0);
	/*printf("debug:");
	int i;
	for(i = 0; i < IM_PKT_HEAD_SIZE + data_size; i++)
		printf("%02x", sendbuf[i]);*/
}

void
query_online(){
	/*build a query packet*/
	memset(sendbuf, 0, sizeof(sendbuf));
	int data_size = 0;
	construct_im_pkt_head((struct im_pkt_head *)sendbuf, TYPE_REQUEST, SERVICE_QUERY_ONLINE, data_size);
	concat_im_pkt_data((struct im_pkt_head *)sendbuf, NULL);
	send(client_sock, sendbuf, IM_PKT_HEAD_SIZE + data_size, 0);

	/*get and parse the response packet*/
	memset(recvbuf, 0 ,sizeof(recvbuf));
	readvrec(client_sock, recvbuf, BUF_SIZE);
	struct im_pkt_head *response_head = (struct im_pkt_head *)recvbuf;
	assert((response_head -> type == TYPE_RESPONSE) && 
		(response_head -> service == SERVICE_QUERY_ONLINE));
	printf("online friends' name are as follow:\n");
	int i;
	for (i = 0; i < response_head -> data_size / 20; ++i){
		printf("%s\t", (char *)(response_head + 1) + 20 * i);
		if(((i + 1) % 4) == 0)
			printf("\n");
	}
	if((i % 4) != 0)
		printf("\n");
	
}