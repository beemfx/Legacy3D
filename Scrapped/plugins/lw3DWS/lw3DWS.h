// lw3DWS.h : main header file for the lw3DWS DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// Clw3DWSApp
// See lw3DWS.cpp for the implementation of this class
//

class Clw3DWSApp : public CWinApp
{
public:
	Clw3DWSApp();

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
