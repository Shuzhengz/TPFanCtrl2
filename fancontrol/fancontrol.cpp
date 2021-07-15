
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
#include "tools.h"
#include "taskbartexticon.h"






//-------------------------------------------------------------------------
//  constructor
//-------------------------------------------------------------------------
FANCONTROL::FANCONTROL(HINSTANCE hinstapp)
	: hinstapp(NULL),
		hwndDialog(NULL),
		CurrentMode(-1),
		PreviousMode(-1),
		Cycle(5),
		IconCycle(1),
		ReIcCycle(0),
		NoExtSensor(0),
		FanSpeedLowByte(0x84),
		CurrentIcon(-1),
		hThread(NULL),
		FanBeepFreq(440),
		FanBeepDura(50),
		ReadErrorCount(0),
		MaxReadErrors(10),
		NoBallons(0),
		HK_BIOS_Method(0),
		HK_BIOS(0),
		HK_Manual_Method(0),
		HK_Manual(0),
		HK_Smart_Method(0),
		HK_Smart(0),
		HK_SM1_Method(0),
		HK_SM1(0),
		HK_SM2_Method(0),
		HK_SM2(0),
		HK_TG_BS_Method(0),
		HK_TG_BS(0),
		HK_TG_BM_Method(0),
		HK_TG_BM(0),
		HK_TG_MS_Method(0),
		HK_TG_MS(0),
		HK_TG_12_Method(0),
		HK_TG_12(0),
		BluetoothEDR(0),
		ManModeExit(80),
		ManModeExit2(80),
		ShowBiasedTemps(0),
		SecWinUptime(0),
		SlimDialog(0),
		SecStartDelay(0),
		Log2File(0),
		StayOnTop(0),
		Log2csv(0),
		ShowAll(0),
		ShowTempIcon(1),
		pTaskbarIcon(NULL),
		Fahrenheit(FALSE),
		MinimizeToSysTray(TRUE),
		IconColorFan(FALSE),
		Lev64Norm(FALSE),
		StartMinimized(FALSE),
		NoWaitMessage(TRUE),
		MinimizeOnClose(TRUE),
        Runs_as_service(FALSE),
		ActiveMode(false),
		ManFanSpeed(7),
		UseTWR(0),
		FinalSeen(false),
		m_fanTimer(NULL),
		m_titleTimer(NULL),
		m_iconTimer(NULL),
		m_renewTimer(NULL),
		m_needClose(false),
        m_hinstapp(hinstapp),
        ppTbTextIcon(NULL),
        pTextIconMutex(new MUTEXSEM(0,"Global\\TPFanControl_ppTbTextIcon"))

