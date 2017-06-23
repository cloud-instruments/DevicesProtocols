// dllmain.c : main functions for ciupServerMaccor.dll

// NOTES for Visual c++ 
// To export undecorated names a .def is required:
// LIBRARY CIUPSERVERMACCORDLL
// EXPORTS
//     OnLoadRevA
//     OnStepStartRevA
//     OnStepEndRevA
//     OnUnLoadRevA
//     OnSuspendRevA
//     OnResumeRevA
//     GetSetpointRevA
//
// Export can be verified in Developer console with dumpbin.exe /EXPORTS
//
// Project -> properties -> linker -> debugging -> generate debug info = NO

// TODO: use PReportCallBack to log errors on Maccor application

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <tchar.h>
#include <stdio.h>
#include <iomanip>

#include "ciupServerMaccorDll.h"
#include "..\ciupClientTest\streamlog.h"
#include "..\w32_message_pipe_test\w32_message_pipe.h"
#include "..\ciupClientDll\ciupCommon.h"

#define LOGDIR L"C:\\ciupServer\\log-ciupServerMaccorDll"
#define INIFILE L"C:\\ciupServer\\ciupServerMaccorDll.ini"

// counters for channels
USHORT gCounter[CIUP_CH_MAX_COUNT] = {0};

// the pipe to send data to server
HANDLE gPipe=NULL;
BOOL gDisablePipe = FALSE;

// log globals
std::ofstream *gLogStream = NULL;
streamlog *gLog = NULL;

// data logging functions //////////////////////////////////////////////////////

// log TSpecData struct
void TSpecDataLog(TSpecData* d) {

	if (!gLog) return;

	if (d == NULL) {
		*gLog << "TSpecDataLog NULL" << streamlog::error << std::endl;
		return;
	}

	*gLog << "TSpecData "
		<< "'Vmax':'" << d->Vmax << "'"
		<< ", 'Vmin':'" << d->Vmin << "'"
		<< ", 'ChI':'" << d->ChI << "'"
		<< ", 'DisI':'" << d->DisI << "'"
		<< streamlog::debug << std::endl;

	for (int i = 0; i < sizeof(*d); ++i)
		*gLog << std::hex << std::setfill('0') << std::setw(2) << (int)*((unsigned char*)d+i) << " ";
	*gLog << streamlog::debug << std::endl;
}

// log TStatusData struct
void TSStatusDataLog(TStatusData* d) {

	if (!gLog) return;

	if (d == NULL) {
		*gLog << "TStatusData NULL" << streamlog::error << std::endl;
		return;
	}

	*gLog << "TStatusData "
		<< "'RF1':'" << (int)d->RF1 << "'"
		<< ", 'RF2':'" << (int)d->RF2 << "'"
		<< ", 'Cycle':'" << d->Cycle << "'"
		<< ", 'Step':'" << d->Step << "'"
		<< ", 'TestTime':'" << d->TestTime << "'"
		<< ", 'StepTime':'" << d->StepTime << "'"
		<< ", 'Voltage':'" << d->Voltage << "'"
		<< ", 'Current':'" << d->Current << "'"
		<< ", 'Capacity':'" << d->Capacity << "'"
		<< ", 'HCCapacity':'" << d->HCCapacity << "'"
		<< ", 'LHCCapacity':'" << d->LHCCapacity << "'"
		<< ", 'Energy':'" << d->Energy << "'"
		<< ", 'HCEnergy':'" << d->HCEnergy << "'"
		<< ", 'tFactor':'" << d->tFactor << "'"
		<< streamlog::debug << std::endl;

		/*
		float   AUXdata[48];			//Values of up to 48 assigned auxiliary inputs
		uint8   AUXinput[48];			//Auxiliary input channel
		uint16  AUXtype[48];			//Auxiliary input type: 1: Voltage; 2: Thermocple; 3: Thermistor; 4: Pressure; 5: Resistance;
		char    AUXunit[48][3];			//Auxiliary input engineering unit
		uint8 SMBdata[512];				//512 byte of SMB data
		uint8 SMBHdrData[512];			//512 byte of SMB header data
		TSMBinfo SMBDataInfo[64];		//Specification of up to 64 assigned SMB registers. The SMB readings are stored in the StatusData.SMBdata
		TSMBinfo SMBHdrInfo[64];		//Specification of up to 64 assigned SMB registers. The SMB readings are stored in the StatusData.SMBHdrData
		*/

	for (int i = 0; i < sizeof(*d); ++i)
		*gLog << std::hex << std::setfill('0') << std::setw(2) << (int)*((unsigned char*)d + i) << " ";
	*gLog << streamlog::debug << std::endl;
}

