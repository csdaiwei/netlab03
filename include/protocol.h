#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#include "bool.h"

/*defination of type field*/
#define TYPE_REQUEST 0x00	/*client to server*/
#define TYPE_RESPONSE 0x01	/*server to client*/

/*defination of service field*/
#define SERVICE_LOGIN 0x00
#define SERVICE_LOGOUT 0x01
#define SERVICE_QUERY_ONLINE 0x02
#define SERVICE_SINGLE_MESSAGE 0x03
#define SERVICE_MULTI_MESSAGE 0x04

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

/* a full im_pkt will be look like the struct below
struct im_pkt{
	char type; 	
	char service;
	short data_size;
	char im_pkd_data[data_size];
};*/

/*these struct below will be 
 *the data field of the im_pkt
struct login_request_data
{
	char username[20];
	char padding[180];
};

struct login_response_data
{
	bool login_success;
	char padding[199];
};

struct single_message_data
{
	char sender[20];
	char recipient[20];
	char text[100];
	char padding[60];
};

struct multi_message_data
{
	char sender[20];
	char text[100];
	char padding[80];
};
*/
#pragma pack()

#endif

