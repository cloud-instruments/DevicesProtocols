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

// max points in SCHEDULE_REPORT_LOGDATA_DELTA
#define A3P_MAX_POINTS (109)

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

/// the protocol message structure is a byte buffer with size
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

// DATA STRUCT /////////////////////////////////////////////////////////////////

// Structs defined in Arbin Third Party Interface 1 Design Proposal V1.1 ///////

// time struct
typedef struct
{
	unsigned long Second = 0; // Second
	unsigned short Us100 = 0; // 100 microseconds
}
MsTime; // 6 bytes in total

// used by SET_SYSTEMTIME command
typedef struct
{
	DWORD m_dwCmd;        // Execution code=0X5912A100
	DWORD m_dwCmd_Extend; // Reserved for extended execution code
	MsTime Time;          // Time (UX-946666800)
	BYTE m_btReserve;     // Reserved
	BYTE m_3RD_SwitchON;  // 0X05, fixed bytes
	WORD m_u16Token;      // Variable for detecting communication
	DWORD m_dwNone_41;    // Reserved
	BYTE m_btCheckSum[2]; // Checking code
}
MP_SET_SYSTEMTIME; // 24 bytes in total

// used by 3RD_SDU command
typedef struct
{
	DWORD m_dwCmd; // Execution code=0X59EDA100
	DWORD m_dwCmd_Extend; // Reserved for extended execution code
	DWORD m_dwReserve[11]; // Reserved
	WORD m_wChannelNumber; // Channel number
	WORD m_wTotalChannelNumber; // Number of channels
	BYTE m_btReserve[3]; // Reserved
	BYTE m_btOnSwitch;  // 0: change to SDU mode, 1: change to 3 rd Party mode
	DWORD m_dwReserve1; // Reserved
	BYTE m_btCheckSum[2]; // Checking code

}
MP_3RD_SDU; // 66 bytes in total

// Struct used as feedback (MCU->PC) for:
// CMD_3RD_SDU_FEEDBACK, CMD_3RD_SAMERANGE_CTRLTYPE_FEEDBACK, MP_3RD_CTRLTYPE_FEEDBACK, CMD_3RD_INFOCFG_FEEDBACK
typedef struct
{
	DWORD m_dwCmd; // Execution code
	DWORD m_dwCmd_Extend; // Reserved for extended execution code
	DWORD m_dwReserve[5]; // Reserved
	DWORD m_dwCopyCmd; // Reserved
	DWORD m_dwCopyCmd_Extend; // Reserved
	WORD m_btResult; // 0:failure, 1:success
	BYTE m_btMultPack_OK; // Reserved
	BYTE m_btError_Code; // Reserved
	DWORD m_dwReserve1; // Reserved
	MsTime Time; // Time
	BYTE m_btCheckSum[2]; // Checking code
}
MP_CONFIRM_FEEDBACK; // 52 bytes in total

// structs for 3RD_CTRLTYPE command
typedef struct
{
	WORD m_wCtrlType; // Control types
	BYTE m_btCurrentRange; // Current range
	BYTE m_btVoltageRange; // Voltage range
	DWORD m_dwNone; // Reserved
	float m_fCtrlVal[4]; // Control value
	DWORD m_dwNone_1; // Reserved
	WORD m_wCtrlVal_Type[4]; // Control value’s type
	DWORD m_dwNone_2[4]; // Reserved
}
ST_STEPCONTROL_DATA; // 52 bytes in total
typedef struct
{
	DWORD m_dwCmd; // Execution code=0X59EDA200
	DWORD m_dwCmd_Extend; // Reserved for extended execution code
	DWORD m_dwReserve1[11]; // Reserved
	WORD m_wChannelNumber; // Channel number
	WORD m_wTotalChannelNumber; // Number of channels
	DWORD m_dwReserve2; // Reserved
	ST_STEPCONTROL_DATA m_st_CtrlData;
	DWORD m_dwReserve3[2]; // Reserved
	float m_fVclampHigh; // High voltage clamp
	float m_fVclampLow; // Low voltage clamp
	BYTE m_btCheckSum[2]; // Checking code
}
MP_3RD_CTRLTYPE; // 130 bytes in total

