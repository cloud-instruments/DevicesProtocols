TEST 1 ==================================================================

MaccorU1Test.exe is a simple loader test for MaccorU1.dll

That's the expected output on console:

>MaccorU1Test1.exe
MaccorU1.dll loaded: check log in C:\MaccorU1 folder

It should create a file (data_pid.txt) inside the C:\MaccorU1 folder

If there are problems with MaccorU1Test.exe you vc_redist.xxx.exe mus be 
installed as for Arbin tests (take it from the 20170515_A3P-1 folder)

TEST 2 =================================================================

Copy MaccorU1.dll in C:\Maccor\System\"System ID"\MacUserFunctions

Assign the MaccorU1.dll to a test as described in "MaccorHelpManual.pdf"
"11.3.2.1 Assigning a DLL to a test" (pag 571)

Running the test the dll logs are collected in C:\MaccorU1 folder

