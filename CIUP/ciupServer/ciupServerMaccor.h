/*
Copyright(c) <2018> <University of Washington>
Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

// ciupServerMaccor: start thread to connect ciupServerMaccorDll via named pipe
// will be used in paralle to ciupSErver MaccorDll

#pragma once

#include "ciupServerCommon.h"
#include "..\w32_message_pipe_test\w32_message_pipe.h"

// start the connection with the dll
int serverMaccorStart();

// stop the connection with the dll
int serverMaccorStop();

// get and delete older log message from FIFO
// return -1 if FIFO is empty
int maccorGetLog(ciupLog *log);

// set loglevel to avoid queueing filtered logs
void maccorSetLogFilter(streamlog::level filter);