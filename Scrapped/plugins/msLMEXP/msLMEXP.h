// msMLEXP.h : main header file for the msMLEXP DLL
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


// CmsMLEXPApp
// See msMLEXP.cpp for the implementation of this class
//

class CmsMLEXPApp : public CWinApp
{
public:
	CmsMLEXPApp();

// Overrides
public:
	virtual BOOL InitInstance();

	DECLARE_MESSAGE_MAP()
};
