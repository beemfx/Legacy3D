#pragma once


// CDlgSaveAs dialog

class CDlgSaveAs : public CDialog
{
	DECLARE_DYNAMIC(CDlgSaveAs)

public:
	CDlgSaveAs(CWnd* pParent = NULL);   // standard constructor
	CDlgSaveAs(LPCTSTR szMesh, LPCTSTR szSkel, CWnd* pParent = NULL);
	virtual ~CDlgSaveAs();

// Dialog Data
	enum { IDD = IDD_SAVEAS };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CString m_szMeshFile;
	CString m_szSkelFile;
public:
	afx_msg void OnBnClickedBrowsemesh();
public:
	afx_msg void OnBnClickedBrowseskel();
public:
	virtual BOOL OnInitDialog();
};
