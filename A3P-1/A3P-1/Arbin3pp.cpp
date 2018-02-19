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

#include "stdafx.h"
#include "Arbin3pp.h"

#include <ctime>
#include <queue>
#include <iostream>

// GLOBAL VARS /////////////////////////////////////////////////////////////////

// TODO: struct for multiple devices
char gAddr[INET_ADDRSTRLEN];
std::queue<a3p_msg*> gCh1InQueue;
std::queue<a3p_msg*> gCh2InQueue; 
bool gDoThreadCh1 = false;
HANDLE gThreadCh1=0;
w32_socket *gCh1Sock;
bool gDoThreadCh2 = false;
HANDLE gThreadCh2=0;
w32_socket *gCh2Sock;
HANDLE  gCh1Mutex = CreateMutex(NULL, FALSE, NULL);
std::queue<std::string> gMessageQueue;

// LOGS ///////////////////////////////////////////////////////////////////////

void a3p_queue_message(a3p_msg_level level, const std::string &message)
{
	// TODO: better time format (japanese)
	std::string msg = std::to_string(time(NULL));
	msg += " ";
	msg = A3P_ML_DESCR(level);
	msg += " ";
	msg += message;

	gMessageQueue.push(msg);
}

int a3p_get_message(std::string *str)
{
	if (gMessageQueue.empty()) return -1;

	*str = gMessageQueue.front();
	gMessageQueue.pop();
	return 0;
}

// UTILS ///////////////////////////////////////////////////////////////////////

int a3p_write(w32_socket *s, a3p_msg &msg, std::string label)
{
	int ret = w32_tcp_socket_write(s, (char*)(msg.buff), msg.size);

	if (ret<0) {

		if (ret == -1) {
			label += " peer disconnected";
			a3p_queue_message(A3P_ERR, label);
			return -1;
		}
		else {
			label += " w32_tcp_socket_write error";
			a3p_queue_message(A3P_ERR, label);
			return -2;
		}
	}
	else if (ret != msg.size) {

		label += " uncomplete write";
		a3p_queue_message(A3P_ERR, label);
		return -3;

	}
	return 0;
}

int a3p_read(w32_socket *s, a3p_msg *msg, std::string label)
{
	int ret = w32_tcp_socket_read(s, (char*)msg->buff, msg->size, A3P_ANS_TIMEOUT_MS);

	if (ret == 0) {

		label += " w32_tcp_socket_read timeout";
		a3p_queue_message(A3P_ERR, label);
		return -1;

	}
	else if (ret<0) {

		label += " w32_tcp_socket_read error: " + s->lasterr;
		return -2;

	}
	else if (ret != msg->size) {

		label += " w32_tcp_socket_read uncomplete";
		return -3;
	}

	return 0;
}

// CH1 /////////////////////////////////////////////////////////////////////////

int a3p_get_ch1(a3p_msg *msg) {

	if (gCh1InQueue.empty()) return -1;

	a3p_msg *front = gCh1InQueue.front();
	*msg = *front;
	gCh1InQueue.pop();
	//delete front; TODO
	return 0;
}

// send A3P_CMD_SET_SYSTEMTIME every A3P_SST_TIMEOUT_S (watchdog)
DWORD WINAPI a3p_ch1_thread_funct(LPVOID lpParam) {

	a3p_queue_message(A3P_DBG, "CH1 thread started");
	time_t prevTime=0;

	while (gDoThreadCh1) {

		// each 5s send CMD_SET_SYSTEMTIME
		if (time(NULL)-prevTime >= A3P_SST_TIMEOUT_S){

			// send CMD_SET_SYSTEMTIME
			a3p_msg sst;
			a3p_CMD_SET_SYSTEMTIME(&sst, (float)A3P_UX_TO_ARBIN_TIME(time(NULL)));

			WaitForSingleObject(gCh1Mutex, INFINITE);

			int ret = a3p_write(gCh1Sock, sst, "CH1");
			if (ret == 0) {

				char buff[1];
				if (w32_tcp_socket_read(gCh1Sock, buff, 1, A3P_ANS_TIMEOUT_MS) == 1) {
					if (*buff != 0x06) {
						a3p_queue_message(A3P_ERR, "CH1 unexpected answer for CMD_SET_SYSTEMTIME");
					}
				} 
				else {
					a3p_queue_message(A3P_ERR, "CH1 no answer for CMD_SET_SYSTEMTIME");
					//break;
				}
				prevTime = time(NULL);
			}
			/*else if (ret == -1) {

				break;
			}*/
			else{
				a3p_queue_message(A3P_ERR, "CH1 cannot write CMD_SET_SYSTEMTIME");
				break;
			}

			ReleaseMutex(gCh1Mutex);
		}
		Sleep(500);
	}
	a3p_queue_message(A3P_DBG, "CH1 thread ended");
	return 0;
}

