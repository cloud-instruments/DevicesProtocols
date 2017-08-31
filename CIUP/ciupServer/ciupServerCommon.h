// intercafe file for ciup server common functions
#pragma once

#include <vector>

#include "../w32_tcp_socket_test/win32_tcp_socket.h"
#include "../ciupClientDll/ciupCommon.h"
#include "../ciupClientTest/streamlog.h"

#define CIUP_LOG_MAX_STORE (100)

#define CIUP_POINT_MAX_STORE (100000)

// data to be sent to cliets are stored in circular buffer
// use this function to enque new datapoint to be sent to evry connected client
void ciupServerEnqueueDatapoint(ciupDataPoint &p);

// log message struct
typedef struct ciupLog_t {

	std::string descr;
	streamlog::level level;

}ciupLog;

// set server info data to be sent to client on request
// this data are set to client in answer to SERVERINFO command
void setServerInfo(
	ciupStatus status,       // see ciupCommon.h
	const char* idstr,       // free id string
	ciupServerRunMode mode); // see ciupCommon.h

// set status only in info
void setServerStatus(ciupStatus status);

// set logging level
void ciupServerSetLogFilter(streamlog::level filter);

// get log from FIFO
// returned log will be delated for FIFO
// return -1 when FIFO is empty
int ciupServerGetLog(ciupLog *log);

// start the server on TCP port
// the server listen on port for client to connect
int ciupServerStart(unsigned short port);

// stop the server
int ciupServerStop();

// return the pointbuffer index
// pointbuffer is the circular buffer of points to be sent to clients
// pointbuffer is filled by ciupServerEnqueueDatapoint
int ciupServerDatapointIndex();

// return the number of active connection
size_t ciupServerConnectionCount();

// for one connection return its current index of point to send from the pointbuffer
int ciupServerQueueIndex(size_t connection);