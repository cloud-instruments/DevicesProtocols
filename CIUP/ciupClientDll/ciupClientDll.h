// interface file for CIUP client dll exported functions
// must be included in C/C++ application to use ciupClientDll.dll

#pragma once

#include "ciupCommon.h"

// max number of receivers managed
#define CIUP_MAX_CONNECTIONS (255)

// Callback definitions ////////////////////////////////////////////////////////

// Data callback definition
// this callback will be called by ciupClientDll for any incoming data
// NOTE: cannot be a time consuming function
typedef void (__stdcall *ciupDataCb)(
	int messageType,          // message type (CIUP_MSG_* defines)
	const char* data,         // incoming message as json string
	int connectionId          // id of connection instance
);

// Error callback definition
// this callback will be called by ciupClientDll to notify errors
// error codes are defined in ciupCommon.h
typedef void(__stdcall *ciupErrorCb)(
	int errorCode,            // error code
	const char* errorDescr,   // error description
	int connectionId          // id of connection instance
);

// exported functions //////////////////////////////////////////////////////////
extern "C" {

	// connect to a server and start receving data 
	// return >0 : ID of the connection instance
	// return <0 : error
	// incoming data will be passed to dataCb
	__declspec(dllexport) int __stdcall ciupcConnect(
		const char *addr,		 // server IP address
		unsigned short port,	 // server TCP port
		ciupDataCb dataCb,       // callback for incoming data
		ciupErrorCb errorCb      // callback for errors
	);

	// ask a serverinfo message
	// incoming data will be passed to dataCb
	__declspec(dllexport) void __stdcall ciupcInfo(
		int ID                // id of connection instance
	);

	// use this to resume the transmission after a stop
	__declspec(dllexport) void __stdcall ciupcStart(
		int ID                // id of connection instance
	);

	// use this to pause the transmission when required
	__declspec(dllexport) void __stdcall ciupcStop(
		int ID                // id of connection instance
	);

	// cose the connection
	__declspec(dllexport) void __stdcall ciupcDisconnect(
		int ID                // id of connection instance
	);
}

