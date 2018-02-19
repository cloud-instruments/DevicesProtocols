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

// Test application for Arbin 3rd party protocol

#include "stdafx.h"
#include "Arbin3pp.h"

#include <Windows.h>

#include <string>
#include <iostream>
#include <iomanip>

#include "streamlog.h"

bool gRepeat = true;

// log globals
std::string logpath;
std::ofstream *logStream = NULL;
streamlog *plog = NULL;

#define CHNUM   (0)
#define CHCOUNT (1)

void print_usage(const char *exe)
{
	std::cerr << "Test application for Arbin 3rd party protocol" << std::endl;
	std::cerr << "usage: " << exe << " [-0|-1|-2-3] device-ip-addr" << std::endl;
	std::cerr << "  -0: (DEFAULT) connect and show incoming messages" << std::endl;
	std::cerr << "  -1: as 0 + send CMD_SET_SYSTEMTIME every 5s" << std::endl;
	std::cerr << "  -2: as 1 + change device third party mode" << std::endl;
	std::cerr << "  -3: as 2 + ask for data/state each second" << std::endl;
	std::cerr << "  -l PATH : enable logfile" << std::endl;
	std::cerr << "  -h: show this help" << std::endl;
}

// ctrl-c handler
BOOL WINAPI HandlerRoutine(_In_ DWORD dwCtrlType) {

	switch (dwCtrlType){
		case CTRL_C_EVENT:

			if (plog) *plog << "CTRL-c" << std::endl;
			std::cout << "[Ctrl]+C\n" << std::endl;

			gRepeat = false;
			return TRUE;
		default:
			return FALSE;
	}
}

// show msg buffer as hex
void show_buff(a3p_msg msg){

	for (unsigned int i = 0; i <  msg.size; i++) {

		if (plog) *plog << std::setw(2) << std::setfill('0') << std::hex << (int)msg.buff[i] << " ";
		std::cout << std::setw(2) << std::setfill('0') << std::hex << (int)msg.buff[i] << " ";

		if ((i + 1) % 8 == 0) std::cout << std::endl;
	}

	std::cout << std::endl;
	if (plog) *plog << std::endl;
}

