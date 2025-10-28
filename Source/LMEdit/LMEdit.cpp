// LMEdit.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "LMEdit.h"
#include "MainFrm.h"

#include "LMEditDoc.h"
#include "LMEditView.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

extern CLMEditView* g_wndPrimary;

void Err_Printf(TCHAR* format, ...)
{
	#ifdef _DEBUG
	char szOutput[1024];
	va_list arglist=NULL;
	
	if(!format)
		return;
		
	va_start(arglist, format);
	_vsnprintf(szOutput, 1023, format, arglist);
	va_end(arglist);
	OutputDebugString(szOutput);
	#endif _DEBUG
}


// CLMEditApp

BEGIN_MESSAGE_MAP(CLMEditApp, CWinApp)
	ON_COMMAND(ID_APP_ABOUT, &CLMEditApp::OnAppAbout)
	// Standard file based document commands
	ON_COMMAND(ID_FILE_NEW, &CWinApp::OnFileNew)
	ON_COMMAND(ID_FILE_OPEN, &CWinApp::OnFileOpen)
	// Standard print setup command
	ON_COMMAND(ID_FILE_PRINT_SETUP, &CWinApp::OnFilePrintSetup)
END_MESSAGE_MAP()


// CLMEditApp construction

CLMEditApp::CLMEditApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
	ML_Init(ML_INSTR_F);
}


// The one and only CLMEditApp object

CLMEditApp theApp;


// CLMEditApp initialization

BOOL CLMEditApp::InitInstance()
{
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

	// Initialize OLE libraries
	if (!AfxOleInit())
	{
		AfxMessageBox(IDP_OLE_INIT_FAILED);
		return FALSE;
	}
	AfxEnableControlContainer();
	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Beem"));
	LoadStdProfileSettings(4);  // Load standard INI file options (including MRU)
	// Register the application's document templates.  Document templates
	//  serve as the connection between documents, frame windows and views
	CSingleDocTemplate* pDocTemplate;
	pDocTemplate = new CSingleDocTemplate(
		IDR_MAINFRAME,
		RUNTIME_CLASS(CLMEditDoc),
		RUNTIME_CLASS(CMainFrame),       // main SDI frame window
		RUNTIME_CLASS(CLMEditView));
	if (!pDocTemplate)
		return FALSE;
	AddDocTemplate(pDocTemplate);

	// Enable DDE Execute open
	EnableShellOpen();
	//RegisterShellFileTypes(TRUE);
	RegLegacyFileTypes();
	
	

	// Parse command line for standard shell commands, DDE, file open
	CCommandLineInfo cmdInfo;
	ParseCommandLine(cmdInfo);


	// Dispatch commands specified on the command line.  Will return FALSE if
	// app was launched with /RegServer, /Register, /Unregserver or /Unregister.
	if (!ProcessShellCommand(cmdInfo))
		return FALSE;

	// The one and only window has been initialized, so show and update it
	m_pMainWnd->ShowWindow(SW_SHOW);
	m_pMainWnd->UpdateWindow();
	// call DragAcceptFiles only if there's a suffix
	//  In an SDI app, this should occur after ProcessShellCommand
	// Enable drag/drop open
	m_pMainWnd->DragAcceptFiles();
	return TRUE;
}



// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()

// App command to run the dialog
void CLMEditApp::OnAppAbout()
{
	CAboutDlg aboutDlg;
	aboutDlg.DoModal();
}


// CLMEditApp message handlers


BOOL CLMEditApp::OnIdle(LONG lCount)
{
	CWinApp::OnIdle(lCount);
	// TODO: Add your specialized code here and/or call the base class
	//Should process animations here.
	if(lCount>=2)
	if(g_wndPrimary)
		g_wndPrimary->ProcessAnimation(); 

	return TRUE;
}

void CLMEditApp::RegLegacyFileTypes()
{
	HKEY hKey=NULL;
	
	RegCreateKeyEx(HKEY_CLASSES_ROOT, 
		_T(".lmsh"), 0,NULL,0, 
		KEY_ALL_ACCESS, NULL,&hKey,NULL);
	//Register the extension	
	
	CString szKey=_T("Legacy.Mesh");
	RegSetValueEx(hKey, NULL, 0, REG_SZ, 
		(LPBYTE)szKey.GetBuffer(), (szKey.GetLength()+1)*sizeof(TCHAR));
	RegCloseKey(hKey);	
	
	RegCreateKeyEx(HKEY_CLASSES_ROOT, 
		_T(".lskl"), 0,NULL,0, 
		KEY_ALL_ACCESS, NULL,&hKey,NULL);
	//Register the extension	
	
	szKey=_T("Legacy.Mesh");
	RegSetValueEx(hKey, NULL, 0, REG_SZ, 
		(LPBYTE)szKey.GetBuffer(), (szKey.GetLength()+1)*sizeof(TCHAR));
	RegCloseKey(hKey);
	
	//Now register the document type.
	RegCreateKeyEx(HKEY_CLASSES_ROOT,
	_T("Legacy.Mesh"),
	0,NULL,0,KEY_ALL_ACCESS,NULL,&hKey,NULL);
		
	if(!hKey)
		return;
	szKey=_T("Legacy Mesh/Skeleton");	
	RegSetValueEx(hKey,NULL,0,REG_SZ,
		(LPBYTE)szKey.GetBuffer(),(szKey.GetLength()+1)*sizeof(TCHAR));
	RegCloseKey(hKey);	
	
	RegCreateKeyEx(HKEY_CLASSES_ROOT,
		_T("Legacy.Mesh\\shell\\open\\command"),
		0,NULL,0,KEY_ALL_ACCESS,NULL,&hKey,NULL);
	TCHAR szTemp[MAX_PATH+1];
	GetModuleFileName(NULL, szTemp, (MAX_PATH)*sizeof(TCHAR));
	szKey=szTemp;
	RegSetValueEx(hKey,NULL,0,REG_SZ,(LPBYTE)szKey.GetBuffer(),
		(szKey.GetLength()+1)*sizeof(TCHAR));
	RegCloseKey(hKey);
	RegCreateKeyEx(HKEY_CLASSES_ROOT,
		_T("Legacy.Mesh\\shell\\open\\ddeexec"),
		0,NULL,0,KEY_ALL_ACCESS,NULL,&hKey,NULL);
	szKey=_T("[open(\"%1\")]");
	RegSetValueEx(hKey,NULL,0,REG_SZ,(LPBYTE)szKey.GetBuffer(),
		(szKey.GetLength()+1)*sizeof(TCHAR));
	RegCloseKey(hKey);
	/*
	RegCreateKeyEx(HKEY_CLASSES_ROOT,
		_T("Legacy.Mesh\\shell\\open\\ddeexec\\application"),
		0,NULL,0,KEY_ALL_ACCESS,NULL,&hKey,NULL);
	szKey=AfxGetAppName();
	RegSetValueEx(hKey,NULL,0,REG_SZ,(LPBYTE)szKey.GetBuffer(),
		(szKey.GetLength()+1)*sizeof(TCHAR));
	RegCloseKey(hKey);
	*/
}