int a3p_start_ch1_thread() {

	if (gDoThreadCh1) return 0;
	gDoThreadCh1 = true;
	gThreadCh1 = CreateThread(0, 0, a3p_ch1_thread_funct, NULL, 0, NULL);
	return 1;
}

int a3p_stop_ch1_thread() {

	if (!gDoThreadCh1) return 0;
	gDoThreadCh1 = false;
	if (WaitForSingleObject(gThreadCh1, 2000) == WAIT_TIMEOUT) {
		a3p_queue_message(A3P_WRN, "CH1 thread force terminated");
		TerminateThread(gThreadCh1, 0);
	}
	CloseHandle(gThreadCh1);
	return 1;
}

// CH2 ////////////////////////////////////////////////////////////////////////

int a3p_get_ch2(a3p_msg *msg) {

	if (gCh2InQueue.empty()) return -1;

	a3p_msg *front = gCh2InQueue.front();
	*msg = *front;
	gCh2InQueue.pop();
	//delete front; // TODO

	return 0;
}

// continuously reasd ch2 and enqueue incoming messages
DWORD WINAPI a3p_ch2_thread_funct(LPVOID lpParam) {

	a3p_queue_message(A3P_DBG, "CH2 thread started");

	a3p_msg *inmsg;
	int ret;

	while (gDoThreadCh2) {

		// receive data from ch2 in incoming queue
		inmsg = new a3p_msg(A3P_MAX_MSG_SIZE);
		ret = w32_tcp_socket_read(gCh2Sock, (char*)inmsg->buff, A3P_MAX_MSG_SIZE);

		if (ret == 0) {

			a3p_queue_message(A3P_ERR, "CH2 peer disconnected");
			delete inmsg;
			break;

		}
		else if (ret == -1) {
			a3p_queue_message(A3P_ERR, "CH2 peer disconnected");
			delete inmsg;
			break;
		}
		else if (ret<0) {

			a3p_queue_message(A3P_ERR, "CH2 w32_tcp_socket_read error: " + gCh2Sock->lasterr);
			delete inmsg;
			Sleep(100);

		}
		else {

			inmsg->size = ret;
			gCh2InQueue.push(inmsg);
		}
	}
	a3p_queue_message(A3P_DBG, "CH2 thread ended");
	return 0;
}

int a3p_start_ch2_thread() {

	if (gDoThreadCh2) return 0;
	gDoThreadCh2 = true;
	gThreadCh2 = CreateThread(0, 0, a3p_ch2_thread_funct, NULL, 0, NULL);
	return 1;
}

int a3p_stop_ch2_thread() {

	if (!gDoThreadCh2) return 0;
	gDoThreadCh2 = false;

	if (WaitForSingleObject(gThreadCh2, 2000) == WAIT_TIMEOUT) {

		a3p_queue_message(A3P_WRN, "CH2 thread force terminated");
		TerminateThread(gThreadCh2, 0);
	}
	CloseHandle(gThreadCh2);
	return 1;
}

// PUBLICS ////////////////////////////////////////////////////////////////////

int a3p_3rd_mode(WORD chNum, WORD chCount, bool enable) {

	a3p_queue_message(A3P_TRC, "a3p_3rd_mode");

	if (!gCh1Sock) return -1;

	bool success = false;

	// send CMD_SET_SYSTEMTIME
	a3p_msg sdu;
	a3p_CMD_3RD_SDU(&sdu, chNum, chCount, enable);

	WaitForSingleObject(gCh1Mutex, INFINITE);

	if (a3p_write(gCh1Sock, sdu, "CH1") == 0) {

		// get answer
		a3p_msg ans(A3P_HEADER_SIZE + sizeof(MP_CONFIRM_FEEDBACK));

		if (a3p_read(gCh1Sock,&ans,"CH1")==0) {
			
			a3p_parse_CONFIRM_FEEDBACK(&ans, &success, A3P_CMD_3RD_SDU_FEEDBACK);
		}
	}
	ReleaseMutex(gCh1Mutex);

	return (success?0:-1);
}

