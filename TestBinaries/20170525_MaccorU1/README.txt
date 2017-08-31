TEST 1 ==================================================================

This test verifies copatibility problems with the dll binary formats.

MaccorU1Test.exe is a simple loader test for MaccorU1.dll

1) Run in cosole the command "MaccorU1Test1.exe"

2) That's the expected output on console:

MaccorU1.dll loaded: check log in C:\MaccorU1 folder

3) It should create a file (unixtime_pid.txt) inside the C:\MaccorU1 folder

If there are problems with MaccorU1Test.exe vc_redist.xxx.exe must be 
installed as for Arbin tests (take it from the 20170515_A3P-1 folder)

TEST 2 =================================================================

This test verifies the Maccor User Function dll using the factory dll binary 

1) Copy MacUserFunctionDEMO.dll in C:\Maccor\System\"System ID"\MacUserFunctions

2)Assign the MacUserFunctionDEMO.dll to a test as described in "MaccorHelpManual.pdf":

- "11.3.2.1 Assigning a DLL to a test" (pag 571)
  In this chapter are described two different ways to do this
  
3)Running the test should create
C:\\Maccor\\DEMO_readings.txt
and 
C:\\Maccor\\DEMO_current_setpoint.txt

TEST 3 =========================================================================

This test verifies the compilation settings for dll using the factory dll code
The MaccorUserFunctionDEMO.dll should behave exactly like MacUserFunctionDEMO.dll
The only difference is that MaccorUserFunctionDEMO.dll has been compiled starting
from sources while MacUserFunctionDEMO.dll has been compiled by maccor factory.

1) Procedure is exactly the same as TEST 2 but using MaccorUserFunctionDEMO.dll

TEST 4 =========================================================================

This test verifies MaccorU1.dll (complete custom dll for Maccor User Function)

1) Copy MaccorU1.dll in C:\Maccor\System\"System ID"\MacUserFunctions

2) Assign the MacUserFunctionDEMO.dll to a test as described in TEST 2

3) Running the test the dll logs are collected in C:\MaccorU1 folder

