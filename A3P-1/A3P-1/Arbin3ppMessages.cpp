/*
Copyright(c) <2018> <University of Washington>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

// Arbin 3rd party protocol messages build and parse functions

#include "stdafx.h"

#include "Arbin3ppMessages.h"

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

// OUT MESSAGES (PC->MCU on 1ch) ///////////////////////////////////////////////

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

	// put in buffer
	memcpy(msg->buff + A3P_HEADER_SIZE, &s, sizeof(s));

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

	// put in buffer
	memcpy(msg->buff + A3P_HEADER_SIZE, &s, sizeof(s));

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

	// put in buffer
	memcpy(msg->buff + A3P_HEADER_SIZE, &s, sizeof(s));

	// add header and checksum
	a3p_add_header(msg);
	a3p_add_checksum(msg);
}

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
	float timeInterval)
{
	msg->size = A3P_HEADER_SIZE + sizeof(MP_3RD_INFOCFG);
	msg->buff = new BYTE[msg->size];

	MP_3RD_INFOCFG s = {};
	s.m_dwCmd = A3P_CMD_3RD_INFOCFG;
	s.m_wChannelNumber = ChNum;
	s.m_wTotalChannelNumber = ChCount;

	s.m_Safetyflg = safetyFlg;
	s.m_wSafetyLimits_Type = safetyLimitsType;
	s.m_Logflg = logFlag;

	ST_3RD_SAFETY d1 = {};
	d1.m_stCurrent.m_fMax = currentMax;
	d1.m_stCurrent.m_fMin = currentMin;
	d1.m_stVoltage.m_fMax = voltageMax;
	d1.m_stVoltage.m_fMin = voltageMin;
	d1.m_stPower.m_fMax = powerMax;
	d1.m_stPower.m_fMin = powerMin;
	d1.m_stVoltageClamp.m_fMax = voltclampMax;
	d1.m_stVoltageClamp.m_fMin = voltclampMin;
	s.m_3rd_safety = d1;

	ST_3RD_LOGDATA d2 = {};
	d2.TimeInterval = timeInterval;
	s.m_3rd_Logdata = d2;

	// put in buffer
	memcpy(msg->buff + A3P_HEADER_SIZE, &s, sizeof(s));

	// add header and checksum
	a3p_add_header(msg);
	a3p_add_checksum(msg);

}
// IN MESSAGES (MCU->PC) ///////////////////////////////////////////////////////

// MCU->PC on ch1 (ack for commands)
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

// MCU->PC on ch1 (state feedback)
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

	// control code 
	if (s.m_dwCmd != A3P_CMD_3RD_READDATAORSTATE_FEEDBACK) return -2;

	// TODO: control checksum 

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

// MCU->PC 2ch after CMD_3RD_CTRLTYPE and CMD_3RD_SDU
int a3p_parse_CMD_3RD_ACTIONDONE(
	const a3p_msg *msg,
	WORD *channelNumber,
	BYTE *ctrltype,
	BYTE *finishFlg,
	BYTE *event,
	BYTE *channelStatus,
	WORD *unsafeType,
	float *ctrlValue
	)
{
	// control expected size
	if (msg->size != (A3P_HEADER_SIZE + sizeof(MP_3RD_ACTIONDONE))) return -1;

	// parse message in struct
	MP_3RD_ACTIONDONE s = {};
	memcpy(&s, msg->buff + A3P_HEADER_SIZE, sizeof(s));

	// control code 
	if (s.m_dwCmd != A3P_CMD_3RD_ACTIONDONE) return -2;

	// TODO: control checksum 

	// parse message content
	*channelNumber = s.m_wChannelNumber;
	*ctrltype = s.m_Ctrltype;
	*finishFlg = s.m_FinishFlg;
	*event = s.m_btEvent;
	*channelStatus = s.m_btChannelStatus;
	*unsafeType = s.m_wUnsafe_Type;
	*ctrlValue = s.m_CtrlValue;

	return 0;
}
// MCU->PC on ch2 (scheduled by A3P_CMD_3RD_INFOCFG)
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
	unsigned char ucChannelIdx[A3P_MAX_POINTS])
{
	// control expected size
	if (msg->size != (A3P_HEADER_SIZE + sizeof(MP_REPORT_LOGDATA_SDL))) return -1;

	// parse message in struct
	MP_REPORT_LOGDATA_SDL s = {};
	memcpy(&s, msg->buff + A3P_HEADER_SIZE, sizeof(s));

	// control code 
	if (s.m_dwCmd != A3P_CMD_SCHEDULE_REPORT_LOGDATA_DELTA) return -2;

	// control code 
	if (s.m_dwCmd != A3P_CMD_3RD_READDATAORSTATE_FEEDBACK) return -2;

	// TODO: control checksum 

	// parse message content
	*itemCount = s.m_u16Token;
	if (*itemCount > A3P_MAX_POINTS) *itemCount = A3P_MAX_POINTS;
	for (int i = 0; i < *itemCount; i++) {
		u32Second[i] = s.Point[i].u32Second;
		u16Us100[i] = s.Point[i].u16Us100;
		ucType[i] = s.Point[i].ucType;
		bIV[i] = s.Point[i].bIV;
		ucSensor[i] = s.Point[i].ucSensor;
		ucValueType[i] = s.Point[i].ucValueType;
		fValue[i] = s.Point[i].fValue;
		ucChannelIdx[i] = s.Point[i].ucChannelIdx;
	}

	return 0;
}