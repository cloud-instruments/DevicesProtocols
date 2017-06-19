// Arbin 3rd party protocol messages

#include "stdafx.h"

#include "Arbin3ppMessages.h"

typedef struct
{
	unsigned long Second = 0; // Second
	unsigned short Us100 = 0; // 100 microseconds
}
MsTime; // 6 bytes in total

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

// CMD_3RD_SDU_FEEDBACK, CMD_3RD_SAMERANGE_CTRLTYPE_FEEDBACK, MP_3RD_CTRLTYPE_FEEDBACK. CMD_3RD_INFOCFG_FEEDBACK
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

// FOR CMD_3RD_CTRLTYPE and CMD_3RD_SDU
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
	St_SDL_Point_Pack Point[109]; // 13*109=1417 bytes
}
MP_REPORT_LOGDATA_SDL; // 1428 bytes in total

// UTILITIES ///////////////////////////////////////////////////////////////////

// fill header
// overwrite first 12 (header) + 4 (fixed) bytes of data
void a3p_add_header(a3p_msg *msg)
{
	for (int i = 0; i < 8; i++) *(msg->buff + i) = 0xAA;

	DWORD s = (DWORD)(msg->size - A3P_HEADER_SIZE);
	*(msg->buff + 8) = (BYTE)(s >> 24);
	*(msg->buff + 9) = (BYTE)(s >> 16);
	*(msg->buff + 10) = (BYTE)(s >> 8);
	*(msg->buff + 11) = (BYTE)(s);

	*(msg->buff + 12) = 0x00;
	*(msg->buff + 13) = 0x98;
	*(msg->buff + 14) = 0x00;
	*(msg->buff + 15) = 0x00;
}

// fill checksum
// overwrite checksum on last 2 BYTES of buffer
// data = message buffer
// datasize = message buffer size (payload struct size  + 16)
void a3p_add_checksum(a3p_msg *msg)
{
	// cs = (BYTE)sum af all payload (including fixed)
	BYTE cs = 0x00;
	for (unsigned int i = 12; i < msg->size - 2; i++) cs += *(msg->buff + i);

	*(msg->buff + msg->size - 2) = cs;
	*(msg->buff + msg->size - 1) = 0xFF - cs;
}

// OUT MESSAGES ////////////////////////////////////////////////////////////////

void a3p_CMD_SET_SYSTEMTIME(a3p_msg *msg, float time_s)
{
	msg->size = A3P_HEADER_SIZE + sizeof(MP_SET_SYSTEMTIME);
	msg->buff = new BYTE[msg->size];

	// fill required fields
	MP_SET_SYSTEMTIME s = {};
	s.m_dwCmd = A3P_CMD_SET_SYSTEMTIME;
	MsTime t = {};
	t.Second = 0x20902cAF;// (unsigned long)time_s;
	t.Us100 = 0x2225;//(unsigned short)((time_s - t.Second) * 10000);
	s.Time = t;
	s.m_3RD_SwitchON = 0X05A;

	// put in buffer
	memcpy(msg->buff + A3P_HEADER_SIZE, &s, sizeof(s));

	// add header and checksum
	a3p_add_header(msg);
	a3p_add_checksum(msg);
}

void a3p_CMD_3RD_SDU(
	a3p_msg *msg, 
	WORD ChNum,
	WORD ChCount, 
	bool mode_3dr_on)
{
	msg->size = A3P_HEADER_SIZE + sizeof(MP_3RD_SDU);
	msg->buff = new BYTE[msg->size];

	// fill required fields
	MP_3RD_SDU s = {};
	s.m_dwCmd = A3P_CMD_3RD_SDU;
	s.m_wChannelNumber = ChNum;
	s.m_wTotalChannelNumber = ChCount;
	if (mode_3dr_on) s.m_btOnSwitch = 0x01;
	else s.m_btOnSwitch = 0x00;

	// put in buffer
	memcpy(msg->buff + A3P_HEADER_SIZE, &s, sizeof(s));

	// add header and checksum
	a3p_add_header(msg);
	a3p_add_checksum(msg);
}

void a3p_CMD_3RD_CTRL_TYPE(a3p_msg *msg, WORD ChNum, WORD ChCount,
	float hVoltClamp,
	float lVoltClamp,
	WORD ctrlType,
	BYTE currentRange,
	BYTE voltageRange,
	float ctrlVal[4],
	WORD ctrlValType[4])
{
	msg->size = A3P_HEADER_SIZE + sizeof(MP_3RD_CTRLTYPE);
	msg->buff = new BYTE[msg->size];

	MP_3RD_CTRLTYPE s = {};
	s.m_dwCmd = A3P_CMD_3RD_CTRLTYPE;
	s.m_wChannelNumber = ChNum;
	s.m_wTotalChannelNumber = ChCount;
	s.m_fVclampHigh = hVoltClamp;
	s.m_fVclampLow = lVoltClamp;

	ST_STEPCONTROL_DATA d = {};
	d.m_wCtrlType = ctrlType;
	d.m_btCurrentRange = currentRange;
	d.m_btVoltageRange = voltageRange;

	for (int i = 0; i < 4; i++) {
		d.m_fCtrlVal[i] = ctrlVal[i];
		d.m_wCtrlVal_Type[i] = ctrlValType[i];
	}
	s.m_st_CtrlData = d;

	// add header and checksum
	a3p_add_header(msg);
	a3p_add_checksum(msg);
}

