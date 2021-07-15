
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

HINSTANCE hInstApp, 
		  hInstRes;

HWND g_dialogWnd = NULL;

int APIENTRY
WinMain(HINSTANCE instance, HINSTANCE , LPSTR aArgs, int )
{
	DWORD rc= 0;

	hInstRes=GetModuleHandle(NULL);
	hInstApp=hInstRes;


	::InitCommonControls();

		FANCONTROL fc(hInstApp);

        g_dialogWnd = fc.GetDialogWnd();

		fc.ProcessDialog();

		::PostMessage(g_dialogWnd, WM_COMMAND, 5020, 0);
	
}
