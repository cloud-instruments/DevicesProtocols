CIUP : Cloud Instruments Unified protocol

ciupClientDll : c++ dll with common functions for client side
ciupClientTest : test application to simulate client side (c++)
ciupClientTest-csc : test application to simulate client side (c#)
ciupServerTest : test application to simulate server side (c++)
w32_udp_socket_test : test application for UDP socket (both sides)

Ciup client/server test:

Open two consoles (cmd.exe) in Release folder
1st console: ciupServerTest 10000
2nd console: ciupClientTest 127.0.0.1 10000

UDP socket test:

Open two consoles (cmd.exe) in Release folder
1st console: w32_udp_socket_test 10000
2nd console: w32_udp_socket_test -s 127.0.0.1 10000

