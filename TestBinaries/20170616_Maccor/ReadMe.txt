EMULATION TEST ==========================================

To run test execute MaccorEmulated.bat

Verify: ciupClientTest receive data produced by Maccor emulator

You can manually run more ciupServerMaccorDllTest.exe on different channels (-c N)
You can manually run more ciupClientTest.exe

Complete data path during maccor emulation: 
Maccor emulator -> ciupServerMaccorDll -> ciupServer -> ciupClientDll -> ciupClientTest

ciupServerMaccorDll is configured by ciupServerMaccorDll (comments inside) installed on C:\ciupServer
ciupServerMaccorDll generates logs inside C:\ciupServer

REAL DEVEICE TEST =======================================

install ciupServerMaccorDll.dll in C:\Maccor\System\"System ID"\MacUserFunctions

create the directory C:\ciupServer\
copy ciupServerMaccorDll.ini in C:\ciupServer\

Inside the maccor test sw assign the ciupServerMaccorDll.dll to a test as described 
in "MaccorHelpManual.pdf" "11.3.2.1 Assigning a DLL to a test" (pag 571)

Run MaccorReal.bat

Execute the configured test on Maccor test software
The ciupClient is expected to receive data
Logs are generated inside log folder and inside c:\ciupServer\ folder







