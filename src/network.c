/*reference: code from the course website
 *http://cscms.nju.edu.cn/pluginfile.php/1024/mod_resource/content/1/readn.c
 *http://cscms.nju.edu.cn/pluginfile.php/1025/mod_resource/content/1/readvrec.c*/

#include <stdio.h>
#include <stdlib.h> 	//size_t
#include <string.h>		//memcpy
#include <errno.h>	//readn errno
#include <assert.h>
#include <sys/socket.h>    
#include <sys/types.h>

#include "protocol.h"

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

/* readvrec - read a variable record */
int readvrec( int sock_fd, char *bp, size_t len )
{
	u_int32_t reclen;
	int rc;

	/* Retrieve the length of the record */

	rc = readn( sock_fd, bp, IM_PKT_HEAD_SIZE );	//modified here to work with my protocol
	if ( rc != IM_PKT_HEAD_SIZE )
		return rc < 0 ? -1 : 0;
	struct im_pkt_head * h = (struct im_pkt_head *)bp;
	reclen = (u_int32_t) h -> data_size;
	if ( reclen > len )
	{
		/*
		 *  Not enough room for the record--
		 *  discard it and return an error.
		 */

		while ( reclen > 0 )
		{
			rc = readn( sock_fd, bp, len );
			if ( rc != len )
				return rc < 0 ? -1 : 0;
			reclen -= len;
			if ( reclen < len )
				len = reclen;
		}
		//set_errno( EMSGSIZE );
		return -1;
	}

	/* Retrieve the record itself */
	if(reclen != 0){
		rc = readn( sock_fd, &bp[IM_PKT_HEAD_SIZE], reclen );
		if ( rc != reclen )
			return rc < 0 ? -1 : 0;
	}
	return rc;
}



/*construct an im packet head to "h"(first parameter)*/
void 
construct_im_pkt_head(struct im_pkt_head *h, char type, char service, short data_size){
	h -> type = type;
	h -> service = service;
	h -> data_size = data_size;
}

/*concatenate the im pkt data after the im pkt head*/
void
concat_im_pkt_data(struct im_pkt_head *h, char *data){

	//data_size and data cannot be 0 at the same time 
	assert(h -> data_size == 0 || data != 0);

	char *p = (char *)(h + 1);// p points right after the im_pkt_head
	memcpy(p, data, h -> data_size);
}