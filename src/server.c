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
			
		char client_ip[20];
		inet_ntop(AF_INET, &cliaddr.sin_addr.s_addr, client_ip, 16);		
	
		//printf("Received request from %s ...\n", client_ip);

		/*create a thread to handle a client*/
		pthread_t tid;
		int rc = pthread_create(&tid, NULL, client_handler, (void*)connfd);
		if(rc){
			printf("ERROR creating thread, tid %d, return code %d\n", (int)tid, rc);
			exit(-1);
		}
	}
	/**/
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

	while( (n = readn(client_socket, recvbuf, IM_PKT_SIZE)) == IM_PKT_SIZE){
				
		/*handle the request packet (usually by a response packet)*/
		struct im_pkt *request_pkt = (struct im_pkt *)recvbuf;
		struct im_pkt *response_pkt = (struct im_pkt *)sendbuf;
		memset(sendbuf, 0, sizeof(sendbuf));

		switch(request_pkt -> service){
			case SERVICE_LOGIN: ;	//void statement necessary
				
				struct login_request_data *req_data = (struct login_request_data *)request_pkt -> data;	
				struct login_response_data *resp_data = (struct login_response_data *)response_pkt -> data;
				
				strncpy(username, req_data -> username, 19);
				username[19] = '\0';

				/*check username repeat or not*/
				pthread_mutex_lock(&mutex);
				if(find_user_by_name( user_q, req_data -> username) == NULL){
					resp_data -> login_success = true;
					struct user_node *n = init_user_node(client_socket, req_data -> username);
					enqueue(user_q, n);
					status = ONLINE_STATUS;
					printf("\nuser %s login.\n", username);
				}
				else
					resp_data -> login_success = false;
				pthread_mutex_unlock(&mutex);
				/*response login info to the client*/
				send(client_socket, response_pkt, IM_PKT_SIZE, 0);
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

