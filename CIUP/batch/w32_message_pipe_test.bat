@echo off
@setlocal EnableDelayedExpansion

REM SETTINGS SECTION :::::::::::::::::::::::::::::::::::::::::::::::::::::::::::

REM clients to start
SET clients=20

REM END OF SETTINGS SECTION ::::::::::::::::::::::::::::::::::::::::::::::::::::

ECHO Run message pipe server/client simulation test
ECHO w32_message_pipe_test project must be compiled in release mode inside ..\Release folder

REM RUN SERVER
SET cmd=..\Release\w32_message_pipe_test.exe
ECHO Executing %cmd%
START %cmd%

FOR /l %%I IN (1,1,%clients%) DO (

REM SLEEP A BIT
@PING 127.0.0.1 -n 2 -w 1000 > nul

SET cmd=..\Release\w32_message_pipe_test.exe -c
ECHO Executing !cmd!
START !cmd!

)

ECHO On the other consoles:
ECHO   Hit [CTRL][C] to stop
ECHO   [Double click]to pause
ECHO   Hit [ENTER] to resume
ECHO Hit [SPACE] to close this console
PAUSE > nul
