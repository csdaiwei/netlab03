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
#include <assert.h>
#include <pthread.h>

#include "protocol.h"
#include "server.h"
#include "bool.h"
#include "queue.h"

#define SERV_PORT 6666
#define LISTENQ 8	/*maximum number of client connections*/

#define BUF_SIZE 256	/*enough for 8 users online*/

/*a global socket for server*/
int listenfd;						

/*a global user queue, containing user info *
 *a lock is necessary when any threads read & write it*/
struct user_queue *online_user_queue = NULL; 	

/*mutual exclusion lock*/
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int main(void){

	int connfd;
	socklen_t clilen;
	struct sockaddr_in cliaddr, servaddr;
	int client_num = 0;
	const int on = 1;	/*set socket option for SO_REUSEADDR*/
	
	/*preparations of the server*/
	if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		perror("problem in creating socket"); exit(-1);
	}
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(SERV_PORT);
	bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
	listen(listenfd, LISTENQ);
    setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

	online_user_queue = init_user_queue();

	if(system("clear")) ;
	printf("Server running... waiting for connections.\n");

	/*keep listening to client request after preparations*/
	for( ; ; ){

		//accept a connection
		clilen = sizeof(cliaddr);
		connfd = accept(listenfd, (struct sockaddr *) &cliaddr, &clilen);
		
		char client_ip[16];
		inet_ntop(AF_INET, (void *)&cliaddr.sin_addr.s_addr, client_ip, 16);

		/*create a thread to handle the client*/
		if(client_num < LISTENQ){
			client_num ++;
			pthread_t tid;
			int rc = pthread_create(&tid, NULL, client_handler, (void*)connfd);
			if(rc){
				printf("ERROR creating thread, tid %d, return code %d\n", (int)tid, rc);
				exit(-1);
			}
		} else
			close(connfd);//too much client
	}
	
	destroy_user_queue(online_user_queue);
	close(listenfd);
}

