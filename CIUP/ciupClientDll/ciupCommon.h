// Common definitions between CIUP client and server
// protocol definition:
//  4 bytes: 'C','I','U','P'
//  1 byte: MESSAGE TYPE (see #define CIUP_MSG_*)
//  2 bytes: payload size (S) in bytes 
//  S bytes: payload
//  1 byte: checksum (sum of previous bytes)
//  1 byte: checksum negated (0xFF-chk)

#pragma once

#include <Windows.h>

// message protocol size size
#define CIUP_MSG_HSIZE (9)

// position of payload inside message
#define CIUP_PAYLOAD_POS (7)

// position of type inside message
#define CIUP_TYPE_POS (4)

// max message size at all
#define CIUP_MAX_MSG_SIZE (512)

// max side of strings inside messages
#define CIUP_MAX_STRING_SIZE (256)

// message buffer size given the payload size
//#define CIUP_MSG_SIZE(payload_size) ((payload_size)+CIUP_MSG_HSIZE)
#define CIUP_MSG_SIZE(payload_size) (CIUP_MAX_MSG_SIZE)

// extract payload size from mmessage pointer
#define CIUP_PAYLOAD_SIZE(pmsg) ((unsigned int)(*(pmsg+CIUP_PAYLOAD_POS-2)*256) + (unsigned int)(*(pmsg+CIUP_PAYLOAD_POS-1)))

// ms timeout to stop threads
#define CIUP_STOP_THREAD_TIMEOUT_MS 500

// error count to disconnect/reconnect
#define CIUP_SOCKET_ERROR_LIMIT 3

// message types
#define CIUP_MSG_SERVERINFO   ((BYTE)0x01)
#define CIUP_MSG_DATAPOINT    ((BYTE)0x02)
#define CIUP_MSG_START        ((BYTE)0x03)
#define CIUP_MSG_STOP         ((BYTE)0x04)
#define CIUP_MSG_DISCONNECT   ((BYTE)0x05)

// error codes
#define CIUP_NO_ERR               (0)
#define CIUP_ERR_SIZE_MISMATCH    (-1)
#define CIUP_ERR_UNKNOWN_TYPE     (-2)
#define CIUP_ERR_SOCKET           (-3)
#define CIUP_ERR_ACK              (-4)
#define CIUP_ERR_SYNTAX           (-5)
#define CIUP_ERR_ID               (-6)
#define CIUP_ERR_MAX_CONNECTIONS  (-7)

// server and connection status 
enum ciupStatus {
	CIUP_ST_UNKNOWN = 0,
	CIUP_ST_IDLE    = 1,
	CIUP_ST_WORKING = 2,
	CIUP_ST_DISCONNECTED = 3
	
	// TODO
};

// strings for ciupStatus description
#define CIUP_STATUS_DESCR(n) (n==CIUP_ST_WORKING?"WRK":\
                              (n==CIUP_ST_IDLE?"IDL":\
                               (n==CIUP_ST_DISCONNECTED?"DSC":\
                                "UKN")))

// server infos packet
#pragma pack(1)
typedef struct {
	ciupStatus status = CIUP_ST_UNKNOWN;
	char id[CIUP_MAX_STRING_SIZE] = {};

	// TODO


} ciupServerInfo;
#pragma pack()

// data point packet
#pragma pack(1)
typedef struct {

	USHORT counter;         // counter for messages sequence
	USHORT channel;         // device channel of the data

	float Stime = 0;        // data time in S
	float Ktemp = 0;        // temperature in K
	float Acurr = 0;        // current in A
	float Vdiff = 0;        // Voltage in V
	float AHcap = 0;        // capacity in Ah

	// TODO

} ciupDataPoint;
#pragma pack()

// WARN: must be a multiple of 8
#define CIUP_CH_MAX_COUNT (128)

// server setup packet
#pragma pack(1)
typedef struct {
	
	BYTE chFilter[CIUP_CH_MAX_COUNT / 8] = {};  // filter for channel (set using CIUP_CH define)
	float dataFreqencyHz = 0;                   // frequency to read data (ignored if device-driven)

} ciupServerSetup;
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

// TODO: read answer function