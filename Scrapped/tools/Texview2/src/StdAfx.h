// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__53E48B63_1C28_4393_AF20_C7F36893C4EC__INCLUDED_)
#define AFX_STDAFX_H__53E48B63_1C28_4393_AF20_C7F36893C4EC__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT


#define MAX(v1, v2) ((v1)>(v2))?(v1):(v2)
#define MIN(v1, v2) ((v1)<(v2))?(v1):(v2)
#define CLAMP(v1, min, max) ( (v1)>(max)?(max):(v1)<(min)?(min):(v1) )
#define CHECK_FLAG(var, flag) ((var&flag))


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__53E48B63_1C28_4393_AF20_C7F36893C4EC__INCLUDED_)