int a3p_readdataorstate(
	WORD chNum, 
	WORD chCount, 
	bool readdata,
	BYTE controlState[A3P_MAXCHANNELNO],
	float current[A3P_MAXCHANNELNO],
	float voltage[A3P_MAXCHANNELNO]) 
{
	a3p_queue_message(A3P_TRC, "a3p_readdataorstate");

	if (!gCh1Sock) return -1;
	int ret = -1;

	// send CMD_SET_SYSTEMTIME
	a3p_msg msg;
	a3p_CMD_3RD_READDATAORSTATE(&msg, chNum, chCount, readdata, !readdata);

	WaitForSingleObject(gCh1Mutex, INFINITE);

	if (a3p_write(gCh1Sock, msg, "CH1") == 0) {

		// get answer
		a3p_msg ans(A3P_HEADER_SIZE + sizeof(MP_INDEPENDENT_READDATAORSTATE));

		if (a3p_read(gCh1Sock, &ans, "CH1") == 0) {

			WORD ChNum, ChCount;
			bool rd, rs;

			a3p_parse_READDATAORSTATE_FEEDBACK(
				&msg,
				&ChNum,
				&ChCount,
				&rd,
				&rs,
				controlState,
				current,
				voltage
			);
			ret = 0;

			// TODO: control ChNum, ChCount, rd, rs
		}
	}
	ReleaseMutex(gCh1Mutex);

	return ret;
}

int a3p_init(const char* addr) {

	a3p_queue_message(A3P_TRC, "a3p_init");

	strncpy_s(gAddr, INET_ADDRSTRLEN, addr, INET_ADDRSTRLEN);

	gDoThreadCh1 = false;
	gDoThreadCh2 = false;
	gCh1Sock = NULL;
	gCh2Sock = NULL;

	// TODO: read w32_tcp_socket errors
	// w32_tcp_socket_quiet(true);

	return 0;
}

int a3p_connect(WORD chNum, WORD chCount, bool sst, bool sdu) {
	
	a3p_queue_message(A3P_TRC, "a3p_connect");

	if (gCh1Sock != NULL) {
		a3p_queue_message(A3P_WRN, "a3p_connect called with socket on");
		return 0;
	}

	// connect CH1
	gCh1Sock = w32_tcp_socket_client_create(gAddr, A3P_CH1_PORT);
	if (!gCh1Sock->lasterr.empty()) {

		// get w32_tcp_sock error
		a3p_queue_message(A3P_ERR, "CH1 Failed opening socket " + gCh1Sock->lasterr);
		w32_tcp_socket_close(&gCh1Sock);
		return -1;
	}

	// enable keepalive
	w32_tcp_socket_keepalive(gCh1Sock, A3P_KEEPALIVE_TIMEOUT_S, 1);

	// connect CH2
	gCh2Sock = w32_tcp_socket_client_create(gAddr, A3P_CH2_PORT);
	if (!gCh2Sock->lasterr.empty()) {

		// get w32_tcp_sock error
		a3p_queue_message(A3P_ERR, "CH2 failed opening socket " + gCh2Sock->lasterr);
		w32_tcp_socket_close(&gCh1Sock);
		return -1;
	}

	// enable keepalive 
	w32_tcp_socket_keepalive(gCh2Sock, A3P_KEEPALIVE_TIMEOUT_S, 1);

	if (sdu) {
		if (a3p_3rd_mode(chNum, chCount,true)!=0) a3p_queue_message(A3P_ERR, "Cannot enable 3rd party mode");
	}

	// start threads
	if (sst) a3p_start_ch1_thread();
	a3p_start_ch2_thread();

	return 0;
}

int a3p_disconnect(WORD chNum, WORD chCount) {

	a3p_queue_message(A3P_TRC, "a3p_disconnect");

	// stop threads
	a3p_stop_ch2_thread();
	a3p_stop_ch1_thread();

	// disable 3rd party mode
	if (gCh1Sock) {
		if (a3p_3rd_mode(chNum, chCount,false) != 0)  a3p_queue_message(A3P_ERR, "Cannot disable 3rd party mode");
	}

	// close sockets
	w32_tcp_socket_close(&gCh1Sock);
	w32_tcp_socket_close(&gCh2Sock);

	return 0;
}

int a3p_delete(WORD chNum, WORD chCount) {

	a3p_queue_message(A3P_TRC, "a3p_delete");

	a3p_disconnect(chNum, chCount);

	// TODO: empty queues

	return 0;
}