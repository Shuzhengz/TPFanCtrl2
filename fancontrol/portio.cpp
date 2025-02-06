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
#define ACPI_EC_DATAPORT    0x1600  // EC data io-port 0x62
#define ACPI_EC_CTRLPORT    0x1604  // EC control io-port 0x66

// Embedded controller status register bits
#define ACPI_EC_FLAG_OBF	0x01	/* Output buffer full */
#define ACPI_EC_FLAG_IBF	0x02	/* Input buffer full */
#define ACPI_EC_FLAG_CMD	0x08	/* Input buffer contains a command */
#define ACPI_EC_FLAG_BURST	0x10	/* burst mode */
#define ACPI_EC_FLAG_SCI	0x20	/* EC-SCI occurred */

// Embedded controller commands
#define ACPI_EC_COMMAND_READ      (char)0x80
#define ACPI_EC_COMMAND_WRITE     (char)0x81
#define ACPI_EC_BURST_ENABLE      (char)0x82
#define ACPI_EC_BURST_DISABLE     (char)0x83
#define ACPI_EC_COMMAND_QUERY     (char)0x84

//--------------------------------------------------------------------------
// wait for the desired status from the embedded controller (EC) via port io 
//--------------------------------------------------------------------------
int FANCONTROL::WaitForFlags(char flags, int onoff, int timeout) {
	char data;

	int iTime = 0;
	int iTick = 10;

	// wait for flags to clear
	for (iTime = 0; iTime < timeout; iTime += iTick) {
		data = ReadPort(ACPI_EC_CTRLPORT);

		int flagstate = (data & flags) != 0;
		int	wantedstate = onoff != 0;

		if (flagstate == wantedstate)
			return true;

		::Sleep(iTick);
	}
	return false;
}

//-------------------------------------------------------------------------
// read a byte from the embedded controller (EC) via port io 
//-------------------------------------------------------------------------
int FANCONTROL::ReadByteFromEC(int offset, char* pdata) {

	// wait for IBF and OBF to clear
	if (!WaitForFlags(ACPI_EC_FLAG_IBF | ACPI_EC_FLAG_OBF)) {
		this->Trace("readec: timed out #1");
		return false;
	}

	// indicate read operation desired
	WritePort(ACPI_EC_CTRLPORT, ACPI_EC_COMMAND_READ);

	// wait for IBF to clear (command byte removed from EC's input queue)
	if (!WaitForFlags(ACPI_EC_FLAG_IBF)) {
		this->Trace("readec: timed out #2");
		return false;
	}

	// indicate read operation desired location
	WritePort(ACPI_EC_DATAPORT, offset);

	// wait for IBF to clear (address byte removed from EC's input queue)
	// Note: Techically we should also waitforflags(OBF,TRUE) here,
	// (a byte being in the EC's output buffer being ready to read).
	if (!WaitForFlags(ACPI_EC_FLAG_IBF)) {
		this->Trace("readec: timed out #3");
		return false;
	}

	*pdata = ReadPort(ACPI_EC_DATAPORT);

	return true;
}

//-------------------------------------------------------------------------
// write a byte to the embedded controller (EC) via port io
//-------------------------------------------------------------------------
int FANCONTROL::WriteByteToEC(int offset, char NewData) {

	// wait for IBF and OBF to clear
	if (!WaitForFlags(ACPI_EC_FLAG_IBF | ACPI_EC_FLAG_OBF)) {
		this->Trace("writeec: timed out #1");
		return false;
	}

	// indicate write operation desired
	WritePort(ACPI_EC_CTRLPORT, ACPI_EC_COMMAND_WRITE);

	// wait for IBF to clear (command byte removed from EC's input queue)
	if (!WaitForFlags(ACPI_EC_FLAG_IBF)) {
		this->Trace("writeec: timed out #2");
		return FALSE;
	}

	// indicate write operation desired location
	WritePort(ACPI_EC_DATAPORT, offset);                           

	// wait for IBF to clear (address byte removed from EC's input queue)
	if (!WaitForFlags(ACPI_EC_FLAG_IBF)) {
		this->Trace("writeec: timed out #3");
		return false;
	}

	// perform the write operation
	WritePort(ACPI_EC_DATAPORT, NewData);

	// wait for IBF to clear (data byte removed from EC's input queue)
	if (!WaitForFlags(ACPI_EC_FLAG_IBF)) {
		this->Trace("writeec: timed out #3");
		return false;
	}

	return true;
}
