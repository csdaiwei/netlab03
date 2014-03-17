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
		//printf("debug:client ip:%s, socket:%d\n", client_ip, connfd);

		/*create a thread to handle the client*/
		pthread_t tid;
		int rc = pthread_create(&tid, NULL, client_handler, (void*)connfd);
		if(rc){
			printf("ERROR creating thread, tid %d, return code %d\n", (int)tid, rc);
			exit(-1);
		}
	}
	//end of server main
	destroy_user_queue(online_user_queue);
}

void *client_handler(void * connfd){

	int n;	/*number received*/
	char username[20];	//the user's name connecting to this thread. will be filled when login
	int status = -1;	//the user's status
	int client_socket = (int)connfd;	//the user's socket connecting to this thread;
	//debug://long tid = pthread_self();

	assert(online_user_queue != NULL);
		
	/*each thread has its own buf*/
	char sendbuf[BUF_SIZE];
	char recvbuf[BUF_SIZE];
	
	//printf("debug:thread %lu created for dealing with client requests\n", tid);

	/*Receive the request im packet by two steps.
	 *Firstly receive the head field, secondly receive the data field.
	 *Then handle the request packet (usually by a response packet)*/
	while( (n = readvrec(client_socket, recvbuf, BUF_SIZE)) > 0){
		struct im_pkt_head *request_head = (struct im_pkt_head *)recvbuf;
		struct im_pkt_head *response_head = (struct im_pkt_head *)sendbuf;
		int response_data_size;
		//printf("debug:received a packet from socket:%d, type:%d, service:%d, data size:%d\n"
		//	, client_socket, request_head -> type, request_head -> service, request_head -> data_size);

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
				}
				else{
					login_result = false;
				}
				pthread_mutex_unlock(&mutex);
				//printf("debug: thread %lu release lock\n", tid);
				construct_im_pkt_head(response_head, TYPE_RESPONSE, SERVICE_LOGIN, response_data_size);
				concat_im_pkt_data(response_head, (char *)&login_result);
				send(client_socket, sendbuf, IM_PKT_HEAD_SIZE + response_data_size, 0);
				//printf("debug:response packet send to socket:%d, type:%d, service:%d, data size:%d\n"
				//	, client_socket, response_head -> type, response_head -> service, response_head -> data_size);
				break;
			case SERVICE_LOGOUT: ;
				/*need to do nothing*/
				break;
			case SERVICE_QUERY_ONLINE: ;
				printf("\tuser %s query for online friends\n", username);
				pthread_mutex_lock(&mutex);
				response_data_size = 20 * copy_all_user_name((char *)(response_head + 1), online_user_queue);
				pthread_mutex_unlock(&mutex);
				construct_im_pkt_head(response_head, TYPE_RESPONSE, SERVICE_QUERY_ONLINE, response_data_size);
				/*//debug:
				int i;
				for (i = 0; i < response_data_size / 20; ++i){
					printf("%s\t", (char *)(response_head + 1) + 20 * i);
				}
				printf("\n");*/
				send(client_socket, sendbuf, IM_PKT_HEAD_SIZE + response_data_size, 0);
				break;
			case SERVICE_SINGLE_MESSAGE: ;
				break;
			case SERVICE_MULTI_MESSAGE: ;
				break;

			default:
				printf("Received a error packet, drop it.\n");break;
		}
		memset(recvbuf, 0, sizeof(recvbuf));
	}
	if( n < 0)
		printf("Read error\n");

	if(status == ONLINE_STATUS){

		pthread_mutex_lock(&mutex);
		delete_user_by_name(online_user_queue, username);
		pthread_mutex_unlock(&mutex);
		status = -1;

		printf("\tuser %s logout\n", username);
	}
	pthread_exit(NULL);
}

