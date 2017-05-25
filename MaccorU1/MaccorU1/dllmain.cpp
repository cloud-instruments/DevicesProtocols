// (c)2017 Matteo Lucarelli matteo.lucarelli@toptal.com
// Date 20170519
// dllmain.c : main functions for MaccorU1.dll
////////////////////////////////////////////////////////////////////////////////

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <tchar.h>
#include <stdio.h>

#include "MaccorU1.h"
#include "streamlog.h"

// log globals
DWORD ppid=0;
std::wstring logpath;
std::ofstream *logStream = NULL;
streamlog *plog = NULL;

// data logging functions //////////////////////////////////////////////////////

// log TSpecData struct
void TSpecDataLog(TSpecData* d) {

	if (d == NULL) {
		*plog << "TSpecData NULL" << std::endl;
		return;
	}

	*plog << "TSpecData Vmax:" << d->Vmax 
		<< " Vmin:" << d->Vmin 
		<< " ChI:" << d->ChI 
		<< " DisI:" << d->DisI 
		<< std::endl;
}

// log TStatusData struct
void TSStatusDataLog(TStatusData* d) {

	if (d == NULL) {
		*plog << "TStatusData NULL" << std::endl;
		return;
	}

	*plog << "TStatusData RF1:" << d->RF1 
		<< " RF2:" << d->RF2
		<< " Cycle:" << d->Cycle
		<< " Step:" << d->Step
		<< " TestTime:" << d->TestTime
		<< " StepTime:" << d->StepTime
		<< " Voltage:" << d->Voltage
		<< " Current:" << d->Current
		<< " Capacity:" << d->Capacity
		<< " HCCapacity:" << d->HCCapacity
		<< " LHCCapacity:" << d->LHCCapacity
		<< " Energy:" << d->Energy
		<< " HCEnergy:" << d->HCEnergy
		<< " tFactor:" << d->tFactor
		<< std::endl;

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
}

// log TTestDataRevA struct
void TTestDataRevALog(TTestDataRevA* d) {

	if (d == NULL) {
		*plog << "TTestDataRevA NULL" << std::endl;
		return;
	}

	*plog << "TTestDataRevA APIversion:" << d->APIversion
		<< " SWversion:" << d->SWversion
		<< " DLLversion:" << d->DLLversion
		<< " DLLversionString:" << d->DLLversionString
		<< " SystemID:" << d->SystemID
		<< " TestName:" << d->TestName
		<< " TestComment:" << d->TestComment
		<< " ProcedureName:" << d->ProcedureName
		<< " ProcedureComment:" << d->ProcedureComment
		<< " TimeStamp:" << d->TimeStamp
		<< " MaccorPath:" << d->MaccorPath
		<< " ProcPath:" << d->ProcPath
		<< " StepNote:" << d->StepNote
		<< " CRate:" << d->CRate
		<< " Mass:" << d->Mass
		<< std::endl;

		/*
		float   VRate;					//Not used yet
		float	Prate;					//Not used yet
		float	Rrate;					//Not used yet
		float	Volume;					//Not used yet
		float	Area;					//Not used yet
		*/
}

// log TSpecData struct
void TOutDataLog(TOutData* d) {

	if (d == NULL) {
		*plog << "TOutData NULL" << std::endl;
		return;
	}

	*plog << "TSpecData Current:" << d->Current
		<< " Flags:" << d->Flags
		<< " Power:" << d->Power
		<< " Resistance:" << d->Resistance
		<< " Voltage:" << d->Voltage
		<< std::endl;
}

