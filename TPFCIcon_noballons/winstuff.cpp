
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
#include "winstuff.h"
#include "DynamicIcon.h"

extern HINSTANCE hInstApp, hInstRes;

//////////////////////////////////////////////////////////////////////////////
//                                                                          // 
//   TASKBARICON                                                            // 
//                                                                          // 
//                                                                          // 
//                                                                          // 
//                                                                          // 
//////////////////////////////////////////////////////////////////////////////

struct NOTIFYICONDATAV5 {
	NOTIFYICONDATA nof;
	TCHAR szTipExtra[64];	//Version 5.0
    DWORD dwState;			//Version 5.0
    DWORD dwStateMask;		//Version 5.0
    TCHAR szInfo[256];		//Version 5.0
    union {
        UINT  uTimeout;		//Version 5.0
        UINT  uVersion;		//Version 5.0
    } DUMMYUNIONNAME;
    TCHAR szInfoTitle[64];	//Version 5.0
    DWORD dwInfoFlags;		//Version 5.0
};

struct NOTIFYICONDATAV6 {
	DWORD cbSize;
    HWND hWnd;
    UINT uID;
    UINT uFlags;
    UINT uCallbackMessage;
    HICON hIcon;
    TCHAR szTip[64];
    DWORD dwState;
    DWORD dwStateMask;
    TCHAR szInfo[256];
    union {
        UINT uTimeout;
        UINT uVersion;
    };
    TCHAR szInfoTitle[64];
    DWORD dwInfoFlags;
    GUID guidItem;
    HICON hBalloonIcon;
};

struct OSVERSIONINFOV4 {  
    DWORD dwOSVersionInfoSize;  
    DWORD dwMajorVersion;  
    DWORD dwMinorVersion;  
    DWORD dwBuildNumber;  
    DWORD dwPlatformId;  
    TCHAR szCSDVersion[128];
};

//-------------------------------------------------------------------------
//  Represent a window in the taskbar
//-------------------------------------------------------------------------
TASKBARICON::TASKBARICON(HWND hwndowner, int idicon, const char *tooltip)
	: Owner(hwndowner), 
		Id(idicon), 
		IconId(idicon),
		osVersion(0)
{
	// ampersand must be escaped
	strcpy_s(this->Tooltip,sizeof(Tooltip), tooltip ? tooltip : "");

	this->Construct();
}

TASKBARICON::~TASKBARICON()
{
	this->Destroy();
}


