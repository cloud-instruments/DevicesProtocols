// Common definitions between CIUP client and server
// must be included in C/C++ application to use ciupClientDll.dll

// protocol definition:
//  4 bytes: 'C','I','U','P'
//  1 byte: MESSAGE TYPE (see #define CIUP_MSG_*)
//  2 bytes: payload size (S) in bytes 
//  S bytes: payload
//  1 byte: checksum (sum of previous bytes)
//  1 byte: checksum negated (0xFF-chk)
typedef unsigned char  uint8;

#pragma once

#include <Windows.h>

// message protocol header size
#define CIUP_MSG_HSIZE (9)

// position of payload inside message
#define CIUP_PAYLOAD_POS (7)

// position of type inside message
#define CIUP_TYPE_POS (4)

// max message size at all
#define CIUP_MAX_MSG_SIZE (2048)

// max side of strings inside messages
#define CIUP_MAX_STRING_SIZE (1024)

// message buffer size given the payload size
//#define CIUP_MSG_SIZE(payload_size) ((payload_size)+CIUP_MSG_HSIZE)
// message size is fixed to CIUP_MAX_MSG_SIZE to avoid TCP ip packet fragmentation 
#define CIUP_MSG_SIZE(payload_size) (CIUP_MAX_MSG_SIZE)

// extract payload size as unsigned int from message pointer
#define CIUP_PAYLOAD_SIZE(pmsg) ((unsigned int)(*((BYTE*)pmsg+CIUP_PAYLOAD_POS-2)*256) + (unsigned int)(*((BYTE*)pmsg+CIUP_PAYLOAD_POS-1)))

// ms timeout to stop threads
#define CIUP_STOP_THREAD_TIMEOUT_MS 500

// error count to disconnect/reconnect
#define CIUP_SOCKET_ERROR_LIMIT (3)

// max channels count
#define CIUP_CH_MAX_COUNT (128)

// maccor dll named pipe
#define CIUP_MACCOR_PIPENAME "ciuppipe"

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
#define CIUP_ERR_SIZE             (-8)
#define CIUP_ERR_CHECKSUM         (-9)

// server and connection status 
enum ciupStatus {
	CIUP_ST_UNKNOWN      = 0,
	CIUP_ST_IDLE         = 1,
	CIUP_ST_WORKING      = 2,
	CIUP_ST_CONNECTED    = 3,
	CIUP_ST_DISCONNECTED = 4
};

// server run mode
enum ciupServerRunMode {

	RM_EMULATOR = 0,
	RM_MACCOR   = 1,
	RM_ARBIN    = 2
};

// strings for ciupStatus description
#define CIUP_STATUS_DESCR(n) (n==CIUP_ST_IDLE?"IDL":\
                              (n==CIUP_ST_WORKING?"WRK":\
                               (n==CIUP_ST_CONNECTED?"CNC":\
                                (n==CIUP_ST_DISCONNECTED?"DNC":\
                                 "UKN"))))

// the server infos packet payload
#pragma pack(1)
typedef struct {

	ciupStatus status = CIUP_ST_UNKNOWN;
	char id[CIUP_MAX_STRING_SIZE] = {};
	ciupServerRunMode mode= RM_EMULATOR;

} ciupServerInfo;
#pragma pack()

// the data point packet payload
#pragma pack(1)
typedef struct {

	USHORT counter;         // counter for messages sequence
	USHORT channel;         // device channel of the data
	float TestTime = 0;        // data time in S
	float Current = 0;        // current in A
	float Voltage = 0;        // Voltage in V
	float Capacity = 0;        // capacity in Ah
	uint8 RF1 = 0;			// This is the mode of the test step - Current/Voltage/Power/Resistance- 4.5.3.2 Mode and Mode Value p.170 
	uint8 RF2 = 0;			// This the ES code of the test step
	int Cycle = 0;			// Cycle number
	int Step = 0;			// Test step (zero based)
	float StepTime = 0;		// Step time in seconds
	float HCCapacity = 0;	// Half cycle capacity in Ah
	float LHCCapacity = 0;	// Last half cycle capacity in Ah
	float Energy = 0;		// Energy in Wh
	float HCEnergy = 0;		// Half cycle energy in Wh
	float LHCEnergy = 0;	// Last half cycle energy in Wh
	float tFactor = 0;		// Tick time in seconds
} ciupDataPoint;
#pragma pack()

// allocate and fill the message buffer
// message buffer size will be CIUP_MSG_SIZE(payload_size)
// WARN: message must be delete [] after use
void *ciupBuildMessage(
	BYTE type,                   // message type, use CIUP_MSG_* defines
	void* payload=NULL,          // payload pointer 
	size_t payload_size=0        // payload size
);

// check the message correctness
// returns CIUP_NO_ERR | CIUP_ERR_SIZE | CIUP_ERR_SYNTAX | CIUP_ERR_CHECKSUM
int ciupCheckMessageSyntax(
	void *msg,                   // message pointer
	size_t msg_size              // size of message in bytes
);