// DLL entry and exit procedure
BOOL APIENTRY DllMain(HINSTANCE hInstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
    switch (fdwReason)
    {
        case DLL_PROCESS_ATTACH:

			// create log sdirectory
			logpath = L"C:\\MaccorU1";
			CreateDirectory(logpath.c_str(), NULL);

			// build log file name
			ppid = GetCurrentProcessId();
			logpath += L"\\" + std::to_wstring(time(NULL)) + L"-" + std::to_wstring(ppid) + L".log";

			// open log file
			logStream = new std::ofstream(logpath, std::ios::app);
			plog = new streamlog(*logStream, streamlog::debug);

			*plog << "DLL_PROCESS_ATTACH, pid:" << ppid << std::endl;

            break;

        case DLL_THREAD_ATTACH:

			*plog << "DLL_THREAD_ATTACH, tid:" << GetCurrentThreadId() << std::endl;
            break;

        case DLL_THREAD_DETACH:
 
			*plog << "DLL_THREAD_DETACH, tid:" << GetCurrentThreadId() << std::endl;
            break;

        case DLL_PROCESS_DETACH:

			*plog << "DLL_PROCESS_DETACH" << std::endl;
			delete plog;
			delete logStream;
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
	*plog << "OnLoadRevA, channel:"<< channel << std::endl;
	TSpecDataLog(SpecData);
	TSStatusDataLog(StatusData);
	TTestDataRevALog(TestData);

	// TODO: callbacks

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
	*plog << "OnStepStartRevA channel:" << channel <<" step:" << step << std::endl;
	TSpecDataLog(SpecData);
	TSStatusDataLog(StatusData);
	TTestDataRevALog(TestData);

	// TODO: callbacks

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
	*plog << "OnStepEndRevA channel:" << channel << " step:" << step << std::endl;
	TSpecDataLog(SpecData);
	TSStatusDataLog(StatusData);

	// TODO: callbacks

	return 0;
}

// This function is called once when the test is ended and the DLL is unloaded.
__declspec(dllexport) int __stdcall OnUnLoadRevA( 
	int channel, 
	int unused_flags, 
	PReportCallBack ReportCallBack, 
	PWriteSMBCallBack WriteSMBCallBack )
{
	*plog << "OnUnLoadRevA channel:" << channel << std::endl;

	// TODO: callbacks

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
	*plog << "OnSuspendRevA channel:" << channel << std::endl;

	TSpecDataLog(SpecData);
	TSStatusDataLog(StatusData);
	TTestDataRevALog(TestData);

	// TODO: callbacks

	/*TSMBGenericWriteData SMBGenericWriteData;
	int i;
	char s[] = "Hello World!";

	i = ReportCallBack(s, 3, -37, TRUE);

	SMBGenericWriteData.BusAddr = 0x16;
	SMBGenericWriteData.BusType = 1;
	SMBGenericWriteData.StartDataAddr = 1;
	SMBGenericWriteData.EndDataAddr = 1;
	SMBGenericWriteData.NumOfBytes = 2;
	SMBGenericWriteData.CheckSum = 0;
	SMBGenericWriteData.MinDelayToNext10ms = 100;
	SMBGenericWriteData.Data[0] = 0x64;
	SMBGenericWriteData.Data[1] = 0x00;
	i = WriteSMBCallBack(&SMBGenericWriteData);*/

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
	*plog << "OnResumeRevA channel:" << channel << std::endl;

	TSpecDataLog(SpecData);
	TSStatusDataLog(StatusData);
	TTestDataRevALog(TestData);

	// TODO: callbacks

	/*
	TSMBGenericWriteData SMBGenericWriteData;
	int i;
	char s[] = "Hello World!";

	i = ReportCallBack(s, 1, 25, TRUE);

	SMBGenericWriteData.BusAddr = 0x16;
	SMBGenericWriteData.BusType = 1;
	SMBGenericWriteData.StartDataAddr = 1;
	SMBGenericWriteData.EndDataAddr = 1;
	SMBGenericWriteData.NumOfBytes = 2;
	SMBGenericWriteData.CheckSum = 0;
	SMBGenericWriteData.MinDelayToNext10ms = 100;
	SMBGenericWriteData.Data[0] = 0x2C;
	SMBGenericWriteData.Data[1] = 0x01;
	i = WriteSMBCallBack(&SMBGenericWriteData);
	*/

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
	*plog << "GetSetpointRevA channel:" << channel << std::endl;
	TSpecDataLog(SpecData);
	TSStatusDataLog(StatusData);
	TOutDataLog(OutData);

	// TODO: outdata
	// The set points and flag are returned by setting the values in the OutData record
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

	// TODO: callbacks

	return 0;
}

