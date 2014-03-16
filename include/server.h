#ifndef __SERVER_H__
#define __SERVER_H__

#define ONLINE_STATUS 1


/* readn - read exactly n bytes */
int readn( int sock_fd, char *bp, size_t len);

void* client_handler(void*);

#endif
