// (c)2017 Matteo Lucarelli matteo.lucarelli@toptal.com
// MaccorU1Test1.cpp : Test app for MaccorU1.dll
// Date 20170519
////////////////////////////////////////////////////////////////////////////////

#pragma comment(lib, "MaccorU1.lib")

#include "stdafx.h"
#include <windows.h>
#include "../MaccorU1/MaccorU1.h"

#include <iostream>

int main()
{
	// just to test dll calls
	OnUnLoadRevA(0, 0, NULL, NULL);

	std::cout << "MaccorU1.dll loaded: check log in C:\\MaccorU1 folder" << std::endl;
    return 0;
}

