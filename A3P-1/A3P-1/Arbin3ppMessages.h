// Arbin 3rd party protocol messages

#pragma once

#include <windows.h>

// convert UNIX time (seconds since Jan 01 1970)
// to Arbin time (seconds since 19:00 12/31/1999)
#define A3P_UX_TO_ARBIN_TIME(n) ((n)-946666800)

// max message size
#define A3P_MAX_MSG_SIZE (2048)

// defined in documentation
#define A3P_MAXCHANNELNO (16)

// size of messages header
#define A3P_HEADER_SIZE (16)

// out messages 1ch (SW->FW)
#define A3P_CMD_SET_SYSTEMTIME                  (0X5912A100)
#define A3P_CMD_3RD_SDU                         (0X59EDA100)
#define A3P_CMD_3RD_CTRLTYPE                    (0X59EDA200)
#define A3P_CMD_3RD_SAMERANGE_CTRLTYPE          (0X59EDA900)
#define A3P_CMD_3RD_READDATAORSTATE             (0X59EDA400)
#define A3P_CMD_3RD_INFOCFG                     (0X59EDAA00)

// in messages 1ch (FW->SW)
#define A3P_CMD_3RD_SDU_FEEDBACK                (0XA9EDA100)
#define A3P_CMD_3RD_CTRLTYPE_FEEDBACK           (0XA9EDA200)
#define A3P_CMD_3RD_SAMERANGE_CTRLTYPE_FEEDBACK (0XA9EDA900)
#define A3P_CMD_3RD_READDATAORSTATE_FEEDBACK    (0XA9EDA400)
#define A3P_CMD_3RD_INFOCFG_FEEDBACK            (0XA9EDAA00)

// in messages 2ch (FW->SW)
#define A3P_CMD_3RD_ACTIONDONE                  (0XA9EDA800)
#define A3P_CMD_SCHEDULE_REPORT_LOGDATA_DELTA   (0XD9418100)

/// the protocol message structure
typedef struct t_a3p_msg {

	BYTE  *buff = NULL;
	size_t size = 0;

	// default constructor (empty message)
	t_a3p_msg(){}

	// size constructor
	t_a3p_msg(size_t s) {
		size = s;
		buff = new BYTE[s];
	}

	// copy constructor
	t_a3p_msg(const t_a3p_msg &c):size(c.size){ 
		if (c.buff && size) {
			buff = new BYTE[size];
			memcpy(buff, c.buff, size);
		}
	}

	// destructor
	~t_a3p_msg() { 
		if (buff) delete[] buff;
		buff = NULL;
		size = 0;
	}
} a3p_msg;

// OUT /////////////////////////////////////////////////////////////////////////

// build CMD_SET_SYSTEMTIME message
void a3p_CMD_SET_SYSTEMTIME(
	a3p_msg *msg,          // the message to fill
	float time_s);         // time to put in the message uses Arbin standard

// build CMD_3RD_SDU message
void a3p_CMD_3RD_SDU(
	a3p_msg *msg,         // the message to fill
	WORD ChNum,           // Starting channel number
	WORD ChCount,         // Number of channels from the starting channel
	bool mode_3dr_on);    // Set true to enter 3rd party mode 

// build A3P_CMD_3RD_CTRLTYPE message 
void a3p_CMD_3RD_CTRL_TYPE(
	a3p_msg *msg,         // the message to fill
	WORD ChNum,           // Starting channel number
	WORD ChCount,         // Number of channels from the starting channel
	float hVoltClamp,
	float lVoltClamp,
	WORD ctrlType,
	BYTE currentRange,
	BYTE voltageRange,
	float ctrlVal[4],
	WORD ctrlValType[4]);

// build A3P_CMD_3RD_SAMERANGE_CTRLTYPE message
void a3p_CMD_3RD_SAMERANGE_CTRLTYPE(
	a3p_msg *msg,           // the message to fill
	WORD ChNum,             // Starting channel number
	WORD ChCount,           // Number of channels from the starting channel
	WORD ctrlType,
	BYTE currentRange,
	BYTE voltageRange,
	float ctrlVal[4],
	WORD ctrlValType[4]);

void a3p_CMD_3RD_READDATAORSTATE(
	a3p_msg *msg,              // the message to fill
	WORD ChNum,                // Starting channel number
	WORD ChCount,              // Number of channels from the starting channel
	bool readData,             // true for read data
	bool readState);           // true for read state


// IN //////////////////////////////////////////////////////////////////////////

// check and parse incoming CONFIRM_FEEDBACK message
// returns: 0 on success
//         -1 on message size error
//         -2 on execution code error
//         -3 on checksum error
int a3p_parse_CONFIRM_FEEDBACK(
	const a3p_msg *msg,  // the message to parse
	bool *success,       // will be set to true in case of positive feedback
	DWORD code);         // expected execution code in message:
	                     //  CMD_3RD_SDU_FEEDBACK, CMD_3RD_SAMERANGE_CTRLTYPE_FEEDBACK, 
	                     //  CMD_3RD_CTRLTYPE_FEEDBACK. CMD_3RD_INFOCFG_FEEDBACK

// return the size of 
int a3p_CONFIRM_FEEDBACK_size();

// check and parse incoming READDATAORSTATE_FEEDBACK message
// returns: 0 on success
//         -1 on message size error
//         -2 on execution code error
//         -3 on checksum error
int a3p_parse_READDATAORSTATE_FEEDBACK(
	const a3p_msg *msg,                       // the message to parse
	WORD *ChNum,
	WORD *ChCount,
	bool *readData,
	bool *readState,
	BYTE controlState[A3P_MAXCHANNELNO],
	float current[A3P_MAXCHANNELNO],
	float voltage[A3P_MAXCHANNELNO]);


