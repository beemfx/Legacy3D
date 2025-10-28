// Texview2.h : main header file for the TEXVIEW2 application
//

#if !defined(AFX_TEXVIEW2_H__EAAF1687_31ED_437C_987B_D43D681924ED__INCLUDED_)
#define AFX_TEXVIEW2_H__EAAF1687_31ED_437C_987B_D43D681924ED__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "..\res\resource.h"       // main symbols

/////////////////////////////////////////////////////////////////////////////
// CTexview2App:
// See Texview2.cpp for the implementation of this class
//

class CTexview2App : public CWinApp
{
public:
	CTexview2App();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTexview2App)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

public:
	//{{AFX_MSG(CTexview2App)
	afx_msg void OnAppAbout();
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TEXVIEW2_H__EAAF1687_31ED_437C_987B_D43D681924ED__INCLUDED_)
