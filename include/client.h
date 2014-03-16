#ifndef __CLIENT_H__
#define __CLIENT_H__

/* readn - read exactly n bytes */
int readn( int sock_fd, char *bp, size_t len);

void login(int);

#endif
