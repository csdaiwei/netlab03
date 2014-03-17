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

#include <pthread.h>

#include "protocol.h"
#include "client.h"
#include "bool.h"
#include "queue.h"

#define SERV_IP "127.0.0.1"
#define SERV_PORT 6666

#define BUF_SIZE 256

char sendbuf[BUF_SIZE];	/*global send buffer, flushed before each send preparations*/
char recvbuf[BUF_SIZE];	/*global receive buffer, flushed before each recieving packets*/
int client_sock; 		/*global client socket, never change after create*/
char username[20];		/*global username, never change after login*/

/*an online friends list, 
 *maintained by the server's SERVICE_ONLINE_NOTICE and SERVICE_OFFLINE_NOTICE 
 *the socket field of each node is always -1. 
 *attention: this variable should be locked on visit*/
struct user_queue *online_friend_queue;

int 
main(void){

	struct sockaddr_in server_addr;//server address
	
	/*client start preparations*/
	if((client_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1){
		printf("error create socket\n");
		exit(-1);
	}	
	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERV_PORT);//the weather server port
	inet_pton(AF_INET, SERV_IP, &server_addr.sin_addr.s_addr);//weather server ip address
	if(connect(client_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1){
		printf("error connect\n");
		exit(-1);
	}
	online_friend_queue = init_user_queue();
	
	//load login page
	login();

	/*after login, the username and current online friends list will be settled down*/
	/*create a thread to receive packets asynchronously */
	pthread_t tid;
	int rc = pthread_create(&tid, NULL, recv_packet_thread, NULL);
	if(rc){
		printf("ERROR creating thread, tid %d, return code %d\n", (int)tid, rc);
		exit(-1);
	}

	/*main thread loads the next page to get user's command*/
	if(system("clear")) ;
	print_prompt_words(username);
	while(true){
		printf("enter >> ");
		char command[20];
		get_keyboard_input(command, 20);
		//printf("debug:%s\n", command);
		if(command[0] != '-'){
			printf("input error, you can try again.\n");
			continue;
		}
		if(strcmp(&command[1], "list") == 0) {
			assert(online_friend_queue -> front != NULL);
			print_online_friends(online_friend_queue);
			continue;

		} else if(strcmp(&command[1], "clear") == 0){
			if(system("clear")) ;
			print_prompt_words(username);
			continue;

		} else if(strcmp(&command[1], "logout") == 0){
			logout();
			break;
		} else{
			printf("input error, you can try again.\n");
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
			if(login_success){
				query_online_all();
				break;
			}
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

/*query for all online friends' names*/
void
query_online_all(){
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

	/*add all user into online_friend_queue*/
	int i;
	for (i = 0; i < response_head -> data_size / 20; ++i){
		struct user_node *pnode = init_user_node(-1, (char *)(response_head + 1) + 20 * i);
		enqueue(online_friend_queue, pnode);
	}
	/*
	printf("online friends' name are as follow:\n");
	int i;
	for (i = 0; i < response_head -> data_size / 20; ++i){
		printf("%s\t", (char *)(response_head + 1) + 20 * i);
		if(((i + 1) % 4) == 0)
			printf("\n");
	}
	if((i % 4) != 0)
		printf("\n");*/
	
}

/*a thread created after login, to receive all packets from server
 *the main thread will no more receive packet after this thread created*/
void *
recv_packet_thread(void *this_is_no_use){

	int n = 0;	//number received 
	while( (n = readvrec(client_sock, recvbuf, BUF_SIZE)) > 0){
		struct im_pkt_head *response_head = (struct im_pkt_head *)recvbuf;
		char *response_data = (char *)(response_head + 1);
		//struct im_pkt_head *response_head = (struct im_pkt_head *)sendbuf;
		//int response_data_size;
		//printf("debug:received a packet from socket:%d, type:%d, service:%d, data size:%d\n"
		//	, client_socket, request_head -> type, request_head -> service, request_head -> data_size);

		memset(sendbuf, 0, sizeof(sendbuf));
		if(response_head -> type != TYPE_RESPONSE){
			printf("Received a error packet, drop it.\n");
			break;
		}
		switch(response_head -> service){
			case SERVICE_LOGIN: ;
				break;
			case SERVICE_LOGOUT: ;
				break;
			case SERVICE_QUERY_ONLINE: ;
				break;
			case SERVICE_SINGLE_MESSAGE: ;
				break;
			case SERVICE_MULTI_MESSAGE: ;
				break;
			case SERVICE_ONLINE_NOTIFY: ;
				char *that_online_username = response_data;
				if(find_user_by_name(online_friend_queue, that_online_username) == NULL){
					struct user_node *n = init_user_node(-1, that_online_username);
					enqueue(online_friend_queue, n);
				}
				/*print this info?*/
				break;
			case SERVICE_OFFLINE_NOTIFY: ;
				char *that_offline_username = response_data;
				delete_user_by_name(online_friend_queue, that_offline_username);
				/*print this info?*/
				break;
			default:
				printf("Received a error packet, drop it.\n");break;
		}
		memset(recvbuf, 0, sizeof(recvbuf));
	}
	if( n < 0)
		printf("Read error\n");
	pthread_exit(NULL);
}




