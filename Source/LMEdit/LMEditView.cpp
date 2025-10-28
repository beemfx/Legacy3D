// LMEditView.cpp : implementation of the CLMEditView class
//
#include "stdafx.h"
#include "LMEdit.h"

#include "LMEditDoc.h"
#include "LMEditView.h"

#define ID_ANIMATION_ANIMATION1 40000
#define ID_ANIMATION_ANIMATION_MAX 41000

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CLMEditView* g_wndPrimary=NULL;
// CLMEditView

IMPLEMENT_DYNCREATE(CLMEditView, CView)

BEGIN_MESSAGE_MAP(CLMEditView, CView)
	// Standard printing commands
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CView::OnFilePrintPreview)
	ON_WM_CREATE()
	ON_WM_DESTROY()
	ON_WM_MOUSEMOVE()
	ON_COMMAND(ID_ANIMATION_SHOWSKELETON, &CLMEditView::OnAnimationShowskeleton)
	ON_COMMAND(ID_MESH_TEXTURED, &CLMEditView::OnMeshTextured)
	ON_COMMAND(ID_MESH_WIREFRAME, &CLMEditView::OnMeshWireframe)
	ON_COMMAND(ID_MESH_SMOOTHSHADED, &CLMEditView::OnMeshSmoothshaded)
	ON_COMMAND(ID_MESH_FLATSHADED, &CLMEditView::OnMeshFlatshaded)
	ON_COMMAND(ID_ANIMATION_DEFAULTMESH, &CLMEditView::OnAnimationDefaultmesh)
	ON_COMMAND_RANGE(ID_ANIMATION_ANIMATION1, ID_ANIMATION_ANIMATION_MAX, &CLMEditView::OnAnimationAnimationRange)
	ON_WM_CONTEXTMENU()
	ON_UPDATE_COMMAND_UI_RANGE(ID_MESH_WIREFRAME, ID_MESH_SMOOTHSHADED, &CLMEditView::OnUpdateMeshRenderMode)
	ON_UPDATE_COMMAND_UI(ID_ANIMATION_SHOWSKELETON, &CLMEditView::OnUpdateAnimationShowskeleton)
	ON_WM_MOUSEWHEEL()
END_MESSAGE_MAP()

// CLMEditView construction/destruction

CLMEditView::CLMEditView()
: m_pD3D(NULL)
, m_pDevice(NULL)
, m_nMouseX(0)
, m_nMouseY(0)
, m_nBBWidth(1024)
, m_nBBHeight(1024)
, m_bRenderSkel(FALSE)
, m_nMeshRenderMode(MESH_RENDER_TEXTURED)
, m_nAnim(0)
{
	D3DXMatrixIdentity(&m_matWorld);
	D3DXMatrixIdentity(&m_matView);
	OutputDebugString("View window class is being created\n");
	g_wndPrimary=this;
}

CLMEditView::~CLMEditView()
{
	OutputDebugString("View window class is being destroyed!\n");
}

BOOL CLMEditView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: Modify the Window class or styles here by modifying
	//  the CREATESTRUCT cs

	return CView::PreCreateWindow(cs);
}

// CLMEditView drawing

void CLMEditView::OnDraw(CDC* /*pDC*/)
{
	CLMEditDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;	
			
	// TODO: add draw code for native data here
	//Render3DScene(FALSE);
	ProcessAnimation();
}


// CLMEditView printing

BOOL CLMEditView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// default preparation
	return DoPreparePrinting(pInfo);
}

void CLMEditView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add extra initialization before printing

}

void CLMEditView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: add cleanup after printing
}


// CLMEditView diagnostics

#ifdef _DEBUG
void CLMEditView::AssertValid() const
{
	CView::AssertValid();
}

void CLMEditView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CLMEditDoc* CLMEditView::GetDocument() const // non-debug version is inline
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CLMEditDoc)));
	return (CLMEditDoc*)m_pDocument;
}
#endif //_DEBUG


