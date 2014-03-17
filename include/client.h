#ifndef __CLIENT_H__
#define __CLIENT_H__

void login();

void logout();

void query_online();

/*the buf[size-1] will always be \0*/
int get_keyboard_input(char *buf, int size);

void print_prompt_words(char *username);

/* readn - read exactly n bytes */
int readn( int sock_fd, char *bp, size_t len);

/* readvrec - read a variable record */
int readvrec( int sock_fd, char *bp, size_t len );

/*construct an im packet head to "h"(first parameter)*/
void construct_im_pkt_head(struct im_pkt_head *h, char type, char service, short data_size);

/*concatenate the im pkt data after the im pkt head*/
void concat_im_pkt_data(struct im_pkt_head *h, char *data);



#endif
