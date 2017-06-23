// ciupCommon.cpp : ciup common functions for both client and server

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
	msg[5] = (BYTE)(payload_size / 256);
	msg[6] = (BYTE)(payload_size % 256);
	if (payload && payload_size) {
		memcpy_s(msg + CIUP_PAYLOAD_POS, payload_size, payload, payload_size);
	}

	// cs = (BYTE)sum af all message 
	BYTE cs = 0x00;
	for (unsigned int i = 0; i < CIUP_MSG_SIZE(payload_size) - 2; i++) cs += msg[i];

	msg[CIUP_MSG_SIZE(payload_size) - 2] = cs;
	msg[CIUP_MSG_SIZE(payload_size) - 1] = 0xFF - cs;

	return msg;
}

int ciupCheckMessageSyntax(void *msg, size_t msg_size)
{
	BYTE *m = (BYTE*)msg;

	if (msg_size < CIUP_MSG_HSIZE) return CIUP_ERR_SIZE;

	size_t payload_size = m[5] * 256 + m[6];
	if (msg_size != CIUP_MSG_SIZE(payload_size)) return CIUP_ERR_SIZE;

	if (m[0] != 'C') return CIUP_ERR_SYNTAX;
	if (m[1] != 'I') return CIUP_ERR_SYNTAX;
	if (m[2] != 'U') return CIUP_ERR_SYNTAX;
	if (m[3] != 'P') return CIUP_ERR_SYNTAX;

	BYTE cs = 0x00;
	for (unsigned int i = 0; i < msg_size - 2; i++) cs += m[i];
	
	if (cs != m[msg_size - 2]) return CIUP_ERR_CHECKSUM;
	if ((0xFF-cs) != m[msg_size - 1]) return CIUP_ERR_CHECKSUM;

	return CIUP_NO_ERR;
}
