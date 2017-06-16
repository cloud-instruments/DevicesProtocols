CIUP : Cloud Instruments Unified protocol

GENERICS =====================================================================================

CIUP is a TCP server client protocol.
Supports any combination of clients and servers (one-to-one, one-to-many, many-to-many)
Server side is the one generating data points (connected to HW or emulating)
All required client functions are exported by ciupClientDll (see ciupClientTest for an example)
ciupServer support multiple modes:
- emulator: generate random data on N channels
- maccor: connect to Maccor devices through user dll procedure using ciupServerMaccorDll
- arbin: connect to Arbin devices

PROJECTS =====================================================================================

ciupClientDll : c++ dll with common functions for client side
ciupClientTest : test application to simulate client side (c++)
ciupClientTest-csc : test application to simulate client side (c#)
ciupServer: server side application, can act as simulator
ciupServerMaccorDll : user dll to connect Maccor test sw to ciup server
ciupServerMaccorDllTest : tesp application to feed the ciupServerMaccorDll as the maccor SW does

w32_tcp_socket_test : test application for UDP socket (both sides)
w32_message_pipe_test : test application for merssage pipe (both sides)

TEST =========================================================================================

Ciup client/server test:

Open two consoles (cmd.exe) in Release folder
On 1st console run: ciupServer -0 10000
On 2nd console run: ciupClientTest 127.0.0.1 10000

TCP socket test:

Open two consoles (cmd.exe) in Release folder
On 1st console run: w32_tcp_socket_test 10000
On 2nd console run: w32_tcp_socket_test -s -c 127.0.0.1 10000

Batch folder contains some other examples of tests

OTHER NOTES =================================================================================

Each application shows its own help if started with '-h' flag




