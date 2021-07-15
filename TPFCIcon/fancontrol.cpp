
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
#include "taskbartexticon.h"

//-------------------------------------------------------------------------
//  constructor
//-------------------------------------------------------------------------
FANCONTROL::FANCONTROL(HINSTANCE hinstapp)
	: hwndDialog(NULL),
		iFarbeIconB(11),
		Log2File(0),
		ShowTempIcon(1),
		hThread(NULL),
		pTaskbarIcon(NULL),
		m_titleTimer(NULL),
        m_hinstapp(hinstapp),
        ppTbTextIcon(NULL)

{
		int i= 0;
		char buf[256]= "";

		// clear title strings
		setzero(this->Title, sizeof(this->Title));
		setzero(this->Title2, sizeof(this->Title2));
		setzero(this->Title3, sizeof(this->Title3));
		setzero(this->Title4, sizeof(this->Title4));

	// code title3
	char bias = 100;
	for (int _i=0; _i < 43; _i++) 
	{
		switch (_i) 
		{
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
*/
		}
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

	this->hwndDialog= ::CreateDialogParam
							(
							hinstapp, 
							MAKEINTRESOURCE(9000), 
							HWND_DESKTOP, 
							(DLGPROC)BaseDlgProc, 
							(LPARAM)this
							);

	if (this->hwndDialog) 
	{
		::GetWindowText(this->hwndDialog, this->Title, sizeof(this->Title));
		strcat_s(this->Title,sizeof(this->Title), " V" FANCONTROLVERSION);
		strcat_s(this->Title,sizeof(this->Title), this->Title3);
		::SetWindowText(this->hwndDialog, this->Title);

		::SetWindowLong(this->hwndDialog, GWL_USERDATA, (ULONG)this);
		::SendDlgItemMessage(this->hwndDialog, 8112, EM_LIMITTEXT, 256, 0);
		::SendDlgItemMessage(this->hwndDialog, 9200, EM_LIMITTEXT, 4096, 0);
		::SetDlgItemText(this->hwndDialog, 8310, "7");
	

		// initial values
		this->TaskbarNew=0;
		this->MaxTemp = 0;
		this->iMaxTemp = 0;
		this->iFarbeIconB =15;

		// make it call HandleControl initially
		::PostMessage(this->hwndDialog, WM__GETDATA, 0, 0);

		m_titleTimer = ::SetTimer(this->hwndDialog, 1, 500, NULL);
	}
}

//-------------------------------------------------------------------------
//  destructor
//-------------------------------------------------------------------------
FANCONTROL::~FANCONTROL()
{	
	if (this->hThread) {
		::WaitForSingleObject(this->hThread, 10);
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
			else This->ProcessTextIcons();
		}
		rc= This->DlgProc(hwnd, msg, mp1, mp2);
	}

	return rc;
}



//-------------------------------------------------------------------------
//  dialog window procedure as class method
//-------------------------------------------------------------------------
#define WANTED_MEM_SIZE 65536*12
char szBuffer[BUFFER_SIZE];
DWORD cbBytes;
char str_value[1024];
BOOL bResult (FALSE);
BOOL lbResult (FALSE);
int fanspeed;
int fanstate;
int CurrentMode;
BOOL tried2connect(FALSE);
BOOL _piscreated(FALSE);
BOOL _lpiscreated(FALSE);
char SensorName[4];
char CCurrentMode[16];
char CCurrentICON[16];
BOOL dishow (TRUE);
BOOL dioicon (TRUE);
BOOL _isConnected (FALSE);
BOOL _hasShownConnected(FALSE);
BOOL _hasShownDisconnected(FALSE);

