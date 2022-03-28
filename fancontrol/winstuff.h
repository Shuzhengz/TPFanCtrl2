
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

#ifndef WINCONTROLS_H
#define WINCONTROLS_H

#pragma once

#include "_prec.h"


#ifndef NIF_STATE			// NotifyIcon V5 defines
	#define NIF_STATE       0x00000008
	#define NIF_INFO        0x00000010
	#define NIF_GUID        0x00000020

	#define NIIF_INFO       0x00000001
	#define NIIF_WARNING    0x00000002
	#define NIIF_ERROR      0x00000003
	#define NIIF_ICON_MASK  0x0000000F
	#define NIIF_NOSOUND    0x00000010
#endif



//-------------------------------------------------------------------------
//
//-------------------------------------------------------------------------
class MUTEXSEM {

    private:
        HANDLE hmux;

        MUTEXSEM(MUTEXSEM &);               // disallow argument passing
        void operator=(MUTEXSEM &) { };     // disallow assignment

    protected:

    public:
        MUTEXSEM(int state= FALSE, const char *name= "Access_Thinkpad_EC");
        ~MUTEXSEM();
        int Lock(int millies= 0xFFFFFFFF);
        void Unlock();

        HANDLE Handle(void)
            { return this->hmux; }
        operator HANDLE(void)
            { return this->hmux; }
};


class TASKBARICON {

	private:
		BOOL UpAndRunning;

	protected: 
		HWND Owner;
		int Id, IconId;
		char Tooltip[256];
		int osVersion; //TODO: war drin?
		volatile BOOL InsideTooltipRebuild; //should be declared as volatile for concurrency reasons


	public:
		TASKBARICON(HWND hwndowner, int id, const char *tooltip);
		~TASKBARICON(void);

		BOOL Construct(void);
		void Destroy(BOOL keep= FALSE);

		BOOL RebuildIfNecessary(BOOL);
		BOOL HasExtendedFeatures(void);

		BOOL IsUpAndRunning(void);
		int SetIcon(int iconid);
		int GetIcon(void);
		int SetTooltip(const char *tip);
		int SetBalloon(ULONG flags, const char *title, const char *text, int timeout= 2);
}; 


class MENU {

	private:

	protected: 
		BOOL IsLoaded;
		HMENU hMenu;
		HWND hWndOwner;

	public:
		MENU(HWND hwnd);
		MENU(HMENU hm);
		MENU(int id, HINSTANCE hdll= (HINSTANCE)(ULONG)-1);
		~MENU()
			{ if (this->IsLoaded) ::DestroyMenu(this->hMenu); this->hMenu= NULL; }

		operator HMENU(void) const
				{ return (HMENU)this->hMenu; }

		int GetNumMenuItems(void);
		BOOL IsMenuItemSeparator(int pos);

		void EnableMenuItem(int id, int status= TRUE);
		void DisableMenuItem(int id);
		int DeleteMenuItem(int id, BOOL idispos= FALSE);
		BOOL InsertItem(const char *text, int id, int pos= -1);
		BOOL InsertMenuItem(const char *text, int id, int pos= -1)
				{ return this->InsertItem(text, id, pos); }
		BOOL InsertSeparator(int pos)
				{ return this->InsertItem(NULL, 0, pos); }
		void CheckMenuItem(int id, int status= TRUE);
		void UncheckMenuItem(int id);
		BOOL IsFlags(int id, int flags);
		BOOL IsMenuItemEnabled(int id);
		BOOL IsMenuItemDisabled(int id);
		BOOL IsMenuItemChecked(int id);
	
		HMENU GetSubmenuFromPos(int pos);
		int GetMenuPosFromID(int id);

		int Popup(HWND hwnd, POINT *ppoint= NULL, BOOL synchtrack= FALSE);
};

#endif // WINCONTROLS_H