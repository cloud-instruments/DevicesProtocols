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

// A3P-msg-view.cpp : simple app to show messages content
//
#include "stdafx.h"
#include "../A3P-1/Arbin3ppMessages.h"

#include <cstdio>
#include <ctime>

void show_buff(a3p_msg msg)
{
	for (unsigned int i = 0; i < msg.size; i++) {

		printf("%02X ", msg.buff[i]);
		if ((i + 1) % 8 == 0) printf("\n");
	}
	printf("\n");
}

int main()
{
	// TODO: cycle test for memory leak

	a3p_msg msg;
	a3p_CMD_SET_SYSTEMTIME(&msg, (float)A3P_UX_TO_ARBIN_TIME(time(NULL)));
	show_buff(msg);

    return 0;
}

