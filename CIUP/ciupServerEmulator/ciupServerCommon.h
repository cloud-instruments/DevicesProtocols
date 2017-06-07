#pragma once

#include <vector>

#include "../w32_udp_socket_test/w32_udp_socket.h"
#include "../ciupClientDll/ciupCommon.h"
#include "../ciupClientTest/streamlog.h"
// 

// TODO: move in ciupCommon
typedef struct ciupLog_t {

	std::string descr;
	streamlog::streamlog_level level;

}ciupLog;

int ciupServerStart(unsigned short port);

int ciupServerStop();

int ciupGetLog(ciupLog *log);