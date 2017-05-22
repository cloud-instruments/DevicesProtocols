// interface file for CIUP client dll exported functions

#pragma once

#include "ciupCommon.h"

__declspec(dllexport) ciupServerInfo __stdcall ciupcGetServerInfo(
	const char *addr,			// server IP address
	unsigned short port			// server UDP port
);

