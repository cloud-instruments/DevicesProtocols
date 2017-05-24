// ciupCommon.cpp : Defines the functions for DLL application.
#include "stdafx.h"

#include "ciupCommon.h"

void *ciupBuildMessage(BYTE type, void* payload, size_t payload_size)
{
	BYTE *msg = new BYTE[CIUP_MSG_SIZE(payload_size)];

	//  4 bytes: 'C','I','U','P'
	//  1 byte: MESSAGE TYPE (see #define CIUP_MSG_*)
	//  2 bytes: payload size (s) in bytes 
	//  s bytes: payload
	//  1 byte: checksum (sum of previous bytes)
	//  1 byte: checksum negated (0xFF-chk)

	msg[0] = 'C';
	msg[1] = 'I';
	msg[2] = 'U';
	msg[3] = 'P';
	msg[4] = type;
	msg[5] = payload_size / 256;
	msg[6] = payload_size % 256;
	if (payload && payload_size) {
		memcpy_s(msg + 7, payload_size, payload, payload_size);
	}

	// cs = (BYTE)sum af all message 
	BYTE cs = 0x00;
	for (unsigned int i = 0; i < CIUP_MSG_SIZE(payload_size) - 2; i++) cs += msg[i];

	msg[CIUP_MSG_SIZE(payload_size) - 2] = cs;
	msg[CIUP_MSG_SIZE(payload_size) - 1] = 0xFF - cs;

	return msg;
}