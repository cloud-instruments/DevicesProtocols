// interface file for CIUP client dll exported functions

#pragma once

#include "ciupCommon.h"

// max number of receivers managed
#define CIUP_MAX_RECEIVER (255)

// ms timeout to stop threads
#define CIUP_STOP_THREAD_TIMEOUT_MS 500

// Callback definitions ////////////////////////////////////////////////////////

// Data callback definition
typedef void (__stdcall *ciupDataCb)(
	const char*,         // incoming message as json string
	int                // receiver id (as returned by ciupcStartReceiver)
);

// Error callback definition
typedef void(__stdcall *ciupErrorCb)(
	int,               // error code
	const char*,       // error description
	int              // receiver id (as returned by ciupcStartReceiver)
);

// exported functions //////////////////////////////////////////////////////////

// return last error code
__declspec(dllexport) int __stdcall ciupcGetLastError(
	char *descr,   // if !NULL return laste error description
	int maxlen     // max len in char for descr
);

// return 0 on success
// return json answer in char *json
__declspec(dllexport) int __stdcall ciupcGetServerInfo(
	const char *addr,			// server IP address
	unsigned short port,		// server UDP port
	char* json,					// returned json message
	int jsonlen                 // max size in char of returned json
);

// return >0 : ID of the started receiver
// return <0 : error
// WARN: cb should not be time consuming functions
__declspec(dllexport) int __stdcall ciupcStartReceiver(
	const char *addr,		 // server IP address
	unsigned short port,	 // server UDP port
	ciupDataCb dataCb,       // callback for incoming datapoints
	ciupErrorCb errorCb      // callback for errors
);

// return 0 on success
__declspec(dllexport) int __stdcall ciupcStopReceiver(
	int ID                // id of the receiver to be stopped (returned by ciupcStartReceiver)
);

// stop all
__declspec(dllexport) void __stdcall ciupcStopAllReceivers();

// NOTE for C# use /////////////////////////////////////////////////////////////

// TO include dll function returning string
// [DllImport("ciupClientDll.dll", CallingConvention = CallingConvention.Cdecl)
// static extern int foo(StringBuilder str, int len);

// To use dll function returning string
// StringBuilder sb = new StringBuilder(10);
// foo(sb, sb.Capacity);

// to manage data callback
// delegate void MyCallback(string datapoint_json);
// [DllImport("ciupClient.dll", CallingConvention = CallingConvention.Cdecl)]
// static extern int startRecv([MarshalAs(UnmanagedType.FunctionPtr)]MyCallback func);
// int ret = startRecv((x, y));
