========================================================================
    CONSOLE APPLICATION : ciupServer Project Overview
========================================================================

Server application for Cloud Instruments Unified Protocol

Basic use (Emulation): ciupServer.exe 10000

open UDP port 10000 and listen to commands
start command start a thread to send data to client

ciupServer receive data from any of the supported HW and send data via TCP 
socket to any connected app through the ciupClientDll

At startup one of the running mode must be choosen:

- Emulator mode: generate random data
- Maccor mode: receive data from ciupServerMaccorDll via named pipe
- Arbin mode: TODO

