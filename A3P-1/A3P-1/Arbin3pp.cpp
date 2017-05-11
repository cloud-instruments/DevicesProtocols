// Arbin 3rd party protocol helpers functions
// (c)2017 Matteo Lucarelli

#include "stdafx.h"
#include "Arbin3pp.h"

// PROTOCOL DEFINES ////////////////////////////////////////////////////////////

// TODO: ask to Arbin !!! undefined in documentation
#define MAX_BUILDIN_2V_COUNT (1)
#define MAXCHANNELNO (16)
typedef struct {
	BYTE b[13];
}St_SDL_Point_Pack;

// out commands 1ch (SW->FW)
#define CMD_SET_SYSTEMTIME                  (0X5912A100)
#define CMD_3RD_SDU                         (0X59EDA100)
#define CMD_3RD_CTRLTYPE                    (0X59EDA200)
#define CMD_3RD_SAMERANGE_CTRLTYPE          (0X59EDA900)
#define CMD_3RD_READDATAORSTATE             (0X59EDA400)
#define CMD_3RD_INFOCFG                     (0X59EDAA00)

// in commands 1ch (FW->SW)
#define CMD_SET_SYSTEMTIME_FEEDBACK         (0XA912A100)
#define CMD_3RD_SDU_FEEDBACK                (0XA9EDA100)
#define CMD_3RD_CTRLTYPE_FEEDBACK           (0XA9EDA200)
#define CMD_3RD_SAMERANGE_CTRLTYPE_FEEDBACK (0XA9EDA900)
#define CMD_3RD_READDATAORSTATE_FEEDBACK    (0XA9EDA400)
#define CMD_3RD_INFOCFG_FEEDBACK            (0XA9EDAA00)

// in commands 2ch (FW->SW)
#define CMD_3RD_REPORT_NETPORT_STATUS       (0XD9418400)
#define CMD_3RD_ACTIONDONE                  (0XA9EDA800)
#define CMD_SCHEDULE_REPORT_LOGDATA_DELTA   (0XD9418100)

typedef struct
{
	unsigned long Second; // Second
	unsigned short Us100; // 100 microseconds
} 
MsTime; // 6 bytes in total

// TODO: remove m_dwCmd m_dwCmd_Extend

typedef struct 
{
	DWORD m_dwCmd;        // Execution code=0X5912A100
	DWORD m_dwCmd_Extend; // Reserved for extended execution code
	MsTime Time;          // Time
	BYTE m_btReserve;     // Reserved
	BYTE m_3RD_SwitchON;  // 0X05, fixed bytes
	WORD m_u16Token;      // Variable for detecting communication
	DWORD m_dwNone_41;    // Reserved
	BYTE m_btCheckSum[2]; // Checking code
} 
MP_SET_SYSTEMTIME; // 24 bytes in total

typedef struct
{
	DWORD m_dwCmd; // Execution code=0XA912A100
	DWORD m_dwCmd_Extend; // Reserved for extended execution code
	DWORD m_dwReserve[5]; // Reserved
	DWORD m_dwCopyCmd; // Reserved
	DWORD m_dwCopyCmd_Extend; // Reserved
	WORD m_wResult; // 0: failure, 1: success
	WORD m_dwNone_22; // Reserved
	MsTime Time; // Time
	BYTE m_btCheckSum[2]; // Checking code
						  
} 
MP_SET_SYSTEMTIME_FEEDBACK; // 6 bytes in total

typedef struct
{
	DWORD m_dwCmd; // Execution code=0XD9418400
	DWORD m_dwCmd_Extend; // Reserved for extended execution code
	BYTE m_btCheckSum[2]; // Checking code
	WORD m_u16Token;  // This value equals ‘m_u16Token’ in firmware add 1
} 
MP_3RD_REPORT_NETPORT_STATUS; // 12 bytes in total

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
	WORD m_None[5]; // Reserved
	ST_STEPCONTROL_DATA m_st_CtrlData;
	DWORD m_dwReserve2[4]; // Reserved
	BYTE m_btCheckSum[2]; // Checking code
} 
MP_3RD_SAMERANGE_CTRLTYPE;

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
	BYTE m_btNone_11[3]; // Reserved
	float m_fCurrent; // Current Value
	float m_fVoltage; // Voltage Value
	float m_fBuildInVolt[MAX_BUILDIN_2V_COUNT];
} 
ST_READDATAORSTATE_VARIABLE; // 12 bytes in total

