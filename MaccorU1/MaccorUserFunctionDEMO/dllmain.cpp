/****************************************************************************
*                                                                          *
* File    : dllmain.c                                                      *
*                                                                          *
* Purpose : Generic Win32 Dynamic Link Library (DLL).                      *
*                                                                          *
* History : Date      Reason                                               *
*           00/00/00  Created                                              *
*                                                                          *
****************************************************************************/

#define WIN32_LEAN_AND_MEAN  /* speed up */
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <tchar.h>
#include <stdio.h>

/*
* Include our "interface" file and define a symbol
* to indicate that we are *building* the DLL.
*/

#include "MacUserF.h"

/****************************************************************************
*                                                                          *
* Function: DllMain                                                        *
*                                                                          *
* Purpose : DLL entry and exit procedure.                                  *
*                                                                          *
* History : Date      Reason                                               *
*           00/00/00  Created                                              *
*                                                                          *
****************************************************************************/

BOOL APIENTRY DllMain(HINSTANCE hInstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	switch (fdwReason)
	{
	case DLL_PROCESS_ATTACH:
		/*
		* Microsoft says:
		*
		* The DLL is being loaded into the virtual address space of the current
		* process as a result of the process starting up or as a result of a call
		* to LoadLibrary. DLLs can use this opportunity to initialize any instance
		* data or to use the TlsAlloc function to allocate a thread local storage
		* (TLS) index.
		*/
		break;

	case DLL_THREAD_ATTACH:
		/*
		* Microsoft says:
		*
		* The current process is creating a new thread. When this occurs, the system
		* calls the entry-point function of all DLLs currently attached to the process.
		* The call is made in the context of the new thread. DLLs can use this opportunity
		* to initialize a TLS slot for the thread. A thread calling the DLL entry-point
		* function with DLL_PROCESS_ATTACH does not call the DLL entry-point function
		* with DLL_THREAD_ATTACH.
		*
		* Note that a DLL's entry-point function is called with this value only by threads
		* created after the DLL is loaded by the process. When a DLL is loaded using
		* LoadLibrary, existing threads do not call the entry-point function of the newly
		* loaded DLL.
		*/
		break;

	case DLL_THREAD_DETACH:
		/*
		* Microsoft says:
		*
		* A thread is exiting cleanly. If the DLL has stored a pointer to allocated memory
		* in a TLS slot, it should use this opportunity to free the memory. The system calls
		* the entry-point function of all currently loaded DLLs with this value. The call
		* is made in the context of the exiting thread.
		*/
		break;

	case DLL_PROCESS_DETACH:
		/*
		* Microsoft says:
		*
		* The DLL is being unloaded from the virtual address space of the calling process as
		* a result of unsuccessfully loading the DLL, termination of the process, or a call
		* to FreeLibrary. The DLL can use this opportunity to call the TlsFree function to
		* free any TLS indices allocated by using TlsAlloc and to free any thread local data.
		*
		* Note that the thread that receives the DLL_PROCESS_DETACH notification is not
		* necessarily the same thread that received the DLL_PROCESS_ATTACH notification.
		*/
		break;
	}

	/* Return success */
	return TRUE;
}

/****************************************************************************
*                                                                          *
* Function: SampleFunction                                                 *
*                                                                          *
* Purpose : Sample function which does nothing useful.                     *
*                                                                          *
* History : Date      Reason                                               *
*           00/00/00  Created                                              *
*                                                                          *
****************************************************************************/


__declspec(dllexport) int __stdcall OnLoadRevA(int channel, int unused_flags, TSpecData* SpecData, TStatusData* StatusData, TTestDataRevA* TestData, PReportCallBack ReportCallBack, PWriteSMBCallBack WriteSMBCallBack)
{

	return 0;
}

__declspec(dllexport) int __stdcall OnStepStartRevA(int channel, int unused_flags, int step, TSpecData* SpecData, TStatusData* StatusData, TTestDataRevA* TestData, PReportCallBack ReportCallBack, PWriteSMBCallBack WriteSMBCallBack)
{

	return 0;
}

__declspec(dllexport) int __stdcall OnStepEndRevA(int channel, int unused_flags, int step, TSpecData* SpecData, TStatusData* StatusData, PReportCallBack ReportCallBack, PWriteSMBCallBack WriteSMBCallBack)
{

	return 0;
}

__declspec(dllexport) int __stdcall OnUnLoadRevA(int channel, int unused_flags, PReportCallBack ReportCallBack, PWriteSMBCallBack WriteSMBCallBack)
{

	return 0;
}

__declspec(dllexport) int __stdcall OnSuspendRevA(int channel, int unused_flags, TSpecData* SpecData, TStatusData* StatusData, TTestDataRevA* TestData, PReportCallBack ReportCallBack, PWriteSMBCallBack WriteSMBCallBack)
{

	return 0;
}

__declspec(dllexport) int __stdcall OnResumeRevA(int channel, int unused_flags, TSpecData* SpecData, TStatusData* StatusData, TTestDataRevA* TestData, PReportCallBack ReportCallBack, PWriteSMBCallBack WriteSMBCallBack)
{

	return 0;
}

__declspec(dllexport) int __stdcall GetSetpointRevA(int channel, int unused_flags, TSpecData* SpecData, TStatusData* StatusData, TOutData* OutData, PReportCallBack ReportCallBack, PWriteSMBCallBack WriteSMBCallBack)
{
	FILE *F;
	float current = 0;
	#pragma warning(disable : 4996)  
	F = fopen("C:\\Maccor\\DEMO_readings.txt", "a");
	fprintf(F, "TestTime: %f; Voltage: %f; Current: %f\n", StatusData->TestTime, StatusData->Voltage, StatusData->Current);
	fclose(F);

	F = fopen("C:\\Maccor\\DEMO_current_setpoint.txt", "r");
	if (F == 0)
	{
		F = fopen("C:\\Maccor\\DEMO_current_setpoint.txt", "w");
		fprintf(F, "%f", StatusData->Current);
	}
	else
	{
		fscanf(F, "%f", &current);
		OutData->Current = current;
		OutData->Voltage = 5.0;
		OutData->Power = 5.0;
	}



	fclose(F);

	return 0;
}

