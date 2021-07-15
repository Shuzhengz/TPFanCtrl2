
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
const size_t logbuf_n = 8192;
char lasterrorstring[256]= "",
	 logbuf[logbuf_n]= "";