// log TTestDataRevA struct
void TTestDataRevALog(TTestDataRevA* d) {

	if (!gLog) return;

	if (d == NULL) {
		*gLog << "TTestDataRevA NULL" << streamlog::error << std::endl;
		return;
	}

	*gLog << "TTestDataRevA "
		<< "'APIversion':'" << d->APIversion << "'"
		<< ", 'SWversion':'" << d->SWversion << "'"
		<< ", 'DLLversion':'" << d->DLLversion << "'"
		<< ", 'DLLversionString':'" << d->DLLversionString << "'"
		<< ", 'SystemID':'" << d->SystemID << "'"
		<< ", 'TestName':'" << d->TestName << "'"
		<< ", 'TestComment':'" << d->TestComment << "'"
		<< ", 'ProcedureName':'" << d->ProcedureName << "'"
		<< ", 'ProcedureComment':'" << d->ProcedureComment << "'"
		<< ", 'TimeStamp':'" << d->TimeStamp << "'"
		<< ", 'MaccorPath':'" << d->MaccorPath << "'"
		<< ", 'SysPath':'" << d->SysPath << "'"
		<< ", 'ProcPath':'" << d->ProcPath << "'"
		<< ", 'StepNote':'" << d->StepNote << "'"
		<< ", 'CRate':'" << d->CRate << "'"
		<< ", 'Mass':'" << d->Mass << "'"
		<< streamlog::debug << std::endl;

	for (int i = 0; i < sizeof(*d); ++i)
		*gLog << std::hex << std::setfill('0') << std::setw(2) << (int)*((unsigned char*)d + i) << " ";
	*gLog << streamlog::debug << std::endl;
}

// log TSpecData struct
void TOutDataLog(TOutData* d) {

	if (!gLog) return;

	if (d == NULL) {
		*gLog << "TOutData NULL" << streamlog::error << std::endl;
		return;
	}

	*gLog << "TSpecData "
		<< "'Current':'" << d->Current << "'"
		<< ", 'Flags':'" << d->Flags << "'"
		<< ", 'Power':'" << d->Power << "'"
		<< ", 'Resistance':'" << d->Resistance << "'"
		<< ", 'Voltage':'" << d->Voltage << "'"
		<< streamlog::debug << std::endl;

	for (int i = 0; i < sizeof(*d); ++i)
		*gLog << std::hex << std::setfill('0') << std::setw(2) << (int)*((unsigned char*)d + i) << " ";
	*gLog << streamlog::debug << std::endl;
}

// DLL entry and exit procedure
BOOL APIENTRY DllMain(HINSTANCE hInstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:

			// load config parameters from INIFILE
			if (GetPrivateProfileInt(L"GLOBAL", L"enablelog", 0, INIFILE) != 0) {

				// init logging

				int loglevel = GetPrivateProfileInt(L"GLOBAL", L"loglevel", 0, INIFILE);
				streamlog::level lfilter = streamlog::error;

				switch (loglevel) {
				case 0: lfilter = streamlog::error; break;
				case 1: lfilter = streamlog::warning; break;
				case 2: lfilter = streamlog::trace; break;
				case 3: lfilter = streamlog::debug; break;
				default:break;
				}

				// creat log directory
				std::wstring logpath = LOGDIR;
				CreateDirectory(logpath.c_str(), NULL);

				// build log file name
				DWORD ppid = GetCurrentProcessId();
				logpath += L"\\ciupServerMaccorDll_" + std::to_wstring(time(NULL)) + L"-" + std::to_wstring(ppid) + L".log";

				// open log file
				gLogStream = new std::ofstream(logpath, std::ios::app);
				gLog = new streamlog(*gLogStream, lfilter);

				*gLog << "DLL_PROCESS_ATTACH, pid:" << ppid << streamlog::trace << std::endl;
			}

			// connect to the pipe server
			gDisablePipe = (GetPrivateProfileInt(L"GLOBAL", L"disablepipe", 0, INIFILE) != 0);
			if (!gDisablePipe) {
				gPipe = w32_named_pipe_connect(CIUP_MACCOR_PIPENAME);
				if ((!gPipe) && (gLog)) {
					*gLog << "Cannot connect to named pipe " << CIUP_MACCOR_PIPENAME << streamlog::error << std::endl;
				}
			}
            break;

        case DLL_THREAD_ATTACH:

			if (gLog) *gLog << "DLL_THREAD_ATTACH, tid:" << GetCurrentThreadId() << streamlog::trace << std::endl;
            break;

        case DLL_THREAD_DETACH:
 
			if (gLog) *gLog << "DLL_THREAD_DETACH, tid:" << GetCurrentThreadId() << streamlog::trace << std::endl;
            break;

        case DLL_PROCESS_DETACH:

			if (gLog) {
				*gLog << "DLL_PROCESS_DETACH" << streamlog::trace << std::endl;
				delete gLog;
				gLog = NULL;
				delete gLogStream;
				gLogStream = NULL;
			}

			// close server connection
			if (gPipe) w32_message_pipe_close(&gPipe);

            break;
    }
    return TRUE;
}