// CLMEditView message handlers

int CLMEditView::InitD3D(void)
{		
	m_pD3D=Direct3DCreate9(D3D_SDK_VERSION);
	if(m_pD3D)
	{
		D3DPRESENT_PARAMETERS pp;
		memset(&pp, 0, sizeof(pp));

		pp.BackBufferWidth=m_nBBWidth;
		pp.BackBufferHeight=m_nBBHeight;
		pp.BackBufferFormat=D3DFMT_UNKNOWN;
		pp.BackBufferCount=1;
		pp.SwapEffect=D3DSWAPEFFECT_DISCARD;
		pp.hDeviceWindow=this->m_hWnd;
		pp.Windowed=TRUE;
		pp.EnableAutoDepthStencil=TRUE;
		pp.AutoDepthStencilFormat=D3DFMT_D24S8;
		pp.Flags=D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL;//D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
		pp.FullScreen_RefreshRateInHz=0;
		pp.PresentationInterval=D3DPRESENT_INTERVAL_ONE;
		pp.MultiSampleType=D3DMULTISAMPLE_NONE;
		pp.MultiSampleQuality=0;
		
		
		HRESULT nResult=m_pD3D->CreateDevice(
			D3DADAPTER_DEFAULT, 
			D3DDEVTYPE_HAL, 
			NULL, 
			D3DCREATE_HARDWARE_VERTEXPROCESSING, 
			&pp, 
			&m_pDevice);
		
		if(FAILED(nResult))
		{
			m_pD3D->Release();
			m_pD3D=NULL;
			m_pDevice=NULL;
		}
	}
	
	if( NULL == m_pDevice )
	{
		MessageBox("Error: Could not initialize Direct3D.\nMake sure the latest version is installed.");
		return -1;
	}
	CLMeshEdit::s_pDevice=m_pDevice;

	
	GetDocument()->SetDevice(m_pDevice);
	//Set the render modes
	ResetView();
	SetRenderModes();

	return 1;
}

int CLMEditView::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CView::OnCreate(lpCreateStruct) == -1)
		return -1;

	InitD3D();
	UpdateMenu();
	return 0;
}

void CLMEditView::OnDestroy()
{
	DestroyD3D();
	CView::OnDestroy();
}

int CLMEditView::Render3DScene(BOOL bPrinter=FALSE)
{
	if(!m_pDevice)
		return -1;
	
	HRESULT Res = m_pDevice->BeginScene();

	if( SUCCEEDED( Res ) )
	{
		SetRenderModes();
		m_pDevice->SetTexture( 0 , NULL );

		D3DCOLOR ClearColor = 0xFF5050FF;
		m_pDevice->Clear(0,0,D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,bPrinter?0xFFFFFFFF:ClearColor,1.0f,0);

		m_pDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX*)&m_matWorld);
		m_pDevice->SetTransform(D3DTS_VIEW, &m_matView);

	
		ML_MAT matProj;
		if(bPrinter)
		{
			ML_MatPerspectiveFovLH(&matProj, D3DX_PI*0.25f, 1.0f, 1.0f, 1000.0f);
		}
		else
		{
			ML_MatPerspectiveFovLH(&matProj, D3DX_PI*0.25f, (float)GetWidth()/(float)GetHeight(), .1f, 500.0f);
		}
	
		m_pDevice->SetTransform(D3DTS_PROJECTION, (D3DMATRIX*)&matProj);

		if( false ) // Primitive draw test.
		{
			const D3DMATERIAL9 d3d9mtr={
				{1.0f, 1.0f, 1.0f, 1.0f},//Diffuse
				{1.0f, 1.0f, 1.0f, 1.0f},//Ambient
				{1.0f, 1.0f, 1.0f, 1.0f},//Specular
				{0.0f, 0.0f, 0.0f, 1.0f},//Emissive
				0.0f}; //Power

			m_pDevice->SetMaterial(&d3d9mtr);

			CLMeshEdit::MeshVertex vTest[] =
			{
				{ 1.f,1.f,0  ,   0,0,-1   ,     1,1 },
				{ 1.f,0,0    ,   0,0,-1   ,     0,1 },
				{ -1.f,0,0   ,   0,0,-1   ,     0,0 },
			};

			m_pDevice->SetRenderState(D3DRS_LIGHTING, TRUE);
			m_pDevice->SetFVF(D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX1);
			m_pDevice->DrawPrimitiveUP( D3DPT_TRIANGLELIST , 1 , &vTest , sizeof(CLMeshEdit::MeshVertex) );
		}
	
		CLMeshEdit* pMesh=&GetDocument()->m_cMesh;
		CLSkelEdit* pSkel=&GetDocument()->m_cSkel;
	
		m_pDevice->SetRenderState(D3DRS_LIGHTING, TRUE);	
		pMesh->Render();
		//pMesh->RenderNormals();

		if(m_bRenderSkel)
		{
			m_pDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
			pSkel->Render();
		}
		m_pDevice->EndScene();
		m_pDevice->Present(NULL, NULL, NULL, NULL);
	}
		
	return 0;
}

