// LMEditDoc.h : interface of the CLMEditDoc class
//


#pragma once
#include "lm_mesh_edit.h"
#include "lm_skel_edit.h"


class CLMEditDoc : public CDocument
{
friend class CLMEditView;
protected: // create from serialization only
	CLMEditDoc();
	DECLARE_DYNCREATE(CLMEditDoc)
//Document input output for callbacks:
	static int CLMEditDoc::Doc_Close(void* stream);
	static lg_uint CLMEditDoc::Doc_Read(void* buffer, lg_uint size, lg_uint count, void* stream);
	static lg_uint CLMEditDoc::Doc_Write(void* buffer, lg_uint size, lg_uint count, void* stream);
	static long CLMEditDoc::Doc_Tell(void* stream);
	static int CLMEditDoc::Doc_Seek(void* stream, long offset, int origin);
// Attributes
public:

// Operations
public:

// Overrides
public:
	virtual BOOL OnNewDocument();
	virtual void Serialize(CArchive& ar);

// Implementation
public:
	virtual ~CLMEditDoc();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

protected:

// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
	CLMeshEdit m_cMesh;
	CString       m_szMeshFilename;
	CLSkelEdit    m_cSkel;
	CString       m_szSkelFilename;
public:
	virtual void OnCloseDocument();
	BOOL SetDevice(IDirect3DDevice9* lpDevice);
	IDirect3DDevice9* m_pDevice;
	afx_msg void OnAnimationEditanimations();
public:
	afx_msg void OnAnimationImportframes();
public:
	virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);
public:
	virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
public:
	virtual BOOL DoSave(LPCTSTR lpszPathName, BOOL bReplace = TRUE);
public:
	BOOL OnSaveSkel(LPCTSTR lpszPathName);
public:
	BOOL OnSaveMesh(LPCTSTR lpszPathName);
public:
	virtual void SetTitle(LPCTSTR lpszTitle);
public:
	afx_msg void OnAnimationCalculateaabb();
};