// called once when the test is started and the DLL is loaded.
__declspec(dllexport) int __stdcall OnLoadRevA( 
	int channel,
	int unused_flags,
	TSpecData* SpecData,
	TStatusData* StatusData,
	TTestDataRevA* TestData,               
	PReportCallBack ReportCallBack,
	PWriteSMBCallBack WriteSMBCallBack )
{
	if (gLog) {
		*gLog << "OnLoadRevA, channel:" << channel << streamlog::trace << std::endl;
		TSpecDataLog(SpecData);
		TSStatusDataLog(StatusData);
		TTestDataRevALog(TestData);
	}

	return 0;
}

// This function is called once when a DLL step is started
__declspec(dllexport) int __stdcall OnStepStartRevA( 
	int channel, 
	int unused_flags, 
	int step, 
	TSpecData* SpecData, 
	TStatusData* StatusData, 
	TTestDataRevA* TestData, 
	PReportCallBack ReportCallBack, 
	PWriteSMBCallBack WriteSMBCallBack )
{
	if (gLog) {
		*gLog << "OnStepStartRevA channel:" << channel << " step:" << step << streamlog::trace << std::endl;
		TSpecDataLog(SpecData);
		TSStatusDataLog(StatusData);
		TTestDataRevALog(TestData);
	}

	return 0;
}

// This function is called once when a DLL step ends
__declspec(dllexport) int __stdcall OnStepEndRevA( 
	int channel,
	int unused_flags, 
	int step, 
	TSpecData* SpecData, 
	TStatusData* StatusData, 
	PReportCallBack ReportCallBack, 
	PWriteSMBCallBack WriteSMBCallBack )
{
	if (gLog) {
		*gLog << "OnStepEndRevA channel:" << channel << " step:" << step << streamlog::trace << std::endl;
		TSpecDataLog(SpecData);
		TSStatusDataLog(StatusData);
	}

	return 0;
}

// This function is called once when the test is ended and the DLL is unloaded.
__declspec(dllexport) int __stdcall OnUnLoadRevA( 
	int channel, 
	int unused_flags, 
	PReportCallBack ReportCallBack, 
	PWriteSMBCallBack WriteSMBCallBack )
{
	if (gLog) *gLog << "OnUnLoadRevA channel:" << channel << streamlog::trace << std::endl;

	return 0;
}

// This function is called once when a DLL step is suspended 
__declspec(dllexport) int __stdcall OnSuspendRevA( 
	int channel, 
	int unused_flags, 
	TSpecData* SpecData, 
	TStatusData* StatusData, 
	TTestDataRevA* TestData, 
	PReportCallBack ReportCallBack, 
	PWriteSMBCallBack WriteSMBCallBack )
{
	if (gLog) {
		*gLog << "OnSuspendRevA channel:" << channel << streamlog::trace << std::endl;
		TSpecDataLog(SpecData);
		TSStatusDataLog(StatusData);
		TTestDataRevALog(TestData);
	}

	return 0;
}

