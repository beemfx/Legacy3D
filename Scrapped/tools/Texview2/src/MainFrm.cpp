// MainFrm.cpp : implementation of the CMainFrame class
//

#include "stdafx.h"
#include "Texview2.h"

#include "MainFrm.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

DWORD __cdecl L_nextpow2_C(const DWORD n) 
{
	DWORD result = 1;
	if(n>=0x80000000)
		return 0x80000000;
	while(result<n) 
	{
		result <<= 1;
	}
	return result;
}



/////////////////////////////////////////////////////////////////////////////
// CMainFrame

IMPLEMENT_DYNAMIC(CMainFrame, CFrameWnd)

BEGIN_MESSAGE_MAP(CMainFrame, CFrameWnd)
	//{{AFX_MSG_MAP(CMainFrame)
	ON_WM_CREATE()
	ON_WM_SETFOCUS()
	ON_COMMAND(ID_FILE_OPEN, OnFileOpen)
	//}}AFX_MSG_MAP
	ON_COMMAND_RANGE(ID_TEXTURESIZE, ID_TEXTURESIZEMAX, OnViewSize)
	ON_COMMAND_RANGE(ID_VIEW_LINEARFILTER, ID_VIEW_POINTFILTER, OnFilterType)
	ON_COMMAND(ID_TOOLS_REGISTERFILETYPES, &CMainFrame::OnToolsRegisterfiletypes)
END_MESSAGE_MAP()

static UINT indicators[] =
{
	ID_SEPARATOR,           // status line indicator
	ID_INDICATOR_CAPS,
	ID_INDICATOR_NUM,
	ID_INDICATOR_SCRL,
};

/////////////////////////////////////////////////////////////////////////////
// CMainFrame construction/destruction

CMainFrame::CMainFrame()
{
	// TODO: add member initialization code here
	this->m_bAutoMenuEnable=FALSE;
}

CMainFrame::~CMainFrame()
{
}

int CMainFrame::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CFrameWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	// create a view to occupy the client area of the frame
	if (!m_wndView.Create(NULL, NULL, AFX_WS_DEFAULT_VIEW|WS_VSCROLL|WS_HSCROLL,
		CRect(0, 0, 0, 0), this, AFX_IDW_PANE_FIRST, NULL))
	{
		TRACE0("Failed to create view window\n");
		return -1;
	}

	if (!m_wndStatusBar.Create(this) ||
		!m_wndStatusBar.SetIndicators(indicators,
		  sizeof(indicators)/sizeof(UINT)))
	{
		TRACE0("Failed to create status bar\n");
		return -1;      // fail to create
	}

	UpdateMenuTexSizes(0, 0);

	return 0;
}

BOOL CMainFrame::PreCreateWindow(CREATESTRUCT& cs)
{
	if( !CFrameWnd::PreCreateWindow(cs) )
		return FALSE;
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
	cs.lpszClass = AfxRegisterWndClass(0, 0, 0, AfxGetApp()->LoadIcon(_T("ICON_IMG")));
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CMainFrame diagnostics

#ifdef _DEBUG
void CMainFrame::AssertValid() const
{
	CFrameWnd::AssertValid();
}

void CMainFrame::Dump(CDumpContext& dc) const
{
	CFrameWnd::Dump(dc);
}

#endif //_DEBUG

/////////////////////////////////////////////////////////////////////////////
// CMainFrame message handlers
void CMainFrame::OnSetFocus(CWnd* pOldWnd)
{
	// forward focus to the view window
	m_wndView.SetFocus();
}

BOOL CMainFrame::OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo)
{
	// let the view have first crack at the command
	if (m_wndView.OnCmdMsg(nID, nCode, pExtra, pHandlerInfo))
		return TRUE;

	// otherwise, do default handling
	return CFrameWnd::OnCmdMsg(nID, nCode, pExtra, pHandlerInfo);
}


