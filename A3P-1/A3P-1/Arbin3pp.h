// Arbin 3rd party protocol helpers functions
// (c)2017 Matteo Lucarelli

#pragma once

#include "win32_tcp_socket.h"
#include <vector>
#include <ctime>

// channels ports
#define A3P_CH1_PORT 5678
#define A3P_CH2_PORT 5679

typedef struct t_a3p_msg {
	char buff[2096];
	unsigned short size;
} a3p_msg;

// connect device to addr
// start watchdog, keepalive and receiving threads
int a3p_connect(const char* addr);

// gracefully discconnect from device
int a3p_disconnect();

int a3p_send(a3p_msg msg);
int a3p_get_ch1(a3p_msg *msg);
int a3p_get_ch2(a3p_msg *msg);