ULONG 
FANCONTROL::DlgProc(HWND hwnd, ULONG msg, WPARAM mp1, LPARAM mp2)
{
	ULONG rc= 0;
		switch (msg) 
			{
			case WM_INITDIALOG:
				break;
			case WM_TIMER:
				switch (mp1)
				{
				case 1:	
					::PostMessage(this->hwndDialog, WM__GETDATA, 0, 0);
					if (tried2connect)
					{
						if (this->ShowTempIcon==1) 
						{
							this->ProcessTextIcons();  //icon Einstieg
						}
						else 
						{
							this->RemoveTextIcons();
						}

						if (this->pTaskbarIcon)
						{
							this->pTaskbarIcon->SetIcon(this->iFarbeIconB);

							if (_isConnected == FALSE)this->pTaskbarIcon->SetTooltip("not connected to tpfc service");
							else this->pTaskbarIcon->SetTooltip(Title2);

							if (_isConnected == TRUE && _hasShownConnected == FALSE){
									this->pTaskbarIcon->SetBalloon(NIIF_INFO, "TPFanControl Icon",
									"connected to TPFanControl service",11);
									_hasShownConnected = TRUE;}
							if (_isConnected == FALSE && _hasShownDisconnected == FALSE){
									this->pTaskbarIcon->SetBalloon(NIIF_ERROR, "TPFanControl Icon",
									"NOT connected to TPFanControl service",11);
									_hasShownDisconnected = TRUE;}
						}
					}
					break;
				default:
					break;

				}	//switch(mp1)



			//say windows not to hold much more memspace
            SetProcessWorkingSetSize(GetCurrentProcess(),65536,WANTED_MEM_SIZE);
			break;

		case WM_COMMAND:
				if (HIWORD(mp1)==BN_CLICKED || HIWORD(mp1)==EN_CHANGE) 
				{
					int cmd= LOWORD(mp1);

					switch (cmd) {
						case 5001: // bios
						case 5002: // smart
								   break;

						case 5010: // show window
								   ::ShowWindow(this->hwndDialog, TRUE);
								   ::SetForegroundWindow(this->hwndDialog);
								   break;

						case 5050: // donate
								   ::ShellExecute(NULL, "open", Title4, //"http://www.staff.uni-marburg.de/~schmitzr/donate.html", 
									   NULL, NULL, SW_SHOW);
								   break;

						case 5070: // show old temp icon
								   this->ShowTempIcon = NULL;
								   this->pTaskbarIcon= new	TASKBARICON(this->hwndDialog, 15, Title2);
								   //this->pTaskbarIcon->SetIcon(this->iFarbeIconB);
								   break;

						case 5080: // show temp icon
									delete this->pTaskbarIcon;
									this->pTaskbarIcon= NULL;
									this->ShowTempIcon = 1;
									break;

						case 5030: // hide window
								::CloseWindow(this->hwndDialog);
								break;

						case 5020: // end program
							::KillTimer(this->hwndDialog, m_titleTimer);
							::PostMessage(hwnd, WM__DISMISSDLG, IDCANCEL, 0); // exit from ProcessDialog() 
							break;
					}
				}
				break;

		case WM_CLOSE:
			::CloseWindow(this->hwndDialog);
				break;
		case WM_MOVE:
				break;
		case WM_SIZE:
				if (mp1==SIZE_MINIMIZED) {
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
			if (!this->hThread)
			{
				this->hThread= this->CreateThread(FANCONTROL_Thread, (ULONG)this);
			}
			break;


		case WM__NEWDATA:
	/*			if (this->hThread) {
					::WaitForSingleObject(this->hThread, INFINITE);
					::CloseHandle(this->hThread);
					this->hThread= 0;
				}
	*/
			break;

		case WM__TASKBAR:

				switch (mp2) 
				{
				    case WM_LBUTTONDOWN:
						if (!IsWindowVisible(this->hwndDialog))
						{
								::ShowWindow(this->hwndDialog, TRUE);
								::SetForegroundWindow(this->hwndDialog);
						}
						else	::CloseWindow(this->hwndDialog);
				        break;
				    case WM_LBUTTONUP:
						{
						BOOL 
						isshift= ::GetAsyncKeyState(VK_SHIFT) & 0x8000,
						isctrl= ::GetAsyncKeyState(VK_CONTROL) & 0x8000;
									int action= -1;
								// some fancy key dependent stuff could be done here
						}
						break;
				    case WM_LBUTTONDBLCLK:
						if (!IsWindowVisible(this->hwndDialog)){
								::ShowWindow(this->hwndDialog, TRUE);
								::SetForegroundWindow(this->hwndDialog);
						}
						else	::CloseWindow(this->hwndDialog);
				        break;
				    case WM_RBUTTONDOWN:
								{
								MENU m(5000);
									if (!IsWindowVisible(this->hwndDialog))
										m.DeleteMenuItem(5030);
									if (IsWindowVisible(this->hwndDialog))
										m.DeleteMenuItem(5010);
									if (this->ShowTempIcon == 0)
										m.DeleteMenuItem(5070);
									if (this->ShowTempIcon == 1)
										m.DeleteMenuItem(5080);
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
//  reading the Icon status may take a while, hence do it in a thread
//-------------------------------------------------------------------------
int 
FANCONTROL::WorkThread()
{ 

	while (TRUE) {
	//int ok= this->ReadEcStatus(&this->State);
//-------------------------------------------------------------------
					if (_lpiscreated == FALSE)
					{
						hPipe = CreateFile
						( 
						g_szPipeName,   // pipe name 
						GENERIC_READ,   // read access 
						FILE_SHARE_READ,// sharing 
						NULL,           // default security attributes
						OPEN_EXISTING,  // opens existing pipe 
						0,              // default attributes 
						NULL);          // no template file 
     

						if (INVALID_HANDLE_VALUE == hPipe)
						{
							this->Trace("Error while connecting to the server");
							_piscreated = FALSE;
							_isConnected = FALSE;
							iFarbeIconB = 15;
							dishow = TRUE;
							dioicon = TRUE;
							Sleep(2000);
						}
						else
						{
							this->Trace("Connecting to server was successful.");
							iFarbeIconB = 11;
							_piscreated = TRUE;
							_isConnected = TRUE;
						}
					}
					tried2connect = TRUE;
					if (_lpiscreated != _piscreated)
					{
						_hasShownConnected = FALSE;
						_hasShownDisconnected = FALSE;
					}
					_lpiscreated = _piscreated ;

					if (_lpiscreated == TRUE) //Read server message
					{
						strcpy_s(szBuffer,sizeof(szBuffer), "");
						bResult = ReadFile( 
						hPipe,                // handle to pipe 
						szBuffer,             // buffer to receive data 
						sizeof(szBuffer),     // size of buffer 
						&cbBytes,             // number of bytes read 
						NULL);                // not overlapped I/O

						if ( (!bResult) || (0 == cbBytes))
						{
							this->Trace("Error occurred while reading from server"); 
							CloseHandle(hPipe);
							_piscreated = FALSE;
							_isConnected = FALSE;
							iFarbeIconB = 15;
						}
						else // Output the data read
						{
							_isConnected = TRUE;
							sscanf_s(szBuffer, "%d %d %s %d %d %d",
								&CurrentMode, &MaxTemp, &SensorName, sizeof(SensorName),
								&this->iFarbeIconB, &fanspeed, &fanstate);
							switch (CurrentMode)
							{
							case 1: strcpy_s(CCurrentMode,sizeof(CCurrentMode), "BIOS"); break;
							case 2: strcpy_s(CCurrentMode,sizeof(CCurrentMode), "Smart"); break;
							case 3: strcpy_s(CCurrentMode,sizeof(CCurrentMode), "FIXED"); break;
							default: strcpy_s(CCurrentMode,sizeof(CCurrentMode), "BIOS"); break;
							}
							switch (iFarbeIconB)
							{
							case 10: strcpy_s(CCurrentICON,sizeof(CCurrentICON), "gray"); break;
							case 11: strcpy_s(CCurrentICON,sizeof(CCurrentICON), "blue"); break;
							case 12: strcpy_s(CCurrentICON,sizeof(CCurrentICON), "yellow"); break;
							case 13: strcpy_s(CCurrentICON,sizeof(CCurrentICON), "orange"); break;
							case 14: strcpy_s(CCurrentICON,sizeof(CCurrentICON), "red"); break;
							case 21: strcpy_s(CCurrentICON,sizeof(CCurrentICON), "vlgreen"); break;
							case 22: strcpy_s(CCurrentICON,sizeof(CCurrentICON), "lgreen"); break;
							case 23: strcpy_s(CCurrentICON,sizeof(CCurrentICON), "green"); break;
							case 24: strcpy_s(CCurrentICON,sizeof(CCurrentICON), "dgreen"); break;
							case 25: strcpy_s(CCurrentICON,sizeof(CCurrentICON), "vdgreen"); break;
							default: strcpy_s(CCurrentICON,sizeof(CCurrentICON), "gray"); break;
							}
							strcpy_s(str_value,sizeof(str_value), "");
							sprintf_s(str_value, sizeof(str_value),
								"Mode= %s Temp= %d Sensor= %s IconColor= %s Speed= %d State= %d", 
								CCurrentMode, MaxTemp, SensorName, CCurrentICON, fanspeed, fanstate);
							this->Trace(str_value);
							strcpy_s(Title2,sizeof(Title2), "");
							sprintf_s(this->Title2, sizeof(this->Title2), "%d° Fan %d (%s) %d rpm ",
								this->MaxTemp, fanstate, CCurrentMode, fanspeed);
						}
					}
					//	_lpiscreated = _piscreated ;

//end named pipe client session
//-------------------------------------------------------------------
	}
	::PostMessage(this->hwndDialog, WM__NEWDATA, 1, 0);

	return 0;
}
// The texticons will be shown depending on variables
static const int MAX_TEXT_ICONS = 16;
int icon;
TCHAR myszTip[64] ;
void FANCONTROL::ProcessTextIcons()

{	
		lstrcpyn(myszTip, this->Title2, sizeof(myszTip)-1);

		//INIT ppTbTextIcon
        if(!ppTbTextIcon || this->TaskbarNew) 
		{	this->TaskbarNew=0;
            ppTbTextIcon = new CTaskbarTextIcon*[MAX_TEXT_ICONS];
            for(int i = 0;i<MAX_TEXT_ICONS;++i) 
			{
                ppTbTextIcon[i] = NULL;
			}
            ppTbTextIcon[0] = new CTaskbarTextIcon(this->m_hinstapp,		//erstmal nur eins
                    this->hwndDialog,WM__TASKBAR,0,"","",			//WM_APP+5000 -> WM__TASKBAR
					this->iFarbeIconB, myszTip);
		}
		for (int i = 0;i<MAX_TEXT_ICONS;++i)     //aktualisieren
			{
				if(ppTbTextIcon[i]) 
				{
					if (_isConnected == FALSE) lstrcpyn(myszTip, "not connected to tpfc service", sizeof(myszTip)-1);
					_itoa_s(MaxTemp,str_value,sizeof(str_value),10);
					sprintf_s(str_value, sizeof(str_value),"%s",str_value); 
					ppTbTextIcon[i]->ChangeText(
						str_value, SensorName,
						this->iFarbeIconB, myszTip);
				}
			}

			if (_isConnected == TRUE && _hasShownConnected == FALSE){
				ppTbTextIcon[0]->DiShowballon(_T("connected to TPFanControl service"),
					_T("TPFanControl Icon"),NIIF_INFO,11);
				_hasShownConnected = TRUE;}
			if (_isConnected == FALSE && _hasShownDisconnected == FALSE){
					ppTbTextIcon[0]->DiShowballon(_T("NOT connected to TPFanControl service"),
						_T("TPFanControl Icon"),NIIF_ERROR,11);
					_hasShownDisconnected = TRUE;} 
}  

void FANCONTROL::RemoveTextIcons(void) 
{
	if(ppTbTextIcon)
	{
		for(int i = 0;i<MAX_TEXT_ICONS;++i)
		{
			if(ppTbTextIcon[i])
			{
				delete ppTbTextIcon[i];
			}
		}
		delete[] ppTbTextIcon;
		ppTbTextIcon = NULL;
	}
}
