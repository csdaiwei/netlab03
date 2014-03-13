#ifndef __READN_H__
#define __READN_H__


/* readn - read exactly n bytes */
int readn( int sock_fd, char *bp, size_t len)
{
	int cnt;
	int rc;

	cnt = len;
	while ( cnt > 0 )
	{
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

#endif