void CMainFrame::OnFileOpen() 
{
	#define MFC_FILEDLG
	#ifdef MFC_FILEDLG
	CFileDialog fdOpen(
		TRUE, 
		_T("tga"), 
		NULL,
		OFN_EXPLORER|OFN_ENABLESIZING|OFN_NOCHANGEDIR, 
		_T("Supported Images (*.tga; *.bmp; *.dib; *.jpg; *.gif; *.png)|*.tga;*.bmp;*.dib;*.jpg;*.gif;*.png;|All Files (*.*)|*.*||"),
		this);

	fdOpen.m_ofn.lpstrTitle=_T("Open Image...");
	
	if(fdOpen.DoModal()==IDOK)
		LoadTexture(fdOpen.GetPathName().GetBuffer(0));

	#else MFC_FILEDLG

	OPENFILENAME of;
	TCHAR szFilename[MAX_PATH];
	ZeroMemory(&of, sizeof(OPENFILENAME));
	szFilename[0]=0;
	of.lStructSize=sizeof(OPENFILENAME);
	of.hwndOwner=this->m_hWnd;
	of.hInstance=NULL;
	of.lpstrFilter=_T("Supported Images (*.tga; *.bmp; *.dib)\0*.tga;*.bmp;*.dib\0All Files (*.*)\0*.*\0");
	of.lpstrCustomFilter=NULL;
	of.nMaxFile=MAX_PATH;
	of.lpstrTitle=_T("Open Image...");
	of.lpstrDefExt=_T("tga");
	of.lpstrFile=szFilename;

	if(GetOpenFileName(&of))
		this->m_wndView.LoadTexture(szFilename);
	
	#endif MFC_FILEDLG
}


BOOL CMainFrame::LoadTexture(LPTSTR szFilename)
{
	return m_wndView.LoadTexture(szFilename);
}

void CMainFrame::OnViewSize(UINT nID)
{
	CMenu* menu=GetMenu();
	DWORD i=0;
	for(i=0; i<=NUM_TEX_SIZES; i++)
	{
		if(nID==(ID_TEXTURESIZE+i))
			menu->CheckMenuItem(ID_TEXTURESIZE+i, MF_CHECKED|MF_BYCOMMAND);
		else
			menu->CheckMenuItem(ID_TEXTURESIZE+i, MF_UNCHECKED|MF_BYCOMMAND);
	}
	
	if(nID==ID_TEXTURESIZE)
	{
		m_wndView.SetBitmapSize(0, 0);
	}
	else
	{
		DWORD size=nID-ID_TEXTURESIZE;
		//size=L_pow2((L_byte)size);
		size=1<<(size-1);
		m_wndView.SetBitmapSize(size, size);
	}

	menu->Detach();
}

void CMainFrame::UpdateMenuTexSizes(int cx, int cy)
{
	CMenu* parentmenu=GetMenu();
	//parentmenu->InsertMenu(2, MF_BYPOSITION, MF_POPUP, "Texture");
	CMenu* mnTexture=parentmenu->GetSubMenu(1);
	

	int i=0, j=0;
	for(i=0; i<NUM_TEX_SIZES; i++)
		mnTexture->DeleteMenu(ID_TEXTURESIZE+i+1, MF_BYCOMMAND);

	mnTexture->CheckMenuItem(ID_TEXTURESIZE, MF_CHECKED);

	if(!cx || !cy)
	{
		mnTexture->EnableMenuItem(ID_TEXTURESIZE, MF_BYCOMMAND|MF_GRAYED);
		mnTexture->Detach();
		parentmenu->Detach();
		return;
	}

	cx=L_nextpow2_C(cx);
	cy=L_nextpow2_C(cy);
	cx=MAX(cy, cx);
	cy=cx;


	TCHAR szTemp[100];
	for(i=0,j=1; j<=cx && i<NUM_TEX_SIZES; i++, j*=2)
	{
		_stprintf(szTemp, _T("%ix%i"), j, j);
		mnTexture->InsertMenu(1, MF_BYPOSITION, ID_TEXTURESIZE+i+1, szTemp);
	}

	mnTexture->EnableMenuItem(ID_TEXTURESIZE, MF_BYCOMMAND|MF_ENABLED);
	mnTexture->Detach();
	parentmenu->Detach();

	DrawMenuBar();
	
}

void CMainFrame::OnFilterType(UINT nID)
{
	CMenu* menu=GetMenu();

	if(nID==ID_VIEW_LINEARFILTER)
		m_wndView.SetFilter(IMGFILTER_LINEAR);
	else
		m_wndView.SetFilter(IMGFILTER_POINT);

	menu->CheckMenuItem(ID_VIEW_POINTFILTER, MF_BYCOMMAND|MF_UNCHECKED);
	menu->CheckMenuItem(ID_VIEW_LINEARFILTER, MF_BYCOMMAND|MF_UNCHECKED);
	menu->CheckMenuItem(nID, MF_BYCOMMAND|MF_CHECKED);



	menu->Detach();
}


