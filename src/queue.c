#include <stdio.h>
#include <stdlib.h>

typedef unsigned char bool;
#define true 1
#define false 0


#define MAX_QUEUE_SIZE 100

struct user{
	char username[20];
	int socket;
};


struct node {
	int data;
	struct node* next;
};

struct queue{
	int size;
	struct node* front;
	struct node* rear;
};

/*create a empty queue*/
struct queue* init_queue(){
	struct queue *q = (struct queue *)malloc(sizeof(struct queue));
	q -> size = 0;
	q -> front = NULL;
	q -> rear = NULL;
	return q;
};

bool is_empty(struct queue *q){
	if(q -> size == 0)
		return true;
	return false;
}

bool is_full(struct queue *q){

	if(q -> size == MAX_QUEUE_SIZE)
		return true;
	return false;
}

/*insert into rear of the queue*/
bool enqueue(struct queue *q, struct node *n){
	if(is_full(q))
		return false;

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
	return true;
}

/*delete the front node*/
void dequeue(struct queue *q){
	if(is_empty(q))
		return NULL;
	struct node *n = q -> front;
	q -> front = q -> front -> next;
	q -> size --;
	free(n);
	if( is_empty(q))
		q -> rear = NULL;
}

void visit(struct node *n){
	if(n != NULL)
		printf("%d ", n -> data);
}

void transverse(struct queue *q){
	struct node *n = q -> front;
	for( n = q -> front; n!= NULL; n = n ->next){
		visit(n);
	}	
	printf("\n"); 
	
}


int main(){
	int i;
	struct queue *q = init_queue();
	
	q -> size = 0;
	q -> front = NULL;
	q -> rear = NULL;
	//printf("success\n");

	//printf("here\n");
	struct node n[5];
	struct node *pn[5];


	for (i = 0; i < 5; ++i){

		pn[i] = &n[i];
		pn[i] -> data = i;
		enqueue(q, pn[i]);
	}

	transverse(q);
	

}