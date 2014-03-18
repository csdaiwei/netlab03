#include <stdio.h>
#include <stdlib.h>

#include "queue.h"
#include "client.h"

/*get keyoard input within limited characters.
 *\t and \n will not be accepted
 *the buf[size-1] will always be \0
 *return the number of input characeters not including \0 in the last*/
int
get_keyboard_input(char *buf, int size) {
	int buf_len = 0;
	char c;

	while ((c = getchar()) != '\t' && c != '\n' && buf_len < size - 1) {
		buf[buf_len] = c;
		buf_len++;
	}
	buf[buf_len] = '\0';

	if(buf_len == size - 1)
		while(getchar()!='\n');//flush the system buffer
	
	return buf_len;
}

void
print_prompt_words(char *username){
	printf(	"Hello, %s. You can enter these command below to use me.\n", username);
	printf(	"1.\"-list\"	show online friends\n"
			"2.\"-chat\"	chat with friends\n"
			"3.\"-recent\"	show recent 5 messages\n"
			"4.\"-clear\"	clear the screen\n"
			"5.\"-logout\"	logout and exit\n");
	printf(	"===========================================================\n");
}

void
print_online_friends(struct user_queue *q){

	struct user_node *n;
	int i = 0;
	printf("online friends are:\n");
	for (n = q -> front; n != NULL; n = n -> next){
		printf("%s\t", n -> username);
		i++;
		if((i % 4) == 0)
			printf("\n");
	}
	if((i % 4) != 0)
		printf("\n");
}

void 
print_recent_messages(struct message *recent_messages, int msg_num){
	int i;
	int n = (msg_num > 5) ? 5 : msg_num;
	printf("==============================================\n");
	for(i = 0; i < n; i++){
		printf("Message from %s: \n", recent_messages[i].sender);
		printf("Text: %s\n", recent_messages[i].text);
	}
	printf("==============================================\n");

}