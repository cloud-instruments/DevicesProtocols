REM THE SCRIPT START A CIUP SERVER EMULATOR WITH n TEST CLIENTS

@echo off
@setlocal EnableDelayedExpansion

REM SETTINGS SECTION :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

REM TCP port to use
SET port=10000

REM address to use
SET ip=127.0.0.1

REM logs filter (E:errors only, W: E+warning, T:W+trace, D:all)
SET logFilter=E

REM server sleep in mS
REM sleep=8 = 10000+ msg/s
SET sleep=8

REM server channels count
SET ch=100

REM clients to start
SET clients=3

REM END OF SETTINGS SECTION ::::::::::::::::::::::::::::::::::::::::::::::::::::

REM CREATE LOG DIRECTORY
@MKDIR log >nul 2>&1

REM GET DATETIME STRING
FOR /f "tokens=2,3,4 delims=/ " %%f IN ('date /t') DO SET d=%%h%%g%%f
FOR /f "tokens=1,2 delims=: " %%f IN ('time /t') DO SET t=%%f%%g
SET dt=%d%_%t%

REM RUN SERVER EMULATOR
SET cmd=ciupServerEmulator.exe -c 100 -p -s %sleep% -f %logFilter% -l log\%dt%-ciupServerEmulator.log %port% 
ECHO Executing %cmd%
START %cmd%

FOR /l %%I IN (1,1,%clients%) DO (

REM SLEEP A BIT
@PING 127.0.0.1 -n 2 -w 1000 > nul

SET cmd=ciupClientTest.exe -p -f %logFilter% -l log\%dt%-ciupClientCpp-%%I.log %ip% %port%
ECHO Executing !cmd!
START !cmd!

)

ECHO On the other consoles:
ECHO   Hit [CTRL][C] to stop
ECHO   [Double click]to pause
ECHO   Hit [ENTER] to resume
ECHO Hit [SPACE] to close this console
PAUSE > nul
