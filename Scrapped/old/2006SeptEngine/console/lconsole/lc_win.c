#include <windows.h>
#include "lc_sys.h"

#define CD_LINES 15
#define CD_FONTSIZE 15


#define ID_QUITBUTTON 103
#define ID_TEXTEDIT 102

LRESULT CALLBACK EntryProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static HFONT hFont=NULL;
	static HLCONSOLE hConsole=NULL;
	static BOOL bBlinkOn=0;
	static BOOL bHasFocus=0;

	#define BLINKER_TIMER (0x12001200)

	switch(uMsg)
	{
	case WM_DESTROY:
		KillTimer(hwnd, BLINKER_TIMER);
		break;
	case WM_SETFOCUS:
		bHasFocus=TRUE;
		break;
	case WM_KILLFOCUS:
		bHasFocus=FALSE;
		break;
	case WM_TIMER:
		if(wParam==BLINKER_TIMER)
		{
			bBlinkOn=!bBlinkOn;
			InvalidateRect(hwnd, NULL, TRUE);
		}
		break;
	case WM_CREATE:
		/* Get the font that was passed over here. */
		hFont=((LPCREATESTRUCT)lParam)->lpCreateParams;
		SetTimer(hwnd, BLINKER_TIMER, GetCaretBlinkTime(), NULL);
		break;
	case WM_USER_INSTALLCONSOLE:
		hConsole=(HLCONSOLE)lParam;
		/* Fall through and update. */
	case WM_USER_UPDATE:
		InvalidateRect(hwnd, NULL, TRUE);
		break;
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
		SetFocus(hwnd);
		break;
	case WM_CHAR:
		Con_OnChar(hConsole, (WORD)wParam);
		SendMessage(GetParent(hwnd), WM_USER_UPDATE, 0, 0);
		break;
	case WM_PAINT:
	{
		HDC hdc=NULL;
		PAINTSTRUCT ps;
		RECT wrc;
		TEXTMETRIC tm;
		char szString[1024];
		HFONT hOldFont=NULL;
		int i=0, j=0;
		unsigned long nNumEntries=0;
		GetClientRect(hwnd, &wrc);
		nNumEntries=Con_GetNumEntries(hConsole);
		
		hdc=BeginPaint(hwnd, &ps);
		hOldFont=SelectObject(hdc, hFont);
		GetTextMetrics(hdc, &tm);
		if(hConsole)
		{
			Con_GetEntry(hConsole, 1, szString);
			if(bBlinkOn && bHasFocus)
				strcat(szString, "_");

			TextOut(hdc, 0, 0, szString, strlen(szString));
		}
		SelectObject(hdc, hOldFont);
		EndPaint(hwnd, &ps);
		break;
	}
	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	return 0l;
}

