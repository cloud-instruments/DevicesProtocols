

/****************************************************************************
 *                                                                          *
 * NOTES ON DIFFERENT C IMPLEMENTATIONS
 *
 * In Pelles C it is important to set "Project/Project options/Compiler" -> 
                                       undecorate exported __stdcall functions
   If not, then "GetProcAddress" doesn't work.

 * Structures must be byte aligned.

 *                                                                          *
 ****************************************************************************/


typedef unsigned char  uint8;
typedef unsigned short uint16;
typedef unsigned int   uint32;

#pragma pack(1)
typedef struct   
{
    float   Vmax;					//The maximum voltage of the channel
    float   Vmin;					//The minimum voltage of the channel
    float   ChI;					//The maximum charge current of the channel
    float   DisI;					//The maximum discharge current of the channel
} TSpecData;
#pragma pack()

#pragma pack(1)
typedef struct 
{
    uint16  APIversion;				//Always check this number in case the interface have been changed
    uint32  SWversion;				//Always check this number in case the interface have been changed
    uint32  DLLversion;				//Value returned from external DLL
    char    DLLversionString[80];	//Value returned from external DLL
    char    SystemID[256];			//ID of the test system
    char    TestName[256];			//Name of the test (data file)
    char    TestComment[256];		//Test comment entered at test start
    char    ProcedureName[256];		//Test procedure name
    char    ProcedureComment[256];	//Test procedure comment
    double  TimeStamp;				//Current date and time.
    char    MaccorPath[256];		//File path to the Maccor folder. Typically C:\Maccor
    char    SysPath[256];			//File path to the System folder. Typically C:\Maccor\System\"SystemID"
    char    ProcPath[256];			//File path to the procedure folder. Typically C:\Maccor\Procedur
    char    StepNote[256];			//Step note from the current step of the test procedure
    float   CRate;					//C-rate entered at the start of the test.
	float   VRate;					//Not used yet
	float	Prate;					//Not used yet
	float	Rrate;					//Not used yet
	float	Mass;					//Mass of the battery
	float	Volume;					//Not used yet
	float	Area;					//Not used yet
} TTestDataRevA;
#pragma pack()

#pragma pack(1)
typedef struct 
{
    uint8   Addr;					//SMB address
    uint16  Ptr;					//Pointer to the first byte in the StatusData.SMBdata and Status.SMBHdrData
    uint8   Len;					//Number of bytes for the register
}TSMBinfo;
#pragma pack()

#pragma pack(1)
typedef struct 
{
    uint8   RF1;					//The is the mode of the test step
    uint8   RF2;					//This the ES code of the test step
    int     Cycle;					//Cycle number
    int     Step;					//Test step (zero based)
    float   TestTime;				//Test time in seconds
    float   StepTime;				//Step time in seconds
    float   Voltage;				//Voltage in Volts
    float   Current;				//Current in Amps
    float   Capacity;				//Capacity in Ah
    float   HCCapacity;				//Half cycle capacity in Ah
    float   LHCCapacity;			//Last half cycle capacity in Ah
    float   Energy;					//Energy in Wh
    float   HCEnergy;				//Half cycle energy in Wh
    float   LHCEnergy;				//Last half cycle energy in Wh
    float   tFactor;				//Tick time in seconds
    float   AUXdata[48];			//Values of up to 48 assigned auxiliary inputs
    uint8   AUXinput[48];			//Auxiliary input channel
    uint16  AUXtype[48];			//Auxiliary input type: 1: Voltage; 2: Thermocple; 3: Thermistor; 4: Pressure; 5: Resistance;
    char    AUXunit[48][3];			//Auxiliary input engineering unit
    uint8 SMBdata[512];				//512 byte of SMB data
    uint8 SMBHdrData[512];			//512 byte of SMB header data
    TSMBinfo SMBDataInfo[64];		//Specification of up to 64 assigned SMB registers. The SMB readings are stored in the StatusData.SMBdata
    TSMBinfo SMBHdrInfo[64];		//Specification of up to 64 assigned SMB registers. The SMB readings are stored in the StatusData.SMBHdrData
} TStatusData;
#pragma pack()

