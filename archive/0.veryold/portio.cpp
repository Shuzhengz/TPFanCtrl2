
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

#include "fancontrol.h"
#include "winio.h"


// Registers of the embedded controller
#define EC_DATAPORT		0x62	// EC data io-port
#define EC_CTRLPORT		0x66	// EC control io-port


// Embedded controller status register bits
#define EC_STAT_OBF		0x01    // Output buffer full 
#define EC_STAT_IBF		0x02    // Input buffer full 
#define EC_STAT_CMD		0x08    // Last write was a command write (0=data) 


// Embedded controller commands
// (write to EC_CTRLPORT to initiate read/write operation)
#define EC_CTRLPORT_READ		(char)0x80	
#define EC_CTRLPORT_WRITE		(char)0x81
#define EC_CTRLPORT_QUERY		(char)0x84





int verbosity= 0;	// verbosity for the logbuf (0= nothing)
char lasterrorstring[256]= "",
	 logbuf[8192]= "";




//-------------------------------------------------------------------------
//  read control port and wait for set/clear of a status bit
//-------------------------------------------------------------------------
int 
waitportstatus(int bits, int onoff= false, int timeout= 1000)
{
	int ok= false,
		port= EC_CTRLPORT,
		time= 0,
		tick= 10;

	strcpy(lasterrorstring, "");

	//
	// wait until input on control port has desired state or times out
	//
	for (time= 0; time<timeout; time+= tick) {

		DWORD data= 0;
		int done= GetPortVal(port, &data, 1);

		if (verbosity>0)
			sprintf(logbuf+strlen(logbuf), "waitportstatus: tick port= %02x, status= %02x => data was %02x\n", port, bits, data);

		if (!done) {
			strcpy(lasterrorstring, "waitportstatus: GetPortValue failed");
			break;
		}

		// check for desired result
		int flagstate= (((char)data) & bits)!=0,
			wantedstate= onoff!=0;

		if (flagstate==wantedstate) {
			ok= true;
			break;
		}

		// try again after a moment
		::Sleep(tick);
	} 

	if (!ok && strlen(lasterrorstring)==0) {
		sprintf(lasterrorstring, "waitportstatus: timeout waiting for port %02x to change to status %02x/%d", port, bits, onoff);
	}

	if (!ok) {
		sprintf(logbuf+strlen(logbuf), "waitportstatus returns failed: ok= %d\n", ok);
	}


	return ok;
}


//-------------------------------------------------------------------------
//  write a character to an io port through WinIO device
//-------------------------------------------------------------------------
int 
writeport(int port, char data)
{
	strcpy(lasterrorstring, "");

	// write byte via WINIO.SYS
	int done= SetPortVal(port, data, 1);
	if (!done) {
		sprintf(lasterrorstring, "writeport: port %02x, data %02x failed\n", port, data);
	}

	if (verbosity>1) {
		sprintf(logbuf+strlen(logbuf), "writeport port= %02x, data= %02x, done= %d\n", port, data, done);
	}

	return done;
}


//-------------------------------------------------------------------------
//  read a character from an io port through WinIO device
//-------------------------------------------------------------------------
int 
readport(int port, char *pdata)
{
	DWORD data= -1;

	// read byte via WINIO.SYS
	int done= GetPortVal(port, &data, 1);
	if (done) {
		*pdata= (char)data;
	}
	else {
		sprintf(lasterrorstring, "readport: port %02x failed\n", port, data);
	}

	if (verbosity>1)
		sprintf(logbuf+strlen(logbuf), "readport port= %02x, data= %02x, done= %d\n", port, (char)data, done);

	return done;
}


//-------------------------------------------------------------------------
//  read a byte from the embedded controller (EC) via port io 
//-------------------------------------------------------------------------
int 
FANCONTROL::ReadByteFromEC(int offset, char *pdata)
{
	int ok;

	// wait for IBF and OBF to clear
	ok= waitportstatus(EC_STAT_IBF | EC_STAT_OBF, false);
	if (ok) {

		// tell 'em we want to "READ"
		ok= writeport(EC_CTRLPORT, EC_CTRLPORT_READ);
		if (ok) {

			// wait for IBF to clear (command byte removed from EC's input queue)
			ok= waitportstatus(EC_STAT_IBF, false);
			if (ok) {

				// tell 'em where we want to read from
				ok= writeport(EC_DATAPORT, offset);
				if (ok) {

					// wait for IBF to clear (address byte removed from EC's input queue)
					// Note: Techically we should waitportstatus(OBF,TRUE) here,(a byte being 
					//       in the EC's output buffer being ready to read).  For some reason
					//       this never seems to happen
					ok= waitportstatus(EC_STAT_IBF, false);
					if (ok) {
						char data= -1;

						// read result (EC byte at offset)
						ok= readport(EC_DATAPORT, &data);
						if (ok)
							*pdata= data;
					}
				}
			}
		}
	}

	if (verbosity>0)
		sprintf(logbuf+strlen(logbuf), "readec: offset= %x, data= %02x, ok= %d\n", offset, *pdata, ok);

	return ok;
}



//-------------------------------------------------------------------------
//  write a byte to the embedded controller (EC) via port io
//-------------------------------------------------------------------------
int 
FANCONTROL::WriteByteToEC(int offset, char data)
{
	int ok;

	// wait for IBF and OBF to clear
	ok= waitportstatus(EC_STAT_IBF| EC_STAT_OBF, false);
	if (ok) {

		// tell 'em we want to "WRITE"
		ok= writeport(EC_CTRLPORT, EC_CTRLPORT_WRITE);
		if (ok) {

			// wait for IBF to clear (command byte removed from EC's input queue)
			ok= waitportstatus(EC_STAT_IBF, false);
			if (ok) {

				// tell 'em where we want to write to
				ok= writeport(EC_DATAPORT, offset);
				if (ok) {

					// wait for IBF to clear (address byte removed from EC's input queue)
					ok= waitportstatus(EC_STAT_IBF, false);
					if (ok) {
						// tell 'em what we want to write there
						ok= writeport(EC_DATAPORT, data);
						if (ok) {					
							// wait for IBF to clear (data byte removed from EC's input queue)
							ok= waitportstatus(EC_STAT_IBF, false);
						}
					}
				}
			}
		}
	}

	if (verbosity>2)
		sprintf(logbuf+strlen(logbuf), "writeec: offset= %x, data= %02x, ok= %d\n", offset, data, ok);

	return ok;
}




//-------------------------------------------------------------------------
//  experimental code
//-------------------------------------------------------------------------
void
FANCONTROL::Test(void)
{

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
