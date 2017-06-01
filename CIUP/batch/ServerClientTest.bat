@echo off

: SETTINGS SECTION
SET port=10000

echo Run CIUP server/client simulation test
echo CIUP solution must be compiled in release mode

: GET DATETIME STRING
for /f "tokens=2,3,4 delims=/ " %%f in ('date /t') do set d=%%h%%g%%f
for /f "tokens=1,2 delims=: " %%f in ('time /t') do set t=%%f%%g
SET dt=%d%_%t%

: RUN SERVER SIMULATOR
echo Executing ciupServer simulator on UDP port %port%
start ..\Release\ciupServerTest.exe -l %dt%-ciupServer.log %port%

: SLEEP A BIT
@ping 127.0.0.1 -n 2 -w 1000 > nul

: RUN C++ CLIENT SIMULATOR 
echo Executing ciupClientSimulator connecting the server simulator
start ..\Release\ciupClientTest.exe  -l %dt%-ciupClientCpp.log 127.0.0.1 %port%

: RUN C# CLIENT SIMULATOR
echo Executing ciupClientSimulator C# version connecting the server simulator
start ..\Release\ciupClientTest-csc.exe -l %dt%-ciupClientCsc.log 127.0.0.1 %port%

echo On the other consoles:
echo   Hit [CTRL][C] to stop
echo   [Double click]to pause
echo   Hit [ENTER] to resume
echo Hit [SPACE] to close this console
pause > nul
