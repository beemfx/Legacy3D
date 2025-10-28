// ExportDialog.cpp : implementation file
//

#include "stdafx.h"
#include "msLMEXP.h"
#include "ExportDialog.h"


// CExportDialog dialog

IMPLEMENT_DYNAMIC(CExportDialog, CDialog)

CExportDialog::CExportDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CExportDialog::IDD, pParent)
	, m_szMeshPath(_T(""))
	, m_szSkelPath(_T(""))
	, m_szMeshName(_T(""))
	, m_szSkelName(_T(""))
	, m_bMeshExport(TRUE)
	, m_bSkelExport(TRUE)
{

}

CExportDialog::~CExportDialog()
{
}

void CExportDialog::DoDataExchange(CDataExchange* pDX)
{
	DDX_Text(pDX, IDC_MESHPATH, m_szMeshPath);
	DDX_Text(pDX, IDC_SKELPATH, m_szSkelPath);
	DDX_Text(pDX, IDC_MESHNAME, m_szMeshName);
	DDX_Text(pDX, IDC_SKELNAME, m_szSkelName);
	
	DDX_Check(pDX, IDC_MESHEXPORT, m_bMeshExport);
	DDX_Check(pDX, IDC_SKELEXPORT, m_bSkelExport);
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CExportDialog, CDialog)
	ON_BN_CLICKED(IDC_MESHBROWSE, &CExportDialog::OnBnClickedMeshbrowse)
	ON_BN_CLICKED(IDC_SKELBROWSE, &CExportDialog::OnBnClickedSkelbrowse)
	ON_BN_CLICKED(IDC_MESHEXPORT, &CExportDialog::OnBnClickedMeshexport)
	ON_BN_CLICKED(IDC_SKELEXPORT, &CExportDialog::OnBnClickedSkelexport)
END_MESSAGE_MAP()


// CExportDialog message handlers

void CExportDialog::OnBnClickedMeshbrowse()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CFileDialog dlgFile(
		FALSE,
		_T(".lmsh"),
		NULL,
		OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,
		_T("Legacy Mesh (*.lmsh)|*.lmsh|All Files (*.*)|*.*||"));
	if(dlgFile.DoModal()!=IDOK)
		return;
		
	SetDlgItemText(IDC_MESHPATH, dlgFile.GetPathName());
	CLMBase::LMName szShortName;
	LG_GetShortNameFromPathA(szShortName, dlgFile.GetPathName());
	SetDlgItemText(IDC_MESHNAME, szShortName);
}

void CExportDialog::OnBnClickedSkelbrowse()
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	CFileDialog dlgFile(
		FALSE,
		_T(".lskl"),
		NULL,
		OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT,
		_T("Legacy Skeleton (*.lskl)|*.lskl|All Files (*.*)|*.*||"));
	if(dlgFile.DoModal()!=IDOK)
		return;
		
	SetDlgItemText(IDC_SKELPATH, dlgFile.GetPathName());
	CLMBase::LMName szShortName;
	LG_GetShortNameFromPathA(szShortName, dlgFile.GetPathName());
	SetDlgItemText(IDC_SKELNAME, szShortName);
}

BOOL CExportDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	GetDlgItem(IDC_MESHEXPORT)->EnableWindow(m_bMeshExport);
	GetDlgItem(IDC_SKELEXPORT)->EnableWindow(m_bSkelExport);
	GetDlgItem(IDOK)->EnableWindow(m_bMeshExport||m_bSkelExport);
	GetDlgItem(IDC_VERSION)->SetWindowText(_T("version x.xx ("__DATE__" "__TIME__")"));
	UpdateActiveObjects();
	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CExportDialog::OnBnClickedMeshexport()
{
	UpdateActiveObjects();
}

void CExportDialog::OnBnClickedSkelexport()
{
	UpdateActiveObjects();
}

void CExportDialog::UpdateActiveObjects(void)
{
	//Update mesh export objects
	if(((CButton*)GetDlgItem(IDC_MESHEXPORT))->GetCheck())
	{
		GetDlgItem(IDC_MESHNAME)->EnableWindow(TRUE);
		GetDlgItem(IDC_MESHBROWSE)->EnableWindow(TRUE);
	}
	else
	{
		GetDlgItem(IDC_MESHNAME)->EnableWindow(FALSE);
		GetDlgItem(IDC_MESHBROWSE)->EnableWindow(FALSE);
	}
	
	//Update skeleton export objects
	if(((CButton*)GetDlgItem(IDC_SKELEXPORT))->GetCheck())
	{
		GetDlgItem(IDC_SKELNAME)->EnableWindow(TRUE);
		GetDlgItem(IDC_SKELBROWSE)->EnableWindow(TRUE);
	}
	else
	{
		GetDlgItem(IDC_SKELNAME)->EnableWindow(FALSE);
		GetDlgItem(IDC_SKELBROWSE)->EnableWindow(FALSE);
	}
}
