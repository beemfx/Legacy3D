// DlgSaveAs.cpp : implementation file
//

#include "stdafx.h"
#include "LMEdit.h"
#include "DlgSaveAs.h"


// CDlgSaveAs dialog

IMPLEMENT_DYNAMIC(CDlgSaveAs, CDialog)

CDlgSaveAs::CDlgSaveAs(CWnd* pParent /*=NULL*/)
	: CDialog(CDlgSaveAs::IDD, pParent)
	, m_szMeshFile(_T(""))
	, m_szSkelFile(_T(""))
{

}

CDlgSaveAs::CDlgSaveAs(LPCTSTR szMesh, LPCTSTR szSkel, CWnd* pParent/* = NULL*/)
	: CDialog(CDlgSaveAs::IDD, pParent)
	, m_szMeshFile(szMesh)
	, m_szSkelFile(szSkel)
{
}

CDlgSaveAs::~CDlgSaveAs()
{
}

void CDlgSaveAs::DoDataExchange(CDataExchange* pDX)
{
	DDX_Text(pDX, IDC_MESHFILE, m_szMeshFile);
	DDX_Text(pDX, IDC_SKELFILE, m_szSkelFile);
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDlgSaveAs, CDialog)
	ON_BN_CLICKED(IDC_BROWSEMESH, &CDlgSaveAs::OnBnClickedBrowsemesh)
	ON_BN_CLICKED(IDC_BROWSESKEL, &CDlgSaveAs::OnBnClickedBrowseskel)
END_MESSAGE_MAP()


// CDlgSaveAs message handlers

void CDlgSaveAs::OnBnClickedBrowsemesh()
{
	CFileDialog dlgFile(
		FALSE,
		_T(".lmsh"),
		m_szMeshFile,
		OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,
		_T("Legacy Mesh (*.lmsh)|*.lmsh|All Files (*.*)|*.*||"));
	if(dlgFile.DoModal()!=IDOK)
		return;
		
	SetDlgItemText(IDC_MESHFILE, dlgFile.GetPathName());
}

void CDlgSaveAs::OnBnClickedBrowseskel()
{
	CFileDialog dlgFile(
		FALSE,
		_T(".lskl"),
		m_szSkelFile,
		OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,
		_T("Legacy Skeleton (*.lskl)|*.lskl|All Files (*.*)|*.*||"));
	if(dlgFile.DoModal()!=IDOK)
		return;
		
	SetDlgItemText(IDC_SKELFILE, dlgFile.GetPathName());
}

BOOL CDlgSaveAs::OnInitDialog()
{
	CDialog::OnInitDialog();
	
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}
