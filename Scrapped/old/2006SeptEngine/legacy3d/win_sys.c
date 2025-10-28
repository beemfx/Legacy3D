/*************************************************************
	File: win_sys.c
	Copyright (c) 2006, Blaine Myers. 

	Purpose: Entry point for Legacy3D engine.  I've tryed to
	hide as much of the Windows API as possible from the
	actual Legacy 3D game (possibly to make future portability
	easier.  All the actual Windows API function calls are in
	this file, and all the linked to librarys are in this
	file.  The actual game is invoked and run with calls
	to LG_GameLoop in the WindowLoop() function.  The console
	is also controlled from the MainWindowProc() function,
	but everything else in teh game is controlled elswhere.
*************************************************************/

/************************************************************************************* 
	Legacy3D Game Engine
	--------------------
	The Legacy3D game engine (will be) a functional 3D graphics and physics engine.
	It will be engineered for both first person action games, as well as slower paced
	3rd person adventure games.  The project is in the development stages.
*************************************************************************************/

#define WM_USER_ACTIVEAPP (WM_USER+1)

/*	Including all required libs, I included them
	here as it makes it easier if I have to re-construct
	the project. */

#ifdef _DEBUG
#pragma comment(lib, "d3dx9d.lib")

#pragma comment(lib, "ogg_d.lib")
#pragma comment(lib, "vorbis_d.lib")
#pragma comment(lib, "vorbisfile_d.lib")

#pragma comment(lib, "..\\common\\lc_sys_d.lib")
#pragma comment(lib, "..\\common\\lf_sys_d.lib")
#pragma comment(lib, "..\\common\\img_lib_mt_d.lib")
#else /*_DEBUG*/
#pragma comment(lib, "d3dx9.lib")

#pragma comment(lib, "ogg.lib")
#pragma comment(lib, "vorbis.lib")
#pragma comment(lib, "vorbisfile.lib")

#pragma comment(lib, "..\\common\\lc_sys.lib")
#pragma comment(lib, "..\\common\\lf_sys.lib")
#pragma comment(lib, "..\\common\\img_lib_mt.lib")
#endif /*_DEBUG*/

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "dsound.lib")
#pragma comment(lib, "dxguid.lib")
#pragma comment(lib, "dxerr9.lib")
#pragma comment(lib, "winmm.lib")




#include "common.h"
#include <windows.h>
#include <direct.h>
#include "lg_sys.h"
#include "resource.h"


