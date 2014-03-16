#ifndef __CLIENT_H__
#define __CLIENT_H__

/* readn - read exactly n bytes */
int readn( int sock_fd, char *bp, size_t len);

/*the buf[size-1] will always be \0*/
int get_keyboard_input(char *buf, int size);

void login(int);

#endif
