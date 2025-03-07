// --------------------------------------------------------------
//
//  Thinkpad Fan Control
//
// --------------------------------------------------------------
//
//	This program and source code is in the public domain.
//
//	The author claims no copyright, copyleft, license or
//	whatsoever for the program itself (with exception of
//	WinIO driver).  You may use, reuse or distribute it's 
//	binaries or source code in any desired way or form,  
//	Useage of binaries or source shall be entirely and 
//	without exception at your own risk. 
// 
// --------------------------------------------------------------

#include "_prec.h"
#include "fancontrol.h"
#include "TVicPort.h"

// Registers of the embedded controller
// V0.6.3+ V.2.2.0+
constexpr auto ACPI_EC_TYPE1_CTRLPORT = 0x1604;
constexpr auto ACPI_EC_TYPE1_DATAPORT = 0x1600  ;
// V0.6.2 final
constexpr auto ACPI_EC_TYPE2_CTRLPORT = 0x66  ;
constexpr auto ACPI_EC_TYPE2_DATAPORT = 0x62   ;

// Embedded controller status register bits
constexpr auto ACPI_EC_FLAG_OBF = 0x01	/* Output buffer full */;
constexpr auto ACPI_EC_FLAG_IBF = 0x02	/* Input buffer full */;
constexpr auto ACPI_EC_FLAG_CMD = 0x08	/* Input buffer contains a command */;

// Embedded controller commands
constexpr auto ACPI_EC_COMMAND_READ = (char)0x80;
constexpr auto ACPI_EC_COMMAND_WRITE = (char)0x81;
constexpr auto ACPI_EC_BURST_ENABLE = (char)0x82;
constexpr auto ACPI_EC_BURST_DISABLE = (char)0x83;
constexpr auto ACPI_EC_COMMAND_QUERY = (char)0x84;

//--------------------------------------------------------------------------
// wait for the desired status from the embedded controller (EC) via port io 
//--------------------------------------------------------------------------
static bool
WaitForFlags(USHORT port, char flags, int onoff = false, int timeout = 1000) {
	char data;

	int time = 0, sleepTicks = 10;

	// wait for flags to clear and reach desired state
	for (time = 0; time < timeout; time += sleepTicks) {
		data = ReadPort(port);

		int flagstate = (data & flags) != 0;
		int	wantedstate = onoff != 0;

		if (flagstate == wantedstate) return TRUE;

		::Sleep(sleepTicks);
	}
	return false;
}

//-------------------------------------------------------------------------
// read a byte from the embedded controller (EC) via port io 
//-------------------------------------------------------------------------
bool
FANCONTROL::ReadByteFromEC(int offset, char* pdata) {

	if (this->EC_CTRL == 0) {
		this->EC_CTRL = ACPI_EC_TYPE1_CTRLPORT;
		this->EC_DATA = ACPI_EC_TYPE1_DATAPORT;
		this->Trace("Using ACPI_EC_TYPE1");
	}

	// wait for IBF and OBF to clear
	if (!WaitForFlags(this->EC_CTRL, ACPI_EC_FLAG_IBF | ACPI_EC_FLAG_OBF)) {
		this->Trace("readec: timed out #1");
		if (this->EC_CTRL == ACPI_EC_TYPE1_CTRLPORT) {
			this->EC_CTRL = ACPI_EC_TYPE2_CTRLPORT;
			this->EC_DATA = ACPI_EC_TYPE2_DATAPORT;
			this->Trace("Now using ACPI_EC_TYPE2");
		}
		else {
			this->EC_CTRL = ACPI_EC_TYPE1_CTRLPORT;
			this->EC_DATA = ACPI_EC_TYPE1_DATAPORT;
			this->Trace("Now using ACPI_EC_TYPE1");
		}
		return false;
	}

	// indicate read operation desired
	WritePort(this->EC_CTRL, ACPI_EC_COMMAND_READ);

	// wait for IBF to clear (command byte removed from EC's input queue)
	if (!WaitForFlags(this->EC_CTRL, ACPI_EC_FLAG_IBF)) {
		this->Trace("readec: timed out #2");
		return false;
	}

	// indicate read operation desired location
	WritePort(this->EC_DATA, offset);

	// wait for IBF to clear (address byte removed from EC's input queue)
	// Note: Techically we should also waitforflags(OBF,TRUE) here,
	// (a byte being in the EC's output buffer being ready to read).
	if (!WaitForFlags(this->EC_CTRL, ACPI_EC_FLAG_IBF)) {
		this->Trace("readec: timed out #3");
		return false;
	}

	*pdata = ReadPort(this->EC_DATA);

	return TRUE;
}

//-------------------------------------------------------------------------
// write a byte to the embedded controller (EC) via port io
//-------------------------------------------------------------------------
bool
FANCONTROL::WriteByteToEC(int offset, char NewData) {

	// wait for IBF and OBF to clear
	if (!WaitForFlags(this->EC_CTRL, ACPI_EC_FLAG_IBF | ACPI_EC_FLAG_OBF)) {
		this->Trace("writeec: timed out #1");
		return false;
	}

	// indicate write operation desired
	WritePort(this->EC_CTRL, ACPI_EC_COMMAND_WRITE);

	// wait for IBF to clear (command byte removed from EC's input queue)
	if (!WaitForFlags(this->EC_CTRL, ACPI_EC_FLAG_IBF)) {
		this->Trace("writeec: timed out #2");
		return FALSE;
	}

	// indicate write operation desired location
	WritePort(this->EC_DATA, offset);

	// wait for IBF to clear (address byte removed from EC's input queue)
	if (!WaitForFlags(this->EC_CTRL, ACPI_EC_FLAG_IBF)) {
		this->Trace("writeec: timed out #3");
		return false;
	}

	// perform the write operation
	WritePort(this->EC_DATA, NewData);

	// wait for IBF to clear (data byte removed from EC's input queue)
	if (!WaitForFlags(this->EC_CTRL, ACPI_EC_FLAG_IBF)) {
		this->Trace("writeec: timed out #4");
		return false;
	}

	return TRUE;
}
