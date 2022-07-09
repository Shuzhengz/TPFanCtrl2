#include "_prec.h"

//begin named pipe TPFanControl01
#define g_szPipeName "\\\\.\\Pipe\\TPFanControl01"  //Name given to the pipe
HANDLE hPipe1;
HANDLE hPipe2;
HANDLE hPipe3;
HANDLE hPipe4;
//Pipe name format - \\.\pipe\pipename

HINSTANCE hInstApp, hInstRes;

LPSTR g_ServiceName = "TPFanControl";
SERVICE_STATUS g_SvcStatus = {0};
SERVICE_STATUS_HANDLE g_SvcHandle = NULL;
HWND g_dialogWnd = NULL;
HANDLE g_workerThread = NULL;

void ShowError(DWORD ec, const char *description);
void ShowMessage(const char *title, const char *description);
void ShowHelp();
DWORD InstallService(bool quiet);
DWORD UninstallService(bool quiet);
VOID WINAPI Handler(DWORD fdwControl);
VOID WINAPI ServiceMain(DWORD aArgc, LPTSTR* aArgv);
void StartWorkerThread();
void StopWorkerThread();
void WorkerThread(void *dummy);

void debug(const char *msg);