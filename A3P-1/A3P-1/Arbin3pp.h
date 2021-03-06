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

// Arbin 3rd party protocol functions

#pragma once

#include <string>

// additional code required
#include "..\w32_tcp_socket_test\win32_tcp_socket.h"
#include "Arbin3ppMessages.h"

/// channels ports
#define A3P_CH1_PORT (5678)
#define A3P_CH2_PORT (5679)

// timeout to send  SET_SYSTEM_TIME 
#define A3P_SST_TIMEOUT_S (4)

// timeout to wait answer on commands
#define A3P_ANS_TIMEOUT_MS (100)

// timeout for tcp keepalive
#define A3P_KEEPALIVE_TIMEOUT_S (10)

// message codes
enum a3p_msg_level{
	A3P_DBG = 0,
	A3P_TRC = 1,
	A3P_WRN = 2,
	A3P_ERR = 3,
};

#define A3P_ML_DESCR(n) (n==A3P_DBG?"DBG":\
                         (n==A3P_TRC?"TRC":\
                          (n==A3P_WRN?"WRN":\
                           (n==A3P_ERR?"ERR":\
                            "UKN"))))

// init a new connection
int a3p_init(const char* addr);

// connect device to addr
// start watchdog, keepalive and receiving threads
int a3p_connect(WORD chNum, WORD chCount, bool sst, bool sdu);

// gracefully discconnect from device
int a3p_disconnect(WORD chNum, WORD chCount);

// free connection instance
int a3p_delete(WORD chNum, WORD chCount);

// get log messages from queue
int a3p_get_message(std::string *str);

// get incoming messages from ch1 queue
int a3p_get_ch1(a3p_msg *msg);

// get incoming messages from ch2 queue
int a3p_get_ch2(a3p_msg *msg);

// send a A3P_CMD_3RD_READDATAORSTATE command
// return 0 on success
// data are returned in controlState, current and voltage
int a3p_readdataorstate(
	WORD chNum,
	WORD chCount,
	bool readdata,
	BYTE controlState[A3P_MAXCHANNELNO],
	float current[A3P_MAXCHANNELNO],
	float voltage[A3P_MAXCHANNELNO]);