{
	int i= 0;
	char buf[256]= "";

// SensorNames
		// 78-7F (state index 0-7)
			strcpy_s(this->gSensorNames[0],sizeof(this->gSensorNames[0]), "cpu"); // main processor
			strcpy_s(this->gSensorNames[1],sizeof(this->gSensorNames[1]), "aps"); // harddisk protection gyroscope
			strcpy_s(this->gSensorNames[2],sizeof(this->gSensorNames[2]), "crd"); // under PCMCIA slot (front left)
			strcpy_s(this->gSensorNames[3],sizeof(this->gSensorNames[3]), "gpu"); // graphical processor
			strcpy_s(this->gSensorNames[4],sizeof(this->gSensorNames[4]), "bat"); // inside T43 battery
			strcpy_s(this->gSensorNames[5],sizeof(this->gSensorNames[5]), "x7d"); // usually n/a
			strcpy_s(this->gSensorNames[6],sizeof(this->gSensorNames[6]), "bat"); // inside T43 battery
			strcpy_s(this->gSensorNames[7],sizeof(this->gSensorNames[7]), "x7f"); // usually n/a
//  	// C0-C4 (state index 8-11)
			strcpy_s(this->gSensorNames[8],sizeof(this->gSensorNames[8]), "bus"); // unknown
			strcpy_s(this->gSensorNames[9],sizeof(this->gSensorNames[9]), "pci"); // mini-pci, WLAN, southbridge area
			strcpy_s(this->gSensorNames[10],sizeof(this->gSensorNames[10]), "pwr"); // power supply (get's hot while charging battery)
			strcpy_s(this->gSensorNames[11],sizeof(this->gSensorNames[11]), "xc3"); // usually n/a
		// future
			strcpy_s(this->gSensorNames[12],sizeof(this->gSensorNames[12]), "");
			strcpy_s(this->gSensorNames[13],sizeof(this->gSensorNames[13]), "");
			strcpy_s(this->gSensorNames[14],sizeof(this->gSensorNames[14]), "");
			strcpy_s(this->gSensorNames[15],sizeof(this->gSensorNames[15]), "");
			strcpy_s(this->gSensorNames[16],sizeof(this->gSensorNames[16]), "");


	// clear title strings
	setzero(this->Title, sizeof(this->Title));
	setzero(this->Title2, sizeof(this->Title2));
	setzero(this->Title3, sizeof(this->Title3));
	setzero(this->Title4, sizeof(this->Title4));
	setzero(this->Title5, sizeof(this->Title5));
	setzero(this->LastTitle, sizeof(this->LastTitle));
	setzero(this->CurrentStatus, sizeof(this->CurrentStatus));
	setzero(this->CurrentStatuscsv, sizeof(this->CurrentStatuscsv));
	setzero(this->IgnoreSensors, sizeof(this->IgnoreSensors));

	this->IconLevels[0]= 50;	// yellow icon level
	this->IconLevels[1]= 55;	// orange icon level
	this->IconLevels[2]= 60;	// red icon level


	// initial default "smart" table
	setzero(this->SmartLevels, sizeof(this->SmartLevels));
	this->SmartLevels[i].temp= 50;  this->SmartLevels[i].fan= 0; i++;
	this->SmartLevels[i].temp= 55;  this->SmartLevels[i].fan= 3; i++;
	this->SmartLevels[i].temp= 60;  this->SmartLevels[i].fan= 5; i++;
	this->SmartLevels[i].temp= 65;  this->SmartLevels[i].fan= 7; i++;
	this->SmartLevels[i].temp= 70;  this->SmartLevels[i].fan= 128; i++;
	this->SmartLevels[i].temp= -1;  this->SmartLevels[i].fan= 0; i++;

	setzero(this->SmartLevels1, sizeof(this->SmartLevels1));
	i= 0;
	this->SmartLevels1[i].temp1= 50;  this->SmartLevels1[i].fan1= 0; i++;
	this->SmartLevels1[i].temp1= 55;  this->SmartLevels1[i].fan1= 3; i++;
	this->SmartLevels1[i].temp1= 60;  this->SmartLevels1[i].fan1= 5; i++;
	this->SmartLevels1[i].temp1= 65;  this->SmartLevels1[i].fan1= 7; i++;
	this->SmartLevels1[i].temp1= 70;  this->SmartLevels1[i].fan1= 128; i++;
	this->SmartLevels1[i].temp1= -1;  this->SmartLevels1[i].fan1= 0; i++;

	setzero(this->SmartLevels2, sizeof(this->SmartLevels2));
	i= 0;
	this->SmartLevels2[i].temp2= 0;  this->SmartLevels2[i].fan2= 0; i++;
	// später if ( this->SmartLevels2[i].temp2 != 0 ) dann smart2
	this->SmartLevels2[i].temp2= 55;  this->SmartLevels2[i].fan2= 3; i++;
	this->SmartLevels2[i].temp2= 60;  this->SmartLevels2[i].fan2= 5; i++;
	this->SmartLevels2[i].temp2= 65;  this->SmartLevels2[i].fan2= 7; i++;
	this->SmartLevels2[i].temp2= 70;  this->SmartLevels2[i].fan2= 128; i++;
	this->SmartLevels2[i].temp2= -1;  this->SmartLevels2[i].fan2= 0; i++;


	// code title3
	char bias = 100;
	for (int _i = 0; _i < 111; _i++) {
		switch (_i) {
		case 0 : this->Title3[0]= 32; break;			//blank
		case 1 : this->Title3[1]=bias -  2; break;		//b
		case 2 : this->Title3[2]=bias + 21; break;		//y
		case 3 : this->Title3[3]=32 ; break;			//blank
		case 4 : this->Title3[4]=bias + 16; break;		//t
		case 5 : this->Title3[5]=bias + 14; break;		//r
		case 6 : this->Title3[6]=bias + 11; break;		//o
		case 7 : this->Title3[7]=bias + 17; break;		//u
		case 8 : this->Title3[8]=bias - 2; break;		//b
		case 9 : this->Title3[9]=bias - 3; break;		//a
		case 10 : this->Title3[10]=bias; break;			//d
		case 11 : this->Title3[11]=bias + 5; break;		//i
		case 12 : this->Title3[12]=bias + 20; break;	//x
		case 13 : this->Title3[13]=32; break;			//blank
/*		case 14 : this->Title3[14]=32; break;			//blank
		case 15 : this->Title3[15]=32; break;			//blank
		case 16 : this->Title3[16]=32; break;			//blank
		case 17 : this->Title3[17]=32; break;			//blank
		case 18 : this->Title3[18]=32; break;			//blank
		case 19 : this->Title3[19]=bias - 58; break;	//*
		case 20 : this->Title3[20]=bias - 58; break;	//*
		case 21 : this->Title3[21]=bias + 2; break;		//f		
		case 22 : this->Title3[22]=bias + 14; break;	//r
		case 23 : this->Title3[23]=bias + 1; break;		//e
		case 24 : this->Title3[24]=bias + 1; break;		//e
		case 25 : this->Title3[25]=bias - 68 ; break;	//blank
		case 26 : this->Title3[26]=bias + 2; break;		//f		
		case 27 : this->Title3[27]=bias + 11; break;	//o
		case 28 : this->Title3[28]=bias + 14; break;	//r
		case 29 : this->Title3[29]=bias - 68 ; break;	//blank
		case 30 : this->Title3[30]=bias + 12; break;	//p
		case 31 : this->Title3[31]=bias + 14; break;	//r
		case 32 : this->Title3[32]=bias + 5; break;		//i
		case 33 : this->Title3[33]=bias + 18; break;	//v
		case 34 : this->Title3[34]=bias - 3 ; break;	//a
		case 35 : this->Title3[35]=bias + 16; break;	//t
		case 36 : this->Title3[36]=bias + 1; break;		//e
		case 37 : this->Title3[37]=bias - 68 ; break;	//blank
		case 38 : this->Title3[38]=bias + 17; break;	//u
		case 39 : this->Title3[39]=bias + 15; break;	//s
		case 40 : this->Title3[40]=bias + 1; break;		//e
		case 41 : this->Title3[41]=bias - 58; break;	//*
		case 42 : this->Title3[42]=bias - 58; break;	//*
	//	case 14 : this->Title3[14]=115-1; break;
	//	case 15 : this->Title3[15]=115-1; break;
*/
		}

	// code Title4 http://www.staff.uni-marburg.de/~schmitzr/donate.html

	for (int __i=0; __i < 111; __i++) {
		switch (__i) {
		case 0 : this->Title4[0]=bias + 4; break;		//h
		case 1 : this->Title4[1]=bias + 16; break;		//t
		case 2 : this->Title4[2]=bias + 16; break;		//t
		case 3 : this->Title4[3]=bias + 12; break;		//p
		case 4 : this->Title4[4]=bias - 42; break;		//:
		case 5 : this->Title4[5]=bias - 8; break;		///
		case 6 : this->Title4[6]=bias - 8; break;		///
		case 7 : this->Title4[7]=bias + 19; break;		//w
		case 8 : this->Title4[8]=bias + 19; break;		//w
		case 9 : this->Title4[9]=bias + 19; break;		//w
		case 10 : this->Title4[10]=bias - 54; break;	//.
		case 11 : this->Title4[11]=bias + 15; break;	//s
		case 12 : this->Title4[12]=bias + 16; break;	//t
		case 13 : this->Title4[13]=bias - 3 ; break;	//a
		case 14 : this->Title4[14]=bias + 2; break;		//f
		case 15 : this->Title4[15]=bias + 2; break;		//f
		case 16 : this->Title4[16]=bias - 54; break;	//.
		case 17 : this->Title4[17]=bias + 17; break;	//u
		case 18 : this->Title4[18]=bias + 10; break;	//n
		case 19 : this->Title4[19]=bias + 5; break;		//i
		case 20 : this->Title4[20]=bias - 55; break;	//-
		case 21 : this->Title4[21]=bias + 9; break;		//m
		case 22 : this->Title4[22]=bias - 3 ; break;	//a
		case 23 : this->Title4[23]=bias + 14; break;	//r
		case 24 : this->Title4[24]=bias - 2; break;		//b
		case 25 : this->Title4[25]=bias + 17; break;	//u
		case 26 : this->Title4[26]=bias + 14; break;	//r	
		case 27 : this->Title4[27]=bias + 3; break;		//g
		case 28 : this->Title4[28]=bias - 54; break;	//.
		case 29 : this->Title4[29]=bias  ; break;		//d
		case 30 : this->Title4[30]=bias + 1; break;		//e
		case 31 : this->Title4[31]=bias - 8; break;		///
		case 32 : this->Title4[32]=bias + 26; break;	//~
		case 33 : this->Title4[33]=bias + 15; break;	//s
		case 34 : this->Title4[34]=bias - 1 ; break;	//c
		case 35 : this->Title4[35]=bias + 4; break;		//h
		case 36 : this->Title4[36]=bias + 9; break;		//m
		case 37 : this->Title4[37]=bias + 5; break;		//i
		case 38 : this->Title4[38]=bias + 16; break;	//t
		case 39 : this->Title4[39]=bias + 22; break;	//z
		case 40 : this->Title4[40]=bias + 14; break;	//r
		case 41 : this->Title4[41]=bias - 8; break;		///
		case 42 : this->Title4[42]=bias ; break;		//d
		case 43 : this->Title4[43]=bias + 11; break;	//o
		case 44 : this->Title4[44]=bias + 10; break;	//n
		case 45 : this->Title4[45]=bias - 3; break;		//a
		case 46 : this->Title4[46]=bias + 16; break;	//t
		case 47 : this->Title4[47]=bias + 1; break;		//e
		case 48 : this->Title4[48]=bias - 54; break;	//.
		case 49 : this->Title4[49]=bias + 4; break;		//h
		case 50 : this->Title4[50]=bias + 16; break;	//t
		case 51 : this->Title4[51]=bias + 9; break;		//m
		case 52 : this->Title4[52]=bias + 8; break;		//l
		}
	}

	// code Title5 http://tpfancontrol.com

	for (int __i=0; __i < 23; __i++) {
		switch (__i) {
		case 0 : this->Title5[0]=bias + 4; break;		//h
		case 1 : this->Title5[1]=bias + 16; break;		//t
		case 2 : this->Title5[2]=bias + 16; break;		//t
		case 3 : this->Title5[3]=bias + 12; break;		//p
		case 4 : this->Title5[4]=bias - 42; break;		//:
		case 5 : this->Title5[5]=bias - 8; break;		///
		case 6 : this->Title5[6]=bias - 8; break;		///
		case 7 : this->Title5[7]=bias + 16; break;		//t
		case 8 : this->Title5[8]=bias + 12; break;		//p
		case 9 : this->Title5[9]=bias + 2; break;		//f
		case 10 : this->Title5[10]=bias - 3 ; break;	//a
		case 11 : this->Title5[11]=bias + 10; break;	//n
		case 12 : this->Title5[12]=bias - 1 ; break;	//c
		case 13 : this->Title5[13]=bias + 11; break;	//o
		case 14 : this->Title5[14]=bias + 10; break;	//n
		case 15 : this->Title5[15]=bias + 16; break;	//t
		case 16 : this->Title5[16]=bias + 14; break;	//r	
		case 17 : this->Title5[17]=bias + 11; break;	//o
		case 18 : this->Title5[18]=bias + 8; break;		//l
		case 19 : this->Title5[19]=bias - 54; break;	//.
		case 20 : this->Title5[20]=bias - 1 ; break;	//c
		case 21 : this->Title5[21]=bias + 11; break;	//o
		case 22 : this->Title5[22]=bias + 9; break;		//m
		}
	}


	}



	// read config file
		this->ReadConfig("TPFanControl.ini");


	if (this->hwndDialog){
		::GetWindowText(this->hwndDialog, this->Title, sizeof(this->Title));
		strcat_s(this->Title,sizeof(this->Title), " V" FANCONTROLVERSION);
		strcat_s(this->Title,sizeof(this->Title), this->Title3);
		::SetWindowText(this->hwndDialog, this->Title);

		::SetWindowLong(this->hwndDialog, GWL_USERDATA, (ULONG)this);
		::SendDlgItemMessage(this->hwndDialog, 8112, EM_LIMITTEXT, 256, 0);
		::SendDlgItemMessage(this->hwndDialog, 9200, EM_LIMITTEXT, 4096, 0);
		::SetDlgItemText(this->hwndDialog, 8310, _itoa(this->ManFanSpeed, buf ,10));
	}


	
		if(SlimDialog == 1) {



		if (this->StayOnTop)

	this->hwndDialog= ::CreateDialogParam(hinstapp, 
										MAKEINTRESOURCE(9001), 
										HWND_DESKTOP, 
										(DLGPROC)BaseDlgProc, 
										(LPARAM)this);

		else
	this->hwndDialog= ::CreateDialogParam(hinstapp, 
										MAKEINTRESOURCE(9003), 
										HWND_DESKTOP, 
										(DLGPROC)BaseDlgProc, 
										(LPARAM)this);

	if (this->hwndDialog) {
		::GetWindowText(this->hwndDialog, this->Title, sizeof(this->Title));
		strcat_s(this->Title,sizeof(this->Title), ".63 multiHotKey");
		if (SlimDialog == 0) strcat_s(this->Title,sizeof(this->Title), this->Title3);
		::SetWindowText(this->hwndDialog, this->Title);

		::SetWindowLong(this->hwndDialog, GWL_USERDATA, (ULONG)this);
		::SendDlgItemMessage(this->hwndDialog, 8112, EM_LIMITTEXT, 256, 0);
		::SendDlgItemMessage(this->hwndDialog, 9200, EM_LIMITTEXT, 4096, 0);
		::SetDlgItemText(this->hwndDialog, 8310, _itoa(this->ManFanSpeed, buf, 10));
			this->ShowAllToDialog(ShowAll);

	}
		}



	//  wait xx seconds to start tpfc while booting to save icon
		char bufsec[1024]= "";
		int tickCount = GetTickCount(); // +262144;

		sprintf_s(bufsec, sizeof(bufsec), "Windows uptime since boot %d sec., SecWinUptime= %d sec.", tickCount/1000, SecWinUptime);
		this->Trace(bufsec);

		if ((tickCount/1000) <= SecWinUptime){
				sprintf_s(bufsec,sizeof(bufsec), "Save the icon by a start delay of %d seconds (SecStartDelay)", SecStartDelay);
				this->Trace(bufsec);
				if (!NoWaitMessage) {
					sprintf_s(bufsec, sizeof(bufsec), 
						"TPFanControl is started %d sec. after\nboot time (SecWinUptime=%d sec.)\n\nTo prevent missing systray icons\nand communication errors between\nTPFanControl and embedded controller\nit will sleep for %d sec. (SecStartDelay)\n\nTo void this message box please set\nNoWaitMessage=1 in TPFanControl.ini"
						, tickCount/1000, SecWinUptime, SecStartDelay);
					// Don't show message box when as service in Vista
					OSVERSIONINFO os = { sizeof(os) };
					GetVersionEx(&os);
					if (os.dwMajorVersion >= 6 && Runs_as_service==TRUE);
					else 
						MessageBox(NULL, bufsec, "TPFanControl is sleeping", MB_ICONEXCLAMATION);
				}
		}
		
		// sleep until start time + delay time
		if ((GetTickCount()/1000) <= (DWORD)SecWinUptime) { 
			while ((DWORD) (tickCount + SecStartDelay*1000) >= GetTickCount())
				Sleep(200);
		}


	// taskbaricon (keep code after reading config)

		if (this->MinimizeToSysTray) {

   // da liegt der ICON-Hund begraben

		if (!this->ShowTempIcon) {
	    this->pTaskbarIcon= new	TASKBARICON(this->hwndDialog, 10, "TPFanControl");
		}
		else {
		this->pTaskbarIcon= NULL;
		}

		}

		// read current fan control status and set mode buttons accordingly 

		this->CurrentMode=this->ActiveMode;

		this->ModeToDialog(this->CurrentMode);
		this->PreviousMode= 1;

		if (HK_BIOS_Method) RegisterHotKey(this->hwndDialog,1,HK_BIOS_Method,HK_BIOS);
		if (HK_Smart_Method) RegisterHotKey(this->hwndDialog,2,HK_Smart_Method,HK_Smart); 
		if (HK_Manual_Method) RegisterHotKey(this->hwndDialog,3,HK_Manual_Method,HK_Manual);
		if (HK_SM1_Method) RegisterHotKey(this->hwndDialog,4,HK_SM1_Method,HK_SM1); 
		if (HK_SM2_Method) RegisterHotKey(this->hwndDialog,5,HK_SM2_Method,HK_SM2); 
		if (HK_TG_BS_Method) RegisterHotKey(this->hwndDialog,6,HK_TG_BS_Method,HK_TG_BS); 
		if (HK_TG_BM_Method) RegisterHotKey(this->hwndDialog,7,HK_TG_BM_Method,HK_TG_BM); 
		if (HK_TG_MS_Method) RegisterHotKey(this->hwndDialog,8,HK_TG_MS_Method,HK_TG_MS); 
		if (HK_TG_12_Method) RegisterHotKey(this->hwndDialog,9,HK_TG_12_Method,HK_TG_12); 
		


		// enable/disable mode radiobuttons
		::EnableWindow(::GetDlgItem(this->hwndDialog, 8300), this->ActiveMode);
		::EnableWindow(::GetDlgItem(this->hwndDialog, 8301), this->ActiveMode);
		::EnableWindow(::GetDlgItem(this->hwndDialog, 8302), this->ActiveMode);
		::EnableWindow(::GetDlgItem(this->hwndDialog, 8310), this->ActiveMode);

		// make it call HandleControl initially
		::PostMessage(this->hwndDialog, WM__GETDATA, 0, 0);


		m_fanTimer = ::SetTimer(this->hwndDialog, 1, this->Cycle*1000, NULL);	// fan update
		m_titleTimer = ::SetTimer(this->hwndDialog, 2, 500, NULL);				// title update
		m_iconTimer = ::SetTimer(this->hwndDialog, 3, this->IconCycle*1000, NULL);				// Vista icon update
		if(this->ReIcCycle) m_renewTimer = ::SetTimer(this->hwndDialog, 4, this->ReIcCycle*1000, NULL);				// Vista icon update

		if (!this->StartMinimized)
			::ShowWindow(this->hwndDialog, TRUE);

		if (this->StartMinimized)
			::ShowWindow(this->hwndDialog, SW_MINIMIZE);
	}




