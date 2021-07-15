// ========================================================================
// =================    TVicPort  DLL interface        ====================
// ==========                 Version  4.0                      ===========
// ==========     Copyright (c) 1997-2005, EnTech Taiwan        ===========
// ========================================================================
// ==========           http://www.entechtaiwan.com             ===========
// ==========          mailto: tools@entechtaiwan.com           ===========
// ========================================================================


#ifndef __TVicPort_H
#define __TVicPort_H

#define VICFN  __stdcall

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(1)

typedef struct _HDDInfo {
	ULONG	BufferSize;
	ULONG	DoubleTransfer;
	ULONG	ControllerType;
	ULONG	ECCMode;
	ULONG	SectorsPerInterrupt;
	ULONG	Cylinders;
	ULONG	Heads;
	ULONG	SectorsPerTrack;
	char	Model[41];
	char	SerialNumber[21];
	char	Revision[9];
} HDDInfo, *pHDDInfo;



void	VICFN CloseTVicPort();
BOOL	VICFN OpenTVicPort();
BOOL	VICFN IsDriverOpened();

BOOL	VICFN TestHardAccess();
void	VICFN SetHardAccess(BOOL bNewValue);

UCHAR	VICFN ReadPort  (USHORT PortAddr);
void	VICFN WritePort (USHORT PortAddr, UCHAR nNewValue);
USHORT	VICFN ReadPortW (USHORT PortAddr);
void	VICFN WritePortW(USHORT PortAddr, USHORT nNewValue);
ULONG	VICFN ReadPortL (USHORT PortAddr);
void	VICFN WritePortL(USHORT PortAddr, ULONG nNewValue);

void	VICFN ReadPortFIFO  (USHORT PortAddr, ULONG NumPorts, UCHAR  * Buffer);
void	VICFN WritePortFIFO (USHORT PortAddr, ULONG NumPorts, UCHAR  * Buffer);
void	VICFN ReadPortWFIFO (USHORT PortAddr, ULONG NumPorts, USHORT * Buffer);
void	VICFN WritePortWFIFO(USHORT PortAddr, ULONG NumPorts, USHORT * Buffer);
void	VICFN ReadPortLFIFO (USHORT PortAddr, ULONG NumPorts, ULONG  * Buffer);
void	VICFN WritePortLFIFO(USHORT PortAddr, ULONG NumPorts, ULONG  * Buffer);

USHORT	VICFN GetLPTNumber();
void	VICFN SetLPTNumber(USHORT nNewValue);
USHORT	VICFN GetLPTNumPorts();
USHORT	VICFN GetLPTBasePort();
UCHAR	VICFN AddNewLPT(USHORT PortBaseAddress);

BOOL	VICFN GetPin(USHORT nPin);
void	VICFN SetPin(USHORT nPin, BOOL bNewValue);

BOOL	VICFN GetLPTAckwl();
BOOL	VICFN GetLPTBusy();
BOOL	VICFN GetLPTPaperEnd();
BOOL	VICFN GetLPTSlct();
BOOL	VICFN GetLPTError();

void	VICFN LPTInit();
void	VICFN LPTSlctIn();
void	VICFN LPTStrobe();
void	VICFN LPTAutofd(BOOL Flag);

void	VICFN GetHDDInfo  (UCHAR IdeNumber, UCHAR Master, pHDDInfo Info);

ULONG	VICFN MapPhysToLinear(ULONG PhAddr, ULONG PhSize);
void	VICFN UnmapMemory    (ULONG PhAddr, ULONG PhSize);

UCHAR	VICFN GetMem (ULONG MappedAddr, ULONG Offset);
void	VICFN SetMem (ULONG MappedAddr, ULONG Offset, UCHAR nNewValue);
USHORT	VICFN GetMemW(ULONG MappedAddr, ULONG Offset);
void	VICFN SetMemW(ULONG MappedAddr, ULONG Offset, USHORT nNewValue);
ULONG	VICFN GetMemL(ULONG MappedAddr, ULONG Offset);
void	VICFN SetMemL(ULONG MappedAddr, ULONG Offset, ULONG nNewValue);

void	VICFN SetLPTReadMode();
void	VICFN SetLPTWriteMode();

void	VICFN LaunchWeb();
void	VICFN LaunchMail();

int VICFN EvaluationDaysLeft();

#pragma pack()

#ifdef __cplusplus
} // extern "C"
#endif

#endif
