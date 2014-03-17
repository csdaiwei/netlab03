#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "bool.h"
#include "queue.h"

int MAX_QUEUE_SIZE = 100;

/*create a user node by parameters*/
struct user_node* init_user_node(int socket, char * username){
	struct user_node *n = (struct user_node *)malloc(sizeof(struct user_node));
	strncpy(n -> username, username, 19);
	n -> username[19] = '\0'; 
	n -> socket = socket;
	n -> next = NULL;
	return n;
}

/*create a empty queue*/
struct user_queue* init_user_queue(){
	struct user_queue *q = (struct user_queue *)malloc(sizeof(struct user_queue));
	q -> size = 0;
	q -> front = NULL;
	q -> rear = NULL;
	return q;
};

void
destroy_user_queue(struct user_queue *q){
	while(q -> size != 0)
		dequeue(q);
	free(q);
}

/*insert into rear of the queue*/
void enqueue(struct user_queue *q, struct user_node *n){
	if(is_full(q))
		MAX_QUEUE_SIZE *= 2;/*doubled size*/

	if(is_empty(q)){
		
		q -> front = n;
		q -> rear = n;
		n -> next = NULL;
	} else{
		
		q -> rear -> next =  n;
		q -> rear = n;
		n -> next = NULL;
	}
	q -> size ++;
	//printf("debug: add user %s in the queue\n", n -> username);
	//printf("debug: front %s, rear %s\n", q -> front -> username, q -> rear -> username);
	
}

struct user_node* find_user_by_name(struct user_queue *q, char *name){
	struct user_node *n;
	for(n = q -> front; n != NULL; n = n -> next){
		if(strncmp(n -> username, name, 20) == 0)
			return n;
	}
	return NULL;
}

void delete_user_by_name(struct user_queue *q, char *name){
	
	struct user_node *prev, *curr, *n;
	prev = curr = NULL;
	for(n = q -> front; n != NULL; n = n -> next){
		if(strncmp(n -> username, name, 20) == 0){
			curr = n;
			delete_user_node(q, prev, curr);
			return ;
		}
		prev = n;
	}
	//printf("error when delete: no user names %s\n", name);
}

/*delete the user node "curr"*/
void delete_user_node(struct user_queue *q, struct user_node *prev, struct user_node *curr){
	assert(curr != NULL);
	assert(!is_empty(q));
	if(prev == NULL){
		assert(curr == q-> front);
		q -> front = curr -> next;
	} else{
		prev -> next = curr -> next;
		if(q -> rear == curr)
			q -> rear = prev;
	}
	q -> size --;
	//printf("debug: delete user %s in queue\n", curr -> username);
	//printf("debug: front %s, rear %s\n", q -> front -> username, q -> rear -> username);
	free(curr);
}

/*delete the front node*/
void dequeue(struct user_queue *q){
	if(is_empty(q))
		return ;
	struct user_node *n = q -> front;
	q -> front = q -> front -> next;
	q -> size --;
	free(n);
	if( is_empty(q))
		q -> rear = NULL;
}

bool is_empty(struct user_queue *q){
	if(q -> size == 0)
		return true;
	return false;
}

bool is_full(struct user_queue *q){

	if(q -> size == MAX_QUEUE_SIZE)
		return true;
	return false;
}

/*copy all names into buf
 *return the name number*/
int copy_all_user_name(char * buf, struct user_queue *q){	
	
	int i = 0;
	struct user_node *n = q -> front;
	for(; n != NULL; n = n -> next){
		//printf("debug: size %d i %d name %s \n", q -> size, i, n -> username);
		strncpy(&buf[20*i], n -> username, 20);
		i++;
	}

	return q -> size;
}