//-------------------------------------------------------------------------
//  destructor
//-------------------------------------------------------------------------
FANCONTROL::~FANCONTROL()
{
	if (this->hThread) {
		::WaitForSingleObject(this->hThread, 2000);
		this->hThread= NULL;
	}

	if (this->pTaskbarIcon) {
		delete this->pTaskbarIcon;
		this->pTaskbarIcon= NULL;
	}

	if (this->ppTbTextIcon) {
		delete ppTbTextIcon[0];
		delete[] ppTbTextIcon;
		ppTbTextIcon = NULL;
	}

	if (this->hwndDialog)
		::DestroyWindow(this->hwndDialog);

    if(pTextIconMutex)
        delete pTextIconMutex;
}


//-------------------------------------------------------------------------
//  mode integer from mode radio buttons
//-------------------------------------------------------------------------
int
FANCONTROL::CurrentModeFromDialog()
{
	BOOL modetpauto= ::SendDlgItemMessage(this->hwndDialog, 8300, BM_GETCHECK, 0L, 0L),
		 modefcauto= ::SendDlgItemMessage(this->hwndDialog, 8301, BM_GETCHECK, 0L, 0L),
		 modemanual= ::SendDlgItemMessage(this->hwndDialog, 8302, BM_GETCHECK, 0L, 0L);

	if (modetpauto) 
		this->CurrentMode= 1;
	else
	if (modefcauto) 
		this->CurrentMode= 2;
	else
	if (modemanual)
		this->CurrentMode= 3;
	else
		this->CurrentMode= -1;


	return this->CurrentMode;
}

