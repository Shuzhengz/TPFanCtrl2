
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

	//
	// wait until input on control port has desired state or times out
	//
	for (time= 0; time<timeout; time+= tick) {

		DWORD data=0;
		data = ReadPort(port);

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

	return ok;
}


//-------------------------------------------------------------------------
//  write a character to an io port through WinIO device
//-------------------------------------------------------------------------
int 
writeport(int port, char data)
{
		WritePort(port, data);

	return 1;
}


//-------------------------------------------------------------------------
//  read a character from an io port through WinIO device
//-------------------------------------------------------------------------
int 
readport(int port, char *pdata)
{
	DWORD data= -1;
	data = ReadPort(port);
	*pdata= (char)data;
	return 1;
}


//-------------------------------------------------------------------------
//  read a byte from the embedded controller (EC) via port io 
//-------------------------------------------------------------------------
int 
FANCONTROL::ReadByteFromECint(int offset, char *pdata)
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

	return ok;
}

int 
FANCONTROL::ReadByteFromEC(int offset, char *pdata)
{
	int ok = 1;
	while (ok)
	{
		char b;
		ok = ReadByteFromECint(offset, &b);
		if (ok)
		{
			Sleep(10);
			char b2;
			ok = ReadByteFromECint(offset, &b2);
			if (ok && b == b2)
			{
				*pdata = b;
				return ok;
			}
		}
		::Sleep(50);
	}
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

	return ok;
}
