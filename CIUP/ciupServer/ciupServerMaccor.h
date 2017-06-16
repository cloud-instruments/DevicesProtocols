// ciupServerMaccor: start thread to connect ciupServerMaccorDll via pipe
#pragma once

#include "ciupServerCommon.h"
#include "..\w32_message_pipe_test\w32_message_pipe.h"

int serverMaccorStart();
int serverMaccorStop();

int maccorGetLog(ciupLog *log);