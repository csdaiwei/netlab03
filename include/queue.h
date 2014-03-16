#ifndef __QUEUE_H__
#define __QUEUE_H__


//extern int MAX_QUEUE_SIZE;

struct user_node {
	char username[20];
	int socket;
	struct user_node* next;
};

struct user_queue{
	int size;
	struct user_node* front;
	struct user_node* rear;
};


struct user_node* init_user_node(int socket, char * username);	/*create a user node by parameters*/

struct user_queue* init_user_queue(); 	/*create a empty queue*/


struct user_node* find_user_by_name(struct user_queue *q, char *name);

void delete_user_by_name(struct user_queue *q, char *name);

/*delete the user node "curr". not suitable when delete the front node*/
void delete_user_node(struct user_queue *q, struct user_node *prev, struct user_node *curr);

void enqueue(struct user_queue *q, struct user_node *n);	/*insert into rear of the queue*/

void dequeue(struct user_queue *q);	/*delete the front node*/

bool is_empty(struct user_queue *q);

bool is_full(struct user_queue *q);

/*
void visit(struct user_node *n);

void transverse(struct user_queue *q);
*/

#endif