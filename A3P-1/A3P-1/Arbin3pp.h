/*!
*  @brief     Arbin 3rd party protocol functions
*  @author    Matteo Lucarelli
*  @date      2017
*/

#pragma once

#include <string>

// additional code required
#include "win32_tcp_socket.h"
#include "Arbin3ppMessages.h"

/// channels ports
#define A3P_CH1_PORT (5678)
#define A3P_CH2_PORT (5679)

// timeout to send  SET_SYSTEM_TIME 
#define A3P_SST_TIMEOUT_S (4)

// timeout to wait answer on commands
#define A3P_ANS_TIMEOUT_S (0.01)

// TODO: CHNUM CHCOUNT should be parameters
#define A3P_DEFAULT_CH_NUM    (0)
#define A3P_DEFAULT_CH_COUNT  (8)

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

// TODO: init a new connection instance
int a3p_init(const char* addr);

/// @brief connect device to addr
/// @param addr IPv4 address of device
/// start watchdog, keepalive and receiving threads
int a3p_connect(bool sst, bool sdu);

/// @brief gracefully discconnect from device
int a3p_disconnect();

// TODO: free new connection instance
int a3p_delete();

int a3p_get_message(std::string *str);
int a3p_get_ch1(a3p_msg *msg);
int a3p_get_ch2(a3p_msg *msg);


