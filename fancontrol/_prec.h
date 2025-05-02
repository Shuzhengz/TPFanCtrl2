//systemheaders in one file for using precompiled headers.

// be compatible downto Windows Server 2003 SP1
#define _WIN32_WINNT 0x0502
//only most neccessary things from windows
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <process.h>
#include <commctrl.h>
#include <commdlg.h>
#include <shellapi.h>
#include <tchar.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "winuser.h"
#include "windows.h"
