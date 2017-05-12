========================================================================
    CONSOLE APPLICATION : w32_tcp_socket_test Project Overview
========================================================================

Generic socket test for console (cmd.exe)

Can create client or server connection both listening incoming messages or sending strings

usage: w32_tcp_socket_test.exe [-c server-ip] [-k] [-s] port
-c server-ip : connect as client to server-ip:port
-k : enable keepalive
-s : run in send mode
-b : HEX output mode (for listen mode)
default : open server on port in listen mode

To exit type [CTRL][c] or inser a single 'q' in send mode
Client socket in listen mode will exit if the server closes the connection

Examples:

# open a TCP server on port 10000 and 
# show incoming messages as string
w32_tcp_socket_test.exe 10000

# connect as client to 127.0.0.1 at port 10000
# show incoming messages as hex
w32_tcp_socket_test.exe -c 127.0.0.1 -b 10000

# connect as client to 127.0.0.1 at port 10000
# and send typed string
w32_tcp_socket_test.exe -c 127.0.0.1 -r 10000


PERFORM A SIMPLE CONNECTION TEST
# run two instances in two consoles (or on two different hosts)
# server side
w32_tcp_socket_test.exe 10000
# client side (x.x.x.x must be IPv4 of server, 127.0.0.1 for localhost)
w32_tcp_socket_test.exe -c x.x.x.x -s 5679
# expected behaviour:
# server side show string typed on client side
# if the client exits the server return in accepting connection
# if the server exitst the client exit with "send error" on the first send try


TEST FOR ARBIN DEVICE:
# run two instances in two consoles 
# where x.x.x.x is the IPv4 addr of connected Arbin device
# expected to fail after some seconds because device not receiving 
# the required CMD_SET_SYSTEMTIME every 5s will close connection
w32_tcp_socket_test.exe -c x.x.x.x -k -b 5678
w32_tcp_socket_test.exe -c x.x.x.x -k -b 5679