LONG CLMEditView::GetWidth(void)
{
	CRect rc;
	GetClientRect(&rc);
	return rc.Width();//rc.right-rc.left;
}

LONG CLMEditView::GetHeight(void)
{
	CRect rc;
	GetClientRect(&rc);
	return rc.Height();//rc.right-rc.left;
}

void CLMEditView::OnMouseMove(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default
	if(MK_LBUTTON&nFlags && !(MK_CONTROL&nFlags) && !(MK_SHIFT&nFlags))
	{
		//The basic left mouse button with no modifiers simply
		//rotates the object.	
		D3DXMATRIX MatRot;
		D3DXMatrixRotationY(&MatRot, (float)(m_nMouseX-point.x)/(float)GetWidth()*4.0f);
		m_matWorld=MatRot*m_matWorld;
		
		D3DXMatrixRotationX(&MatRot, (float)(m_nMouseY-point.y)/(float)GetWidth()*4.0f);
		m_matWorld=m_matWorld*MatRot;
		ProcessAnimation();
	}
	else if((MK_LBUTTON&nFlags && MK_CONTROL&nFlags) || MK_MBUTTON&nFlags)
	{
		//The control flag moves the camera up and down.
		m_matView._41+=(float)(m_nMouseX-point.x)*-m_matView._43/1000.0f;
		m_matView._42+=(float)(m_nMouseY-point.y)*m_matView._43/1000.0f;
		ProcessAnimation();
	}
	else if(MK_LBUTTON&nFlags && MK_SHIFT&nFlags)
	{
		//The shift flag zooms the camera in and out.
		m_matView._43+=(float)(m_nMouseY-point.y)*-m_matView._43/1000.0f;
		ProcessAnimation();
	}
	m_nMouseX=point.x;
	m_nMouseY=point.y;
	CView::OnMouseMove(nFlags, point);
}

int CLMEditView::DestroyD3D(void)
{
	int nCount=0;
	GetDocument()->SetDevice(NULL);
	CLMeshEdit::s_pDevice=LG_NULL;
	m_pDevice->SetTexture(0, LG_NULL);
	if(m_pDevice){nCount=m_pDevice->Release();m_pDevice=NULL;}
	Err_Printf("There are %d IDirect3DDevice objects left!\n", nCount);
	if(m_pD3D){nCount=m_pD3D->Release();m_pD3D=NULL;}
	Err_Printf("There are %d IDirect3D objects left!\n", nCount);
	return 0;
}

void CLMEditView::OnPrint(CDC* pDC, CPrintInfo* pInfo)
{
	// TODO: Add your specialized code here and/or call the base class
	RenderToDC(pDC);
	CView::OnPrint(pDC, pInfo);
}