int
FANCONTROL::ShowAllFromDialog()
{
	BOOL modefcauto= ::SendDlgItemMessage(this->hwndDialog, 7001, BM_GETCHECK, 0L, 0L),
		 modemanual= ::SendDlgItemMessage(this->hwndDialog, 7002, BM_GETCHECK, 0L, 0L);

	if (modefcauto) 
		this->ShowAll= 1;
	else
	if (modemanual)
		this->ShowAll= 0;
	else
		this->ShowAll= -1;


	return this->ShowAll;
}


void
FANCONTROL::ModeToDialog(int mode)
{
   ::SendDlgItemMessage(this->hwndDialog, 8300, BM_SETCHECK, mode==1, 0L);
   ::SendDlgItemMessage(this->hwndDialog, 8301, BM_SETCHECK, mode==2, 0L);
   ::SendDlgItemMessage(this->hwndDialog, 8302, BM_SETCHECK, mode==3, 0L);
}


void
FANCONTROL::ShowAllToDialog(int show)
{
   ::SendDlgItemMessage(this->hwndDialog, 7001, BM_SETCHECK, show==1, 0L);
   ::SendDlgItemMessage(this->hwndDialog, 7002, BM_SETCHECK, show==0, 0L);
}


//-------------------------------------------------------------------------
//  process main dialog
//-------------------------------------------------------------------------
int FANCONTROL::ProcessDialog()
{

	MSG qmsg, qmsg2;
	int dlgrc= -1;

	if (this->hwndDialog) {
		for (;;) {
			BOOL nodlgmsg= FALSE;

			::GetMessage(&qmsg, NULL, 0L, 0L);

			// control movements
			if (qmsg.message!=WM__DISMISSDLG && IsDialogMessage(this->hwndDialog, &qmsg)) {
				continue;
			}

			qmsg2= qmsg;
			TranslateMessage(&qmsg);
			DispatchMessage(&qmsg);

			if (qmsg2.message==WM__DISMISSDLG && qmsg2.hwnd==this->hwndDialog) {
				dlgrc= qmsg2.wParam;
				break;
			}
		}
	}

	return dlgrc;
}



//-------------------------------------------------------------------------
//  dialog window procedure (map to class method)
//-------------------------------------------------------------------------
ULONG CALLBACK
FANCONTROL::BaseDlgProc(HWND hwnd, ULONG msg, WPARAM mp1, LPARAM mp2)
{
	ULONG rc= FALSE;

	static UINT s_TaskbarCreated;

    if (msg == WM_INITDIALOG)
	{
		s_TaskbarCreated = RegisterWindowMessage("TaskbarCreated");
	}
	FANCONTROL *This= (FANCONTROL*)GetWindowLong(hwnd, GWL_USERDATA);

	if (This)
	{
		if (msg == s_TaskbarCreated)
		{	This->TaskbarNew=1;

			if (This->pTaskbarIcon)
			{
			This->pTaskbarIcon->RebuildIfNecessary(TRUE);
			}
			else {
				This->RemoveTextIcons();
				This->ProcessTextIcons();}
		}
		rc= This->DlgProc(hwnd, msg, mp1, mp2);
	}

	return rc;
}



