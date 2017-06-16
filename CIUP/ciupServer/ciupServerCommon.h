#pragma once

#include <vector>

#include "../w32_tcp_socket_test/win32_tcp_socket.h"
#include "../ciupClientDll/ciupCommon.h"
#include "../ciupClientTest/streamlog.h"

#define CIUP_LOG_MAX_STORE (100)

#define CIUP_POINT_MAX_STORE (100000)

// enque new datapoint to be sent
void ciupServerEnqueueDatapoint(ciupDataPoint &p);

// log message
typedef struct ciupLog_t {

	std::string descr;
	streamlog::level level;

}ciupLog;

// get log from FIFO 
// return -1 when FIFO is empty
int ciupServerGetLog(ciupLog *log);

// start the server on UDP port
int ciupServerStart(unsigned short port);
int ciupServerStop();

// return the pointbuffer index
int ciupServerDatapointIndex();

size_t ciupServerConnectionCount();
int ciupServerQueueIndex(size_t connection);