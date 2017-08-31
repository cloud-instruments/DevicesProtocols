========================================================================
    DLL: ciupServerMaccorDll Project Overview
========================================================================

ciupServerMaccorDll is designed to work with maccor sw as a 
"User DLL functions" described in 11.3.2 of 
"Automated Test System Instruction Manual"

ciupServerMaccorDll will be loaded by Maccor sw and feed with data during test

ciupServerMaccorDll pass the data to the ciupServer via named message pipe

To receive data ciupServer must run in maccor mode
