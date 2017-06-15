@echo off

: SETTINGS SECTION :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

: TCP port to use
SET port=10000

: address to use
SET ip=127.0.0.1

: logs filter (E:errors only, W: E+warning, T:W+trace, D:all)
SET logFilter=E

: server sleep in mS
: sleep=8 = 10000+ msg/s
SET sleep=8

: server channels count
SET ch=100

: PROGRAM SECTION ::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

: CREATE LOG DIRECTORY
@mkdir log >nul 2>&1

echo Run CIUP server/client simulation test
echo CIUP solution must be compiled in debug mode inside ..\Debug folder

: GET DATETIME STRING
for /f "tokens=2,3,4 delims=/ " %%f in ('date /t') do set d=%%h%%g%%f
for /f "tokens=1,2 delims=: " %%f in ('time /t') do set t=%%f%%g
SET dt=%d%_%t%

: RUN SERVER EMULATOR
SET cmd=..\Debug\ciupServer.exe -0 -p -c %ch% -s %sleep% -f %logFilter% %port% 
echo Executing %cmd%
start %cmd%

: SLEEP A BIT
@ping 127.0.0.1 -n 2 -w 1000 > nul

: RUN C++ CLIENT TEST
SET cmd=..\Debug\ciupClientTest.exe -p -f %logFilter% -l log\%dt%-ciupClientCpp-DEBUG.log %ip% %port%
echo Executing %cmd%
start %cmd%

: RUN C# CLIENT TEST
SET cmd=..\Debug\ciupClientTest-csc.exe -p -f %logFilter% -l log\%dt%-ciupClientCsc-DEBUG.log %ip% %port%
echo Executing %cmd%
start %cmd%

echo On the other consoles:
echo   Hit [CTRL][C] to stop
echo   [Double click]to pause
echo   Hit [ENTER] to resume
echo Hit [SPACE] to close this console
pause > nul