int main(int argc, char **argv)
{
	// intercept ctrl-c to gracefully close
	SetConsoleCtrlHandler(HandlerRoutine, TRUE);

	bool sst = false;
	bool sdu = false;
	int expected_argc = 2;

	for (int i = 1; i < argc; i++){

		if (!strcmp(argv[i], "-h")) {
			print_usage(argv[0]);
			return 0;
		}

		// enable log file
		if (!strcmp(argv[i], "-l")) {

			if (i >= argc - 1) {
				print_usage(argv[0]);
				return -1;
			}
			logpath = argv[i + 1];
			expected_argc += 2;
		}

		if (!strcmp(argv[i], "-0")) {

			expected_argc++;
		}

		if (!strcmp(argv[i], "-1")) {

			sst = true;
			expected_argc++;
		}

		if (!strcmp(argv[i], "-2")) {

			sst = true;
			sdu = true;
			expected_argc++;
		}
	}

	// Validate parameters
	if ( argc < expected_argc ) {
		print_usage(argv[0]);
		return 1;
	}

	// init log
	if (!logpath.empty()) {
		// open log file
		logStream = new std::ofstream(logpath, std::ios::app);
		plog = new streamlog(*logStream, streamlog::debug);
	}

	if (sst && sdu) {

		if (plog) *plog << STREAMLOG_FINFO << "-2"  << std::endl;
	}
	else if (sst) {

		if (plog) *plog << STREAMLOG_FINFO << "-1" << std::endl;
	}
	else {

		if (plog) *plog << STREAMLOG_FINFO << "-0" << std::endl;
	}

	if (plog) *plog << "connecting " << argv[argc - 1] << std::endl;
	std::cout << "connecting " << argv[argc-1] << std::endl;

	a3p_init(argv[argc - 1]);

	if (a3p_connect(CHNUM,CHCOUNT,sst,sdu) != 0) {

		if (plog) *plog << "Connection refused" << streamlog::error << std::endl;
		std::cerr << "Connection refused" << std::endl;

		std::string msg;
		while (a3p_get_message(&msg) == 0) {

			if (plog) *plog << "a3p:" << msg << streamlog::warning << std::endl;
			std::cerr << "a3p:" << msg << std::endl;
			Sleep(100);
		}

		return -1;
	}

	int count = 0;
	bool dataorstate = true;

	// connection test
	std::cout << "[CTRL][C] to gracefully disconnect" << std::endl;
	while (gRepeat) {

		std::string msg;
		while (a3p_get_message(&msg) == 0) {

			if (plog) *plog << "a3p:" << msg << streamlog::warning << std::endl;
			std::cout << "a3p:" << msg << std::endl;
			Sleep(100);
		}

		a3p_msg mch1;
		while (a3p_get_ch1(&mch1) == 0) {

			if (plog) *plog << "ch1 received " << std::dec << mch1.size << " bytes" << std::endl;
			std::cout << "ch1 received " << std::dec << mch1.size << " bytes" << std::endl;
			show_buff(mch1);
			Sleep(100);
		}

		a3p_msg mch2;
		while (a3p_get_ch2(&mch2) == 0) {
			if (plog) *plog << "ch2 received " << std::dec << mch1.size << " bytes" << std::endl;
			std::cout << "ch2 received " << std::dec << mch2.size << " bytes" << std::endl;
			show_buff(mch2);
			Sleep(100);
		}

		Sleep(100);

		count++;
		if (count == 10) {

			if (plog) *plog << "Sending readdataorstate "<< (dataorstate?"data":"state") << streamlog::trace << std::endl;
			std::cout << "Sending readdataorstate " << (dataorstate ? "data" : "state") << std::endl;

			BYTE controlState[A3P_MAXCHANNELNO];
			float current[A3P_MAXCHANNELNO];
			float voltage[A3P_MAXCHANNELNO];

			int ret = a3p_readdataorstate(CHNUM, CHCOUNT, dataorstate, controlState, current, voltage);
			if (ret == 0) {

				int i;

				if (plog) *plog << "controlState: ";
				std::cout << "controlState: ";
				for (i = 0; i < A3P_MAXCHANNELNO; i++) {
					if (plog) *plog << (int)controlState[i] << " ";
					std::cout << (int)controlState[i] << " ";
				}
				if (plog) *plog << streamlog::debug << std::endl;
				std::cout << std::endl;

				if (plog) *plog << "current: ";
				std::cout << "current: ";
				for (i = 0; i < A3P_MAXCHANNELNO; i++) {
					if (plog) *plog << current[i] << " ";
					std::cout << current[i] << " ";
				}
				if (plog) *plog << streamlog::debug << std::endl;
				std::cout << std::endl;

				if (plog) *plog << "voltage: ";
				std::cout << "voltage: ";
				for (i = 0; i < A3P_MAXCHANNELNO; i++) {
					if (plog) *plog << voltage[i] << " ";
					std::cout << voltage[i] << " ";
				}
				if (plog) *plog << streamlog::debug << std::endl;
				std::cout << std::endl;
			}
			else {
				if (plog) *plog << "readdataorstate return "<< ret << streamlog::error << std::endl;
				std::cerr << "readdataorstate return " << ret << std::endl;
			}


			count = 0;
			dataorstate = !dataorstate;
		}


	}

	if (plog) *plog << STREAMLOG_FINFO << "disconnecting" << std::endl;
	std::cout << "disconnecting" << std::endl;

	a3p_disconnect(CHNUM, CHCOUNT);
	a3p_delete(CHNUM, CHCOUNT);

	if (plog) delete plog;
	if (logStream) delete logStream;

    return 0;
}

