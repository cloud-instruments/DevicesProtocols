/*!
*  @brief     Arbin 3rd party protocol messages
*  @author    Matteo Lucarelli
*  @date      2017
*/

#pragma once

#include <windows.h>

// convert UNIX time (seconds since Jan 01 1970)
// to Arbin time (seconds since 19:00 12/31/1999)
#define UX_TO_ARBIN_TIME(n) ((n)-946666800)

// max message size
#define A3P_MAX_MSG_SIZE (2048)

// out messages 1ch (SW->FW)
#define CMD_SET_SYSTEMTIME                  (0X5912A100)
#define CMD_3RD_SDU                         (0X59EDA100)
#define CMD_3RD_CTRLTYPE                    (0X59EDA200)
#define CMD_3RD_SAMERANGE_CTRLTYPE          (0X59EDA900)
#define CMD_3RD_READDATAORSTATE             (0X59EDA400)
#define CMD_3RD_INFOCFG                     (0X59EDAA00)

// in messages 1ch (FW->SW)
#define CMD_3RD_SDU_FEEDBACK                (0XA9EDA100)
#define CMD_3RD_CTRLTYPE_FEEDBACK           (0XA9EDA200)
#define CMD_3RD_SAMERANGE_CTRLTYPE_FEEDBACK (0XA9EDA900)
#define CMD_3RD_READDATAORSTATE_FEEDBACK    (0XA9EDA400)
#define CMD_3RD_INFOCFG_FEEDBACK            (0XA9EDAA00)

// in messages 2ch (FW->SW)
#define CMD_3RD_ACTIONDONE                  (0XA9EDA800)
#define CMD_SCHEDULE_REPORT_LOGDATA_DELTA   (0XD9418100)

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

/// @brief       build CMD_SET_SYSTEMTIME message
/// @param[out]  msg the message to fill
/// @param[in]   time_s the time to put in the message uses Arbin standard
void a3p_CMD_SET_SYSTEMTIME(a3p_msg *msg, float time_s);

/// @brief       build CMD_3RD_SDU message
/// @param[out]  msg : the message to fill
/// @param[in]   mode_3dr_on : Set true to enter 3rd party mode 
/// @param[in]   ChNum : Starting channel number
/// @param[in]   ChCount : Number of channels from the starting channel
void a3p_CMD_3RD_SDU(a3p_msg *msg, bool mode_3dr_on, WORD ChNum, WORD ChCount);

/// @brief        check and parse incoming CONFIRM_FEEDBACK message
/// @param[in]    msg : the message to parse
/// @param[in]    success : will be set to true in case of positive feedback
/// @param[in]    code : expected execution code in message
///                      CMD_3RD_SDU_FEEDBACK, CMD_3RD_SAMERANGE_CTRLTYPE_FEEDBACK, 
///                      CMD_3RD_CTRLTYPE_FEEDBACK. CMD_3RD_INFOCFG_FEEDBACK
/// @returns      0 on success
/// @returns      -1 on message size error
/// @returns      -2 on execution code error
/// @returns      -3 on checksum error
int a3p_parse_CONFIRM_FEEDBACK(const a3p_msg *msg, bool *success, DWORD code);

int a3p_CONFIRM_FEEDBAK_size();