/**********************************************************
	MainWindowProc()

	The main window proc for Legacy 3D, very little is
	done from actual windows functions as far as the game
	is concerned.  The input into the console is managed
	from here, as well as someshutdown, and activation
	and deactivation of the app stuf
**********************************************************/
LRESULT CALLBACK MainWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch(uMsg)
	{
	case WM_ACTIVATEAPP: 
		/* We have the WM_ACTIVEATEAPP keep track whether or not the application is
		active, this can be useful if the application should pause or something 
		when not being used. */
		PostMessage(hwnd, WM_USER_ACTIVEAPP, wParam, 0);
		break;
	case WM_COMMAND:
	{
		if(LOWORD(wParam)==ID_FILE_EXIT)
		{
			SendMessage(hwnd, WM_CLOSE, 0, 0);
		}
		break;
	}
	case WM_KEYDOWN:
		switch(wParam)
		{
		case VK_PRIOR:
			LG_OnChar(LK_PAGEUP);
			break;
		case VK_NEXT:
			LG_OnChar(LK_PAGEDOWN);
			break;
		case VK_END:
			LG_OnChar(LK_END);
			break;
		}	
		break;
	/*
	case WM_MOUSEMOVE:
	{
		static D3DXMATRIX RotX;
		static D3DXMATRIX RotY;
		static L_bool bFirstTime=1;
		if(bFirstTime)
		{
			D3DXMatrixIdentity(&RotX);
			D3DXMatrixIdentity(&RotY);
			bFirstTime=0;
		}
		if(L_CHECK_FLAG(wParam, MK_LBUTTON))
		{
			D3DXMATRIX Temp;
			D3DXMatrixRotationX(&Temp, (float)LOWORD(lParam)/1000.0f);
			D3DXMatrixMultiply(&RotX, &Temp, &RotX);
			D3DXMatrixRotationY(&Temp, (float)HIWORD(lParam)/1000.0f);
			D3DXMatrixMultiply(&RotY, &Temp, &RotY);
			D3DXMatrixMultiply(&Temp, &RotX, &RotY);
			IDirect3DDevice9_SetTransform(LG_GetGame()->v.m_lpDevice, D3DTS_WORLD,&Temp);
		}
		break;
	}
	*/
	case WM_CHAR:
	/*
		if(wParam==VK_ESCAPE)
			SendMessage(hwnd, WM_CLOSE, 0, 0);
	*/
		LG_OnChar((char)wParam);
		break;
	case WM_CLOSE:
		/* Of course we need to be able to shutdown our application, this will probably
		be done through an ingame menu later on, but in this stage of development we
		need a way to shutdown our application. */
		DestroyWindow(hwnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	return 0l;
}

/********************************************************
	WindowLoop()

	The main loop for windows, the actual game loop is
	called from here when the function LG_GameLoop() is
	called, the rest of this deals with some windows
	stuff.
********************************************************/

int WindowLoop(HWND hwnd)
{
	BOOL bActiveApp=TRUE;
	BOOL bAlwaysProc=FALSE;
	MSG msg;
	
	memset(&msg, 0, sizeof(msg));
	/* ===The window loop===
	The idea behind the window loop is that we want to check to see if
	Windows wants to do something, but other than that we want to do what
	our game wants to do, so most of the time the GameLoop() function will
	be called, but if a message comes up, then windows will process it.  Most
	of the game will be managed from the GameLoop function. */
	
	/* This game loop should be used if the game should stop running when
		the window is not active. */
	
	while(TRUE)
	{
		if(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if(msg.message==WM_QUIT)
			{
				LG_GameLoop(hwnd, TRUE, bActiveApp);
				break;
			}
			else if(msg.message==WM_USER_ACTIVEAPP)
				bActiveApp=msg.wParam;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			if(bAlwaysProc || bActiveApp)
			{
				if(!LG_GameLoop(hwnd, FALSE, bActiveApp))
					SendMessage(msg.hwnd, WM_CLOSE, 0, 0);
			}
			else
			{
				WaitMessage();
			}
		}
	}
	
	return msg.wParam;
}

/*******************************************************
	ChangeDirectory()

	This insures that the game starts in the directory
	that the exe file is in.  Called from WinMain().
*******************************************************/

void ChangeDirectory(char* szCommandLine)
{
	char szNewDir[MAX_PATH];
	int i=0;
	int nLen=0;
	int bFoundQuote=0;

	/* Get rid of the initial quote. */
	szCommandLine++;

	nLen=L_strlen(szCommandLine);
	for(i=0; i<nLen; i++)
		if(szCommandLine[i]=='\"')
			break;

	L_strncpy(szNewDir, szCommandLine, i);
	/* Now go back until we find a backslash and set
	that to zero. */
	for(i=L_strlen(szNewDir); i--; )
	{
		if(szNewDir[i]=='\\')
		{
			szNewDir[i]=0;
			break;
		}
	}
	_chdir(szNewDir);
	/* Now if we're debugging then the app is in the
	legacy3d\debug directory so we need to change that. */
	#ifdef _DEBUG
	_chdir("..\\..");
	#endif /*_DEBUG*/

}

/*************************************************************
	WinMain()

	The entry point for Legacy 3D, not that
	most of the game stuff is managed in LG_GameLoop(), but
	that function is from the windows loop.  Everything in
	here is standard windows creation.  When MainWindowLoop()
	is called that is when the game loop starts.
*************************************************************/
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
	HWND hwnd=NULL;
	WNDCLASSEX wc;
	int nResult=0;

	static TCHAR szAppName[]=TEXT("Legacy3D Engine");


	/* We want the starting directory to be the directory where the
		EXE file is located. */
	ChangeDirectory(GetCommandLine());

	/* The first thing we need to do is all the standard Window creation calls. */

	/* Define and register the Window's Class. */
	wc.cbSize=sizeof(wc);
	wc.style=CS_HREDRAW|CS_VREDRAW|CS_OWNDC;
	wc.cbClsExtra=0;
	wc.cbWndExtra=0;
	wc.lpfnWndProc=MainWindowProc;
	wc.hInstance=hInstance;
	wc.hbrBackground=(HBRUSH)GetStockObject(DKGRAY_BRUSH);
	wc.hIcon=LoadIcon(hInstance, "L3DICO");
	wc.hIconSm=LoadIcon(hInstance, "L3DICO");
	wc.hCursor=LoadCursor(NULL, IDC_ARROW);
	wc.lpszMenuName="LEGACY3DMENU";
	wc.lpszClassName=szAppName;
	
	if(!RegisterClassEx(&wc))
	{
		MessageBox(
			NULL, 
			TEXT("This program requires Windows NT!"), 
			szAppName, 
			MB_OK|MB_ICONERROR);

		return 0;
	}

	/* Create the Window. */
	
	hwnd=CreateWindowEx(
		0,
		szAppName,
		szAppName,
		WS_CAPTION|WS_SYSMENU|WS_VISIBLE|WS_MINIMIZEBOX,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		320,
		240,
		NULL,
		NULL,
		hInstance,
		NULL);
	

	if(hwnd==NULL)
	{
		MessageBox(
			NULL,
			TEXT("Failed to create window!  Shutting down."),
			szAppName,
			MB_OK|MB_ICONERROR);

		return 0;
	}

	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);
	
	#ifdef _DEBUG
	L_BEGIN_D_DUMP
	nResult=WindowLoop(hwnd);
	L_END_D_DUMP("legacy3d.exe")
	_CrtDumpMemoryLeaks();
	return nResult;
	#else /*_DEBUG*/
	return WindowLoop(hwnd);
	#endif /*_DEBUG*/
}
