#include <windows.h>
#include <stdio.h>
#include <direct.h>
#include "..\lconsole\lc_sys.h"

#ifdef _DEBUG
#define DLIB_PATH "..\\lconsole\\debug\\lc_sys_d.dll"
#else /*_DEBUG*/
#define DLIB_PATH "lc_sys.dll"
#endif /*_DEBUG*/


extern ObtainFunctions();


typedef struct tagEXTRADATAEX{
	HWND hwnd;
}EXTRADATAEX, *LPEXTRADATAEX;

#define CONF_QUIT   0x00000010
#define CONF_GETDIR 0x00000020

extern LPCON_CREATE pCon_Create;
extern LPCON_DELETE pCon_Delete;
extern LPCON_SENDMESSAGE pCon_SendMessage;
extern LPCON_SENDERRORMSG pCon_SendErrorMsg;
extern LPCON_SENDCOMMAND pCon_SendCommand;
extern LPCON_SETCURRENT pCon_SetCurrent;
extern LPCON_ERASECURRENT pCon_EraseCurrent;
extern LPCON_CLEAR pCon_Clear;
extern LPCON_GETNUMENTRIES pCon_GetNumEntries;
extern LPCON_GETENTRY pCon_GetEntry;
extern LPCON_REGISTERCMD pCon_RegisterCmd;
extern LPCON_ONCHAR pCon_OnChar;
extern LPCON_ATTACHCVARLIST pCon_AttachCVarList;
extern LPCON_CREATEDLGBOX pCon_CreateDlgBox;

extern LPCCPARSE_GETPARAM pCCParse_GetParam;
extern LPCCPARSE_GETFLOAT pCCParse_GetFloat;
extern LPCCPARSE_GETINT pCCParse_GetInt;
extern LPCCPARSE_CHECKPARAM pCCParse_CheckParam;

extern LPCVAR_CREATELIST pCVar_CreateList;
extern LPCVAR_DELETELIST pCVar_DeleteList;
extern LPCVAR_SET pCVar_Set;
extern LPCVAR_SETVALUE pCVar_SetValue;
extern LPCVAR_GET pCVar_Get;
extern LPCVAR_GETVALUE pCVar_GetValue;
extern LPCVAR_GETCVAR pCVar_GetCVar;
extern LPCVAR_GETFIRSTCVAR pCVar_GetFirstCVar;
extern LPCVAR_ADDDEF pCVar_AddDef;
extern LPCVAR_GETDEF pCVar_GetDef;


int ConsoleCommand(
	unsigned long nCommand, 
	const char* szParams, 
	HLCONSOLE hConsole, 
	void* pExtraData)
{
	switch(nCommand)
	{
	case CONF_QUIT:
		SendMessage(((LPEXTRADATAEX)pExtraData)->hwnd, WM_CLOSE, 0, 0);
		break;
	case CONF_GETDIR:
	{
		char szDir[1024];
		_getdcwd(_getdrive(), szDir, 1024);
		pCon_SendErrorMsg(hConsole, "DIR=\"%s\"", szDir);
		break;
	}
	default:
		return 0;
	}
	return 1;
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nShowCmd)
{
	MSG msg;
	HWND hwnd=NULL;
	HLCONSOLE hConsole=NULL;
	HCVARLIST cvarlist=NULL;
	HMODULE hDllFile=NULL;
	HGLOBAL hDlg=NULL;

	EXTRADATAEX data={NULL};

	/* Load the library and all the functions. */
	hDllFile=LoadLibrary(DLIB_PATH);

	if(hDllFile==NULL)
	{
		MessageBox(
			NULL, 
			"Failed to load "DLIB_PATH, 
			"lconsoledemo.exe", 
			MB_OK|MB_ICONERROR);
		return 0;
	}
	if(!ObtainFunctions(hDllFile))
	{
		MessageBox(
			NULL,
			"Could not load functions from library!",
			"lconsoledemo.exe",
			MB_OK|MB_ICONERROR);

		FreeLibrary(hDllFile);
		return 0;
	}


	/* First thing to do is start the Legacy Console. */
	if((hConsole=pCon_Create(ConsoleCommand, 256, 2048, CONCREATE_USEINTERNAL, &data))==0)
		return 0;
	/* Create a cvarlist and attach it to the console. */
	cvarlist=pCVar_CreateList(hConsole);
	pCon_AttachCVarList(hConsole, cvarlist);
	/* Register our custom commands. */
	pCon_RegisterCmd(hConsole, "QUIT", CONF_QUIT);
	pCon_RegisterCmd(hConsole, "GETDIR", CONF_GETDIR);

	/* Create our window, and attach the console too it. */
	data.hwnd=hwnd=pCon_CreateDlgBox();
	SendMessage(hwnd, WM_USER_INSTALLCONSOLE, 0, (LPARAM)hConsole);
	/* Not that if we send a message to the console, we have to tell the
		dialog box to update, or it won't know that it needs to. */
	pCon_SendMessage(hConsole, "Successfully initialized lconsoledemo.exe!");
	SendMessage(hwnd, WM_USER_UPDATE, 0 ,0);


	ShowWindow(hwnd, nShowCmd);
	UpdateWindow(hwnd);

	while(GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	/* Free our allocated resources. */
	pCon_Delete(hConsole);
	pCVar_DeleteList(cvarlist);
	FreeLibrary(hDllFile);
	return msg.wParam;
}