//-------------------------------------------------------------------------
//  dialog window procedure as class method
//-------------------------------------------------------------------------
#define WANTED_MEM_SIZE 65536*12
BOOL dioicon (TRUE);
char szBuffer[BUFFER_SIZE];
char str_value[256];
DWORD cbBytes;
BOOL bResult (FALSE);
BOOL lbResult (FALSE);
int fanspeed;
int fanctrl;
int IconFontSize;
BOOL _piscreated(FALSE);
char obuftd[256]= "", obuftd2[128]="", templisttd[512];	
char obuf[256]= "", obuf2[128]="", templist2[512];	
ULONG 
FANCONTROL::DlgProc(HWND hwnd, ULONG msg, WPARAM mp1, LPARAM mp2)
{
	ULONG rc= 0, ok, res;
	char buf[1024];

//	HANDLE hLockS = CreateMutex(NULL,FALSE,"TPFanControlMutex01");

switch (msg) {

		case WM_HOTKEY:
			switch (mp1){

		case 1:
						this->ModeToDialog(1);
						::PostMessage(this->hwndDialog, WM__GETDATA, 0, 0);
				break;

		case 2:
							this->ModeToDialog(2);
							::PostMessage(this->hwndDialog, WM__GETDATA, 0, 0);
				break;

		case 3:
							this->ModeToDialog(3);
							::PostMessage(this->hwndDialog, WM__GETDATA, 0, 0);
				break;

		case 4:
									this->ModeToDialog(2);
									if (this->IndSmartLevel == 1) {
										sprintf_s(obuf, sizeof(obuf), "Activation of Fan Control Profile 'Smart Mode 1'");
									this->Trace(obuf);}
									this->IndSmartLevel = 0;

						for (int i= 0; i<32; i++) {
									this->SmartLevels[i].temp = this->SmartLevels1[i].temp1; 
									this->SmartLevels[i].fan = this->SmartLevels1[i].fan1;
						}
						::PostMessage(this->hwndDialog, WM__GETDATA, 0, 0);
				break;

		case 5:
									this->ModeToDialog(2);
									if (this->IndSmartLevel == 0) {
										sprintf_s(obuf, sizeof(obuf), "Activation of Fan Control Profile 'Smart Mode 2'");
									this->Trace(obuf);}
									this->IndSmartLevel = 1;

						for (int i= 0; i<32; i++) {
									this->SmartLevels[i].temp = this->SmartLevels2[i].temp2; 
									this->SmartLevels[i].fan = this->SmartLevels2[i].fan2;
						}
						::PostMessage(this->hwndDialog, WM__GETDATA, 0, 0);	   
				break;

		case 6:
			if (this->CurrentMode > 1) {
				this->ModeToDialog(1);
				::PostMessage(this->hwndDialog, WM__GETDATA, 0, 0);}
													
			if (this->CurrentMode == 1){
				this->ModeToDialog(2);
				::PostMessage(this->hwndDialog, WM__GETDATA, 0, 0);}
			break;

		case 7:
			if (this->CurrentMode > 1) {
				this->ModeToDialog(1);
				::PostMessage(this->hwndDialog, WM__GETDATA, 0, 0);}								
			if (this->CurrentMode == 1){
				this->ModeToDialog(3);
				::PostMessage(this->hwndDialog, WM__GETDATA, 0, 0);}
			break;

		case 8:
			if (this->CurrentMode < 3) {
				this->ModeToDialog(3);
				::PostMessage(this->hwndDialog, WM__GETDATA, 0, 0);}						
			if (this->CurrentMode == 3){
				this->ModeToDialog(2);
				::PostMessage(this->hwndDialog, WM__GETDATA, 0, 0);}
			break;

		case 9:
			this->ModeToDialog(2);
			switch (IndSmartLevel){
		case 0:
			sprintf_s(obuf, sizeof(obuf), "Activation of Fan Control Profile 'Smart Mode 2'");
			this->Trace(obuf);
			this->IndSmartLevel = 1;
			for (int i= 0; i<32; i++) {
				this->SmartLevels[i].temp = this->SmartLevels2[i].temp2; 
				this->SmartLevels[i].fan = this->SmartLevels2[i].fan2;
			}
			::PostMessage(this->hwndDialog, WM__GETDATA, 0, 0);
			break;
		case 1: 
			sprintf_s(obuf, sizeof(obuf), "Activation of Fan Control Profile 'Smart Mode 1'");
			this->Trace(obuf);
			this->IndSmartLevel = 0;
			for (int i= 0; i<32; i++) {
				this->SmartLevels[i].temp = this->SmartLevels1[i].temp1; 
				this->SmartLevels[i].fan = this->SmartLevels1[i].fan1;
			}
			::PostMessage(this->hwndDialog, WM__GETDATA, 0, 0);
			break;
			}
			break;
			}

		case WM_INITDIALOG:
				// placing code here will NOT work!
				// (put it into BaseDlgProc instead)
				break;

		case WM_TIMER:

				switch (mp1) 
				{

					case 1:		// update fan state	
						::PostMessage(this->hwndDialog, WM__GETDATA, 0, 0);
						if (this->Log2csv==1) 
						{
						this->Tracecsv(this->CurrentStatuscsv);
						}
						break;


					case 2:		// update window title

					// skip ManMode?	
						if (this->CurrentMode == 3 && this->MaxTemp > this->ManModeExit2){
							this->ModeToDialog(2);
							::PostMessage(this->hwndDialog, WM__GETDATA, 0, 0);}

						res= this->IsMinimized();
						if (res && strcmp(this->LastTitle, this->Title2)!=0) 
						{
							strcpy_s(this->LastTitle,sizeof(this->LastTitle), this->Title2);
						}
						else 
						if (!res && strcmp(this->LastTitle, this->Title)!=0) 
						{
							::SetWindowText(this->hwndDialog, this->Title);
							strcpy_s(this->LastTitle,sizeof(this->LastTitle), this->Title);
						}

						if (this->pTaskbarIcon) 
						{
							this->pTaskbarIcon->SetTooltip(this->Title2);
							strcpy_s(this->LastTooltip, sizeof(this->LastTooltip), this->Title2);
							int icon= -1;

							if (this->CurrentModeFromDialog()==1) 
							{
								icon= 10;	// gray
							}
							else 
							{
								icon= 11;	// blue
								for (int i= 0; i<ARRAYMAX(this->IconLevels); i++) 
								{
									if (this->MaxTemp>=this->IconLevels[i]) 
									{
										icon= 12+i;	// yellow, orange, red
									}
								}
							}


							if (icon!=this->CurrentIcon && icon!=-1) 
							{
								this->pTaskbarIcon->SetIcon(icon);
								this->CurrentIcon= icon;
								if (dioicon && !this->NoBallons) {
								this->pTaskbarIcon->SetBalloon(NIIF_INFO, "TPFanControl old symbol icon",
								"shows temperature level by color and state in tooltip, left click on icon shows or hides control window, right click shows menue",
								11); 
								dioicon = FALSE;}

							}
							this->iFarbeIconB = icon;
						}
					break;	
					case 3:		// update vista icon

//*************************************************************************************
//begin named pipe client session
//
			if (bResult == FALSE && lbResult == TRUE) 
			{
				_piscreated = FALSE;
				lbResult = FALSE;
				bResult = FALSE;
				CloseHandle(hPipe0);
				CloseHandle(hPipe1);
				CloseHandle(hPipe2);
				CloseHandle(hPipe3);
				CloseHandle(hPipe4);
				CloseHandle(hPipe5);
				CloseHandle(hPipe6);
				CloseHandle(hPipe7);
			}
			
			if (_piscreated == FALSE)
			{
				hPipe0 = CreateNamedPipe
					( 
					g_szPipeName,             // pipe name 
					PIPE_ACCESS_OUTBOUND,     // write access 
					PIPE_TYPE_MESSAGE |       // message type pipe 
					PIPE_READMODE_MESSAGE |   // message-read mode 
					PIPE_NOWAIT,              // blocking mode 
					PIPE_UNLIMITED_INSTANCES, // max. instances
					BUFFER_SIZE,              // output buffer size 
					BUFFER_SIZE,              // input buffer size 
					NMPWAIT_USE_DEFAULT_WAIT, // client time-out 
					NULL);                    // default security attribute 
				hPipe1 = CreateNamedPipe
					( 
					g_szPipeName,             // pipe name 
					PIPE_ACCESS_OUTBOUND,     // write access 
					PIPE_TYPE_MESSAGE |       // message type pipe 
					PIPE_READMODE_MESSAGE |   // message-read mode 
					PIPE_NOWAIT,              // blocking mode 
					PIPE_UNLIMITED_INSTANCES, // max. instances
					BUFFER_SIZE,              // output buffer size 
					BUFFER_SIZE,              // input buffer size 
					NMPWAIT_USE_DEFAULT_WAIT, // client time-out 
					NULL);                    // default security attribute 
				hPipe2 = CreateNamedPipe
					( 
					g_szPipeName,             // pipe name 
					PIPE_ACCESS_OUTBOUND,     // write access 
					PIPE_TYPE_MESSAGE |       // message type pipe 
					PIPE_READMODE_MESSAGE |   // message-read mode 
					PIPE_NOWAIT,              // blocking mode 
					PIPE_UNLIMITED_INSTANCES, // max. instances
					BUFFER_SIZE,              // output buffer size 
					BUFFER_SIZE,              // input buffer size 
					NMPWAIT_USE_DEFAULT_WAIT, // client time-out 
					NULL);                    // default security attribute 
				hPipe3 = CreateNamedPipe
					( 
					g_szPipeName,             // pipe name 
					PIPE_ACCESS_OUTBOUND,     // write access 
					PIPE_TYPE_MESSAGE |       // message type pipe 
					PIPE_READMODE_MESSAGE |   // message-read mode 
					PIPE_NOWAIT,              // blocking mode 
					PIPE_UNLIMITED_INSTANCES, // max. instances
					BUFFER_SIZE,              // output buffer size 
					BUFFER_SIZE,              // input buffer size 
					NMPWAIT_USE_DEFAULT_WAIT, // client time-out 
					NULL);                    // default security attribute 
				hPipe4 = CreateNamedPipe
					( 
					g_szPipeName,             // pipe name 
					PIPE_ACCESS_OUTBOUND,     // write access 
					PIPE_TYPE_MESSAGE |       // message type pipe 
					PIPE_READMODE_MESSAGE |   // message-read mode 
					PIPE_NOWAIT,              // blocking mode 
					PIPE_UNLIMITED_INSTANCES, // max. instances
					BUFFER_SIZE,              // output buffer size 
					BUFFER_SIZE,              // input buffer size 
					NMPWAIT_USE_DEFAULT_WAIT, // client time-out 
					NULL);                    // default security attribute 
				hPipe5 = CreateNamedPipe
					( 
					g_szPipeName,             // pipe name 
					PIPE_ACCESS_OUTBOUND,     // write access 
					PIPE_TYPE_MESSAGE |       // message type pipe 
					PIPE_READMODE_MESSAGE |   // message-read mode 
					PIPE_NOWAIT,              // blocking mode 
					PIPE_UNLIMITED_INSTANCES, // max. instances
					BUFFER_SIZE,              // output buffer size 
					BUFFER_SIZE,              // input buffer size 
					NMPWAIT_USE_DEFAULT_WAIT, // client time-out 
					NULL);                    // default security attribute 
				hPipe6 = CreateNamedPipe
					( 
					g_szPipeName,             // pipe name 
					PIPE_ACCESS_OUTBOUND,     // write access 
					PIPE_TYPE_MESSAGE |       // message type pipe 
					PIPE_READMODE_MESSAGE |   // message-read mode 
					PIPE_NOWAIT,              // blocking mode 
					PIPE_UNLIMITED_INSTANCES, // max. instances
					BUFFER_SIZE,              // output buffer size 
					BUFFER_SIZE,              // input buffer size 
					NMPWAIT_USE_DEFAULT_WAIT, // client time-out 
					NULL);                    // default security attribute 
				hPipe7 = CreateNamedPipe
					( 
					g_szPipeName,             // pipe name 
					PIPE_ACCESS_OUTBOUND,     // write access 
					PIPE_TYPE_MESSAGE |       // message type pipe 
					PIPE_READMODE_MESSAGE |   // message-read mode 
					PIPE_NOWAIT,              // blocking mode 
					PIPE_UNLIMITED_INSTANCES, // max. instances
					BUFFER_SIZE,              // output buffer size 
					BUFFER_SIZE,              // input buffer size 
					NMPWAIT_USE_DEFAULT_WAIT, // client time-out 
					NULL);                    // default security attribute 

			if (INVALID_HANDLE_VALUE == hPipe0){
				this->Trace("Creating Named Pipe client GUI was NOT successful.");
				::PostMessage(this->hwndDialog, WM_COMMAND, 5020, 0);}
			if (INVALID_HANDLE_VALUE == hPipe1){
				this->Trace("Creating Named Pipe client GUI was NOT successful.");
				::PostMessage(this->hwndDialog, WM_COMMAND, 5020, 0);}
			if (INVALID_HANDLE_VALUE == hPipe2){
				this->Trace("Creating Named Pipe client GUI was NOT successful.");
				::PostMessage(this->hwndDialog, WM_COMMAND, 5020, 0);}
			if (INVALID_HANDLE_VALUE == hPipe3){
				this->Trace("Creating Named Pipe client GUI was NOT successful.");
				::PostMessage(this->hwndDialog, WM_COMMAND, 5020, 0);}
			if (INVALID_HANDLE_VALUE == hPipe4){
				this->Trace("Creating Named Pipe client GUI was NOT successful.");
				::PostMessage(this->hwndDialog, WM_COMMAND, 5020, 0);}
			if (INVALID_HANDLE_VALUE == hPipe5){
				this->Trace("Creating Named Pipe client GUI was NOT successful.");
				::PostMessage(this->hwndDialog, WM_COMMAND, 5020, 0);}
			if (INVALID_HANDLE_VALUE == hPipe6){
				this->Trace("Creating Named Pipe client GUI was NOT successful.");
				::PostMessage(this->hwndDialog, WM_COMMAND, 5020, 0);}
			if (INVALID_HANDLE_VALUE == hPipe7){
				this->Trace("Creating Named Pipe client GUI was NOT successful.");
				::PostMessage(this->hwndDialog, WM_COMMAND, 5020, 0);}
			
			_piscreated = TRUE;
			}

	// fan speed
			if(Fahrenheit){
				if (fanspeed > 0x1fff) fanspeed = lastfanspeed;
					sprintf_s(str_value, sizeof(str_value),"%d %d %s %d %d %d ",
					this->CurrentMode,(this->MaxTemp* 9 /5 +32),this->gSensorNames[iMaxTemp],
					iFarbeIconB, fanspeed, fanctrl2);
			}
			else {
				if (fanspeed > 0x1fff) fanspeed = lastfanspeed;
					sprintf_s(str_value, sizeof(str_value),"%d %d %s %d %d %d ",
					this->CurrentMode,(this->MaxTemp),this->gSensorNames[iMaxTemp],
					iFarbeIconB, fanspeed, fanctrl2);
			}
			strcpy_s(szBuffer,str_value); //write buffer


	//send to client
				lbResult = bResult;
				bResult = WriteFile
					(
					hPipe0,                // handle to pipe 
					szBuffer,             // buffer to write from 
					strlen(szBuffer)+1,   // number of bytes to write, include the NULL 
					&cbBytes,             // number of bytes written 
					NULL);                // not overlapped I/O 
				bResult = WriteFile
					(
					hPipe1,                // handle to pipe 
					szBuffer,             // buffer to write from 
					strlen(szBuffer)+1,   // number of bytes to write, include the NULL 
					&cbBytes,             // number of bytes written 
					NULL);                // not overlapped I/O 
				bResult = WriteFile
					(
					hPipe2,                // handle to pipe 
					szBuffer,             // buffer to write from 
					strlen(szBuffer)+1,   // number of bytes to write, include the NULL 
					&cbBytes,             // number of bytes written 
					NULL);                // not overlapped I/O 
				bResult = WriteFile
					(
					hPipe3,                // handle to pipe 
					szBuffer,             // buffer to write from 
					strlen(szBuffer)+1,   // number of bytes to write, include the NULL 
					&cbBytes,             // number of bytes written 
					NULL);                // not overlapped I/O 
				bResult = WriteFile
					(
					hPipe4,                // handle to pipe 
					szBuffer,             // buffer to write from 
					strlen(szBuffer)+1,   // number of bytes to write, include the NULL 
					&cbBytes,             // number of bytes written 
					NULL);                // not overlapped I/O 
				bResult = WriteFile
					(
					hPipe5,                // handle to pipe 
					szBuffer,             // buffer to write from 
					strlen(szBuffer)+1,   // number of bytes to write, include the NULL 
					&cbBytes,             // number of bytes written 
					NULL);                // not overlapped I/O 
				bResult = WriteFile
					(
					hPipe6,                // handle to pipe 
					szBuffer,             // buffer to write from 
					strlen(szBuffer)+1,   // number of bytes to write, include the NULL 
					&cbBytes,             // number of bytes written 
					NULL);                // not overlapped I/O 
				bResult = WriteFile
					(
					hPipe7,                // handle to pipe 
					szBuffer,             // buffer to write from 
					strlen(szBuffer)+1,   // number of bytes to write, include the NULL 
					&cbBytes,             // number of bytes written 
					NULL);                // not overlapped I/O 

//end named pipe client session
//
//*************************************************************************************


			break;

			case 4:		// renew tempicon
				if (ShowTempIcon && ReIcCycle) {
					this->RemoveTextIcons();
					this->ProcessTextIcons();}
				break;

			default:
			break;
		} // End switch mp1

					if (this->ShowTempIcon==1) 
					{
					this->ProcessTextIcons();  //icon Einstieg
					}
					else {this->RemoveTextIcons();}

                //	say windows not to hold much more memspace
				//	SetProcessWorkingSetSize(GetCurrentProcess(),65536,WANTED_MEM_SIZE);
				break;


		case WM_COMMAND:
				if (HIWORD(mp1)==BN_CLICKED || HIWORD(mp1)==EN_CHANGE) 
	{
					int cmd= LOWORD(mp1);

		//display temperature list

		char obuf[256]= "", obuf2[128]="", templist2[512];	
		
		strcpy_s(templist2,sizeof(templist2), "");
					
		if(cmd==7001 || cmd==7002) 
		{
		this->ShowAllFromDialog();
		int i;
			for (i= 0; i<12; i++) 
			{
				int temp= this->State.Sensors[i];

				if (temp < 128 && temp!= 0) 
				{
					if (Fahrenheit)
						sprintf_s(obuf2,sizeof(obuf2), "%d°F", temp* 9 /5 +32);
					else
						sprintf_s(obuf2, sizeof(obuf2), "%d°C", temp);

					size_t strlen_templist2 = strlen_s(templist2,sizeof(templist2));

				if (SlimDialog && StayOnTop)
					sprintf_s(templist2+strlen_templist2,sizeof(templist2)-strlen_templist2,
						"%d %s %s (0x%02x)", i+1,this->State.SensorName[i],
						obuf2, this->State.SensorAddr[i]);
				else
 					sprintf_s(templist2+strlen_templist2,sizeof(templist2)-strlen_templist2,
						"%d %s %s", i+1,this->State.SensorName[i],
						obuf2);



					strcat_s(templist2,sizeof(templist2), "\r\n");
				}
				else 
				{
					if (this->ShowAll==1) 
					{
						sprintf_s(obuf2, sizeof(obuf2), "n/a");
                        size_t strlen_templist2 = strlen_s(templist2,sizeof(templist2));

				if (SlimDialog && StayOnTop)
					sprintf_s(templist2+strlen_templist2,sizeof(templist2)-strlen_templist2,
						"%d %s %s (0x%02x)", i+1,this->State.SensorName[i],
						obuf2, this->State.SensorAddr[i]);
				else
 					sprintf_s(templist2+strlen_templist2,sizeof(templist2)-strlen_templist2,
						"%d %s %s", i+1,this->State.SensorName[i],
						obuf2);

						strcat_s(templist2,sizeof(templist2), "\r\n");
					}
				}
			}
			::SetDlgItemText(this->hwndDialog, 8101, templist2);
			this->icontemp= this->State.Sensors[iMaxTemp];
		};
//end temp display


					if (cmd>=8300 && cmd<=8302 || cmd==8310 ) {  // radio button or manual speed entry
						::PostMessage(hwnd, WM__GETDATA, 0, 0);
					}
					else 
					switch (cmd) {
						case 5001: // bios
						this->ModeToDialog(1);
						::PostMessage(this->hwndDialog, WM__GETDATA, 0, 0);
						break;

						case 5002: // smart
							this->ModeToDialog(2);
							::PostMessage(this->hwndDialog, WM__GETDATA, 0, 0);
							break;

						case 5003: // smart1
									this->ModeToDialog(2);
									if (this->IndSmartLevel == 1) {
										sprintf_s(obuf+strlen(obuf),sizeof(obuf)-strlen(obuf), "Activation of Fan Control Profile 'Smart Mode 1'");
									this->Trace(obuf);}
									this->IndSmartLevel = 0;
// rüberkopieren
						for (int i= 0; i<32; i++) {
									this->SmartLevels[i].temp = this->SmartLevels1[i].temp1; 
									this->SmartLevels[i].fan = this->SmartLevels1[i].fan1;
						}
						::PostMessage(this->hwndDialog, WM__GETDATA, 0, 0);
						break;

						case 5004: // smart2
									this->ModeToDialog(2);
									if (this->IndSmartLevel == 0) {
										sprintf_s(obuf+strlen(obuf),sizeof(obuf)-strlen(obuf), "Activation of Fan Control Profile 'Smart Mode 2'");
									this->Trace(obuf);}
									this->IndSmartLevel = 1;

						for (int i= 0; i<32; i++) {
									this->SmartLevels[i].temp = this->SmartLevels2[i].temp2; 
									this->SmartLevels[i].fan = this->SmartLevels2[i].fan2;
						}
						::PostMessage(this->hwndDialog, WM__GETDATA, 0, 0);	   
						break;

						case 5005: // manual
							this->ModeToDialog(3);
							::PostMessage(this->hwndDialog, WM__GETDATA, 0, 0);
							break;


						case 5010: // show window
								   ::ShowWindow(this->hwndDialog, TRUE);
								   ::SetForegroundWindow(this->hwndDialog);
								   break;

						case 5040: // show window
							if (BluetoothEDR) this->SetHdw("Bluetooth", 16, 58, 32);
							else this->SetHdw("Bluetooth", 32, 59, 16);
							break;

						case 5050: // donate
								   ::ShellExecute(NULL, "open", Title5,  
									   NULL, NULL, SW_SHOW);
								   break;

						case 5060: // hide window
								this->SetHdw("Thinklight", 2, 59, 16);
								break;

						case 5070: // show temp icon
								   this->ShowTempIcon = 0;
								   this->pTaskbarIcon = new	TASKBARICON(this->hwndDialog, 10, "TPFanControl");
								   this->pTaskbarIcon->SetIcon(this->CurrentIcon);
								   break;

						case 5080: // show temp icon
									delete this->pTaskbarIcon;
									this->pTaskbarIcon = NULL;
									this->ShowTempIcon = 1;
									break;

						case 5030: // hide window
								::ShowWindow(this->hwndDialog, SW_MINIMIZE);
								break;

						case 5020: // end program
									// Wait for the work thread to terminate
									if (this->hThread) {
										::WaitForSingleObject(this->hThread, INFINITE);
									}
									if(!this->EcAccess.Lock(100))
									{
										// Something is going on, let's do this later
										this->Trace("Delaying close");
										m_needClose = true;
										break;
									}


									// don't close if we can't set the fan back to bios controlled
									if (!this->ActiveMode || this->SetFan("On close", 0x80, true)) {
										::KillTimer(this->hwndDialog, m_fanTimer);
										::KillTimer(this->hwndDialog, m_titleTimer);
										::KillTimer(this->hwndDialog, m_iconTimer);
										::KillTimer(this->hwndDialog, m_renewTimer);
										BOOL CloHT=CloseHandle(this->hThread);
//										BOOL CloHM=CloseHandle(this->hLock);
//										BOOL CloHS=CloseHandle(this->hLockS);
										this->Trace("Exiting ProcessDialog");
										::PostMessage(hwnd, WM__DISMISSDLG, IDCANCEL, 0); // exit from ProcessDialog() 
									}
									else
									{
										m_needClose = true;
									}
									this->EcAccess.Unlock();
								    break;

					}
				}
				break;
			

		case WM_CLOSE:
				//if (this->MinimizeOnClose && (this->MinimizeToSysTray || this->Runs_as_service))   // 0.24 new:  || this->Runs_as_service) 
				//{MessageBox(NULL, "will Fenster schließen", "TPFanControl", MB_ICONEXCLAMATION);
				::ShowWindow(this->hwndDialog, SW_MINIMIZE);   //}
				rc= TRUE;
				break;

							
		case WM_ENDSESSION:  //WM_QUERYENDSESSION?
	//if running as service do not end
	if(!this->Runs_as_service){

				// end program
									// Wait for the work thread to terminate
									if (this->hThread) {
										::WaitForSingleObject(this->hThread, INFINITE);
									}
									if(!this->EcAccess.Lock(100))
									{
										// Something is going on, let's do this later
										this->Trace("Delaying close");
										m_needClose = true;
										break;
									}


									// don't close if we can't set the fan back to bios controlled
									if (!this->ActiveMode || this->SetFan("On close", 0x80, true)) {
										::KillTimer(this->hwndDialog, m_fanTimer);
										::KillTimer(this->hwndDialog, m_titleTimer);
										::KillTimer(this->hwndDialog, m_iconTimer);
										::KillTimer(this->hwndDialog, m_renewTimer);
										BOOL CloHT=CloseHandle(this->hThread);
//										BOOL CloHM=CloseHandle(this->hLock);
//										BOOL CloHS=CloseHandle(this->hLockS);
										this->Trace("Exiting ProcessDialog");
										::PostMessage(hwnd, WM__DISMISSDLG, IDCANCEL, 0); // exit from ProcessDialog() 
									}
									else
									{
										m_needClose = true;
									}
									this->EcAccess.Unlock();
	}
								    break;


//		case WM_MOVE:
		case WM_SIZE:
				if (mp1==SIZE_MINIMIZED && this->MinimizeToSysTray) {
				    ::ShowWindow(this->hwndDialog, FALSE);
				}
				rc= TRUE;
				break;

		case WM_DESTROY:
				break;




		//
		// USER messages
		//

		case WM__GETDATA:
			if (!this->hThread && !this->FinalSeen) 
			{
				this->hThread= this->CreateThread(FANCONTROL_Thread, (ULONG)this);
			}
			break;


		case WM__NEWDATA:
				if (this->hThread) {
					::WaitForSingleObject(this->hThread, INFINITE);
					if (this->hThread) ::CloseHandle(this->hThread);
					this->hThread= 0;
				}

				ok= mp1;  // equivalent of "ok= this->ReadEcStatus(&this->State);" via thread
				
				if (ok) {
					this->ReadErrorCount= 0;
					this->HandleData();
					if (m_needClose)
					{
						::PostMessage(this->hwndDialog, WM_COMMAND, 5020, 0);
						m_needClose = false;
					}
				}
				else {
					sprintf_s(buf, sizeof(buf),"Warning: can't read Status, read error count = %d", this->ReadErrorCount);
					this->Trace(buf);
					sprintf_s(buf, sizeof(buf),"We will close to BIOS-Mode after %d consecutive read errors", this->MaxReadErrors);
					this->Trace(buf);
					this->ReadErrorCount++;

					// after so many consecutive read errors, try to switch back to bios mode
					if (this->ReadErrorCount > this->MaxReadErrors) {
						this->ModeToDialog(1);
						ok= this->SetFan("Max. Errors", 0x80);
						if (ok) {
							this->Trace("Set to BIOS Mode, to many consecutive read errors");
							::Sleep(2000);
							::SendMessage(this->hwndDialog, WM_ENDSESSION, 0, 0);
						}
					}
				}
				break;


		case WM__TASKBAR:

				switch (mp2) {

				    case WM_LBUTTONDOWN:

						if (!IsWindowVisible(this->hwndDialog)){
								::ShowWindow(this->hwndDialog, TRUE);
								::SetForegroundWindow(this->hwndDialog);
						}
						else	::ShowWindow(this->hwndDialog, SW_MINIMIZE);
				        break;

				    case WM_LBUTTONUP:
								{
									BOOL 
									  isshift= ::GetAsyncKeyState(VK_SHIFT) & 0x8000,
									  isctrl= ::GetAsyncKeyState(VK_CONTROL) & 0x8000;

									int action= -1;

									// some fancy key dependent stuff could be done here.

								}
								break;


				    case WM_LBUTTONDBLCLK:

						if (!IsWindowVisible(this->hwndDialog)){
								::ShowWindow(this->hwndDialog, TRUE);
								::SetForegroundWindow(this->hwndDialog);
						}
						else	::ShowWindow(this->hwndDialog, SW_MINIMIZE);
				        break;

					char testpara;
				    case WM_RBUTTONDOWN:
								{
								MENU m(5000);
								
								int ok_ecaccess = false;
								for (int i = 0; i < 10; i++){
									if ( ok_ecaccess = this->EcAccess.Lock(100))break;
									else ::Sleep(100);
								}

								if (!ok_ecaccess){
									this->Trace("Could not acquire mutex to read BT/TL status");
									break;
								}

								ok= this->ReadByteFromEC(59, &testpara);
								if (testpara & 2) m.CheckMenuItem(5060);

								if (this->BluetoothEDR){
								ok= this->ReadByteFromEC(58, &testpara);
								if (testpara & 16) m.CheckMenuItem(5040); 
								}
								else {
								ok= this->ReadByteFromEC(59, &testpara);
								if (testpara & 32) m.CheckMenuItem(5040); 
								}

								int mode= this->CurrentModeFromDialog();
								if (mode==1){
									m.CheckMenuItem(5001);

									if (this->ActiveMode==0){
										m.DisableMenuItem(5002);  // v0.25
										m.DisableMenuItem(5003);  // v0.25
										m.DisableMenuItem(5004);  // v0.25
										m.DisableMenuItem(5005);  // v0.25
									}
								}
								else

								if (mode==2)
										m.CheckMenuItem(5002); 
									
								if (mode==3)
										m.CheckMenuItem(5005); 

								m.InsertItem (this->MenuLabelSM1, 5003, 10);
								m.InsertItem (this->MenuLabelSM2, 5004, 11);

								if ( this->SmartLevels2[0].temp2 == 0 ) 
								{
										m.DeleteMenuItem(5003);
										m.DeleteMenuItem(5004);
								}

								if ( this->SmartLevels2[0].temp2 != 0 ) 
								{
										m.DeleteMenuItem(5002);

									if (mode == 2  && this->IndSmartLevel == 0 ) 
											m.CheckMenuItem(5003);

									if (mode == 2  && this->IndSmartLevel != 0 ) 
											m.CheckMenuItem(5004);
								}

									if (Runs_as_service) 
										m.DeleteMenuItem(5020);
					
									if (!IsWindowVisible(this->hwndDialog))
										m.DeleteMenuItem(5030);

									if (IsWindowVisible(this->hwndDialog))
										m.DeleteMenuItem(5010);

									if (this->ShowTempIcon == 0)
										m.DeleteMenuItem(5070);

									if (this->ShowTempIcon == 1)
										m.DeleteMenuItem(5080);
									
									this->EcAccess.Unlock();

								m.Popup(this->hwndDialog);  
								}
							break;
				}
				rc= TRUE;
				break;


		default:
			break;

	}

	return rc;
}