BOOL 
TASKBARICON::Construct()
{
	NOTIFYICONDATAV5 nofv5= NULLSTRUCT;
	NOTIFYICONDATA &nof= nofv5.nof;

	this->osVersion= 0;

	nof.cbSize= sizeof(nof);
	nof.hWnd= this->Owner;
	nof.uID= this->Id;
	nof.uFlags= NIF_MESSAGE;
	nof.uCallbackMessage= WM__TASKBAR;

	if (this->IconId) {
		nof.hIcon= (HICON)::LoadImage(hInstRes, MAKEINTRESOURCE(this->IconId), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
		nof.uFlags|= NIF_ICON;
	}

	if (strlen(this->Tooltip)) {
		lstrcpyn(nof.szTip, this->Tooltip, sizeof(nof.szTip)-1);
		nof.uFlags|= NIF_TIP;
	}

	//
	// try a version 5 init (Shell_NotifyIcon has different behaviour from up win2000=version 5)
	// 
	nof.cbSize= sizeof(nofv5);
	this->UpAndRunning= ::Shell_NotifyIcon(NIM_ADD, &nof);

	if (this->UpAndRunning) 
		this->osVersion= 5;
	else
		this->UpAndRunning= ::Shell_NotifyIcon(NIM_ADD, &nof);

	if (nof.hIcon) {
		::DestroyIcon(nof.hIcon);
		nof.hIcon= NULL;
	}

	return this->UpAndRunning;
}

void
TASKBARICON::Destroy(BOOL keep)
{
	NOTIFYICONDATA nof= NULLSTRUCT;

	nof.cbSize= sizeof(nof);
	nof.hWnd= this->Owner;
	nof.uID= this->Id;
	::Shell_NotifyIcon(NIM_DELETE, &nof);

	if (!keep) {
		this->Owner= 0;
		this->Id= 0;
		this->IconId= 0;
		strcpy_s(this->Tooltip,sizeof(Tooltip), "");
	}
}

BOOL
TASKBARICON::IsUpAndRunning()
{
	return this->UpAndRunning;
}

BOOL 
TASKBARICON::RebuildIfNecessary(BOOL force)
{
	char tt[256];

	strcpy_s(tt,sizeof(tt), this->Tooltip); // avoid selfassignment

	if (force || !this->SetTooltip(tt)) {
		this->Destroy(TRUE);
		this->Construct();
	}

	return this->SetTooltip(tt);
}


int 
TASKBARICON::SetIcon(int iconid)
{
	BOOL ok;
	NOTIFYICONDATA nof= NULLSTRUCT;

	this->IconId= iconid;

	nof.cbSize= sizeof(nof);
	nof.hWnd= this->Owner;
	nof.uID= this->Id;
	nof.uFlags= NIF_ICON;
	nof.hIcon= (HICON)::LoadImage(hInstRes, MAKEINTRESOURCE(this->IconId), IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);

	ok= ::Shell_NotifyIcon(NIM_MODIFY, &nof);

	if (nof.hIcon) {
		::DestroyIcon(nof.hIcon);
		nof.hIcon= NULL;
	}

	// try to rebuild if SetIcon failed
	if (!ok)
		this->RebuildIfNecessary(TRUE);

	return ok;
}

int 
TASKBARICON::SetTooltip(const char *tooltip)
{
	BOOL ok= 0;

	
	if (strcmp(this->Tooltip, tooltip)!=0) {
		strcpy_s(this->Tooltip,sizeof(Tooltip), tooltip);


		NOTIFYICONDATA nof= NULLSTRUCT;

		nof.cbSize= sizeof(nof);
		nof.hWnd= this->Owner;
		nof.uID= this->Id;
		nof.uFlags= NIF_TIP;
		lstrcpyn(nof.szTip, this->Tooltip, sizeof(nof.szTip)-1);


		ok= ::Shell_NotifyIcon(NIM_MODIFY, &nof);

		// try to rebuild if SetTooltip failed
		if (!ok && !this->InsideTooltipRebuild) {
			this->InsideTooltipRebuild= TRUE;
			this->RebuildIfNecessary(TRUE);
			this->InsideTooltipRebuild= FALSE;
		}
	}
	return ok;
}


int 
TASKBARICON::SetBalloon(ULONG flags, const char *title, const char *text, int timeout)
{
	BOOL ok;

	NOTIFYICONDATA nof= NULLSTRUCT;

	nof.cbSize= sizeof(NOTIFYICONDATA);
	nof.hWnd= this->Owner;
	nof.uID= this->Id;
	nof.uFlags= NIF_INFO;
	nof.dwInfoFlags = flags;
	nof.uTimeout = timeout;
	lstrcpyn(nof.szInfo, text, sizeof(nof.szInfo)-1);
	lstrcpyn(nof.szInfoTitle, title, sizeof(nof.szInfoTitle)-1);

	return Shell_NotifyIcon(NIM_MODIFY, &nof);

	// try to rebuild if SetBalloon failed

	if (!ok)

		this->RebuildIfNecessary(TRUE);

	return ok;
}


//////////////////////////////////////////////////////////////////////////////
//                                                                          // 
//   MENU                                                                   // 
//                                                                          // 
//                                                                          // 
//                                                                          // 
//                                                                          // 
//////////////////////////////////////////////////////////////////////////////

MENU::MENU(HWND w)
	: hMenu(::GetMenu(w)), 
		hWndOwner(w),
		IsLoaded(FALSE)
{

}

MENU::MENU(HMENU hmenu)
	: hMenu(hmenu),
		hWndOwner(NULL),
		IsLoaded(FALSE)
{

}

MENU::MENU(int id, HINSTANCE hdll)
	: hMenu(::LoadMenu(((ULONG)hdll==(ULONG)-1)? hInstRes : hdll, MAKEINTRESOURCE(id))), 
		hWndOwner(NULL),
		IsLoaded(TRUE)
{

}


void
MENU::EnableMenuItem(int id, int status)
{
	::EnableMenuItem(*this, id, 
					MF_BYCOMMAND | (status ? MF_ENABLED : MF_GRAYED));
}

void
MENU::DisableMenuItem(int id)
{
	this->EnableMenuItem(id, FALSE);
}

int
MENU::DeleteMenuItem(int id, BOOL idispos)
{
	int rc= ::DeleteMenu(*this, id, idispos ? MF_BYPOSITION : MF_BYCOMMAND);
	if (this->hWndOwner)
		::DrawMenuBar(this->hWndOwner);

	return rc;
}

void
MENU::CheckMenuItem(int id, int status)
{
	::CheckMenuItem(*this, id, 	
					MF_BYCOMMAND | (status ? MF_CHECKED : MF_UNCHECKED));
}

void
MENU::UncheckMenuItem(int id)
{
	this->CheckMenuItem(id, FALSE);
}

BOOL
MENU::IsFlags(int id, int flags)
{
	return ((::GetMenuState(*this, id, MF_BYCOMMAND) & flags)!=0);
}

BOOL
MENU::IsMenuItemSeparator(int pos)
{
	MENUITEMINFO mii= {sizeof(mii), MIIM_TYPE, };
	::GetMenuItemInfo(*this, pos, TRUE, &mii);
	return mii.fType==MFT_SEPARATOR;
}

BOOL
MENU::IsMenuItemEnabled(int id)
{
	return !this->IsFlags(id, MF_DISABLED|MF_GRAYED);
}

BOOL
MENU::IsMenuItemDisabled(int id)
{
	return this->IsFlags(id, MF_DISABLED|MF_GRAYED);
}

BOOL
MENU::IsMenuItemChecked(int id)
{
	return this->IsFlags(id, MF_CHECKED);
}

int
MENU::GetNumMenuItems()
{
	return ::GetMenuItemCount(*this);
}


//--------------------------------------------------------------------
//  return the sub-menu handle of a menu item at a given position
//--------------------------------------------------------------------
HMENU 
MENU::GetSubmenuFromPos(int pos)
{
	HMENU rc= NULL;

	rc= (HMENU)GetSubMenu(*this, pos);

	return rc;
}


//--------------------------------------------------------------------
//  return the item pos of a menu entry (search by id)
//--------------------------------------------------------------------
int 
MENU::GetMenuPosFromID(int id)
{
	int rc= -1;

	int i, mid, numof= ::GetMenuItemCount(*this);

	for (i=0; i<numof; i++) {
		mid= ::GetMenuItemID(*this, i);
		if (mid==id) {
			rc= i;
			break;
		}
	}

	return rc;
}


//-------------------------------------------------------------------------
//  
//-------------------------------------------------------------------------
BOOL
MENU::InsertItem(const char *text, int id, int pos)
{
	MENUITEMINFO mi= NULLSTRUCT;
	mi.cbSize= sizeof(mi);
	mi.fMask= MIIM_TYPE | MIIM_ID;
	mi.wID= id;

	if (!text) {
		mi.fType= MFT_SEPARATOR;
	}
	else {
		mi.fMask|= MIIM_DATA;
		mi.fType= MFT_STRING;
		mi.dwTypeData= (char *)text;
	}

	return ::InsertMenuItem(*this, pos, TRUE, &mi);
}


//-------------------------------------------------------------------------
//  
//-------------------------------------------------------------------------
int
MENU::Popup(HWND hwndowner, POINT *ppoint, BOOL synchtrack)
{
	POINT point;
	HMENU hmenu, hmenuShow;

	if (ppoint) 
		point= *ppoint;
	else
		::GetCursorPos(&point);

	hmenu= CreateMenu();
	::AppendMenu(hmenu, MF_POPUP | MF_STRING, (UINT)this->hMenu, "BLUB");
	hmenuShow= ::GetSubMenu(hmenu, 0);
	RECT r= { 0, 0, 10, 10 };

	if (hwndowner)
		::SetForegroundWindow(hwndowner);


	ULONG flags= TPM_LEFTALIGN | TPM_LEFTBUTTON;

	if (synchtrack & 1)	
		flags|= TPM_RETURNCMD;

	if (synchtrack & TPM_RIGHTALIGN)	
		flags|= TPM_RIGHTALIGN;

	int rc= ::TrackPopupMenu(hmenuShow, flags,
									point.x,point.y, 0, 
									hwndowner, &r);

	if (hwndowner)
		::PostMessage(hwndowner, WM_NULL, 0, 0);

	::RemoveMenu(hmenu, 0, MF_BYPOSITION);
	::DestroyMenu(hmenu);

	return rc;
}