typedef struct
{
	DWORD m_dwCmd; // Execution code=0XA9EDA400
	DWORD m_dwCmd_Extend; // Reserved for extended execution code
	WORD m_wChannelNumber; // Channel number
	WORD m_wTotalChannelNumber; // Number of channels
	WORD m_wReserve; // Reserved
	BYTE m_btIsReadData; // 1: Read data, 0: Not read data
	BYTE m_btIsState; // 1: Read state, 0: Not read state
	ST_READDATAORSTATE_VARIABLE m_st_Read_Value[MAXCHANNELNO];
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
	WORD m_wSafetyLimits_Type;
	WORD m_Logflg;
	// Safety types we support
	// If set the interval time to let MCU reports data and states, 0: Unset, 1: Set
	WORD m_Reserved; // Reserved
	ST_3RD_SAFETY m_3rd_safety;
	ST_3RD_LOGDATA m_3rd_Logdata;
	float m_Reserved2[4]; // Reserved
	BYTE m_btCheckSum[2]; // Checking code
}
MP_3RD_INFOCFG; // 118 bytes in total

typedef struct
{
	DWORD m_dwCmd; // Execution code=0XD9418100
	DWORD m_dwCmd_Extend; // Reserved for extended execution code
	BYTE m_btCheckSum[2]; // Checking code
	WORD m_u16Token; unsigned char ItemCount;
	St_SDL_Point_Pack Point[109]; // 13*109=1417 bytes
} 
MP_REPORT_LOGDATA_SDL; // 1428 bytes in total

// GLOBAL VARS /////////////////////////////////////////////////////////////////

// TODO: struct for multiple devices
static unsigned short gU16Token = 0xFFFE;
static char gAddr[INET_ADDRSTRLEN];
static std::vector<a3p_msg> gCh1OutQueue;
static std::vector<a3p_msg> gCh1InQueue;
static std::vector<a3p_msg> gCh2InQueue;
static bool gDoThreadCh1;
static HANDLE gThreadCh1;
static bool gDoThreadCh2;
static HANDLE gThreadCh2;

// TODO: error msg cue

// CH1 /////////////////////////////////////////////////////////////////////////

int a3p_send(a3p_msg msg) {

	// TODO
	return -1;
}

int a3p_get_ch1(a3p_msg *msg) {

	// TODO
	return -1;
}

DWORD WINAPI a3p_ch1_thread_funct(LPVOID lpParam) {

	// connect device
	SOCKET sock = w32_tcp_socket_client_create(gAddr, A3P_CH1_PORT);
	if (sock == INVALID_SOCKET) {
		// TODO manage error
	}

	// enable keepalive 
	w32_tcp_socket_keepalive(sock, 5, 1);

	time_t prevTime=0;

	while (gDoThreadCh1) {

		// each 5s send CMD_SET_SYSTEMTIME
		if (time(NULL)-prevTime >= 5){

			// TODO: send CMD_SET_SYSTEMTIME

			prevTime = time(NULL);
		}


		//   send data fron queue
		//   read answer in queue


		// TODO: define or parameter
		Sleep(100);

	}

	w32_tcp_socket_close(sock);
	return 0;
}

int a3p_start_ch1_thread() {

	if (gDoThreadCh1) return 0;
	gDoThreadCh1 = true;
	gThreadCh1 = CreateThread(0, 0, a3p_ch1_thread_funct, NULL, 0, NULL);
	return 1;
}
int a3p_stop_ch1_thread() {

	if (!gDoThreadCh1) return 0;	gDoThreadCh1 = false;

	// TODO: error msg
	if (WaitForSingleObject(gThreadCh1, 2000) == WAIT_TIMEOUT) {		TerminateThread(gThreadCh1, 0);	}
	return 1;
}

// CH2 ////////////////////////////////////////////////////////////////////////

int a3p_get_ch2(a3p_msg *msg) {
	// TODO
	return -1;
}

DWORD WINAPI a3p_ch2_thread_funct(LPVOID lpParam) {

	// connect device
	SOCKET sock = w32_tcp_socket_client_create(gAddr, A3P_CH2_PORT);
	if (sock == INVALID_SOCKET) {
		// TODO manage error
	}

	while (gDoThreadCh2) {

		//   receive data from ch2
		//   gU16Token msg updates gU16Token value
		//   other msg are put in incoming queue
	}

	w32_tcp_socket_close(sock);
	return 0;
}

int a3p_start_ch2_thread() {

	if (gDoThreadCh2) return 0;
	gDoThreadCh2 = true;
	gThreadCh2 = CreateThread(0, 0, a3p_ch2_thread_funct, NULL, 0, NULL);
	return 1;
}
int a3p_stop_ch2_thread() {

	if (!gDoThreadCh2) return 0;	gDoThreadCh2 = false;

	// TODO: error msg
	if (WaitForSingleObject(gThreadCh2, 2000) == WAIT_TIMEOUT) {		TerminateThread(gThreadCh2, 0);	}
	return 1;
}

// PUBLICS ////////////////////////////////////////////////////////////////////

int a3p_connect(const char* addr) {

	strncpy_s(gAddr, INET6_ADDRSTRLEN, addr, INET6_ADDRSTRLEN);

	// start threads
	a3p_start_ch1_thread();
	a3p_start_ch2_thread();

	return 0;
}

int a3p_disconnect() {

	//( stop threads
	a3p_stop_ch2_thread();
	a3p_stop_ch1_thread();

	return 0;
}