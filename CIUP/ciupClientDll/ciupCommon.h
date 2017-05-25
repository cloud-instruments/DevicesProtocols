// Common definitions between CIUP client and server
// protocol definition:
//  4 bytes: 'C','I','U','P'
//  1 byte: MESSAGE TYPE (see #define CIUP_MSG_*)
//  2 bytes: payload size (s) in bytes 
//  s bytes: payload
//  1 byte: checksum (sum of previous bytes)
//  1 byte: checksum negated (0xFF-chk)

#pragma once

#include <Windows.h>

// message buffer size given the payload size
#define CIUP_MSG_SIZE(payload_size) ((payload_size)+9)

// position of payload inside message
#define CIUP_PAYLOAD_POS (7)

// max message size at all
#define CIUP_MAX_MSG_SIZE (2048)

// extract payload size from mmessage pointer
#define CIUP_PAYLOAD_SIZE(pmsg) ((unsigned int)(*(pmsg+5)*256) + (unsigned int)(*(pmsg+6)))

// timeout for answer read
#define CIUP_ANS_TIMEOUT_MS 100

// message types
#define CIUP_MSG_SERVERINFO   ((BYTE)0x01)
#define CIUP_MSG_DATAPOINT    ((BYTE)0x02)
#define CIUP_MSG_START        ((BYTE)0x03)
#define CIUP_MSG_STOP         ((BYTE)0x04)

// error codes
#define CIUP_NO_ERR             (0)
#define CIUP_ERR_SIZE_MISMATCH  (-1)
#define CIUP_ERR_UNKNOWN_TYPE   (-2)
#define CIUP_ERR_SOCKET         (-3)
#define CIUP_ERR_ACK            (-4)
#define CIUP_ERR_SYNTAX         (-5)
#define CIUP_ERR_ID             (-6)
#define CIUP_ERR_MAX_RECEIVERS  (-7)


// server status 
enum ciupStatus {
	UNKNOWN = 0,
	IDLE    = 1,
	WORKING = 2,
	
	// TODO
};

// strings for ciupStatus description
#define CIUP_STATUS_DESCR(n) (n==WORKING?"WRK":\
                              (n==IDLE?"IDL":\
                               "UKN"))

// server infos packet
#pragma pack(1)
typedef struct {
	ciupStatus status = UNKNOWN;
	// TODO
} ciupServerInfo;
#pragma pack()

// data point packet
#pragma pack(1)
typedef struct {
	float Ktemp = 0;
	float Acurr = 0;
	float Vdiff = 0;
	float AHcap = 0;
} ciupDataPoint;
#pragma pack()

// allocate and fill the message buffer
// message buffer size will be CIUP_MSG_SIZE(payload_size)
// WARN: message must be delete [] after use
void *ciupBuildMessage(
	BYTE type, 
	void* payload=NULL, 
	size_t payload_size=0
);

int ciupCheckMessageSyntax(
	void *msg,
	size_t msg_size
);