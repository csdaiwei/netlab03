#include <stdio.h>
#include <stdlib.h> 	//size_t
#include <errno.h>	//readn errno
#include <sys/socket.h>    
#include <sys/types.h>

/* readn - read exactly n bytes */
int readn( int sock_fd, char *bp, size_t len)
{
	int cnt;
	int rc;

	cnt = len;
	while ( cnt > 0 )
	{
		printf("in readn, sock:%d\n", sock_fd);
		rc = recv( sock_fd, bp, cnt, 0 );
		if ( rc < 0 )				/* read error? */
		{
			if ( errno == EINTR )	/* interrupted? */
				continue;			/* restart the read */
			return -1;				/* return error */
		}
		if ( rc == 0 )				/* EOF? */
			return len - cnt;		/* return short count */
		bp += rc;
		cnt -= rc;
	}
	return len;
}

