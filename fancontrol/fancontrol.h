
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

#ifndef FANCONTROL_H
#define FANCONTROL_H

#include "_prec.h"
#pragma once


#include "winstuff.h"
#include "TaskbarTextIcon.h"



#define FANCONTROLVERSION "0.63 multiHotKey"

#define WM__DISMISSDLG WM_USER+5
#define WM__GETDATA WM_USER+6
#define WM__NEWDATA WM_USER+7
#define WM__TASKBAR WM_USER+8

#define setzero(adr, size) memset((void*)(adr), (char)0x00, (size))
#define ARRAYMAX(tab) (sizeof(tab)/sizeof((tab)[0]))
#define NULLSTRUCT	{ 0, }

//begin named pipe TPFanControl01
#define g_szPipeName "\\\\.\\Pipe\\TPFanControl01"  //Name given to the pipe
//Pipe name format - \\.\pipe\pipename

#define BUFFER_SIZE 1024 //1k
#define ACK_MESG_RECV "Message received successfully"
//end named pipe TPFanControl01

class FANCONTROL 
{
	protected:
		HINSTANCE hinstapp;
        HINSTANCE m_hinstapp;
		HWND hwndDialog;
        
		UINT_PTR m_fanTimer;
		UINT_PTR m_titleTimer;
		UINT_PTR m_iconTimer;
		UINT_PTR m_renewTimer;

        struct FCSTATE {

			char FanCtrl,
				 FanSpeedLo,
				 FanSpeedHi;

			char Sensors[12];
			int  SensorAddr[12];
			const char *SensorName[12];


		} State;

		struct SMARTENTRY {
				int temp, fan;
		} SmartLevels[32];

		struct SMARTENTRY1 {
				int temp1, fan1;
		} SmartLevels1[32];

		struct SMARTENTRY2 {
				int temp2, fan2;
		} SmartLevels2[32];

		struct FSMARTENTRY {		//fahrenheit values
				int ftemp, ffan;
		} FSmartLevels[32];


		int IconLevels[3];	// temp levels for coloring the icon
		int FIconLevels[3];	// fahrenheit temp levels for coloring the icon
		int CurrentIcon;
		int IndSmartLevel;
		int SensorOffset[16];
		int FSensorOffset[16];
		int iFarbeIconB;
		int iFontIconB;
		int icontemp;
		int Cycle; 
		int IconCycle; 
		int ReIcCycle; 
		int NoExtSensor;
		int FanSpeedLowByte;
		int ActiveMode,
			UseTWR,
			ManFanSpeed,
			FinalSeen;
		int CurrentMode, fanctrl2,
			PreviousMode;
		int TaskbarNew;
		int MaxTemp;
		int iMaxTemp;
		int fanspeed, lastfanspeed, showfanspeed;
		int FanBeepFreq, FanBeepDura;
		int MinimizeToSysTray,
			Lev64Norm,
			IconColorFan,
			Fahrenheit,
			MinimizeOnClose,
			StartMinimized,
			NoWaitMessage,
            Runs_as_service;
		int ReadErrorCount;
		int MaxReadErrors;
		int SecWinUptime;
		int SlimDialog;
		int NoBallons,
			HK_BIOS_Method,
			HK_Manual_Method,
			HK_Smart_Method,
			HK_SM1_Method,
			HK_SM2_Method,
			HK_TG_BS_Method,
			HK_TG_BM_Method,
			HK_TG_MS_Method,
			HK_TG_12_Method,
			HK_BIOS,
			HK_Manual,
			HK_Smart,
			HK_SM1,
			HK_SM2,
			HK_TG_BS,
			HK_TG_BM,
			HK_TG_MS,
			HK_TG_12;
		int BluetoothEDR;
		int ManModeExit;
		int ManModeExit2;
		int ShowBiasedTemps;
		int SecStartDelay;
		char gSensorNames[17][4];
		int Log2File;
		int Log2csv;
		int StayOnTop;
		int ShowAll;
		int ShowTempIcon;
		char IgnoreSensors[256];
		char MenuLabelSM1[32];
		char MenuLabelSM2[32];
		HANDLE hThread;
		HANDLE hPipe0;
		HANDLE hPipe1;
		HANDLE hPipe2;
		HANDLE hPipe3;
		HANDLE hPipe4;
		HANDLE hPipe5;
		HANDLE hPipe6;
		HANDLE hPipe7;
		HANDLE hLock;
		HANDLE hLockS;
		BOOL Closing;
		MUTEXSEM EcAccess;
		bool m_needClose;

		char Title[128];
		char Title2[128];
		char Title3[128];
		char Title4[128];
		char Title5[128];
		char LastTitle[128];
		char LastTooltip[128];
		char CurrentStatus[256];
		char CurrentStatuscsv[256];

		// dialog.cpp
		int CurrentModeFromDialog();
		int ShowAllFromDialog();
		void ModeToDialog(int mode);
		void ShowAllToDialog(int mode);
		ULONG DlgProc(HWND hwnd, ULONG msg, WPARAM mp1, LPARAM mp2);
		static ULONG CALLBACK BaseDlgProc(HWND hwnd, ULONG msg, WPARAM mp1, LPARAM mp2);

        //The default app-icon with changing colors
		TASKBARICON *pTaskbarIcon;
        //
        CTaskbarTextIcon **ppTbTextIcon;
        MUTEXSEM *pTextIconMutex;



		static int _stdcall FANCONTROL_Thread(ULONG parm) \
                        { return ((FANCONTROL *)parm)->WorkThread(); }

		int WorkThread();


		// fancontrol.cpp
		int ReadEcStatus(FCSTATE *pfcstate);
		int ReadEcRaw(FCSTATE *pfcstate);
		int HandleData();
		int SmartControl();
		int SetFan(const char *source, int level, BOOL final= false);
		int SetHdw(const char *source, int hdwctrl, int HdwOffset, int AnyWayBit);


		// misc.cpp
		int ReadConfig(const char *filename);
		void Trace(const char *text);
		void Tracecsv(const char *textcsv);
		void Tracecsvod(const char *textcsv);
		BOOL IsMinimized(void);
		void CurrentDateTimeLocalized(char *result, size_t sizeof_result);
		void CurrentTimeLocalized(char *result, size_t sizeof_result);
		HANDLE CreateThread(int (_stdcall *fnct)(ULONG), ULONG p);


		// portio.cpp
		int ReadByteFromECint(int offset, char *pdata);
		int ReadByteFromEC(int offset, char *pdata);
		int WriteByteToEC(int offset, char data);

	public:

		FANCONTROL(HINSTANCE hinstapp);
		~FANCONTROL();

		void Test(void);

		int ProcessDialog();

        HWND GetDialogWnd() { return hwndDialog; }
		HANDLE GetWorkThread() { return hThread; }
        // The texticons will be shown depending on variables
        void ProcessTextIcons(void);
        void RemoveTextIcons(void);
};

#endif // FANCONTROL_H