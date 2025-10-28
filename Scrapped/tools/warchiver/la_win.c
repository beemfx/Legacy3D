#include <windows.h>
#include <shlobj.h>
#include <process.h>
#include <stdio.h>
#include "resource.h"

#include "..\\filesys\\lf_sys.h"

#define ARCHIVE_COMPRESS 0x00000010

#pragma comment(lib, "winmm.lib")

#ifdef _DEBUG
#define LIBA_PATH "..\\filesys\\debug\\lf_sys_d.dll"
#else
#define LIBA_PATH "lf_sys.dll"
#endif

static char g_szAppName[]="WinArchiver v1.01";

#define WM_USER_DONEARCHIVING (WM_USER+1)


int (*pArc_Archive)(char*, char*, unsigned long, int (*)(void*, unsigned int, char*));


int SetPos(void* extra, unsigned int nPercentage, char* szOutput)
{
	static HWND hwnd=NULL;
	static HWND hwndt=NULL;
	if(extra)
	{
		hwnd=extra;

		SendMessage(hwnd, PBM_SETRANGE, 0, MAKELPARAM(0, 100));
		SendMessage(hwnd, PBM_SETPOS, nPercentage, 0);
		return 1;
	}
	else
	{
		HWND hwndp=GetParent(hwnd);
		char szTemp[280];

		if(nPercentage>100)
			nPercentage=100;
		
		if(hwnd)
			SendMessage(hwnd, PBM_SETPOS, nPercentage, 0);

		_snprintf(szTemp, 279, "Archiving \"%s\"...", szOutput);
		SetDlgItemText(hwndp, IDC_ARCHFILE, szTemp);
		return 1;
	}
}

typedef struct _APARAMS{
	HWND hwnd;
	char szOutput[MAX_PATH];
	int bCompress;
}APARAMS, *PAPARAMS;

typedef struct _AWPARAMS{
	int bCompress;
	char szSaveFile[MAX_PATH];
}AWPARAMS;

void ArchiveProc(void* pvoid)
{
	unsigned long dwFlags=0;

	APARAMS* pparams=(APARAMS*)pvoid;

	if(pparams->bCompress)
		dwFlags|=ARCHIVE_COMPRESS;

	if(!pArc_Archive(pparams->szOutput, ".", dwFlags, SetPos))
			MessageBox(
				NULL, 
				"An error occured while attempting to archive the files.", 
				g_szAppName, 
				MB_OK|MB_ICONINFORMATION);
	

	PostMessage(pparams->hwnd, WM_USER_DONEARCHIVING, 0, 0);
	_endthread();
}


BOOL CALLBACK ArcWait(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static char szOutput[MAX_PATH];
	static APARAMS params;
	static HFONT hfont=L_null;
	unsigned long hThreadID=0;

	switch(uMsg)
	{
	case WM_INITDIALOG:
	{
		DWORD dwID=0;
		HWND hwndt=GetDlgItem(hwnd, IDC_ARCHFILE);

		strncpy(params.szOutput, (char*)((AWPARAMS*)lParam)->szSaveFile, MAX_PATH-1);
		params.bCompress=((AWPARAMS*)lParam)->bCompress;
		params.hwnd=hwnd;
		SetPos(GetDlgItem(hwnd, IDC_ARCPROG), 0, "");
		hThreadID=_beginthread(ArchiveProc, 0, &params);

		hfont=CreateFont(
			12, 
			0,
			0,
			0,
			FW_THIN,
			0,
			0,
			0,
			ANSI_CHARSET,
			OUT_DEFAULT_PRECIS,
			CLIP_DEFAULT_PRECIS,
			DEFAULT_QUALITY,
			DEFAULT_PITCH,
			"MS Sans Serif");

		SendMessage(hwndt, WM_SETFONT, (WPARAM)hfont, 0);
		SendMessage(hwndt, WM_SETTEXT, 0, (LPARAM)"Preparing to archive...");
		/*
		GetWindowRect(GetParent(hwnd), &rc);
		MoveWindow(hwnd, (rc.right-rc.left)/2-100, (rc.bottom-rc.top)/2-50, 200, 100, TRUE);
		*/
		return FALSE;
	}
	case WM_DESTROY:
		DeleteObject(hfont);
		hfont=L_null;
		break;
	case WM_USER_DONEARCHIVING:
		PlaySound(TEXT("SystemAsterisk"), NULL, SND_ASYNC|SND_ALIAS);
		EndDialog(hwnd, 0);
		break;
	default:
		return FALSE;
	}
	return TRUE;
}