void CLMEditView::RenderToDC(CDC* pDC)
{
	if(!m_pDevice)
		return;
		

	D3DXMATRIX matProj;
	D3DXMatrixIdentity(&matProj);
	D3DXMatrixPerspectiveFovLH(&matProj, D3DX_PI/4.0f, 1.0f, 1.0f, 1000.0f);
	m_pDevice->SetTransform(D3DTS_PROJECTION, &matProj);
	
	Render3DScene(TRUE);
	CDC dcBB;
	CDC dcImage;
	IDirect3DSurface9* pBB;
	//MessageBox("Starting Printing");
	m_pDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &pBB);
	if(FAILED(pBB->GetDC(&dcBB.m_hDC)))
	{
		Err_Printf("Could not get DC from back buffer!\nThe back buffer must be lockable.\n");
		pBB->Release();
		return;
	}
	//Create a bitmap for the surface.
	dcImage.CreateCompatibleDC(&dcBB);
	CBitmap hBM;
	D3DSURFACE_DESC desc;
	//Need to find out the actually bit depth
	//instead of just putting 32.
	pBB->GetDesc(&desc);
	int nBitdepth=desc.Format==D3DFMT_X8R8G8B8?32:16;
	
	hBM.CreateBitmap(pDC->GetDeviceCaps(HORZRES), pDC->GetDeviceCaps(VERTRES), 1, nBitdepth, NULL);
	dcImage.SelectObject(hBM);
	BOOL bResult=dcImage.StretchBlt(
		0, 
		0, 
		pDC->GetDeviceCaps(HORZRES)-1, 
		m_nBBHeight*pDC->GetDeviceCaps(HORZRES)/m_nBBWidth-1,
		&dcBB, 
		0, 
		0, 
		m_nBBWidth, 
		m_nBBHeight, 
		SRCCOPY);
	if(!bResult)
		Err_Printf("Could not copy back buffer to DC!\n");
		
	bResult=pDC->BitBlt(
		0, 
		0, 
		pDC->GetDeviceCaps(HORZRES)-1, 
		m_nBBHeight*pDC->GetDeviceCaps(HORZRES)/m_nBBWidth-1,
		&dcImage, 
		0, 
		0, 
		SRCCOPY);
	if(!bResult)
		Err_Printf("Could not blit image to printing surface!");
		
	
	//MessageBox("Printing!");			
	pBB->ReleaseDC(dcBB.m_hDC);
	pBB->Release();
}

void CLMEditView::OnAnimationAnimationRange(UINT nID)
{
	m_nAnim=nID-ID_ANIMATION_ANIMATION1+1;
	UpdateMenu();
}

BOOL CLMEditView::ProcessAnimation(void)
{	
	CLMeshEdit* pMesh=&GetDocument()->m_cMesh;
	CLSkelEdit* pSkel=&GetDocument()->m_cSkel;
	const CLSkel2::SkelAnim* pAnim=pSkel->IsLoaded() ? pSkel->GetAnim(m_nAnim-1) : NULL;
	
	if(!pAnim)
	{
		pMesh->SetupDefFrame();	
		pSkel->SetupFrame(0, 0, 0.0f);	
		pMesh->DoTransform();
		Render3DScene();
		return TRUE;
	}
	
	static lg_dword nTime=timeGetTime();
	lg_dword nElapsed=timeGetTime()-nTime;
	lg_dword nMSPerAnim=(lg_dword)((float)pAnim->nNumFrames/pAnim->fRate*1000.0f);
	
	if(nElapsed>nMSPerAnim)
	{
		nTime=timeGetTime();	
		nElapsed=timeGetTime()-nTime;
	}
	
	float fTime=(float)nElapsed/(float)nMSPerAnim*100.0f;
	pMesh->SetupFrame(m_nAnim-1, fTime, pSkel);	
	//pMesh->PrepareFrame(m_nFrameTemp1, 1, 0.0f);
	pSkel->SetupFrame(m_nAnim-1, fTime);

	pMesh->DoTransform();
	Render3DScene();
	return TRUE;
}