// This function is called once when a DLL step is resumed 
__declspec(dllexport) int __stdcall OnResumeRevA( 
	int channel, 
	int unused_flags, 
	TSpecData* SpecData, 
	TStatusData* StatusData, 
	TTestDataRevA* TestData, 
	PReportCallBack ReportCallBack, 
	PWriteSMBCallBack WriteSMBCallBack )
{
	if (gLog) {
		*gLog << "OnResumeRevA channel:" << channel << streamlog::trace << std::endl;
		TSpecDataLog(SpecData);
		TSStatusDataLog(StatusData);
		TTestDataRevALog(TestData);
	}

	return 0;
}

// This function is called up to 10 times per second.The return values are used as setpoints.
// The set points and flag are returned by setting the values in the OutData record
__declspec(dllexport) int __stdcall GetSetpointRevA( 
	int channel, 
	int unused_flags, 
	TSpecData* SpecData, 
	TStatusData* StatusData, 
	TOutData* OutData, 
	PReportCallBack ReportCallBack, 
	PWriteSMBCallBack WriteSMBCallBack )
{
	if (!gDisablePipe) {

		// control pipe
		if (!gPipe) {

			// try reconnect pipe server
			if (gLog) *gLog << "Server disconnected, trying to reconnect" << streamlog::warning << std::endl;
			gPipe = w32_named_pipe_connect(CIUP_MACCOR_PIPENAME);
			if ((!gPipe) && (gLog)) {
				*gLog << "Cannot connect to named pipe " << CIUP_MACCOR_PIPENAME << streamlog::error << std::endl;
			}
		}

		// write point to the pipe
		if (gPipe) {

			// fill ciupDataPoint
			ciupDataPoint p;
			if (channel < CIUP_CH_MAX_COUNT) p.counter = gCounter[channel];
			p.channel = channel;
			p.Stime = StatusData->TestTime;
			p.Ktemp = 0; // unkknow in maccor
			p.Acurr = StatusData->Current;
			p.Vdiff = StatusData->Voltage;
			p.AHcap = StatusData->Capacity;

			DWORD wrote;
			DWORD ret = w32_message_pipe_write(gPipe, &p, sizeof(ciupDataPoint), &wrote);

			if (ret == ERROR_BROKEN_PIPE) {
				if (gLog) *gLog << "Server disconnected, GLE=" << GetLastError() << streamlog::error << std::endl;
				w32_message_pipe_close(&gPipe);
			}
			else if (ret != 0) {
				if (gLog) *gLog << "WriteFile failed, GLE=" << ret << streamlog::error << std::endl;
				w32_message_pipe_close(&gPipe);
			}
			else if (wrote != sizeof(ciupDataPoint)) {
				if (gLog) *gLog << "Uncomplete write, wrote:"<< wrote << streamlog::error << std::endl;
				w32_message_pipe_close(&gPipe);
			}
		}
	}

	// logging
	if (gLog) {
		*gLog << "GetSetpointRevA channel:" << channel << streamlog::trace << std::endl;
		TSpecDataLog(SpecData);
		TSStatusDataLog(StatusData);
		TOutDataLog(OutData);
	}


	// TODO: outdata
	// The set points and flag are returned by setting the values in the OutData record
	// For now negative numbers means ignore
	OutData->Current = -1;
	OutData->Voltage= -1;
	OutData->Power = -1;
	OutData->Resistance= -1;
	/*
	typedef struct
	{
		float   Current;				// Current set point in A. Negative values indicates ignore.
		float   Voltage;				// Voltage set point in V. Negative values indicates ignore.
		float   Power;					// Power set point in W. Negative values indicates ignore.
		float   Resistance;				// Resistance set point in Ohm. Negative values indicates ignore.
		int     Flags;					// 1. OK, continue in current step
										// 2. OK, go to next step
										// 3. Error - terminate test
										// 4. Pause
	} TOutData;
	*/

	// update channel counter
	if (channel < CIUP_CH_MAX_COUNT) {
		gCounter[channel]++;
	}
	else if (gLog) {
		*gLog << "Channel:" << channel << " oor (max:" << CIUP_CH_MAX_COUNT << ")" << streamlog::error << std::endl;
	}

	return 0;
}

