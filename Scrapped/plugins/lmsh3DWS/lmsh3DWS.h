// lmsh3DWS.h : main header file for the lmsh3DWS DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// Clmsh3DWSApp
// See lmsh3DWS.cpp for the implementation of this class
//

class Clmsh3DWSApp : public CWinApp
{
public:
	Clmsh3DWSApp();

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