void CLMEditView::OnAnimationShowskeleton()
{
	m_bRenderSkel=!m_bRenderSkel;
}

void CLMEditView::UpdateMenu(void)
{
	CMenu* pMenu=GetParent()->GetMenu();
	if(!pMenu)
		return;
		
	//We now update the 
	CLSkelEdit* pSkel=&GetDocument()->m_cSkel;
	if(m_nAnim>pSkel->GetNumAnims())
		m_nAnim=0;	
	pMenu->CheckMenuItem(ID_ANIMATION_DEFAULTMESH, m_nAnim==0?MF_CHECKED:MF_UNCHECKED|MF_BYCOMMAND);	
	//Remove menu items
	CMenu* pAnimMenu=pMenu->GetSubMenu(3);
	for(int i=0; i<pAnimMenu->GetMenuItemCount(); i++)
	{
		if(pAnimMenu->GetMenuItemID(i)>=ID_ANIMATION_ANIMATION1)
		{	
			pAnimMenu->DeleteMenu(i, MF_BYPOSITION);
			i--;
		}
	}
	

	for(DWORD i=0; i<pSkel->GetNumAnims(); i++)
	{	
		const CLSkel2::SkelAnim* pAnim=pSkel->GetAnim(i);
		pAnimMenu->InsertMenu(ID_ANIMATION_DEFAULTMESH, MF_BYCOMMAND, ID_ANIMATION_ANIMATION1+i, CString(pAnim->szName) + " Animation");
		if(m_nAnim==(i+1))
			pAnimMenu->CheckMenuItem(ID_ANIMATION_ANIMATION1+i, MF_CHECKED|MF_BYCOMMAND);
	}
	
	this->DrawMenuBar();
}

BOOL CLMEditView::SetRenderModes(void)
{
	if(!m_pDevice)
		return FALSE;
		
	CLMEditDoc* pDoc=GetDocument();
	//The basic light is just a directional light pointing at the character.
	static const D3DLIGHT9 Light={
		D3DLIGHT_DIRECTIONAL, //Type
		{1.0f, 1.0f, 1.0f, 1.0f},//Diffuse
		{0.25f, 0.0f, 0.0f, 1.0f},//Specular
		{0.0f, 0.0f, 0.0f, 1.0f},//Ambient
		{0.0f, 0.0f, 0.0f},//Position
		{0.0f, 0.0f, 1.0f},//Direction
		0.0f,//Range
		0.0f,//Falloff
		0.0f,//Attenuation0
		0.0f,//Attenuation1
		0.0f,//Attenuation2
		0.0f,//Theta
		0.0f};//Phi
	
	m_pDevice->SetLight(0, &Light);
	m_pDevice->LightEnable(0, TRUE);
	
	switch(m_nMeshRenderMode)
	{
	default:
	case MESH_RENDER_TEXTURED:
		m_pDevice->SetRenderState(D3DRS_LIGHTING, TRUE);
		m_pDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
		m_pDevice->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
		pDoc->m_cMesh.SetRenderTexture(LG_TRUE);
		break;
	case MESH_RENDER_WIREFRAME:
		m_pDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
		m_pDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_WIREFRAME);
		pDoc->m_cMesh.SetRenderTexture(LG_TRUE);
		break;
	case MESH_RENDER_SMOOTHSHADED:
		m_pDevice->SetRenderState(D3DRS_LIGHTING, TRUE);
		m_pDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
		m_pDevice->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_GOURAUD);
		pDoc->m_cMesh.SetRenderTexture(LG_FALSE);
		break;
	case MESH_RENDER_FLATSHADED:
		m_pDevice->SetRenderState(D3DRS_LIGHTING, TRUE);
		m_pDevice->SetRenderState(D3DRS_FILLMODE, D3DFILL_SOLID);
		m_pDevice->SetRenderState(D3DRS_SHADEMODE, D3DSHADE_FLAT);
		pDoc->m_cMesh.SetRenderTexture(LG_FALSE);
		break;
	};
	
	/* Set alpha blending. */	
	m_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);	
	m_pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	m_pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	m_pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	return TRUE;
}

