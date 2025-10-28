// AnimDialog.cpp : implementation file
//

#include "stdafx.h"
#include "LMEdit.h"
#include "AnimDialog.h"
#include "lg_func.h"


// CAnimDialog dialog

IMPLEMENT_DYNAMIC(CAnimDialog, CDialog)

CAnimDialog::CAnimDialog(CWnd* pParent /*=NULL*/)
	: CDialog(CAnimDialog::IDD, pParent)
	, m_pSkel(NULL)
{

}

CAnimDialog::~CAnimDialog()
{
}

void CAnimDialog::DoDataExchange(CDataExchange* pDX)
{
	if(pDX->m_bSaveAndValidate)
	{
		ModifySkeleton();
	}
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_ANIMS, m_ListCtrl);
	DDX_Control(pDX, IDC_LOOPTYPE, m_cLoopBox);
}


BEGIN_MESSAGE_MAP(CAnimDialog, CDialog)
	ON_WM_CREATE()
	ON_BN_CLICKED(IDC_ADD_ANIM, &CAnimDialog::OnBnClickedAddAnim)
	ON_BN_CLICKED(IDC_DELETE_ANIM, &CAnimDialog::OnBnClickedDeleteAnim)
END_MESSAGE_MAP()


// CAnimDialog message handlers

BOOL CAnimDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	CListCtrl* pCtrl=(CListCtrl*)GetDlgItem(IDC_ANIMS);
	
	pCtrl->InsertColumn(ANIM_NAME, _T("Name"), LVCFMT_LEFT, 100);
	pCtrl->InsertColumn(ANIM_FIRST_FRAME, _T("First Frame"), LVCFMT_LEFT, 75);
	pCtrl->InsertColumn(ANIM_NUM_FRAMES, _T("Frame Count"), LVCFMT_LEFT, 75);
	pCtrl->InsertColumn(ANIM_LOOPING_FRAMES, _T("Looping Frames"), LVCFMT_LEFT, 90);
	pCtrl->InsertColumn(ANIM_RATE, _T("Framerate"), LVCFMT_LEFT, 75);
	m_ListCtrl.InsertColumn(ANIM_FLAGS, _T("Flags"), LVCFMT_LEFT, 75);
	if(!m_pSkel || !m_pSkel->IsLoaded())
	{
		MessageBox(
			"You need to load a skeleton before\n you can edit the animations!", 
			0, 
			MB_ICONINFORMATION);
			
		CButton* pOkButton=(CButton*)GetDlgItem(IDOK);
		pOkButton->EnableWindow(FALSE);
		EndDialog(-1);
		return TRUE;
	}
	
	for(lg_long i=m_pSkel->GetNumAnims()-1; i>=0; i--)
	{
		const CLSkel2::SkelAnim* pAnim=m_pSkel->GetAnim(i);
		AddAnimation(pAnim);
	}
	/*
	for(lg_dword i=0; i<m_pSkel->GetNumAnims(); i++)
	{
		LSKEL_ANIM* pAnim=m_pSkel->GetAnim(i);
		AddAnimation(pAnim);
	}
	*/
	
	CString szTemp;
	
	szTemp.Format("%d", m_pSkel->GetNumFrames());
	GetDlgItem(IDC_AVAILABLEFRAMES)->SetWindowText(szTemp);
	
	//Set some initial values for the animations
	SetDlgItemText(IDC_ANIM_NAME, _T(""));
	SetDlgItemText(IDC_ANIM_FIRST_FRAME, _T("0"));
	SetDlgItemText(IDC_ANIM_NUM_FRAMES, _T("1"));
	SetDlgItemText(IDC_ANIM_LOOPING_FRAMES, _T("0"));
	SetDlgItemText(IDC_ANIM_RATE, _T("30.0"));
	m_cLoopBox.SetCurSel(0);
	
	//Set focus to the name control.
	GetDlgItem(IDC_ANIM_NAME)->SetFocus();
	return FALSE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CAnimDialog::SetSkel(CLSkel2* pSkel)
{
	m_pSkel=pSkel;
}

BOOL CAnimDialog::AddAnimation(const CLSkel2::SkelAnim* pAnim)
{
	if(!pAnim)
		return FALSE;
	/*
	CListCtrl* pCtrl=(CListCtrl*)GetDlgItem(IDC_ANIMS);	
	if(!pCtrl)
		return FALSE;
	*/
	CString szNumber;
	
	m_ListCtrl.InsertItem(0, pAnim->szName);
	szNumber.Format("%d", pAnim->nFirstFrame);
	m_ListCtrl.SetItemText(0, ANIM_FIRST_FRAME, szNumber);
	szNumber.Format("%d", pAnim->nNumFrames);
	m_ListCtrl.SetItemText(0, ANIM_NUM_FRAMES, szNumber);
	szNumber.Format("%d", pAnim->nLoopingFrames);
	m_ListCtrl.SetItemText(0, ANIM_LOOPING_FRAMES, szNumber);
	szNumber.Format("%f", pAnim->fRate);
	m_ListCtrl.SetItemText(0, ANIM_RATE, szNumber);
	
	szNumber="";
	if(LG_CheckFlag(pAnim->nFlags, pAnim->ANIM_FLAG_LOOPBACK))
	{
		szNumber.AppendChar('B');
	}
	m_ListCtrl.SetItemText(0, ANIM_FLAGS, szNumber);
	return TRUE;
}


