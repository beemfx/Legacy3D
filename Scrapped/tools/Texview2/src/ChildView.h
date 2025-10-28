// ChildView.h : interface of the CChildView class
//
/////////////////////////////////////////////////////////////////////////////

#if !defined(AFX_CHILDVIEW_H__D24ED702_83BE_45D8_BB31_9B370B519989__INCLUDED_)
#define AFX_CHILDVIEW_H__D24ED702_83BE_45D8_BB31_9B370B519989__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "texture.h"

/////////////////////////////////////////////////////////////////////////////
// CChildView window

class CChildView : public CWnd
{
// Construction
public:
	CChildView();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CChildView)
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
public:
	void UpdateImage();
	void SetFilter(IMGFILTER filter);
	void SetBitmapSize(int cx, int cy);
	BOOL LoadTexture(LPTSTR szFilename);
	virtual ~CChildView();

	// Generated message map functions
protected:
	//{{AFX_MSG(CChildView)
	afx_msg void OnPaint();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnViewAlphachannel();
	afx_msg void OnViewImage();
	afx_msg void OnViewBlackbg();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
	int m_nImageHeight;
	int m_nImageWidth;
	BOOL m_bBlackBG;
	BOOL m_bShowImage;
	BOOL m_bAlphaChannel;
	CTexture m_texImage;
	IMGFILTER m_nFilter;
	void UpdateScrollBars(BOOL bResetToZero);
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CHILDVIEW_H__D24ED702_83BE_45D8_BB31_9B370B519989__INCLUDED_)
