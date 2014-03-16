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

#define BUF_SIZE 256

/*a global socket for server*/
int listenfd; 						

/*a global user queue, containing user info *
 *a lock is necessary when any threads read & write it*/
struct user_queue *user_q = NULL; 	

/*mutual exclusion lock*/
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

int main(void){

	int connfd;
	socklen_t clilen;
	struct sockaddr_in cliaddr, servaddr;
	
	/*preparations of the server*/
	if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		perror("problem in creating socket");
		exit(-1);
	}
	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(SERV_PORT);
	bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
	listen(listenfd, LISTENQ);

	user_q = init_user_queue();

	if(system("clear")) ;
	printf("Server running... waiting for connections.\n");

	/*keep listening to client request after preparations*/
	for( ; ; ){

		//accept a connection
		clilen = sizeof(cliaddr);
		connfd = accept(listenfd, (struct sockaddr *) &cliaddr, &clilen);
		
		/*char client_ip[20];
		inet_ntop(AF_INET, &cliaddr.sin_addr.s_addr, client_ip, 16);		
		printf("Received request from %s ...\n", client_ip);*/

		/*create a thread to handle a client*/
		pthread_t tid;
		int rc = pthread_create(&tid, NULL, client_handler, (void*)connfd);
		if(rc){
			printf("ERROR creating thread, tid %d, return code %d\n", (int)tid, rc);
			exit(-1);
		}
	}
	//end of server main
}

void *client_handler(void * connfd){

	int n;	/*number received*/
	char username[20];	//the user's name connecting to this thread. will be filled when login
	int status = -1;	//the user's status
	int client_socket = (int)connfd;	//the user's socket connecting to this thread;
	//long tid = pthread_self();

	assert(user_q != NULL);
		
	/*each thread has its own buf*/
	char sendbuf[BUF_SIZE];
	char recvbuf[BUF_SIZE];	
	
	//printf("\tthread %lu created for dealing with client requests\n", tid);

	/*Receive the request im packet by two steps.
	 *Firstly receive the head field, secondly receive the data field.
	 *Then handle the request packet (usually by a response packet)*/
	while( (n = readvrec(client_socket, recvbuf, BUF_SIZE)) > 0){
		printf("1\n");
		struct im_pkt_head *request_head = (struct im_pkt_head *)recvbuf;
		char *request_data = &recvbuf[IM_PKT_HEAD_SIZE];
		/*read the data field of the im packet
		n = readn(client_socket, &recvbuf[IM_PKT_HEAD_SIZE], 20);
		printf("2\n");*/
		struct im_pkt_head *response_head = (struct im_pkt_head *)sendbuf;
		char *response_data = &sendbuf[IM_PKT_HEAD_SIZE];
		memset(sendbuf, 0, sizeof(sendbuf));
		if(request_head -> type != TYPE_REQUEST){
			printf("Received a error packet, drop it.\n");
			break;
		}
		
		switch(request_head -> service){
			case SERVICE_LOGIN: ;	//void statement necessary
				
				strncpy(username, request_data, 19);
				username[19] = '\0';

				//check username repeat or not
				pthread_mutex_lock(&mutex);
				if(find_user_by_name( user_q, username) == NULL){
					response_data[0] = true;
					struct user_node *pnode = init_user_node(client_socket, username);
					enqueue(user_q, pnode);
					status = ONLINE_STATUS;
					printf("\nuser %s login.\n", username);
				}
				else
					response_data[0] = false;
				pthread_mutex_unlock(&mutex);
				//response login info to the client
				construct_im_pkt_head(response_head, TYPE_RESPONSE, SERVICE_LOGIN, 1);
				send(client_socket, sendbuf, IM_PKT_HEAD_SIZE + 1, 0);
				break;
			case SERVICE_LOGOUT: ;
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

		pthread_mutex_unlock(&mutex);
		delete_user_by_name(user_q, username);
		pthread_mutex_lock(&mutex);

		printf("\nuser %s logout\n", username);
	}
	pthread_exit(NULL);
}