LRESULT CALLBACK ConsoleProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static HFONT hFont=NULL;
	SCROLLINFO si;
	static HLCONSOLE hConsole=NULL;
	switch(uMsg)
	{
	case WM_CREATE:
	{
		unsigned long nEntries=Con_GetNumEntries(hConsole);
		si.cbSize=sizeof(SCROLLINFO);
		si.fMask=SIF_ALL;
		si.nMin=1;
		si.nMax=nEntries;
		si.nPos=nEntries;
		si.nTrackPos=0;
		si.nPage=(CD_LINES+1);
		SetScrollInfo(hwnd, SB_VERT, &si, TRUE);

		/* Get the font that was passed over here. */
		hFont=((LPCREATESTRUCT)lParam)->lpCreateParams;
		break;
	}
	case WM_USER_INSTALLCONSOLE:
		hConsole=(HLCONSOLE)lParam;
		/* Fall through and update. */
	case WM_USER_UPDATE:
		si.cbSize=sizeof(SCROLLINFO);
		si.fMask=SIF_POS;
		GetScrollInfo(hwnd, SB_VERT, &si);
		si.fMask=SIF_RANGE|SIF_POS|SIF_PAGE;
		si.nMax=Con_GetNumEntries(hConsole);
		si.nMin=1;
		si.nPage=(CD_LINES+1);
		si.nPos=si.nMax;
		SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
		InvalidateRect(hwnd, NULL, TRUE);
		break;
	case WM_VSCROLL:
	{
		si.cbSize=sizeof(SCROLLINFO);
		si.fMask=SIF_ALL;
		GetScrollInfo(hwnd, SB_VERT, &si);
		SetScrollInfo(hwnd, SB_VERT, &si, TRUE);

		switch(LOWORD(wParam))
		{
		case SB_PAGEDOWN:
			si.nPos+=si.nPage/2;
			break;
		case SB_LINEDOWN:
			si.nPos++;
			break;
		case SB_PAGEUP:
			si.nPos-=si.nPage/2;
			break;
		case SB_LINEUP:
			si.nPos--;
			break;
		case SB_THUMBPOSITION:
		case SB_THUMBTRACK:
			si.nPos=si.nTrackPos;
			break;
		case SB_TOP:
			si.nPos=si.nMin;
			break;
		case SB_BOTTOM:
			si.nPos=si.nMax;
			break;
		}

		SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
		RedrawWindow(hwnd, NULL, NULL, RDW_ERASE|RDW_INVALIDATE);
		break;
	}
	case WM_PAINT:
	{
		HDC hdc=NULL;
		PAINTSTRUCT ps;
		RECT wrc;
		TEXTMETRIC tm;
		HFONT hOldFont=NULL;
		char szString[1024];
		int i=0, j=0;
		unsigned long nNumEntries=0;

		GetClientRect(hwnd, &wrc);
		nNumEntries=Con_GetNumEntries(hConsole);
		
		hdc=BeginPaint(hwnd, &ps);
		hOldFont=SelectObject(hdc, hFont);
		GetTextMetrics(hdc, &tm);

		/* Update the scroll info. */
		si.cbSize=sizeof(SCROLLINFO);
		si.fMask=SIF_POS|SIF_PAGE|SIF_RANGE;
		GetScrollInfo(hwnd, SB_VERT, &si);
		
		
		for(i=nNumEntries-(si.nPos+si.nPage-2)+1, j=1; ((j<=(int)si.nPage) && (i<=(int)nNumEntries)); i++, j++)
		{
			strcpy(szString, Con_GetEntry(hConsole, i, NULL));
			TextOut(hdc, 0, wrc.bottom-(tm.tmHeight*j), szString, strlen(szString));

		}

		SelectObject(hdc, hOldFont);
		EndPaint(hwnd, &ps);	
		break;
	}
	case WM_DESTROY:
		break;
	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	return 0l;
}

