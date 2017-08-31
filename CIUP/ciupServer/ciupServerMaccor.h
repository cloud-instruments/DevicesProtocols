// ciupServerMaccor: start thread to connect ciupServerMaccorDll via named pipe
// will be used in paralle to ciupSErver MaccorDll

#pragma once

#include "ciupServerCommon.h"
#include "..\w32_message_pipe_test\w32_message_pipe.h"

// start the connection with the dll
int serverMaccorStart();

// stop the connection with the dll
int serverMaccorStop();

// get and delete older log message from FIFO
// return -1 if FIFO is empty
int maccorGetLog(ciupLog *log);

// set loglevel to avoid queueing filtered logs
void maccorSetLogFilter(streamlog::level filter);