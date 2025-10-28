#pragma once


// CExportDialog dialog

class CExportDialog : public CDialog
{
	DECLARE_DYNAMIC(CExportDialog)

public:
	CExportDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CExportDialog();

// Dialog Data
	enum { IDD = IDD_SAVEASDLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedMeshbrowse();
public:
	afx_msg void OnBnClickedSkelbrowse();
public:
	CString m_szMeshPath;
public:
	CString m_szSkelPath;
public:
	CString m_szMeshName;
public:
	CString m_szSkelName;
public:
	virtual BOOL OnInitDialog();
public:
	BOOL m_bMeshExport;
public:
	BOOL m_bSkelExport;
public:
	afx_msg void OnBnClickedMeshexport();
public:
	afx_msg void OnBnClickedSkelexport();
	void UpdateActiveObjects(void);
};
