#ifndef __SERVER_H__
#define __SERVER_H__

#define ONLINE_STATUS 1 // to be improve


void* client_handler(void*);

/*notify all others when a user login/logout*/
void on_off_line_notify(int SERVICE_ON_OFF, char *username, char *sendbuf);

/*the buf[size-1] will always be \0*/
int get_keyboard_input(char *buf, int size);


/* readn - read exactly n bytes */
int readn( int sock_fd, char *bp, size_t len);

/* readvrec - read a variable record */
int readvrec( int sock_fd, char *bp, size_t len );

/*construct an im packet head to "h"(first parameter)*/
void construct_im_pkt_head(struct im_pkt_head *h, char type, char service, short data_size);

/*concatenate the im pkt data after the im pkt head*/
void concat_im_pkt_data(struct im_pkt_head *h, char *data);

#endif