LRESULT CALLBACK LCWindowProc(HWND hwnd,UINT uMsg,WPARAM wParam,LPARAM lParam)
{
	static HWND hwndConsole=NULL;
	static HWND hwndEntry=NULL;
	static HFONT hFont=NULL;
	static HLCONSOLE hConsole=NULL;
	static HWND hwndQuitButton=NULL;

	switch(uMsg)
	{
	case WM_CREATE:
	{
		char szConsoleText[]="ConsoleWindow";
		char szEntryText[]="EntryWindow";
		HINSTANCE hInst=GetModuleHandle(NULL);
		RECT rcMainWindow;

		WNDCLASS wc;

		wc.cbClsExtra=0;
		wc.cbWndExtra=0;
		wc.hbrBackground=GetStockObject(WHITE_BRUSH);
		wc.hCursor=LoadCursor(NULL, IDC_ARROW);
		wc.hIcon=NULL;
		wc.hInstance=hInst;
		wc.lpfnWndProc=ConsoleProc;
		wc.lpszClassName=szConsoleText;
		wc.lpszMenuName=NULL;
		wc.style=CS_VREDRAW|CS_HREDRAW;

		RegisterClass(&wc);

		GetClientRect(hwnd, &rcMainWindow);

		/* Select the font we want. */
		hFont=CreateFont(
			CD_FONTSIZE,
			0,
			0,
			0,
			FW_DEMIBOLD,
			FALSE,
			FALSE,
			FALSE,
			ANSI_CHARSET,
			0,
			0,
			DEFAULT_QUALITY,
			DEFAULT_PITCH,
			"Courier New");

		hwndConsole=CreateWindow(
			szConsoleText,
			szConsoleText,
			WS_CHILD|WS_VSCROLL|WS_VISIBLE,
			5,
			5,
			rcMainWindow.right-10,
			CD_LINES*CD_FONTSIZE,
			hwnd,
			NULL,
			hInst,
			hFont);
		
		UpdateWindow(hwnd);
		ShowWindow(hwnd, SW_SHOWNORMAL);

		wc.cbClsExtra=0;
		wc.cbWndExtra=0;
		wc.hbrBackground=GetStockObject(WHITE_BRUSH);
		wc.hCursor=LoadCursor(NULL, IDC_ARROW);
		wc.hIcon=NULL;
		wc.hInstance=hInst;
		wc.lpfnWndProc=EntryProc;
		wc.lpszClassName=szEntryText;
		wc.lpszMenuName=NULL;
		wc.style=CS_VREDRAW|CS_HREDRAW;

		RegisterClass(&wc);

		hwndEntry=CreateWindowEx(
			WS_EX_CLIENTEDGE,
			szEntryText,
			szEntryText,
			WS_CHILD|WS_VISIBLE,
			5,
			CD_LINES*CD_FONTSIZE+10,
			rcMainWindow.right-10,
			20,
			hwnd,
			(HMENU)ID_TEXTEDIT,
			hInst,
			hFont);

		UpdateWindow(hwndEntry);
		ShowWindow(hwndEntry, SW_SHOWNORMAL);

		hwndQuitButton=CreateWindow(
			"BUTTON",
			"Quit",
			WS_CHILD|WS_VISIBLE,
			(rcMainWindow.right-75)/2,
			rcMainWindow.bottom-25-10,
			75,
			25,
			hwnd,
			(HMENU)ID_QUITBUTTON,
			hInst,
			0);

		SetFocus(hwndEntry);

		return DefWindowProc(hwnd, uMsg, wParam, lParam);
		break;
	}
	case WM_USER_INSTALLCONSOLE:
		hConsole=(HLCONSOLE)lParam;
		SendMessage(hwndConsole, WM_USER_INSTALLCONSOLE, wParam, lParam);
		SendMessage(hwndEntry, WM_USER_INSTALLCONSOLE, wParam, lParam);
		/* And fall through and update. */
	case WM_USER_UPDATE:
		SendMessage(hwndConsole, WM_USER_UPDATE, 0, 0);
		SendMessage(hwndEntry, WM_USER_UPDATE, 0, 0);
		break;
	case WM_COMMAND:
	{
		switch(LOWORD(wParam))
		{
		case ID_QUITBUTTON:
			SendMessage(hwnd, WM_CLOSE, 0, 0);
			break;
		default:
			break;
		}
		break;
	}
	case WM_SETFOCUS:
	case WM_ACTIVATEAPP:
		SetFocus(hwndEntry);
		break;
	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;
	case WM_DESTROY:
		DestroyWindow(hwndConsole);
		DeleteObject(hFont);
		PostQuitMessage(0);
	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	return 0l;
}


void* Con_CreateDlgBox()
{
	#ifdef NDEBUG
	char szLCText[]="Legacy Console";
	#else /*NDEBUG*/
	char szLCText[]="Debug Legacy Console";
	#endif /*NDEBUG*/

	WNDCLASS wc;
	HINSTANCE hInstance=GetModuleHandle("lconsole.dll");


	wc.cbClsExtra=0;
	wc.cbWndExtra=0;
	wc.hbrBackground=(HBRUSH)COLOR_WINDOW;
	wc.hCursor=LoadCursor(NULL, IDI_APPLICATION);
	wc.hIcon=0;
	wc.hInstance=hInstance;
	wc.lpfnWndProc=LCWindowProc;
	wc.lpszClassName=szLCText;
	wc.lpszMenuName=NULL;
	wc.style=0;

	RegisterClass(&wc);

	return CreateWindow(
		szLCText,
		szLCText,
		WS_SYSMENU|WS_MINIMIZEBOX,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		400,
		320,
		NULL,
		NULL,
		hInstance,
		0);
}