void CLMEditView::OnMeshTextured()
{
	m_nMeshRenderMode=(MESH_RENDER_MODE)ID_MESH_TEXTURED;
	//UpdateMenu();
	SetRenderModes();
}

void CLMEditView::OnMeshWireframe()
{
	m_nMeshRenderMode=(MESH_RENDER_MODE)ID_MESH_WIREFRAME;
	//UpdateMenu();
	SetRenderModes();
}


void CLMEditView::OnMeshSmoothshaded()
{
	m_nMeshRenderMode=(MESH_RENDER_MODE)ID_MESH_SMOOTHSHADED;
	//UpdateMenu();
	SetRenderModes();
}

void CLMEditView::OnMeshFlatshaded()
{
	m_nMeshRenderMode=(MESH_RENDER_MODE)ID_MESH_FLATSHADED;
	//UpdateMenu();
	SetRenderModes();
}

void CLMEditView::OnAnimationDefaultmesh()
{
	m_nAnim=0;
	UpdateMenu();
}

void CLMEditView::OnUpdate(CView* /*pSender*/, LPARAM lHint, CObject* /*pHint*/)
{
	UpdateMenu();
	
	//When a new objects is opened the 0x00000011 hint will be sent
	//which means that the view should be reset.
	if(lHint==0x00000011)
		ResetView();
}


void CLMEditView::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	CMenu mnuView;
	mnuView.LoadMenu(_T("VIEW_MENU"));
	CMenu* pContextMenu=mnuView.GetSubMenu(0);
	pContextMenu->TrackPopupMenu(TPM_LEFTALIGN|TPM_LEFTBUTTON|TPM_RIGHTBUTTON, point.x, point.y, AfxGetMainWnd());
}

void CLMEditView::OnUpdateMeshRenderMode(CCmdUI* pCmdUI)
{
	pCmdUI->SetCheck(m_nMeshRenderMode==pCmdUI->m_nID);
}
void CLMEditView::OnUpdateAnimationShowskeleton(CCmdUI *pCmdUI)
{
	pCmdUI->SetCheck(m_bRenderSkel);
}

void CLMEditView::ResetView(void)
{
	CLMeshEdit* pMesh=&GetDocument()->m_cMesh;
	CLSkelEdit* pSkel=&GetDocument()->m_cSkel;
	
	D3DXMatrixIdentity(&m_matWorld);
	D3DXMatrixIdentity(&m_matView);

	if(pMesh->IsLoaded())
	{
		//float fYMin, fYMax, fZMin;
		const ML_AABB* aabb;
		if(aabb=pMesh->GetBoundingBox())//GetMinsAndMaxes(LG_NULL, LG_NULL, &fYMin, &fYMax, &fZMin, LG_NULL))
		{
			m_matView._43=(aabb->v3Max.y-aabb->v3Min.y)*1.5f-aabb->v3Min.z;
			m_matView._42=(-aabb->v3Min.y-aabb->v3Max.y)/2.0f;
		}
	}
	else if( pSkel->IsLoaded() )
	{
		ml_aabb bounds;
		if(pSkel->GetMinsAndMaxes(&bounds))
		{
			m_matView._43=(bounds.v3Max.y-bounds.v3Min.y)*1.5f-bounds.v3Min.z;
			m_matView._42=(-bounds.v3Min.y-bounds.v3Max.y)/2.0f;
		}
	}
	else
	{
		m_matView._43=5.f;
		m_matView._42=0.f;
	}
	SetRenderModes();
}

BOOL CLMEditView::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	m_matView._43+=-zDelta*m_matView._43/1000.0f;

	return CView::OnMouseWheel(nFlags, zDelta, pt);
}
