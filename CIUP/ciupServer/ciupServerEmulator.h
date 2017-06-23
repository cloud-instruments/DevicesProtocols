// Server Emulator start threads to feed ciup server with random data
// Emulator mode can be used for test or debugging porpouse

#pragma once

#include "ciupServerCommon.h"

// start feeding ciupServer message buffer
int serverEmulatorStart(
	int chCount,    // channel count to generate
	int sleep);     // sleep betwenn points

// stop the feeding
int serverEmulatorStop();