void a3p_CMD_3RD_SAMERANGE_CTRLTYPE(
	a3p_msg *msg,
	WORD ChNum,
	WORD ChCount,
	WORD ctrlType,
	BYTE currentRange,
	BYTE voltageRange,
	float ctrlVal[4],
	WORD ctrlValType[4])
{
	msg->size = A3P_HEADER_SIZE + sizeof(MP_3RD_SAMERANGE_CTRLTYPE);
	msg->buff = new BYTE[msg->size];

	MP_3RD_SAMERANGE_CTRLTYPE s = {};
	s.m_dwCmd = A3P_CMD_3RD_SAMERANGE_CTRLTYPE;
	s.m_wChannelNumber = ChNum;
	s.m_wTotalChannelNumber = ChCount;

	ST_STEPCONTROL_DATA d = {};
	d.m_wCtrlType = ctrlType;
	d.m_btCurrentRange = currentRange;
	d.m_btVoltageRange = voltageRange;

	for (int i = 0; i < 4; i++) {
		d.m_fCtrlVal[i] = ctrlVal[i];
		d.m_wCtrlVal_Type[i] = ctrlValType[i];
	}
	s.m_st_CtrlData = d;

	// add header and checksum
	a3p_add_header(msg);
	a3p_add_checksum(msg);
}

void a3p_CMD_3RD_READDATAORSTATE(
	a3p_msg *msg,
	WORD ChNum,
	WORD ChCount,
	bool readData,
	bool readState)
{

	msg->size = A3P_HEADER_SIZE + sizeof(MP_INDEPENDENT_READDATAORSTATE);
	msg->buff = new BYTE[msg->size];

	MP_INDEPENDENT_READDATAORSTATE s = {};
	s.m_dwCmd = A3P_CMD_3RD_READDATAORSTATE;
	s.m_wChannelNumber = ChNum;
	s.m_wTotalChannelNumber = ChCount;
	s.m_btIsReadData = readData ? 1 : 0;
	s.m_btIsState = readState ? 1 : 0;

	// add header and checksum
	a3p_add_header(msg);
	a3p_add_checksum(msg);
}

// TODO
// A3P_CMD_3RD_INFOCFG

// IN MESSAGES /////////////////////////////////////////////////////////////////

int a3p_CONFIRM_FEEDBACK_size() {
	return A3P_HEADER_SIZE + sizeof(MP_CONFIRM_FEEDBACK);
}

int a3p_parse_CONFIRM_FEEDBACK(
	const a3p_msg *msg, 
	bool *success, 
	DWORD code)
{
	// control expected size
	if (msg->size != (A3P_HEADER_SIZE + sizeof(MP_CONFIRM_FEEDBACK))) return -1;

	// parse message in struct
	MP_CONFIRM_FEEDBACK s = {};
	memcpy(&s, msg->buff+ A3P_HEADER_SIZE, sizeof(s));

	// TODO: control checksum 

	// control code 
	if (s.m_dwCmd != code) return -2;

	// parse success flag
	*success = (s.m_btResult != 0);

	return 0;
}

int a3p_parse_READDATAORSTATE_FEEDBACK(
	const a3p_msg *msg,
	WORD *ChNum,
	WORD *ChCount,
	bool *readData,
	bool *readState,
	BYTE controlState[A3P_MAXCHANNELNO],
	float current[A3P_MAXCHANNELNO],
	float voltage[A3P_MAXCHANNELNO]
)
{
	// control expected size
	if (msg->size != (A3P_HEADER_SIZE + sizeof(MP_INDEPENDENT_READDATAORSTATE_FEEDBACK))) return -1;

	// parse message in struct
	MP_INDEPENDENT_READDATAORSTATE_FEEDBACK s = {};
	memcpy(&s, msg->buff + A3P_HEADER_SIZE, sizeof(s));

	// TODO: control checksum 

	// control code 
	if (s.m_dwCmd != A3P_CMD_3RD_READDATAORSTATE_FEEDBACK) return -2;

	// parse message content
	*ChNum = s.m_wChannelNumber;
	*ChCount = s.m_wTotalChannelNumber;
	*readData = (s.m_btIsReadData != 0);
	*readState = (s.m_btIsState != 0);
	for (int i = 0; i < A3P_MAXCHANNELNO; i++) {
		controlState[i] = s.m_st_Read_Value[i].m_btControlState;
		current[i] = s.m_st_Read_Value[i].m_fCurrent;
		voltage[i] = s.m_st_Read_Value[i].m_fVoltage;
	}

	return 0;
}

// TODO
// A3P_CMD_3RD_ACTIONDONE                  (0XA9EDA800)
// A3P_CMD_SCHEDULE_REPORT_LOGDATA_DELTA   (0XD9418100)