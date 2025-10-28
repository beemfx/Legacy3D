#pragma once
#include "afxcmn.h"
#include "afxwin.h"


// CAnimDialog dialog

class CAnimDialog : public CDialog
{
	DECLARE_DYNAMIC(CAnimDialog)

public:
	CAnimDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CAnimDialog();

// Dialog Data
	enum { IDD = ID_ANIM_DIALOG };
	enum {
		ANIM_NAME=0,
		ANIM_FIRST_FRAME=1,
		ANIM_NUM_FRAMES=2,
		ANIM_LOOPING_FRAMES=3,
		ANIM_RATE=4,
		ANIM_FLAGS=5};
		
	typedef TCHAR NUMBER_STRING[32];

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	CLSkel2* m_pSkel;
public:
	void SetSkel(CLSkel2* pSkel);
public:
	BOOL AddAnimation(const CLSkel2::SkelAnim* pAnim);
public:
	afx_msg void OnBnClickedAddAnim();
public:
	afx_msg void OnBnClickedDeleteAnim();
	BOOL ModifySkeleton(void);
public:
	CListCtrl m_ListCtrl;
public:
	CComboBox m_cLoopBox;
};
