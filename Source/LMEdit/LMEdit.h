// LMEdit.h : main header file for the LMEdit application
//
#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"       // main symbols


//Global Functions:
void Err_Printf(TCHAR* szString, ...);

// CLMEditApp:
// See LMEdit.cpp for the implementation of this class
//

class CLMEditApp : public CWinApp
{
public:
	CLMEditApp();


// Overrides
public:
	virtual BOOL InitInstance();

// Implementation
	afx_msg void OnAppAbout();
	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnIdle(LONG lCount);
private:
	void RegLegacyFileTypes();
};

extern CLMEditApp theApp;