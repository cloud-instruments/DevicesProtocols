@echo off

: SETTINGS SECTION :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

: TCP port to use
SET port=10000

: address to use
SET ip=127.0.0.1

: logs filter (E:errors only, W: E+warning, T:W+trace, D:all)
SET logFilter=D

: maccor emlator sleep in mS
SET sleep=1000

: PROGRAM SECTION ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

: CREATE LOG DIRECTORY
@mkdir log >nul 2>&1

echo Run CIUP simulation test with server in Maccor mode

: GET DATETIME STRING
for /f "tokens=2,3,4 delims=/ " %%f in ('date /t') do set d=%%h%%g%%f
for /f "tokens=1,2 delims=: " %%f in ('time /t') do set t=%%f%%g
SET dt=%d%_%t%

: RUN CIUP SERVER IN MACCOR MODE
SET cmd=ciupServer.exe -1 -p -f %logFilter% -l log\%dt%-ciupServer-MACCOR.log %port%
echo Executing %cmd%
start %cmd%

: SLEEP A BIT
@ping 127.0.0.1 -n 2 -w 1000 > nul

: RUN C++ CLIENT TEST 
SET cmd=ciupClientTest.exe -f %logFilter% -l log\%dt%-ciupClientCpp-MACCOR.log %ip% %port%
echo Executing %cmd%
start %cmd%

: RUN MACCOR EMULATOR
SET cmd=ciupServerMaccorDllTest.exe -c 0 %sleep%
echo Executing %cmd%
start %cmd% 

echo On the other consoles:
echo   Hit [CTRL][C] to stop
echo   [Double click]to pause
echo   Hit [ENTER] to resume
echo Hit [SPACE] to close this console
pause > nul
