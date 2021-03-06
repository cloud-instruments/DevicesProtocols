/*
Copyright(c) <2018> <University of Washington>
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

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
