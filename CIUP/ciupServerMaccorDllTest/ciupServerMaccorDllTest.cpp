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

// ciupServerMaccorDllTest.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <Windows.h>

// Need to link with ciupClientDll.dll
#pragma comment(lib, "ciupServerMaccorDll.lib")
#include "..\ciupServerMaccorDll\ciupServerMaccorDll.h"

#define CH_DEFAULT 0

bool gRun = true;

// [CTRL][c] handler
BOOL WINAPI HandlerRoutine(_In_ DWORD dwCtrlType) {

	switch (dwCtrlType) {
	case CTRL_C_EVENT:

		std::cout << "[Ctrl]+C" << std::endl;

		gRun = false;
		return TRUE;

	default:
		// Pass signal on to the next handler
		return FALSE;
	}
}

void print_usage(const char *exe) {

	std::cerr << "Test application for ciupServerMaccorDll" << std::endl;
	std::cerr << "usage: " << exe << " SLEEP_MS" << std::endl;
	std::cerr << "  -c N : write on channel N (default:" << CH_DEFAULT << ")" << std::endl;

}

int __stdcall WriteSMBCallBack(TSMBGenericWriteData* data)
{
	return 0;
}

int __stdcall ReportCallBack(const char *Msg, int I1, int I2, BOOL Displayed)
{
	return 0;
}

int main(int argc, char **argv)
{
	int expected_argc = 2;
	int channel = CH_DEFAULT;

	// parse arguments
	for (int i = 1; i < argc; i++) {

		// set channel
		if (!strcmp(argv[i], "-c")) {

			if (i >= argc - 1) {
				print_usage(argv[0]);
				return -1;
			}
			channel = atoi(argv[i + 1]);
			expected_argc += 2;
		}
	}

	// Validate the parameters
	if (argc < expected_argc) {
		print_usage(argv[0]);
		return -1;
	}

	int sleep_ms = atoi(argv[expected_argc-1]);

	// CRTL-C handler
	SetConsoleCtrlHandler(HandlerRoutine, TRUE);

	// TODO OnLoadRevA
	// TODO OnStepStartRevA

	while (gRun) {

		TSpecData SpecData;
		TStatusData StatusData;
		TOutData OutData;

		// TODO: put some real data 

		GetSetpointRevA(
			channel,             // The test channel(zero based) 
			0,                   // Not currently used 
			&SpecData,           // key specification of the test channel.  
			&StatusData,         // present readings and status of the test channel 
			&OutData,            // returned set points and flag
			ReportCallBack,      // function to generate an event in the tester programs event system 
			WriteSMBCallBack);   // function to write the data in the TSMBGenericWriteData to the SMB assigned to the channel

		std::cout << "Data passed to dll" << std::endl;
		Sleep(sleep_ms);
	}

	// TODO  OnStepEndRevA
	// TODO OnUnLoadRevA

    return 0;
}