// answer for FOR CMD_3RD_CTRLTYPE and CMD_3RD_SDU (sent on ch2)
typedef struct
{
	DWORD m_dwCmd; // Execution code=0XA9EDA800
	DWORD m_dwCmd_Extend; // Reserved for extended execution code
	BYTE m_btCheckSum[2]; // Checking code
	WORD m_wChannelNumber; // Channel number
	WORD m_wReserve; // Reserved
	BYTE m_Ctrltype; // Output control type
	BYTE m_FinishFlg; // Finish flag, 0: failure, 1: success
	BYTE m_btEvent; // Event status, 0: started, 1: stopped
	BYTE m_btChannelStatus; // Channel Status: 0:IDLE, 1: Transition, 2: CC, 3: CCCV, 4: CV, 5: Unsafe
	WORD m_wUnsafe_Type; // Unsafe Type:0: Current unsafe, 1: Voltage unsafe, 2: Power unsafe
	DWORD m_dwReserve2[2]; // Reserved
	float m_CtrlValue; // Control value
	DWORD m_dwReserve3[16]; // Reserved
}
MP_3RD_ACTIONDONE; // 96 bytes in total

// struct for 3RD_SAMERANGE_CTRLTYPE command
typedef struct
{
	DWORD m_dwCmd; // Execution code=0X59EDA900
	DWORD m_dwCmd_Extend; // Reserved for extended execution code
	DWORD m_dwReserve[11]; // Reserved
	WORD m_wChannelNumber; // Channel number
	WORD m_wTotalChannelNumber; // Number of channels
	WORD m_None[4]; // Reserved
	ST_STEPCONTROL_DATA m_st_CtrlData;
	DWORD m_dwReserve2[4]; // Reserved
	BYTE m_btCheckSum[2]; // Checking code
}
MP_3RD_SAMERANGE_CTRLTYPE; // 134 bytes in total

// struct for READDATAORSTATE command
typedef struct
{
	DWORD m_dwCmd; // Execution code=0X59EDA400
	DWORD m_dwCmd_Extend; // Reserved for extended execution code
	DWORD m_dwReserve[11]; // Reserved
	WORD m_wChannelNumber; // Channel number
	WORD m_wTotalChannelNumber; // Number of channels
	BYTE m_btNone_11[2]; // Reserved
	BYTE m_btIsReadData; // 1: Read data, 0: Not read data
	BYTE m_btIsState; // 1: Read state, 0: Not read state
	BYTE m_btCheckSum[2]; // Checking code
}
MP_INDEPENDENT_READDATAORSTATE; // 62 bytes in total

// struct for READDATAORSTATE feedback (answer to READDATAORSTATE command on ch1)
typedef struct
{
	BYTE m_btControlState; // Control state
						   // 0x10: Inactive, 0x11: Idle, 0x12: Tran, 0x13: Output
	BYTE m_btNone_11[3]; // Reserved
	float m_fCurrent; // Current Value
	float m_fVoltage; // Voltage Value
	float m_fValue[4]; // Reserved
}
ST_READDATAORSTATE_VARIABLE; // 28 bytes in total
typedef struct
{
	DWORD m_dwCmd; // Execution code=0XA9EDA400
	DWORD m_dwCmd_Extend; // Reserved for extended execution code
	WORD m_wChannelNumber; // Channel number
	WORD m_wTotalChannelNumber; // Number of channels
	WORD m_wReserve; // Reserved
	BYTE m_btIsReadData; // 1: Read data, 0: Not read data
	BYTE m_btIsState; // 1: Read state, 0: Not read state
	ST_READDATAORSTATE_VARIABLE m_st_Read_Value[A3P_MAXCHANNELNO];
	BYTE m_btCheckSum[2]; // Checking code
}
MP_INDEPENDENT_READDATAORSTATE_FEEDBACK; // 210 bytes in total