BOOL CommandProc(HWND hwnd, WORD wNotifyCode, WORD wID, HWND hwndCtl)
{
	static AWPARAMS awParams={0, 0};

	switch(wID)
	{
	case IDC_QUIT:
		SendMessage(hwnd, WM_CLOSE, 0, 0);
		break;
	case IDC_ARCHIVEIT:
	{
		char szTree[MAX_PATH];
		char szSaveFile[MAX_PATH];
		char szOldDir[MAX_PATH];
		szTree[0]=0;
		szSaveFile[0]=0;

		GetDlgItemText(hwnd, IDC_OUTPUT, szSaveFile, MAX_PATH-1);
		GetDlgItemText(hwnd, IDC_TREE, szTree, MAX_PATH-1);

		if(strlen(szSaveFile)<2 || strlen(szTree)<2)
		{
			MessageBox(hwnd, "A specified path is too short.", g_szAppName, MB_OK|MB_ICONINFORMATION);
			break;
		}
		//Change the current directory to the directory we want to archive.
		GetCurrentDirectory(MAX_PATH-1, szOldDir);
		if(!SetCurrentDirectory(szTree))
		{
			MessageBox(hwnd, "The specified tree for archiving is not valid.", g_szAppName, MB_OK|MB_ICONINFORMATION);
			break;
		}

		if(IsDlgButtonChecked(hwnd, IDC_COMPRESS))
			awParams.bCompress=1;
		else
			awParams.bCompress=0;

		strcpy(awParams.szSaveFile, szSaveFile);

		DialogBoxParam(GetModuleHandle(NULL), "ARCHIVING", hwnd, ArcWait, (LPARAM)&awParams);

		//Restore the old working directory.
		SetCurrentDirectory(szOldDir);
		break;
	}
	case IDC_BROWSEOUTPUT:
	{
		OPENFILENAMEA of;
		char szOutput[MAX_PATH];
		memset(&of, 0, sizeof(OPENFILENAMEA));
		szOutput[0]=0;

		of.lStructSize=sizeof(OPENFILENAMEA);
		of.hwndOwner=hwnd;
		of.hInstance=0;
		of.lpstrFilter="Legacy Archive (*.lpk)\0*.lpk\0All Filess (*.*)\0*.*\0";
		of.lpstrCustomFilter=NULL;
		of.nMaxCustFilter=0;
		of.nFilterIndex=0;
		of.lpstrFile=szOutput;
		of.nMaxFile=MAX_PATH;
		of.lpstrFileTitle=NULL;
		of.nMaxFileTitle=0;
		of.lpstrInitialDir=NULL;
		of.lpstrTitle="Archive Directory To:";
		of.Flags=OFN_NOCHANGEDIR|OFN_OVERWRITEPROMPT|OFN_HIDEREADONLY|OFN_NONETWORKBUTTON;
		of.nFileOffset=0;
		of.nFileExtension=0;
		of.lpstrDefExt="lpk";
		of.lCustData=0;
		of.lpfnHook=NULL;
		of.lpTemplateName=NULL;
		if(GetSaveFileNameA(&of))
			SetDlgItemText(hwnd, IDC_OUTPUT, szOutput);
			
		break;
	}
	case IDC_BROWSETREE:
	{
		BROWSEINFO bi;
		LPITEMIDLIST lpid=NULL;
		IMalloc* imalloc=NULL;

		char szOutput[MAX_PATH];
		memset(&bi, 0, sizeof(bi));
		bi.hwndOwner=hwnd;
		bi.pidlRoot=NULL;
		bi.pszDisplayName=szOutput;
		bi.lpszTitle="Select a directory tree for archiving.";
		bi.ulFlags=BIF_NONEWFOLDERBUTTON;
		bi.lpfn=NULL;
		bi.lParam=0;
		bi.iImage=0;
		if(lpid=SHBrowseForFolder(&bi))
		{
			SHGetPathFromIDList(lpid, szOutput);
			SetDlgItemText(hwnd, IDC_TREE, szOutput);

			if(SUCCEEDED(SHGetMalloc(&imalloc)))
			{
				imalloc->lpVtbl->Free(imalloc, lpid);
				imalloc->lpVtbl->Release(imalloc);
			}
		}
		break;
	}
	default:
		return FALSE;
	}
	return TRUE;
}

BOOL CALLBACK ArcDlgProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static HANDLE hIcon=NULL;

	switch(uMsg)
	{
	case WM_INITDIALOG:
	{
		RECT rc1, rc2;
		SetWindowText(hwnd, g_szAppName);
		GetWindowRect(GetDesktopWindow(), &rc1);
		GetWindowRect(hwnd, &rc2);
		MoveWindow(
			hwnd, 
			rc1.right/2-(rc2.right-rc2.left)/2, 
			rc1.bottom/2-(rc2.bottom-rc2.top), 
			rc2.right-rc2.left, 
			rc2.bottom-rc2.top, 
			TRUE);

		CheckDlgButton(hwnd, IDC_COMPRESS, BST_CHECKED);
		hIcon=LoadImage(GetModuleHandle(NULL), "WARCICON2", IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);
		SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
		return TRUE;
	}
	case WM_CLOSE:
		EndDialog(hwnd, 0);
		DestroyIcon(hIcon);
		PostQuitMessage(0);
		break;
	case WM_COMMAND:
		CommandProc(hwnd, HIWORD(wParam), LOWORD(wParam), (HWND)lParam);
		break;
	default:
		return FALSE;
	}
	return TRUE;
}

int WINAPI WinMain(HINSTANCE hInst, HINSTANCE hPrevInst, LPSTR lpCmdLine, int nShowCmd)
{
	HMODULE hDll=NULL;
	HWND hwnd=NULL;
	MSG msg;
	memset(&msg, 0, sizeof(msg));

	hDll=LoadLibrary(LIBA_PATH);
	pArc_Archive=(void*)GetProcAddress(hDll, "Arc_Archive");
	if(pArc_Archive==NULL)
	{
		MessageBox(NULL, "Could not find Arc_Archive process \""LIBA_PATH"\" required.", g_szAppName, MB_OK|MB_ICONERROR);
		return 0;
	}
	
	hwnd=CreateDialog(hInst, "WARCHIVEDLG", NULL, ArcDlgProc);
	ShowWindow(hwnd, nShowCmd);
	UpdateWindow(hwnd);
	while(GetMessage(&msg, NULL, 0, 0))
	{
		if(!IsDialogMessage(hwnd, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	FreeLibrary(hDll);
	return msg.wParam;
}