#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__

#pragma pack(1)
struct request_packet {
	char request_type;
	char sender_name[20];
	char recipient_name[20];
	char text[100];
};

struct response_packet {
	char response_type;
};
#pragma pack()

const int REQUEST_PACKET_SIZE = sizeof(struct request_packet);
const int RESPONSE_PACKET_SIZE = sizeof(struct response_packet);

#endif

