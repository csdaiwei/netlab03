#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#include "bool.h"

/*defination of type field*/
#define TYPE_REQUEST 0x00	/*client to server*/
#define TYPE_RESPONSE 0x01	/*server to client*/

/*defination of service field*/
#define SERVICE_LOGIN 0x00
#define SERVICE_LOGOUT 0x01				/*only request*/
#define SERVICE_QUERY_ONLINE 0x02		/*plan to abandon this*/
#define SERVICE_SINGLE_MESSAGE 0x03
#define SERVICE_MULTI_MESSAGE 0x04
#define SERVICE_ONLINE_NOTIFY 0x05		/*only response*/
#define SERVICE_OFFLINE_NOTIFY 0x06		/*only response*/


/*sizes*/
#define IM_PKT_HEAD_SIZE  sizeof(struct im_pkt_head)

#pragma pack(1)
/*the instant message packet structure*/
struct im_pkt_head {
	char type; 	/* 0x00 request: client to server
				 * 0x01 response: server to client*/
	char service;	/*see service constants defined below*/
	short data_size;
};
#pragma pack()

/*right after head there will be the im packet data field*/
/*the full im packet looks like this:
 struct im_pkt{
	uint8_t  type;
	uint8_t  service;
	uint16_t data_size;
	uint8_t  data[data_size]; 
 };
 */
/*	type field		service field	-> 		data field size(bytes) 	data field content
 *  REQUEST 		LOGIN 					20   					login username
 	REQUEST   		LOGOUT 					0 
 	REQUEST 		QUERY_ONLINE 			0 
	REQUEST 		SINGLE_MESSAGE 			to be implement
	REQUEST 		MULTI_MESSAGE 			to be implement
	
 	RESPONSE 		LOGIN 					1 						login success/fail
 	RESPONSE 		QUERY_ONLINE 			(n-1)*20 				online usernames
 	RESPONSE 		ONLINE_NOTIFY 			20 						username who get online
 	RESPONSE 		OFFLINE_NOTIFY 			20 						username who get offline
 	RESPONSE 		SINGLE_MESSAGE 			to be implement
 	RESPONSE 		MULTI_MESSAGE 			to be implement
	
	there are also some combinations that will never appear
	type 		service
	REQUEST 	ONLINE_NOTIFY
	REQUEST 	OFFLINE_NOTIFY
	RsESPONSE 	LOGOUT
 */


#endif

