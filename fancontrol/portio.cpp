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
#define EC_DATAPORT    0x1600    // EC data io-port 0x62
#define EC_CTRLPORT    0x1604    // EC control io-port 0x66

// Embedded controller status register bits
#define EC_STAT_OBF     0x01     // Output buffer full
#define EC_STAT_IBF     0x02     // Input buffer full
#define EC_STAT_CMD     0x08     // Last write was a command write (0=data)

// Embedded controller commands
// (write to EC_CTRLPORT to initiate read/write operation)
#define EC_CTRLPORT_READ      (char)0x80
#define EC_CTRLPORT_WRITE     (char)0x81
#define EC_CTRLPORT_QUERY     (char)0x84

//-------------------------------------------------------------------------
// read a byte from the embedded controller (EC) via port io 
//-------------------------------------------------------------------------
int FANCONTROL::ReadByteFromEC(int offset, char* pdata) {
	char data = -1;

	int timedOut;
	int iTime = 0;
	int iTimeoutBuf = 1000;
	int iTimeout = 100;
	int iTick = 10;

	// wait for BOTH buffers to clear or timeout
	timedOut = true;
	for (iTime = 0; iTime < iTimeoutBuf; iTime += iTick) {
		data = (char)ReadPort(EC_CTRLPORT) & 0xff;
		if (!(data & (EC_STAT_IBF | EC_STAT_OBF))) {
			timedOut = false;
			break;
		}
		::Sleep(iTick);
	}
	if (timedOut) {
		this->Trace("readec: timed out #1\n");
		return false;
	}

	if (data & EC_STAT_OBF) {
		int data2 = (char)ReadPort(EC_DATAPORT);
	}

	// indicate read operation desired
	WritePort(EC_CTRLPORT, EC_CTRLPORT_READ);

	// indicate read operation desired location
	WritePort(EC_DATAPORT, offset);

	// wait for OBF to clear
	timedOut = true;
	for (iTime = 0; iTime < iTimeout; iTime += iTick) {        
		data = (char)ReadPort(EC_CTRLPORT) & 0xff;
		if ((data & EC_STAT_OBF)) {
			timedOut = false;
			break;
		}
		::Sleep(iTick);
	}
	if (timedOut) {
		this->Trace("readec: timed out #2\n");
		return false;
	}

	*pdata = ReadPort(EC_DATAPORT);

	return 1;
}

//-------------------------------------------------------------------------
// write a byte to the embedded controller (EC) via port io
//-------------------------------------------------------------------------
int FANCONTROL::WriteByteToEC(int offset, char NewData) {
	char data = -1;

	int timedOut;
	int iTimeout = 100;
	int iTimeoutBuf = 1000;
	int iTime = 0;
	int iTick = 10;

	// wait for BOTH buffers to clear or timeout
	timedOut = true;
	for (iTime = 0; iTime < iTimeoutBuf; iTime += iTick) {
		data = (char)ReadPort(EC_CTRLPORT) & 0xff;
		if (!(data & (EC_STAT_IBF | EC_STAT_OBF))) {
			timedOut = false;
			break;
		}
		::Sleep(iTick);
	}
	if (timedOut) {
		this->Trace("writeec: timed out #1\n");
		return false;
	}

	if (data & EC_STAT_OBF) {
		int data2 = (char)ReadPort(EC_DATAPORT);
	}

	// wait for OBF buffer to clear or timeout
	timedOut = true;
	for (iTime = 0; iTime < iTimeoutBuf; iTime += iTick) {
		data = (char)ReadPort(EC_CTRLPORT) & 0xff;
		if (!(data & EC_STAT_OBF)) {
			timedOut = false;
			break;
		}
		::Sleep(iTick);
	}
	if (timedOut) {
		this->Trace("writeec: timed out #2\n");
		return false;
	}

	// indicate write operation desired
	WritePort(EC_CTRLPORT, EC_CTRLPORT_WRITE);

	// wait for BOTH buffers to clear or timeout
	timedOut = true;
	for (iTime = 0; iTime < iTimeoutBuf; iTime += iTick) {
		data = (char)ReadPort(EC_CTRLPORT) & 0xff;
		if (!(data & (EC_STAT_IBF | EC_STAT_OBF))) {
			timedOut = false;
			break;
		}
		::Sleep(iTick);
	}
	if (timedOut) {
		this->Trace("writeec: timed out #3\n");
		return false;
	}

	// indicate read operation desired location
	WritePort(EC_DATAPORT, offset);                           

	// wait for BOTH buffers to clear or timeout
	timedOut = true;
	for (iTime = 0; iTime < iTimeout; iTime += iTick) {       
		data = (char)ReadPort(EC_CTRLPORT) & 0xff;
		if (!(data & (EC_STAT_IBF | EC_STAT_OBF))) {
			timedOut = false;
			break;
		}
		::Sleep(iTick);
	}
	if (timedOut) {
		this->Trace("writeec: timed out #4\n");
		return false;
	}

	// perform the write operation
	WritePort(EC_DATAPORT, NewData);

	return 1;
}