#pragma pack(1)
typedef struct 
{
    float   Current;				//Current set point in A. Negative values indicates ignore.
    float   Voltage;				//Voltage set point in V. Negative values indicates ignore.
    float   Power;					//Power set point in W. Negative values indicates ignore.
    float   Resistance;				//Resistance set point in Ohm. Negative values indicates ignore.
    int     Flags;					//1. OK, continue in current step
                       				//2. OK, go to next step
                       				//3. Error - terminate test
                       				//4. Pause
} TOutData;
#pragma pack()

#pragma pack(1)
typedef struct 
{
	uint8 	BusAddr;				//0x16: I2C and SMB
	uint8 	StartDataAddr;			//First address of register to write to
	uint8 	EndDataAddr;			//Last address of register to write to. 
									//If StartDataAddr = EndDataAddr all data are written to the same address.
									//If StartDataAddr < EndDataAddr the data are written one byte to each address in the range.
	uint8 	NumOfBytes;				//Number of bytes to write to a single address
	uint8 	BusType;				//01: I2C and SMB
									//02: HDQ
									//03: Future 1-wire
	uint8 	CheckSum;				//Checksum for the actual device
	uint8	MinDelayToNext10ms;		//Minimum delay in multiples of 10 ms before the next write
	uint8 	Data[256];				//Data to write
} TSMBGenericWriteData;
#pragma pack()

typedef int (__stdcall *PWriteSMBCallBack)(TSMBGenericWriteData*);							//Use this function to write the data in the TSMBGenericWriteData to the SMB assigned to the channel
																							//This function is identical to MacNet (7, 10) Write multiple bytes B. 
	//Return value
	//0: OK
	//1: Not called from event
	//2: Internal error
	//3: Buffer full 
																							
typedef int (__stdcall *PReportCallBack)(const char *Msg, int I1, int I2, BOOL Displayed);	//Use this function to generate an event in the tester programs event system. 
																							//The event will be logged in the SystemEvents.log and optionally cause a pop-up dialog if Displayed is set to TRUE
																							//The event will contain the "Msg" string (up to 20 characters) and the I1 and I2 ints
	//Return value
	//0: OK
	//1: Not called from event
	//2: Internal error

__declspec(dllexport) int __stdcall OnLoadRevA( int channel, int unused_flags, TSpecData* SpecData, TStatusData* StatusData, TTestDataRevA* TestData, PReportCallBack ReportCallBack, PWriteSMBCallBack WriteSMBCallBack );
__declspec(dllexport) int __stdcall OnStepStartRevA( int channel, int unused_flags, int step, TSpecData* SpecData, TStatusData* StatusData, TTestDataRevA* TestData, PReportCallBack ReportCallBack, PWriteSMBCallBack WriteSMBCallBack );
__declspec(dllexport) int __stdcall OnStepEndRevA( int channel, int unused_flags, int step, TSpecData* SpecData, TStatusData* StatusData, PReportCallBack ReportCallBack, PWriteSMBCallBack WriteSMBCallBack );
__declspec(dllexport) int __stdcall OnUnLoadRevA( int channel, int unused_flags, PReportCallBack ReportCallBack, PWriteSMBCallBack WriteSMBCallBack );
__declspec(dllexport) int __stdcall GetSetpointRevA( int channel, int unused_flags, TSpecData* SpecData, TStatusData* StatusData, TOutData* OutData, PReportCallBack ReportCallBack, PWriteSMBCallBack WriteSMBCallBack );
__declspec(dllexport) int __stdcall OnSuspendRevA( int channel, int unused_flags, TSpecData* SpecData, TStatusData* StatusData, TTestDataRevA* TestData, PReportCallBack ReportCallBack, PWriteSMBCallBack WriteSMBCallBack );
__declspec(dllexport) int __stdcall OnResumeRevA( int channel, int unused_flags, TSpecData* SpecData, TStatusData* StatusData, TTestDataRevA* TestData, PReportCallBack ReportCallBack, PWriteSMBCallBack WriteSMBCallBack );

