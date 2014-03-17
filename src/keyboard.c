#include <stdio.h>
#include <stdlib.h>

/*the buf[size-1] will always be \0*/
int
get_keyboard_input(char *buf, int size) {
	int buf_len = 0;
	char c;

	while ((c = getchar()) != '\t' && c != '\n' && buf_len < size - 1) {
		buf[buf_len] = c;
		buf_len++;
	}

	buf[buf_len] = '\0';
	return buf_len;
}

void
print_prompt_words(char *username){
	printf(	"Hello, %s. You can enter these command below to use me.\n", username);
	printf(	"1.\"-list\"	show online friends\n"
			"2.\"-clear\"	clear the screen\n"
			"3.\"-logout\"	logout and exit\n");
	printf(	"=========================================================\n");
}