void CMainFrame::OnToolsRegisterfiletypes()
{
	// TODO: Add your command handler code here
	RegisterFileType(".tga", "Targa.Image", "Truevision Targa Image", 1);
	RegisterFileType(".bmp", "Bitmap.Image", "Bitmap Image", 1);
	RegisterFileType(".jpg", "JPEG.Image", "JPEG Image", 1);
	RegisterFileType(".gif", "GIF.Image", "Graphics Interchange Format Image", 1);
	SHChangeNotify(SHCNE_ASSOCCHANGED, SHCNF_IDLIST, NULL, NULL);
	MessageBox(_T("File types are registered."), NULL, MB_ICONINFORMATION);
}

int CMainFrame::RegisterFileType(char* szExt, char* szDesc, char* szText, int nIcon)
{
	
	HKEY hKey=NULL;
	DWORD dwType=0;
	char szValue[1024];
	char szTemp[1024];
	DWORD dwSize=0;
	DWORD dwDisp=0;
	
	char szModPath[MAX_PATH];
	GetModuleFileNameA(NULL, szModPath, MAX_PATH);
		
	RegOpenKeyExA(HKEY_CLASSES_ROOT, szExt, 0, KEY_ALL_ACCESS, &hKey);

	//If the file type is already registers we will simply add the command to open
	//with texture viewer.
	if(hKey)
	{
		dwSize=1024;
		RegQueryValueExA(hKey, NULL, NULL, &dwType, (LPBYTE)szValue, &dwSize);
		RegCloseKey(hKey);
		sprintf(szTemp, "%s\\shell\\Open with Texture Viewer 2\\command", szValue);
		RegCreateKeyExA(HKEY_CLASSES_ROOT, szTemp, 0, NULL, 0, NULL, NULL, &hKey, &dwDisp);
		RegOpenKeyExA(HKEY_CLASSES_ROOT, szTemp, 0, KEY_ALL_ACCESS, &hKey);
		sprintf(szValue, "\"%s\" %%1", szModPath);
		RegSetValueExA(hKey, NULL, 0, REG_SZ, (LPBYTE)szValue, strlen(szValue)+1);
		RegCloseKey(hKey);
	}
	else
	{
		//Otherwise we need to create the file type.
		//REgister the extension.
		RegCreateKeyExA(HKEY_CLASSES_ROOT, szExt, 0, NULL, 0, NULL, NULL, &hKey, &dwDisp);
		RegOpenKeyExA(HKEY_CLASSES_ROOT, szExt, 0, KEY_ALL_ACCESS, &hKey);
		RegSetValueExA(hKey, NULL, 0, REG_SZ, (LPBYTE)szDesc, strlen(szDesc)+1);
		//sprintf(szTemp, "\"%s\", %d", szModPath, nIcoNum);
		//RegSetValueEx(hKey, "DefaultIcon", 0, REG_SZ,  szTemp, strlen(szTemp)+1);
		RegCloseKey(hKey);
		
		
		//Register the file type.
		RegCreateKeyExA(HKEY_CLASSES_ROOT, szDesc, 0, NULL, 0, NULL, NULL, &hKey, &dwDisp);
		RegOpenKeyExA(HKEY_CLASSES_ROOT, szDesc, 0, KEY_ALL_ACCESS, &hKey);
		RegSetValueExA(hKey, NULL, 0, REG_SZ, (LPBYTE)szText, strlen(szText)+1);
		RegCloseKey(hKey);
		
		//Register the icon.
		sprintf(szTemp, "%s\\DefaultIcon", szDesc);
		RegCreateKeyExA(HKEY_CLASSES_ROOT, szTemp, 0, NULL, 0, NULL, NULL, &hKey, &dwDisp);
		RegOpenKeyExA(HKEY_CLASSES_ROOT, szTemp, 0, KEY_ALL_ACCESS, &hKey);
		sprintf(szTemp, "\"%s\",%d", szModPath, nIcon);
		RegSetValueExA(hKey, NULL, 0, REG_SZ, (LPBYTE)szTemp, strlen(szTemp)+1);
		RegCloseKey(hKey);
		
		//Register the command.
		
		sprintf(szTemp, "%s\\shell\\Open with Texture Viewer 2\\command", szDesc);
		RegCreateKeyExA(HKEY_CLASSES_ROOT, szTemp, 0, NULL, 0, NULL, NULL, &hKey, &dwDisp);
		RegOpenKeyExA(HKEY_CLASSES_ROOT, szTemp, 0, KEY_ALL_ACCESS, &hKey);
		sprintf(szTemp, "\"%s\" %%1", szModPath);
		RegSetValueExA(hKey, NULL, 0, REG_SZ, (LPBYTE)szTemp, strlen(szTemp)+1);
		RegCloseKey(hKey);
		
	}
	return 0;
}