// structs for 3RD_INFOCFG command
typedef struct
{
	float m_fMin; // Minimum value
	float m_fMax; // Maximum value
}
St_SchSafeData; // 8 bytes in total
typedef struct
{
	St_SchSafeData m_stCurrent; // Current
	St_SchSafeData m_stVoltage; // Voltage
	St_SchSafeData m_stPower; // Power
	St_SchSafeData m_stVoltageClamp; // Voltage Clamp
	St_SchSafeData m_stOthers[10]; // Reserved
}
ST_3RD_SAFETY; // 40 bytes in total
typedef struct
{
	float TimeInterval; // Time interval
	float Reserved[4]; // Reserved
}
ST_3RD_LOGDATA; // 8 bytes in total
typedef struct
{
	DWORD m_dwCmd; // Execution code=0X59EDAA00
	DWORD m_dwCmd_Extend; // Reserved for extended execution code
	DWORD m_dwReserve[11]; // Reserved
	WORD m_wChannelNumber; // Channel number
	WORD m_wTotalChannelNumber; // Number of channels
	WORD m_Safetyflg; // If set the safety check information: 0: Unset, 1: Set, 2: Ignore
	WORD m_wSafetyLimits_Type; // Safety types we support
	WORD m_Logflg; // If set the interval time to let MCU reports data and states, 0: Unset, 1: Set
	WORD m_Reserved; // Reserved
	ST_3RD_SAFETY m_3rd_safety;
	ST_3RD_LOGDATA m_3rd_Logdata;
	float m_Reserved2[4]; // Reserved
	BYTE m_btCheckSum[2]; // Checking code
}
MP_3RD_INFOCFG; // 118 bytes in total

// struct for REPORT_LOGDATA message (scheduled message on ch2)
typedef struct
{
	unsigned long u32Second; // Seconds after the starting time. The starting time is 12 / 31 / 1999 19:00 p.m..
	unsigned short u16Us100; // 100 microseconds
	unsigned char ucType : 5;  // 0: SDL_DATA_TYPE_LOG, it means the data uploading to PC’s type is MCU reporting data
							   // and states to PC as the interval time. 1: SDL_DATA_TYPE_DISPLAY, don’t use it.
							   // 2 : SDL_DATA_TYPE_IDLE.It means when the third party connects to MCU through TCP / IP.
							   // MCU will report some idle points automatically.You could ignore these points.They are for ‘Mits 7.0’.
	unsigned char bIV; // 0: The data uploaded to PC is not current and voltage data. 1: The data uploaded to PC is current and voltage data.
	unsigned char ucSensor : 2; // Input as 0.
	unsigned char ucValueType; // The points uploaded to PC’s data type.
							   // 21: Current voltage, 22: Current current, 23: Current charging capacity, 24: Current discharging
							   // capacity, 25: Current charging energy, 26 : Current discharging energy, 27 : Current value change rate.
	float fValue;              // Value of the data
	unsigned char ucChannelIdx; // Channel number
}
St_SDL_Point_Pack;
typedef struct
{
	DWORD m_dwCmd; // Execution code=0XD9418100
	DWORD m_dwCmd_Extend; // Reserved for extended execution code
	BYTE m_btCheckSum[2]; // Checking code
	WORD m_u16Token; unsigned char ItemCount;
	St_SDL_Point_Pack Point[A3P_MAX_POINTS]; // 13*109=1417 bytes
}
MP_REPORT_LOGDATA_SDL; // 1428 bytes in total

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

void a3p_CMD_3RD_INFOCFG(
	a3p_msg *msg,
	WORD ChNum,
	WORD ChCount,
	WORD safetyFlg,
	WORD safetyLimitsType,
	WORD logFlag,
	float currentMax,
	float currentMin,
	float voltageMax,
	float  voltageMin,
	float  powerMax,
	float  powerMin,
	float  voltclampMax,
	float  voltclampMin,
	float timeInterval);

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

// check and parse incoming CMD_3RD_ACTIONDONE message
// returns: 0 on success
//         -1 on message size error
//         -2 on execution code error
//         -3 on checksum error
int a3p_parse_CMD_3RD_ACTIONDONE(
	const a3p_msg *msg,
	WORD *channelNumber,
	BYTE *ctrltype,
	BYTE *finishFlg,
	BYTE *event,
	BYTE *channelStatus,
	WORD *unsafeType,
	float *ctrlValue
);

// check and parse incoming CMD_SCHEDULE_REPORT_LOGDATA_DELTA message
// returns: 0 on success
//         -1 on message size error
//         -2 on execution code error
//         -3 on checksum error
int a3p_parse_CMD_SCHEDULE_REPORT_LOGDATA_DELTA(
	const a3p_msg *msg,
	WORD *itemCount,
	unsigned long u32Second[A3P_MAX_POINTS],
	unsigned short u16Us100[A3P_MAX_POINTS],
	unsigned char ucType[A3P_MAX_POINTS],
	unsigned char bIV[A3P_MAX_POINTS],
	unsigned char ucSensor[A3P_MAX_POINTS],
	unsigned char ucValueType[A3P_MAX_POINTS],
	float fValue[A3P_MAX_POINTS],
	unsigned char ucChannelIdx[A3P_MAX_POINTS]);