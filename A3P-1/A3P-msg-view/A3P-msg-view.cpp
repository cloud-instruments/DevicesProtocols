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