//-------------------------------------------------------------------------
//  reading the EC status may take a while, hence do it in a thread
//-------------------------------------------------------------------------
int 
FANCONTROL::WorkThread()
{
	int ok= this->ReadEcStatus(&this->State);

	::PostMessage(this->hwndDialog, WM__NEWDATA, ok, 0);

	return 0;
}
// The texticons will be shown depending on variables
static const int MAX_TEXT_ICONS = 16;
int icon, oldicon;
BOOL dishow (TRUE);
TCHAR myszTip[64] ;
void FANCONTROL::ProcessTextIcons(void){
	oldicon = icon;
	if (this->CurrentModeFromDialog()==1) {
		icon= 10;	// gray
	}
	else {
		icon= 11;	// blue
		for (int i= 0; i<ARRAYMAX(this->IconLevels); i++) {
			if (this->MaxTemp>=this->IconLevels[i]) {
				icon= 12+i;	// yellow, orange, red
			}
		}
	}
	
	if (this->IconColorFan) {
		switch (fanspeed/1000) {
			case 0:
			break;
			case 1:
			icon = 21; //sehr hell grün
			break;
			case 2:
			icon = 22; //hell grün
			break;
			case 3:
			icon = 23; //grün
			break;
			case 4:
			icon = 24; //dunkel grün
			break;
			case 5:
			icon = 25; //sehr dunkel grün
			break;
			case 6:
			icon = 25; //sehr dunkel grün
			break;
			case 7:
			icon = 25; //sehr dunkel grün
			break;
			case 8:
			icon = 25; //sehr dunkel grün
			break;
			default:
			icon = oldicon;
			break;
		};
	}


	this->iFarbeIconB = icon;

	lstrcpyn(myszTip, this->Title2, sizeof(myszTip)-1);
	
    if(pTextIconMutex->Lock(100)) 
	{
        //INIT ppTbTextIcon
        if(!ppTbTextIcon || this->TaskbarNew) 
		{	this->TaskbarNew=0;
            ppTbTextIcon = new CTaskbarTextIcon*[MAX_TEXT_ICONS];
            for(int i = 0;i<MAX_TEXT_ICONS;++i) 
			{
                ppTbTextIcon[i] = NULL;
            }


            //erstmal nur eins

            ppTbTextIcon[0] = new CTaskbarTextIcon(this->m_hinstapp,
                    this->hwndDialog,WM__TASKBAR,0,"","",  //WM_APP+5000 -> WM__TASKBAR
					this->iFarbeIconB, this->iFontIconB, myszTip);
		
			if (dishow && !this->NoBallons){
				if (Fahrenheit){
					ppTbTextIcon[0]->DiShowballon(_T("shows max. temperature in °F and sensor name, left click on icon shows or hides control window, right click shows menue"),
						_T("TPFanControl new text icon"),NIIF_INFO,11);
				}
				else {
					ppTbTextIcon[0]->DiShowballon(_T("shows max. temperature in °C and sensor name, left click on icon shows or hides control window, right click shows menue"),
						_T("TPFanControl new text icon"),NIIF_INFO,11);
				}


// Input:
//  szText: [in] Text for the balloon tooltip.
//  szTitle: [in] Title for the balloon.  This text is shown in bold above
//           the tooltip text (szText).  Pass "" if you don't want a title.
//  dwIcon: [in] Specifies an icon to appear in the balloon.  Legal values are:
//                 NIIF_NONE: No icon
//                 NIIF_INFO: Information
//                 NIIF_WARNING: Exclamation
//                 NIIF_ERROR: Critical error (red circle with X)
//  uTimeout: [in] Number of seconds for the balloon to remain visible.  Can
//            be between 10 and 30 inclusive.
//

					dishow = FALSE;}
        }
    
		char str_value[256];
	//	char buf[256]= "";
    //  aktualisieren
        for(int i = 0;i<MAX_TEXT_ICONS;++i) {
            if(ppTbTextIcon[i]) {
				if(Fahrenheit)
					_itoa_s((this->icontemp* 9/5)+32,str_value,sizeof(str_value),10);
				else
					_itoa_s(this->icontemp,str_value,sizeof(str_value),10);
				sprintf_s(str_value, sizeof(str_value),"%s",str_value);
				ppTbTextIcon[i]->ChangeText(str_value, this->gSensorNames[iMaxTemp],iFarbeIconB, iFontIconB, myszTip);		
            }
        }
        pTextIconMutex->Unlock();
//this->Trace(LastTooltip); 
	}  

}

void FANCONTROL::RemoveTextIcons(void) {
    if(pTextIconMutex->Lock(10000)) {
        if(ppTbTextIcon) {
            for(int i = 0;i<MAX_TEXT_ICONS;++i) {
                if(ppTbTextIcon[i]) {
                    delete ppTbTextIcon[i];
                }
            }
            delete[] ppTbTextIcon;
            ppTbTextIcon = NULL;
        }
        pTextIconMutex->Unlock();
    }
    else {
        _ASSERT(false);//Mutex not av within 10 sec
    }
}
