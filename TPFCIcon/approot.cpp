

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
