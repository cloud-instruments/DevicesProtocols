// interface file for CIUP client dll exported functions

#pragma once

#include "ciupCommon.h"

// max number of receivers managed
#define CIUP_MAX_RECEIVER (255)

// Callback definitions ////////////////////////////////////////////////////////

// Data callback definition
typedef void (__stdcall *ciupDataCb)(
	const char*,         // incoming message as json string
	int,                 // receiver id (as returned by ciupcStartReceiver)
	const char*,         // sender IP address
	unsigned short       // sender port
);

// Error callback definition
typedef void(__stdcall *ciupErrorCb)(
	int,               // error code
	const char*,       // error description
	int                // receiver id (as returned by ciupcStartReceiver)
);

// exported functions //////////////////////////////////////////////////////////
extern "C" {

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
}
// NOTE for C# use /////////////////////////////////////////////////////////////

/*
[DllImport("ciupClientDll.dll")]
static extern int ciupcGetLastError(StringBuilder descr, int maxlen);

[DllImport("ciupClientDll.dll")]
static extern int ciupcGetServerInfo(String addr, ushort port, StringBuilder json, int jsonlen);

delegate void ciupDataCb(String json, int id, String fromAddr, ushort fromPort);
delegate void ciupErrorCb(int code, String description, int id);
[DllImport("ciupClientDll.dll")]
static extern int ciupcStartReceiver(String addr, ushort port, [MarshalAs(UnmanagedType.FunctionPtr)]ciupDataCb dataCb, [MarshalAs(UnmanagedType.FunctionPtr)]ciupErrorCb errorCb);

[DllImport("ciupClientDll.dll")]
static extern int ciupcStopReceiver(int ID);

[DllImport("ciupClientDll.dll")]
static extern void ciupcStopAllReceivers();
*/
