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

int verbosity = 0;    // verbosity for the logbuf (0= nothing)

char lasterrorstring[256] = "", logbuf[8192] = "";

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

	// wait for full buffers to clear or timeout
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
		sprintf_s(logbuf + strlen(logbuf), sizeof(logbuf + strlen(logbuf)), "readec: timed out clear full buffers");
		return false;
	}

	// indicate read operation desired
	WritePort(EC_CTRLPORT, EC_CTRLPORT_READ);

	// wait for IBF and OBF to clear or timeout
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
		sprintf_s(logbuf + strlen(logbuf), sizeof(logbuf + strlen(logbuf)), "readec: timed out clear buffers");
		return false;
	}

	// indicate read operation desired location
	WritePort(EC_DATAPORT, offset);

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
		sprintf_s(logbuf + strlen(logbuf), sizeof(logbuf + strlen(logbuf)), "readec: timed out read data arrived");
		return false;
	}

	*pdata = ReadPort(EC_DATAPORT);

	if (verbosity > 0) sprintf_s(logbuf + strlen(logbuf), sizeof(logbuf + strlen(logbuf)), "readec: offset= %x, data= %02x\n", offset, *pdata);

	return 1;
}

//-------------------------------------------------------------------------
// write a byte to the embedded controller (EC) via port io
//-------------------------------------------------------------------------
int FANCONTROL::WriteByteToEC(int offset, char NewData) {
	char data = -1;
	char data2 = -1;

	int timedOut;
	int iTimeout = 100;
	int iTimeoutBuf = 1000;
	int iTime = 0;
	int iTick = 10;

	// wait for full buffers to clear or timeout
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
		sprintf_s(logbuf + strlen(logbuf), sizeof(logbuf + strlen(logbuf)), "writeec: timed out clear full buffers");
		return false;
	}

	//clear OBF if full
	if (data & EC_STAT_OBF) {
		data2 = (char)ReadPort(EC_DATAPORT);                  
	}

	// wait for IOBF to clear
	timedOut = true;
	for (iTime = 0; iTime < iTimeout; iTime += iTick) {
		data = (char)ReadPort(EC_CTRLPORT) & 0xff;
		if (!(data & EC_STAT_OBF)) {
			timedOut = false;
			break;
		}
		::Sleep(iTick);
	}
	if (timedOut) {
		sprintf_s(logbuf + strlen(logbuf), sizeof(logbuf + strlen(logbuf)), "writeec: timed out clear buffers");
		return false;
	}

	// indicate write operation desired
	WritePort(EC_CTRLPORT, EC_CTRLPORT_WRITE);

	// wait for IBF and OBF to clear or timeout
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
		sprintf_s(logbuf + strlen(logbuf), sizeof(logbuf + strlen(logbuf)), "writeec: timed out clear buffers before write");
		return false;
	}

	// indicate read operation desired location
	WritePort(EC_DATAPORT, offset);                           

	// wait for IBF and OBF to clear
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
		sprintf_s(logbuf + strlen(logbuf), sizeof(logbuf + strlen(logbuf)), "writeec: timed out clear buffers after write");
		return false;
	}

	// perform the write operation
	WritePort(EC_DATAPORT, NewData);

	return 1;
}


//-------------------------------------------------------------------------
//  experimental code
//-------------------------------------------------------------------------
void
FANCONTROL::Test(void) {

	/*
		//
		// defines from various DDK sources
		//

		#define IOCTL_ACPI_ASYNC_EVAL_METHOD            CTL_CODE(FILE_DEVICE_ACPI, 0, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
		#define IOCTL_ACPI_EVAL_METHOD                  CTL_CODE(FILE_DEVICE_ACPI, 1, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
		#define IOCTL_ACPI_ACQUIRE_GLOBAL_LOCK          CTL_CODE(FILE_DEVICE_ACPI, 4, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)
		#define IOCTL_ACPI_RELEASE_GLOBAL_LOCK          CTL_CODE(FILE_DEVICE_ACPI, 5, METHOD_BUFFERED, FILE_READ_ACCESS | FILE_WRITE_ACCESS)

		#define FILE_DEVICE_ACPI                0x00000032

		#define CTL_CODE( DeviceType, Function, Method, Access ) (                 \
			((DeviceType) << 16) | ((Access) << 14) | ((Function) << 2) | (Method) \
		)

		#define METHOD_BUFFERED                 0
		#define METHOD_IN_DIRECT                1
		#define METHOD_OUT_DIRECT               2
		#define METHOD_NEITHER                  3


		#define FILE_ANY_ACCESS             0
		#define FILE_SPECIAL_ACCESS			(FILE_ANY_ACCESS)
		#define FILE_READ_ACCESS			( 0x0001 )    // file & pipe
		#define FILE_WRITE_ACCESS			( 0x0002 )    // file & pipe




		//
		// try to communicate with the ACPI driver via IOCTL
		// (doesn't work, driver won't open on CreateFile under
		// any name)
		//

		HANDLE hDevice= ::CreateFile( "\\\\.\\DRIVER\\ACPI",
									   GENERIC_WRITE,
									   FILE_SHARE_WRITE,
									   NULL,
									   OPEN_EXISTING,
									   0,
									   NULL );

		if (hDevice!=INVALID_HANDLE_VALUE) {

			USHORT t[2];
			ULONG howmany= 0;
			ULONG ret= 0;
			BOOL ioctlresult= 0;

			t[0] = (USHORT)0;
			t[1] = (USHORT)0;

			ioctlresult= DeviceIoControl(
								hDevice,
								IOCTL_ACPI_ACQUIRE_GLOBAL_LOCK,
								t,
								sizeof(ULONG),
								&ret,
								sizeof(ULONG),
								&howmany,
								NULL );



			::CloseHandle(hDevice);
		}

	*/
}