void CAnimDialog::OnBnClickedAddAnim()
{
	CLSkel2::SkelAnim Anim;
	
	CListCtrl* pCtrl=(CListCtrl*)GetDlgItem(IDC_ANIMS);	
	if(!pCtrl)
		return;
		
	
	
	
	CString szTemp;
	GetDlgItemText(IDC_ANIM_NAME, szTemp);
	if(szTemp.IsEmpty())
	{
		MessageBox("A name must be specified for the animation!", 0, MB_ICONINFORMATION);
		GetDlgItem(IDC_ANIM_NAME)->SetFocus();
		return;
	}
	
	for(int i=0; i<pCtrl->GetItemCount(); i++)
	{
		CString szName=pCtrl->GetItemText(i, ANIM_NAME);
		if(szTemp.CompareNoCase(szName)==0)
		{
			MessageBox("That name is already being used for an animation.\nPlease specify a different name.", 0, MB_ICONINFORMATION);
			GetDlgItem(IDC_ANIM_NAME)->SetFocus();
			return;
		}
	}
	
	//_tcsncpy(Anim.szName, szTemp, LMESH_MAX_NAME);
	LG_strncpy(Anim.szName, szTemp, CLMBase::LM_MAX_NAME);
	Anim.nFirstFrame=GetDlgItemInt(IDC_ANIM_FIRST_FRAME);
	Anim.nNumFrames=GetDlgItemInt(IDC_ANIM_NUM_FRAMES);
	Anim.nLoopingFrames=GetDlgItemInt(IDC_ANIM_LOOPING_FRAMES);
	GetDlgItemText(IDC_ANIM_RATE, szTemp);
	Anim.fRate=(float)atof(szTemp);
	
	//Here is where we set all the flags.
	szTemp="";
	Anim.nFlags=0;
	if(m_cLoopBox.GetCurSel()==1)
	{
		Anim.nFlags|=Anim.ANIM_FLAG_LOOPBACK;//szTemp.AppendChar('B');
	}
	
	//Check to make sure the variables are within bounds.
	BOOL bError=FALSE;
	lg_dword nNumFrames=m_pSkel?m_pSkel->GetNumFrames():0;
	szTemp="Please correct the following problems with the animation:\n";
	if(Anim.nFirstFrame>nNumFrames)
	{
		bError=TRUE;
		szTemp.Append("The first frame is out of range.\n");
	}
	if((Anim.nFirstFrame+Anim.nNumFrames-1)>nNumFrames)
	{
		bError=TRUE;
		szTemp.Append("The number of frames is out of range.\n");
	}
	if(Anim.nLoopingFrames>Anim.nNumFrames)
	{
		bError=TRUE;
		szTemp.Append("There are more looping frames than total frames in the animation.\n");
	}
	
	if(bError)
	{
		MessageBox(szTemp, 0, MB_ICONINFORMATION);
		return;
	}
	//Clear all the entries and set the focus back to IDC_ANIM_NAME.
	SetDlgItemText(IDC_ANIM_NAME, _T(""));
	SetDlgItemText(IDC_ANIM_FIRST_FRAME, _T("0"));
	SetDlgItemText(IDC_ANIM_NUM_FRAMES, _T("1"));
	SetDlgItemText(IDC_ANIM_LOOPING_FRAMES, _T("0"));
	//SetDlgItemText(IDC_ANIM_RATE, _T("30.0"));
	m_cLoopBox.SetCurSel(0);
	GetDlgItem(IDC_ANIM_NAME)->SetFocus();
	
	//Add the animation
	AddAnimation(&Anim);
}

void CAnimDialog::OnBnClickedDeleteAnim()
{
	CListCtrl* pCtrl=(CListCtrl*)GetDlgItem(IDC_ANIMS);
	if(!pCtrl)
		return;
		
	int nSelection=pCtrl->GetSelectionMark();
	
	if(nSelection==-1)
		return;
		
	pCtrl->DeleteItem(nSelection);
}

BOOL CAnimDialog::ModifySkeleton(void)
{
	CLSkel2::SkelAnim* pAnims=new CLSkel2::SkelAnim[m_ListCtrl.GetItemCount()];
	if(!pAnims)
		return FALSE;
		
	for(int i=0; i<m_ListCtrl.GetItemCount(); i++)
	{
		CString szTemp;
		m_ListCtrl.GetItemText(i, ANIM_NAME, pAnims[i].szName, CLMBase::LM_MAX_NAME);
		szTemp=m_ListCtrl.GetItemText(i, ANIM_FIRST_FRAME);
		pAnims[i].nFirstFrame=_ttol(szTemp);
		szTemp=m_ListCtrl.GetItemText(i, ANIM_NUM_FRAMES);
		pAnims[i].nNumFrames=_ttol(szTemp);
		szTemp=m_ListCtrl.GetItemText(i, ANIM_LOOPING_FRAMES);
		pAnims[i].nLoopingFrames=_ttol(szTemp);
		szTemp=m_ListCtrl.GetItemText(i, ANIM_RATE);
		pAnims[i].fRate=(float)atof(szTemp);
		
		szTemp=m_ListCtrl.GetItemText(i, ANIM_FLAGS);
		pAnims[i].nFlags=0;
		for(int j=0; j<szTemp.GetLength(); j++)
		{
			switch(szTemp[j])
			{
			default:
				break;
			case 'B':
			case 'b':
				pAnims[i].nFlags|=pAnims[i].ANIM_FLAG_LOOPBACK;
				break;
			}
		}
	}
	CLSkelEdit* pSkel=(CLSkelEdit*)(m_pSkel);
	if(!pSkel->SetAnims(m_ListCtrl.GetItemCount(), pAnims))
		MessageBox("An error occured while modifying the\nanimations, no changes were made!", 0, MB_ICONERROR);
	
	LG_SafeDeleteArray(pAnims);
	return TRUE;
}
