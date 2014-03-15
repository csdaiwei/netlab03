#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#include "bool.h"

#pragma pack(1)

/*the instant message packet structure*/
struct im_pkt {
	char service;
	char data[200];
};

/*these struct below will be 
 *the data field of the im_pkt*/
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

#pragma pack()

/*define of service field*/
#define SERVICE_LOGIN 0x00
#define SERVICE_LOGOUT 0x01
#define SERVICE_SINGLE_MESSAGE 0x02
#define SERVICE_MULTI_MESSAGE 0x03

/*const variables*/
#define USERNAME_LENGTH 20
#define TEXT_LENGTH 100
const int IM_PKT_SIZE = sizeof(struct im_pkt);

#endif

