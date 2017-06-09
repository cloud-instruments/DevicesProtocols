CIUP : Cloud Instruments Unified protocol

ciupClientDll : c++ dll with common functions for client side
ciupClientTest : test application to simulate client side (c++)
ciupClientTest-csc : test application to simulate client side (c#)
ciupServerEmulator : emulator for CIUP server side
ciupServer : server side application
w32_tcp_socket_test : test application for UDP socket (both sides)

Ciup client/server test:

Open two consoles (cmd.exe) in Release folder
On 1st console run: ciupServerEmulator 10000
On 2nd console run: ciupClientTest 127.0.0.1 10000

TCP socket test:

Open two consoles (cmd.exe) in Release folder
On 1st console run: w32_tcp_socket_test 10000
On 2nd console run: w32_tcp_socket_test -s -c 127.0.0.1 10000

