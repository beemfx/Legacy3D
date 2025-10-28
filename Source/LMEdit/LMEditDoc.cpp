// LMEditDoc.cpp : implementation of the CLMEditDoc class
//

#include "stdafx.h"
#include "LMEdit.h"

#include "LMEditDoc.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CLMEditDoc

IMPLEMENT_DYNCREATE(CLMEditDoc, CDocument)

BEGIN_MESSAGE_MAP(CLMEditDoc, CDocument)
	ON_COMMAND(ID_ANIMATION_EDITANIMATIONS, &CLMEditDoc::OnAnimationEditanimations)
	ON_COMMAND(ID_ANIMATION_IMPORTFRAMES, &CLMEditDoc::OnAnimationImportframes)
	ON_COMMAND(ID_ANIMATION_CALCULATEAABB, &CLMEditDoc::OnAnimationCalculateaabb)
END_MESSAGE_MAP()


// CLMEditDoc construction/destruction

CLMEditDoc::CLMEditDoc()
: m_pDevice(NULL)
{
	// TODO: add one-time construction code here
	OutputDebugString("Document class is being created!\n");
}

CLMEditDoc::~CLMEditDoc()
{
	OutputDebugString("Document class is being destroyed!\n");
}

BOOL CLMEditDoc::OnNewDocument()
{
	if (!CDocument::OnNewDocument())
		return FALSE;

	m_cMesh.Unload();
	m_cSkel.Unload();
	return TRUE;
}


lg_uint CLMEditDoc::Doc_Read(void* buffer, lg_uint size, lg_uint count, void* stream)
{
	CFile* par=(CFile*)stream;
	return par->Read(buffer, size*count);
}

lg_uint CLMEditDoc::Doc_Write(void* buffer, lg_uint size, lg_uint count, void* stream)
{
	CFile* par=(CFile*)stream;
	par->Write(buffer, size*count);
	return count;
}

void CLMEditDoc::Serialize(CArchive& ar)
{
	OutputDebugString("Serialize Being Called.\n");
	if (ar.IsStoring())
	{
		
	}
	else
	{
				
	}
}


// CLMEditDoc diagnostics

#ifdef _DEBUG
void CLMEditDoc::AssertValid() const
{
	CDocument::AssertValid();
}

void CLMEditDoc::Dump(CDumpContext& dc) const
{
	CDocument::Dump(dc);
}
#endif //_DEBUG


// CLMEditDoc commands

void CLMEditDoc::OnCloseDocument()
{
	// TODO: Add your specialized code here and/or call the base class
	Err_Printf("Closing %s.\n", this->GetPathName());
	m_cMesh.Unload();
	m_cSkel.Unload();
	CDocument::OnCloseDocument();
}

BOOL CLMEditDoc::SetDevice(IDirect3DDevice9* lpDevice)
{
	//Make sure the current device gets
	//released if it exists.
	LG_SafeRelease(m_pDevice);
	//Set the new device. (Really this function is only called initially
	//when the device is created, then again when the window is destroyed.
	if(lpDevice)
	{
		m_pDevice=lpDevice;
		m_pDevice->AddRef();
		return TRUE;
	}
	return FALSE;
}

void CLMEditDoc::OnAnimationEditanimations()
{
	CAnimDialog dlgAnim;
	dlgAnim.SetSkel(&m_cSkel);
	if(dlgAnim.DoModal()==IDOK)
	{
		UpdateAllViews(NULL);
		SetModifiedFlag(TRUE);
	}
}

void CLMEditDoc::OnAnimationImportframes()
{
	//CLSkelEdit* pSkel=(CLSkelEdit*)m_cMesh.GetSkel();
	
	if(!m_cSkel.IsLoaded())//pSkel || !pSkel->IsLoaded())
	{
		AfxMessageBox("A skeleton must be loaded to import frames!", MB_ICONINFORMATION);
		return;
	}
	
	CFileDialog dlgOpen(
		TRUE, _T("lskl"), 
		0, 
		0, 
		_T("Skeleton Files (*.lskl;*.lmsh)|*.lskl;*.lmsh|All Files (*.*)|*.*|"),
		AfxGetMainWnd());
	if(dlgOpen.DoModal()!=IDOK)
		return;
		
	CFile cFile(dlgOpen.GetPathName(), CFile::modeRead);
	
	if(cFile.m_hFile==CFile::hFileNull)
	{
		AfxMessageBox(CString("An error occured while attempting to open ") + dlgOpen.GetFileName());
		return;
	}
	DWORD nHeader=0;
	cFile.Read(&nHeader, 4);
	
	//We now search for the LSKEL_ID, since it isn't gauranteed
	//to be at the beginning of the file we just search the whole thing.
	ULONGLONG nLength=0, nPosition=0;
	nLength=cFile.GetLength();
	for(nPosition=0; nPosition<nLength; nPosition++)
	{
		cFile.Seek(nPosition, CFile::begin);
		cFile.Read(&nHeader, 4);
		if(nHeader==CLSkel2::LM_SKEL_ID)
			break;
	}
	if(nPosition==nLength)
	{
		AfxMessageBox(CString(cFile.GetFileName()) + " does not contain a skeleton!", MB_ICONINFORMATION);
		return;
	}
	
	cFile.Seek(-4, CFile::current);
	cFile.Close();
	
	//We are now at the beginning of the skeleton.
	CLSkelEdit cTempSkel;

	BOOL bResult=cTempSkel.Load(dlgOpen.GetPathName().GetBuffer());
	
	if(bResult)
	{
		bResult=m_cSkel.ImportFrames(&cTempSkel);//((CLSkelEdit*)&m_cSkel)->ImportFrames((CLSkelEditD3D*)&cTempSkel);
	}
	cTempSkel.Unload();
	
	if(bResult)
	{
		UpdateAllViews(NULL);
		SetModifiedFlag();
		AfxMessageBox("Successfully imported frames!", MB_ICONINFORMATION);
	}
	else
		AfxMessageBox("Could not import frames!", MB_ICONERROR);
}