void 
*client_handler(void * connfd){

	int n;	/*number received*/
	char username[20];	//the user's name connecting to this thread. will be filled when login
	int status = -1;	//the user's status
	int client_socket = (int)connfd;	//the user's socket connecting to this thread;

	assert(online_user_queue != NULL);
		
	/*each thread has its own buf*/
	char sendbuf[BUF_SIZE];
	char recvbuf[BUF_SIZE];
	
	/*Receive the request im packet by two steps.
	 *Firstly receive the head field, secondly receive the data field.
	 *Then handle the request packet (usually by a response packet)*/
	while( (n = readvrec(client_socket, recvbuf, BUF_SIZE)) > 0){
		struct im_pkt_head *request_head = (struct im_pkt_head *)recvbuf;
		struct im_pkt_head *response_head = (struct im_pkt_head *)sendbuf;
		char *request_data = (char *)(request_head + 1);
		int response_data_size = 0;

		memset(sendbuf, 0, sizeof(sendbuf));
		if(request_head -> type != TYPE_REQUEST){
			printf("Received a error packet, drop it.\n");
			break;
		}
		
		switch(request_head -> service){
			case SERVICE_LOGIN: ;
				/*response a login response packet, 
				 *it contains only 1 byte data to indicate that login succeeded or failed*/
				response_data_size = 1;
				bool login_result;
				strncpy(username, (char *)(request_head + 1), 20);
				
				//check repeat
				pthread_mutex_lock(&mutex);
				if(find_user_by_name( online_user_queue, username) == NULL){
					struct user_node *pnode = init_user_node(client_socket, username);
					enqueue(online_user_queue, pnode);
					status = ONLINE_STATUS;
					printf("\tuser %s login.\n", username);
					login_result = true;
					/*notify all othre users*/
					on_off_line_notify(SERVICE_ONLINE_NOTIFY, username, sendbuf);
				}
				else{
					login_result = false;
				}
				pthread_mutex_unlock(&mutex);
				construct_im_pkt_head(response_head, TYPE_RESPONSE, SERVICE_LOGIN, response_data_size);
				concat_im_pkt_data(response_head, (char *)&login_result);
				send(client_socket, sendbuf, IM_PKT_HEAD_SIZE + response_data_size, 0);
				break;
			case SERVICE_LOGOUT: ;
				/*nothing need to do here.*/
				break;
			case SERVICE_QUERY_ONLINE: ;
				/*copy all usernames into the data field of the response packet*/
				pthread_mutex_lock(&mutex);
				response_data_size = 20 * copy_all_user_name((char *)(response_head + 1), online_user_queue);
				pthread_mutex_unlock(&mutex);
				construct_im_pkt_head(response_head, TYPE_RESPONSE, SERVICE_QUERY_ONLINE, response_data_size);
				/*send the packet*/
				send(client_socket, sendbuf, IM_PKT_HEAD_SIZE + response_data_size, 0);
				break;
			case SERVICE_SINGLE_MESSAGE: ;
				/*simply resend the packet to the recipient*/
				char *recipient = request_data + 20;
				struct user_node *recipient_node = find_user_by_name(online_user_queue, recipient); 
				if(recipient_node != NULL){
					response_data_size = request_head -> data_size;
					construct_im_pkt_head(response_head, TYPE_RESPONSE, SERVICE_SINGLE_MESSAGE, response_data_size);
					concat_im_pkt_data(response_head, request_data);
					send(recipient_node -> socket, sendbuf,IM_PKT_HEAD_SIZE + response_data_size, 0);
				}else
					printf("Error, no recipient %s. drop the packet\n", recipient);
				break;
			case SERVICE_MULTI_MESSAGE: ;
				response_data_size = request_head -> data_size;
				construct_im_pkt_head(response_head, TYPE_RESPONSE, SERVICE_MULTI_MESSAGE, response_data_size);
				concat_im_pkt_data(response_head, request_data);
				char *sender = request_data;
				/*send to all online users except this message's sender*/
				for(recipient_node = online_user_queue -> front; recipient_node != NULL; recipient_node = recipient_node -> next)
					if(strcmp(recipient_node -> username, sender) != 0)
						send(recipient_node -> socket, sendbuf,IM_PKT_HEAD_SIZE + response_data_size, 0);
				break;
			default:
				printf("Received a error packet, drop it.\n");break;
		}
		memset(recvbuf, 0, sizeof(recvbuf));
	}
	if( n < 0)
		printf("Read error\n");

	if(status == ONLINE_STATUS){
		/*remove logout or disconnected  users from the queue*/
		on_off_line_notify(SERVICE_OFFLINE_NOTIFY, username, sendbuf);
		pthread_mutex_lock(&mutex);
		delete_user_by_name(online_user_queue, username);
		pthread_mutex_unlock(&mutex);
		status = -1;
		printf("\tuser %s logout\n", username);
	}
	pthread_exit(NULL);
}

/*notify all others when a user login/logout*/
void 
on_off_line_notify(int SERVICE_ON_OFF, char *username, char *sendbuf){

	assert((SERVICE_ON_OFF == SERVICE_ONLINE_NOTIFY) ||
		(SERVICE_ON_OFF == SERVICE_OFFLINE_NOTIFY));

	/*build a notif packet*/
	int response_data_size = 20;
	struct im_pkt_head *response_head = (struct im_pkt_head *)sendbuf;
	memset(sendbuf, 0, sizeof(sendbuf));
	construct_im_pkt_head(response_head, TYPE_RESPONSE, SERVICE_ON_OFF, response_data_size);
	concat_im_pkt_data(response_head, username);
	//send(client_socket, sendbuf, IM_PKT_HEAD_SIZE + response_data_size, 0);

	/*send to all other user*/
	struct user_node *n;
	for(n = online_user_queue -> front; n != NULL; n = n -> next){
		if(strcmp(n -> username, username) != 0)
			send(n -> socket, sendbuf, IM_PKT_HEAD_SIZE + response_data_size, 0);
	}
}