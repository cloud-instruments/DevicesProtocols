========================================================================
    CONSOLE APPLICATION : A3P-1 Project Overview
========================================================================

Basic connection test for Arbin 

Must be used in console (cmd.exe)

usage: A3P-1.exe [-0|-1|-2] device-ip-addr
-0: (DEFAULT) connect and show incoming messages
-1: 0 + send CMD_SET_SYSTEMTIME every 5s
-2: 1 + change device third party mode

Examples 
(x.x.x.x is the IPv4 address of connected Arbin device):

# execute a simple connection test (connect TCP and stay showing incoming messages)
# the connection is expected to fail after some seconds because the Arbin is not
# receiving the required CMD_SET_SYSTEMTIME every 5s
A3P-1.exe x.x.x.x

# connect and mantain the connection (send CMD_SET_SYSTEMTIME every 5s)
# the test is expected to run without errors for an arbitrary time
A3P-1.exe -1 x.x.x.x

# as previous but change device mode to 3rd party 
# expected to run without errors
# WARN: could interfere with yet runnig tests on the Arbin device
A3P-1.exe -2 x.x.x.x  