BOOL CLMEditDoc::OnSaveDocument(LPCTSTR lpszPathName)
{
	// TODO: Add your specialized code here and/or call the base class
	//AfxMessageBox(CString(lpszPathName) + "\n" + m_szSkelFilename + "\n" + m_szMeshFilename);
	OnSaveMesh(m_szMeshFilename);
	OnSaveSkel(m_szSkelFilename);
	SetModifiedFlag(FALSE);
	return TRUE;
}

BOOL CLMEditDoc::OnOpenDocument(LPCTSTR lpszPathName)
{
	/*
	if (!CDocument::OnOpenDocument(lpszPathName))
		return FALSE;
	*/
	CFile File(lpszPathName, CFile::modeRead|CFile::shareDenyWrite);
	//The first thing we need to do is find out if we are loading
	//a skeleton or a mesh.
	DWORD dwType;
	File.Read(&dwType, 4);
	File.SeekToBegin();
	CString szFilename=File.GetFilePath();
	
	if(dwType==CLMesh2::LM_MESH_ID)
	{
		File.Close();
		m_szMeshFilename=szFilename;
		m_cMesh.Load((lg_char*)m_szMeshFilename.GetBuffer());
	}
	else if(dwType==CLSkel2::LM_SKEL_ID)
	{
		File.Close();
		//Load the skeleton (in the future the files may not be attached at all)
		m_szSkelFilename=szFilename;
		m_cSkel.Load((lg_char*)m_szSkelFilename.GetBuffer());
	}
	else
	{
		return FALSE;//Not a valid file.
	}
	
	m_cMesh.SetCompatibleWith(&m_cSkel);
	
	UpdateAllViews(NULL, 0x00000011);
	return TRUE;
}

BOOL CLMEditDoc::DoSave(LPCTSTR lpszPathName, BOOL bReplace)
{
	CString szNewName=lpszPathName;
	if(szNewName.IsEmpty())
	{
		//If the filename was empty it means that save as
		//was called.
		CDlgSaveAs dlgSaveAs(m_szMeshFilename, m_szSkelFilename);
		if(dlgSaveAs.DoModal()==IDCANCEL)
			return FALSE;
			
		if(!dlgSaveAs.m_szSkelFile.IsEmpty())
			m_szSkelFilename=dlgSaveAs.m_szSkelFile;
		if(!dlgSaveAs.m_szMeshFile.IsEmpty())
			m_szMeshFilename=dlgSaveAs.m_szMeshFile;
			
		lpszPathName=m_szMeshFilename;
	}
	return CDocument::DoSave(lpszPathName, bReplace);
}

BOOL CLMEditDoc::OnSaveSkel(LPCTSTR lpszPathName)
{
	/*
	CFile File;
	if(!File.Open(lpszPathName, CFile::modeCreate|CFile::modeWrite|CFile::shareDenyWrite))
	{	
		OutputDebugString(CString("Could not open file ") + lpszPathName + " to save skel.\n");
		return FALSE;
	}	
	*/
	return m_cSkel.Save((lg_char*)lpszPathName);
}

BOOL CLMEditDoc::OnSaveMesh(LPCTSTR lpszPathName)
{
	return m_cMesh.Save((lg_char*)lpszPathName);//, Doc_Write);
}

void CLMEditDoc::SetTitle(LPCTSTR /*lpszTitle*/)
{
	// TODO: Add your specialized code here and/or call the base class
	TCHAR *szMesh, *szSkel;
	//L_GetNameFromPath(szMesh, m_szMeshFilename);
	//L_GetNameFromPath(szSkel, m_szSkelFilename);
	szMesh=m_szMeshFilename.GetBuffer();
	szSkel=m_szSkelFilename.GetBuffer();
	CString szTitle=CString("Mesh: ")+ 
	szMesh + 
	" Skeleton: " + 
	szSkel;
	CDocument::SetTitle(szTitle);
	//CDocument::SetTitle(lpszTitle);
}

void CLMEditDoc::OnAnimationCalculateaabb()
{
	if(m_cSkel.IsLoaded())
	{
		m_cSkel.CalculateAABBs(0.0f);
		SetModifiedFlag();
	}
	else
	{
		AfxMessageBox(_T("A skeleton must be loaded to calculate AABBs!"), MB_ICONINFORMATION);
	}
}
