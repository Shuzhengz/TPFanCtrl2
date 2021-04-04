// TPFC2.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "TPFC2.h"
#include "tools.h"
#include "taskbartexticon.h"
#include "_prec.h"

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_TPFC2, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_TPFC2));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TPFC2));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_TPFC2);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

//-------------------------------------------------------------------------
//  constructor
//-------------------------------------------------------------------------
TPFC2::TPFC2(HINSTANCE hinstapp)
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
	pTextIconMutex(new MUTEXSEM(0, "Global\\TPFanControl_ppTbTextIcon"))

{
	int i = 0;
	char buf[256] = "";

	// SensorNames
			// 78-7F (state index 0-7)
	strcpy_s(this->gSensorNames[0], sizeof(this->gSensorNames[0]), "cpu"); // main processor
	strcpy_s(this->gSensorNames[1], sizeof(this->gSensorNames[1]), "aps"); // harddisk protection gyroscope
	strcpy_s(this->gSensorNames[2], sizeof(this->gSensorNames[2]), "crd"); // under PCMCIA slot (front left)
	strcpy_s(this->gSensorNames[3], sizeof(this->gSensorNames[3]), "gpu"); // graphical processor
	strcpy_s(this->gSensorNames[4], sizeof(this->gSensorNames[4]), "bat"); // inside T43 battery
	strcpy_s(this->gSensorNames[5], sizeof(this->gSensorNames[5]), "x7d"); // usually n/a
	strcpy_s(this->gSensorNames[6], sizeof(this->gSensorNames[6]), "bat"); // inside T43 battery
	strcpy_s(this->gSensorNames[7], sizeof(this->gSensorNames[7]), "x7f"); // usually n/a
//  	// C0-C4 (state index 8-11)
	strcpy_s(this->gSensorNames[8], sizeof(this->gSensorNames[8]), "bus"); // unknown
	strcpy_s(this->gSensorNames[9], sizeof(this->gSensorNames[9]), "pci"); // mini-pci, WLAN, southbridge area
	strcpy_s(this->gSensorNames[10], sizeof(this->gSensorNames[10]), "pwr"); // power supply (get's hot while charging battery)
	strcpy_s(this->gSensorNames[11], sizeof(this->gSensorNames[11]), "xc3"); // usually n/a
// future
	strcpy_s(this->gSensorNames[12], sizeof(this->gSensorNames[12]), "");
	strcpy_s(this->gSensorNames[13], sizeof(this->gSensorNames[13]), "");
	strcpy_s(this->gSensorNames[14], sizeof(this->gSensorNames[14]), "");
	strcpy_s(this->gSensorNames[15], sizeof(this->gSensorNames[15]), "");
	strcpy_s(this->gSensorNames[16], sizeof(this->gSensorNames[16]), "");


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

	this->IconLevels[0] = 50;	// yellow icon level
	this->IconLevels[1] = 55;	// orange icon level
	this->IconLevels[2] = 60;	// red icon level


	// initial default "smart" table
	setzero(this->SmartLevels, sizeof(this->SmartLevels));
	this->SmartLevels[i].temp = 50;  this->SmartLevels[i].fan = 0; i++;
	this->SmartLevels[i].temp = 55;  this->SmartLevels[i].fan = 3; i++;
	this->SmartLevels[i].temp = 60;  this->SmartLevels[i].fan = 5; i++;
	this->SmartLevels[i].temp = 65;  this->SmartLevels[i].fan = 7; i++;
	this->SmartLevels[i].temp = 70;  this->SmartLevels[i].fan = 128; i++;
	this->SmartLevels[i].temp = -1;  this->SmartLevels[i].fan = 0; i++;

	setzero(this->SmartLevels1, sizeof(this->SmartLevels1));
	i = 0;
	this->SmartLevels1[i].temp1 = 50;  this->SmartLevels1[i].fan1 = 0; i++;
	this->SmartLevels1[i].temp1 = 55;  this->SmartLevels1[i].fan1 = 3; i++;
	this->SmartLevels1[i].temp1 = 60;  this->SmartLevels1[i].fan1 = 5; i++;
	this->SmartLevels1[i].temp1 = 65;  this->SmartLevels1[i].fan1 = 7; i++;
	this->SmartLevels1[i].temp1 = 70;  this->SmartLevels1[i].fan1 = 128; i++;
	this->SmartLevels1[i].temp1 = -1;  this->SmartLevels1[i].fan1 = 0; i++;

	setzero(this->SmartLevels2, sizeof(this->SmartLevels2));
	i = 0;
	this->SmartLevels2[i].temp2 = 0;  this->SmartLevels2[i].fan2 = 0; i++;
	// später if ( this->SmartLevels2[i].temp2 != 0 ) dann smart2
	this->SmartLevels2[i].temp2 = 55;  this->SmartLevels2[i].fan2 = 3; i++;
	this->SmartLevels2[i].temp2 = 60;  this->SmartLevels2[i].fan2 = 5; i++;
	this->SmartLevels2[i].temp2 = 65;  this->SmartLevels2[i].fan2 = 7; i++;
	this->SmartLevels2[i].temp2 = 70;  this->SmartLevels2[i].fan2 = 128; i++;
	this->SmartLevels2[i].temp2 = -1;  this->SmartLevels2[i].fan2 = 0; i++;


	// code title3
	char bias = 100;
	for (int _i = 0; _i < 111; _i++) {
		switch (_i) {
		case 0: this->Title3[0] = 32; break;			//blank
		case 1: this->Title3[1] = bias - 2; break;		//b
		case 2: this->Title3[2] = bias + 21; break;		//y
		case 3: this->Title3[3] = 32; break;			//blank
		case 4: this->Title3[4] = bias + 16; break;		//t
		case 5: this->Title3[5] = bias + 14; break;		//r
		case 6: this->Title3[6] = bias + 11; break;		//o
		case 7: this->Title3[7] = bias + 17; break;		//u
		case 8: this->Title3[8] = bias - 2; break;		//b
		case 9: this->Title3[9] = bias - 3; break;		//a
		case 10: this->Title3[10] = bias; break;			//d
		case 11: this->Title3[11] = bias + 5; break;		//i
		case 12: this->Title3[12] = bias + 20; break;	//x
		case 13: this->Title3[13] = 32; break;			//blank
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

		for (int __i = 0; __i < 111; __i++) {
			switch (__i) {
			case 0: this->Title4[0] = bias + 4; break;		//h
			case 1: this->Title4[1] = bias + 16; break;		//t
			case 2: this->Title4[2] = bias + 16; break;		//t
			case 3: this->Title4[3] = bias + 12; break;		//p
			case 4: this->Title4[4] = bias - 42; break;		//:
			case 5: this->Title4[5] = bias - 8; break;		///
			case 6: this->Title4[6] = bias - 8; break;		///
			case 7: this->Title4[7] = bias + 19; break;		//w
			case 8: this->Title4[8] = bias + 19; break;		//w
			case 9: this->Title4[9] = bias + 19; break;		//w
			case 10: this->Title4[10] = bias - 54; break;	//.
			case 11: this->Title4[11] = bias + 15; break;	//s
			case 12: this->Title4[12] = bias + 16; break;	//t
			case 13: this->Title4[13] = bias - 3; break;	//a
			case 14: this->Title4[14] = bias + 2; break;		//f
			case 15: this->Title4[15] = bias + 2; break;		//f
			case 16: this->Title4[16] = bias - 54; break;	//.
			case 17: this->Title4[17] = bias + 17; break;	//u
			case 18: this->Title4[18] = bias + 10; break;	//n
			case 19: this->Title4[19] = bias + 5; break;		//i
			case 20: this->Title4[20] = bias - 55; break;	//-
			case 21: this->Title4[21] = bias + 9; break;		//m
			case 22: this->Title4[22] = bias - 3; break;	//a
			case 23: this->Title4[23] = bias + 14; break;	//r
			case 24: this->Title4[24] = bias - 2; break;		//b
			case 25: this->Title4[25] = bias + 17; break;	//u
			case 26: this->Title4[26] = bias + 14; break;	//r	
			case 27: this->Title4[27] = bias + 3; break;		//g
			case 28: this->Title4[28] = bias - 54; break;	//.
			case 29: this->Title4[29] = bias; break;		//d
			case 30: this->Title4[30] = bias + 1; break;		//e
			case 31: this->Title4[31] = bias - 8; break;		///
			case 32: this->Title4[32] = bias + 26; break;	//~
			case 33: this->Title4[33] = bias + 15; break;	//s
			case 34: this->Title4[34] = bias - 1; break;	//c
			case 35: this->Title4[35] = bias + 4; break;		//h
			case 36: this->Title4[36] = bias + 9; break;		//m
			case 37: this->Title4[37] = bias + 5; break;		//i
			case 38: this->Title4[38] = bias + 16; break;	//t
			case 39: this->Title4[39] = bias + 22; break;	//z
			case 40: this->Title4[40] = bias + 14; break;	//r
			case 41: this->Title4[41] = bias - 8; break;		///
			case 42: this->Title4[42] = bias; break;		//d
			case 43: this->Title4[43] = bias + 11; break;	//o
			case 44: this->Title4[44] = bias + 10; break;	//n
			case 45: this->Title4[45] = bias - 3; break;		//a
			case 46: this->Title4[46] = bias + 16; break;	//t
			case 47: this->Title4[47] = bias + 1; break;		//e
			case 48: this->Title4[48] = bias - 54; break;	//.
			case 49: this->Title4[49] = bias + 4; break;		//h
			case 50: this->Title4[50] = bias + 16; break;	//t
			case 51: this->Title4[51] = bias + 9; break;		//m
			case 52: this->Title4[52] = bias + 8; break;		//l
			}
		}

		// code Title5 http://tpfancontrol.com

		for (int __i = 0; __i < 23; __i++) {
			switch (__i) {
			case 0: this->Title5[0] = bias + 4; break;		//h
			case 1: this->Title5[1] = bias + 16; break;		//t
			case 2: this->Title5[2] = bias + 16; break;		//t
			case 3: this->Title5[3] = bias + 12; break;		//p
			case 4: this->Title5[4] = bias - 42; break;		//:
			case 5: this->Title5[5] = bias - 8; break;		///
			case 6: this->Title5[6] = bias - 8; break;		///
			case 7: this->Title5[7] = bias + 16; break;		//t
			case 8: this->Title5[8] = bias + 12; break;		//p
			case 9: this->Title5[9] = bias + 2; break;		//f
			case 10: this->Title5[10] = bias - 3; break;	//a
			case 11: this->Title5[11] = bias + 10; break;	//n
			case 12: this->Title5[12] = bias - 1; break;	//c
			case 13: this->Title5[13] = bias + 11; break;	//o
			case 14: this->Title5[14] = bias + 10; break;	//n
			case 15: this->Title5[15] = bias + 16; break;	//t
			case 16: this->Title5[16] = bias + 14; break;	//r	
			case 17: this->Title5[17] = bias + 11; break;	//o
			case 18: this->Title5[18] = bias + 8; break;		//l
			case 19: this->Title5[19] = bias - 54; break;	//.
			case 20: this->Title5[20] = bias - 1; break;	//c
			case 21: this->Title5[21] = bias + 11; break;	//o
			case 22: this->Title5[22] = bias + 9; break;		//m
			}
		}
	}

}
