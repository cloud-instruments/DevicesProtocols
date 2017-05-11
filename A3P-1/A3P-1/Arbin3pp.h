/*!
*  @brief     Arbin 3rd party protocol functions
*  @author    Matteo Lucarelli
*  @date      2017
*/

#pragma once

// additional code required
#include "win32_tcp_socket.h"
#include "Arbin3ppMessages.h"

/// channels ports
#define A3P_CH1_PORT (5678)
#define A3P_CH2_PORT (5679)

#define A3P_KEEPALIVE_TIMEOUT_S (10)

/// @brief connect device to addr
/// @param addr IPv4 address of device
/// start watchdog, keepalive and receiving threads
int a3p_connect(const char* addr);

/// @brief gracefully discconnect from device
int a3p_disconnect();

// public only for test
int a3p_send(a3p_msg msg);
int a3p_get_ch1(a3p_msg *msg);
int a3p_get_ch2(a3p_msg *msg);


