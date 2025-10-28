// LMEditView.h : interface of the CLMEditView class
//


#pragma once


class CLMEditView : public CView
{
friend class CLMEditDoc;
protected: // create from serialization only
	CLMEditView();
	DECLARE_DYNCREATE(CLMEditView)

// Attributes
public:
	CLMEditDoc* GetDocument() const;

// Operations
public:

// Overrides
public:
	virtual void OnDraw(CDC* pDC);  // overridden to draw this view
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
protected:
	virtual BOOL OnPreparePrinting(CPrintInfo* pInfo);
	virtual void OnBeginPrinting(CDC* pDC, CPrintInfo* pInfo);
	virtual void OnEndPrinting(CDC* pDC, CPrintInfo* pInfo);

// Implementation
public:
	virtual ~CLMEditView();
#ifdef _DEBUG
	virtual void AssertValid() const;
	virtual void Dump(CDumpContext& dc) const;
#endif

private:
	typedef enum _MESH_RENDER_MODE
	{
		MESH_RENDER_DEFAULT=0,
		MESH_RENDER_WIREFRAME=ID_MESH_WIREFRAME,
		MESH_RENDER_FLATSHADED=ID_MESH_FLATSHADED,
		MESH_RENDER_SMOOTHSHADED=ID_MESH_SMOOTHSHADED,
		MESH_RENDER_TEXTURED=ID_MESH_TEXTURED
	}MESH_RENDER_MODE;

//Helper member variables
private:
	IDirect3D9* m_pD3D;
	IDirect3DDevice9* m_pDevice;
	
	BOOL m_bRenderSkel;
	
	int m_nMouseX;
	int m_nMouseY;

	const DWORD m_nBBWidth;
	const DWORD m_nBBHeight;
	
	MESH_RENDER_MODE m_nMeshRenderMode;
	
//Helper member functions
private:
	int InitD3D(void);
	int DestroyD3D(void);
	BOOL SetRenderModes(void);
	void UpdateMenu(void);
	
	LONG GetWidth(void);
	LONG GetHeight(void);
	
	int Render3DScene(BOOL bPrinter);
	void RenderToDC(CDC* pDC);
public:
	BOOL ProcessAnimation(void);
	
// Generated message map functions
protected:
	DECLARE_MESSAGE_MAP()
//Message map functions:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	virtual void OnPrint(CDC* pDC, CPrintInfo* pInfo);
	afx_msg void OnAnimationShowskeleton();
	afx_msg void OnMeshSmoothshaded();
	afx_msg void OnMeshFlatshaded();
	afx_msg void OnMeshTextured();
	afx_msg void OnMeshWireframe();
	afx_msg void OnDestroy();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnAnimationAnimationRange(UINT nID);
	afx_msg void OnUpdateMeshRenderMode(CCmdUI* pCmdUI);

private:
	// The current animation to render.
	DWORD m_nAnim;
	afx_msg void OnAnimationDefaultmesh();
protected:
	virtual void OnUpdate(CView* /*pSender*/, LPARAM /*lHint*/, CObject* /*pHint*/);
public:
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
public:
	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
public:
	afx_msg void OnUpdateAnimationShowskeleton(CCmdUI *pCmdUI);
	D3DXMATRIX m_matWorld;
	D3DXMATRIX m_matView;
public:
	void ResetView(void);
public:
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
};

#ifndef _DEBUG  // debug version in LMEditView.cpp
inline CLMEditDoc* CLMEditView::GetDocument() const
   { return reinterpret_cast<CLMEditDoc*>(m_pDocument); }
#endif

