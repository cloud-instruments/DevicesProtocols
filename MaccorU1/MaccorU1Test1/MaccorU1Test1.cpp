// MaccorU1Test1.cpp : Test app for MaccorU1.dll

#pragma comment(lib, "MaccorU1.lib")

#include "stdafx.h"
#include <windows.h>
#include "../MaccorU1/MaccorU1.h"

#include <iostream>

int main()
{
	// just to test dll calls
	TSpecData SpecData;
	SpecData.ChI = 10;
	SpecData.DisI = 1;
	SpecData.Vmax = 5;
	SpecData.Vmin = -5;

	TStatusData StatusData;
	StatusData.RF1 = 1U;
	StatusData.RF2 = 2U;
	StatusData.Cycle=3;
	StatusData.Step=4;
	StatusData.TestTime=5;
	StatusData.StepTime=5.1F;
	StatusData.Voltage=5.2F;
	StatusData.Current=5.3F;
	StatusData.Capacity=5.4F;
	StatusData.HCCapacity=5.5F;
	StatusData.LHCCapacity=5.6F;
	StatusData.Energy=5.7F;
	StatusData.HCEnergy=5.8F;
	StatusData.LHCEnergy=5.9F;
	StatusData.tFactor=6.0F;

	TTestDataRevA TestData;
	TestData.APIversion=1;
	TestData.SWversion=2;
	TestData.DLLversion=3;
	strncpy_s(TestData.DLLversionString, "DLLversionString", 80);
	strncpy_s(TestData.SystemID, "SystemID", 256);
	strncpy_s(TestData.TestName, "TestName", 256);
	strncpy_s(TestData.TestComment, "TestComment", 256);
	strncpy_s(TestData.ProcedureName, "ProcedureName", 256);
	strncpy_s(TestData.ProcedureComment,"ProcedureComment", 256);
	TestData.TimeStamp=1234.5;
	strncpy_s(TestData.MaccorPath,"MaccorPath", 256);
	strncpy_s(TestData.SysPath,"SysPath", 256);
	strncpy_s(TestData.ProcPath,"ProcPath", 256);
	strncpy_s(TestData.StepNote,"StepNote", 256);
	TestData.CRate=1.1F;
	TestData.Mass=2.2F;

	OnLoadRevA(0, 0, &SpecData, &StatusData, &TestData, NULL, NULL);

	std::cout << "MaccorU1.dll correctly loaded. Check log in C:\\MaccorU1 folder" << std::endl;
    return 0;
}

