// MainFrm.h : interface of the CMainFrame class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_MAINFRM_H__6A16EED8_305C_4188_92AC_72577D431120__INCLUDED_)
#define AFX_MAINFRM_H__6A16EED8_305C_4188_92AC_72577D431120__INCLUDED_

#define NUM_TEX_SIZES     (16)
#define ID_TEXTURESIZE    (ID_VIEW_SIZE)
#define ID_TEXTURESIZEMAX (ID_TEXTURESIZE+NUM_TEX_SIZES)


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "ChildView.h"

class CMainFrame : public CFrameWnd
{
public:
	CMainFrame();
protected: 
	DECLARE_DYNAMIC(CMainFrame)

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CMainFrame)
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL OnCmdMsg(UINT nID, int nCode, void* pExtra, AFX_CMDHANDLERINFO* pHandlerInfo);
	//}}AFX_VIRTUAL

// Implementation
public:
	void UpdateMenuTexSizes(int cx, int cy);
	BOOL LoadTexture(LPTSTR szFilename);
	virtual ~CMainFrame();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:  // control bar embedded members
	CStatusBar  m_wndStatusBar;
	CChildView    m_wndView;

// Generated message map functions
protected:
	//{{AFX_MSG(CMainFrame)
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnSetFocus(CWnd *pOldWnd);
	afx_msg void OnFileOpen();
	//}}AFX_MSG
	afx_msg void OnViewSize(UINT nID);
	afx_msg void OnFilterType(UINT nID);
	DECLARE_MESSAGE_MAP()
private:

	afx_msg void OnToolsRegisterfiletypes();
protected:
	int RegisterFileType(char* szExt, char* szDesc, char* szText, int nIcon);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_MAINFRM_H__6A16EED8_305C_4188_92AC_72577D431120__INCLUDED_)
