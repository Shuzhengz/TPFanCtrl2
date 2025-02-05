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
int FANCONTROL::WaitForFlags(int timeout, char flags, BOOL pos) {
	char data;

	int timedOut;
	int iTime = 0;
	int iTick = 10;

	// wait for flags to clear
	timedOut = true;
	for (iTime = 0; iTime < timeout; iTime += iTick) {
		data = ReadPort(ACPI_EC_CTRLPORT);
		if ((pos && !(data & flags)) || (!pos && (data & flags))) {
			timedOut = FALSE;
			break;
		}
		::Sleep(iTick);
	}
	return timedOut;
}

//-------------------------------------------------------------------------
// read a byte from the embedded controller (EC) via port io 
//-------------------------------------------------------------------------
int FANCONTROL::ReadByteFromEC(int offset, char* pdata) {
	int iTimeoutBuf = 1000;
	int iTimeout = 100;

	if (WaitForFlags(iTimeoutBuf, ACPI_EC_FLAG_IBF | ACPI_EC_FLAG_OBF, TRUE)) {
		this->Trace("readec: timed out #1");
		return FALSE;
	}

	// indicate read operation desired
	WritePort(ACPI_EC_CTRLPORT, ACPI_EC_COMMAND_READ);

	if (WaitForFlags(iTimeout, ACPI_EC_FLAG_IBF | ACPI_EC_FLAG_OBF, TRUE)) {
		this->Trace("readec: timed out #2");
		return FALSE;
	}

	// indicate read operation desired location
	WritePort(ACPI_EC_DATAPORT, offset);

	if (WaitForFlags(iTimeout, ACPI_EC_FLAG_OBF, FALSE)) {
		this->Trace("readec: timed out #3");
		return FALSE;
	}

	*pdata = ReadPort(ACPI_EC_DATAPORT);

	return TRUE;
}

//-------------------------------------------------------------------------
// write a byte to the embedded controller (EC) via port io
//-------------------------------------------------------------------------
int FANCONTROL::WriteByteToEC(int offset, char NewData) {
	char data;

	int iTimeout = 100;
	int iTimeoutBuf = 1000;

	if (WaitForFlags(iTimeoutBuf, ACPI_EC_FLAG_IBF | ACPI_EC_FLAG_OBF, TRUE)) {
		this->Trace("writeec: timed out #1");
		return FALSE;
	}

	// indicate write operation desired
	WritePort(ACPI_EC_CTRLPORT, ACPI_EC_COMMAND_WRITE);

	if (WaitForFlags(iTimeout, ACPI_EC_FLAG_IBF | ACPI_EC_FLAG_OBF, TRUE)) {
		this->Trace("writeec: timed out #2");
		return FALSE;
	}

	// indicate write operation desired location
	WritePort(ACPI_EC_DATAPORT, offset);                           

	if (WaitForFlags(iTimeout, ACPI_EC_FLAG_IBF | ACPI_EC_FLAG_OBF, TRUE)) {
		this->Trace("writeec: timed out #3");
		return FALSE;
	}

	// perform the write operation
	WritePort(ACPI_EC_DATAPORT, NewData);

	return TRUE;
}
