#include <d3dx9.h>
#include "common.h"
#include "lv_img2d.h"
#include "lf_sys2.h"
#include "lg_func.h"
#include "lg_tmgr.h"


lg_dword g_ViewWidth=0, g_ViewHeight=0;
lg_bool g_bDrawingStarted=LG_FALSE;

/*************************************************
*** CLImg2D - The Legacy Engine 2D Image Class ***
*************************************************/

CLImg2D::CLImg2D():
	m_pDevice(0),
	m_pTexture(0),
	m_pVB(0),
	m_dwWidth(0),
	m_dwHeight(0),
	m_bIsColor(0),
	m_bFromTexture(0),
	m_pCreate(0),
	m_dwTransparent(0)
{
	memset(m_Vertices, 0, sizeof(m_Vertices));
	memset(m_szFilename, 0, sizeof(m_szFilename));
	memset(&m_rcSrc, 0, sizeof(m_rcSrc));
}

CLImg2D::~CLImg2D()
{
	Delete();
}

void CLImg2D::Delete()
{
	/* Release the texture and VB. */
	//L_safe_release(m_pTexture);
	m_pTexture=0;
	L_safe_release(m_pVB);
	L_safe_release(m_pDevice);
	
	
	m_pDevice=0;
	m_pTexture=0;
	m_pVB=0;
	m_dwWidth=0;
	m_dwHeight=0;
	m_bIsColor=0;
	m_bFromTexture=0;
	m_pCreate=0;
	m_dwTransparent=0;

	memset(m_Vertices, 0, sizeof(m_Vertices));
	memset(m_szFilename, 0, sizeof(m_szFilename));
	memset(&m_rcSrc, 0, sizeof(m_rcSrc));
}

lg_bool CLImg2D::CreateFromFile(
	IDirect3DDevice9* lpDevice,
	char* szFilename,
	lg_rect* rcSrc,
	lg_dword dwWidth,
	lg_dword dwHeight,
	lg_dword dwTransparent)
{
	Delete();
	
	HRESULT hr=0;
	if(!lpDevice)
		return LG_FALSE;

	m_pTexture=NULL;
	m_pVB=NULL;
	m_pCreate=LG_NULL;
	m_dwWidth=dwWidth;
	m_dwHeight=dwHeight;
	LG_strncpy(m_szFilename, szFilename, LG_MAX_PATH);
	m_dwTransparent=dwTransparent;
	if(rcSrc)
		memcpy(&m_rcSrc, rcSrc, sizeof(lg_rect));
	else
		memset(&m_rcSrc, 0, sizeof(lg_rect));
	m_pDevice=lpDevice;
	m_pDevice->AddRef();
	m_bIsColor=LG_FALSE;
	m_bFromTexture=LG_FALSE;

	/* Create the texture and vertex buffer by validating the image.*/
	if(!Validate(LG_NULL))
	{
		L_safe_release(m_pDevice);
		return LG_FALSE;
	}
	return LG_TRUE;
}

lg_bool CLImg2D::CreateFromColor(
	IDirect3DDevice9* lpDevice,
	lg_dword dwWidth,
	lg_dword dwHeight,
	lg_dword dwColor)
{
	Delete();
	
	HRESULT hr=0;
	if(!lpDevice)
		return LG_FALSE;

	m_pTexture=LG_NULL;
	m_pVB=LG_NULL;


	m_dwHeight=dwHeight;
	m_dwWidth=dwWidth;

	m_bIsColor=LG_TRUE;

	SetVertices(LG_NULL, 0.0f, 0.0f, (float)dwWidth, (float)dwHeight);

	m_pDevice=lpDevice;
	m_pDevice->AddRef();

	/* Set the color. */
	m_Vertices[0].Diffuse=dwColor;
	m_Vertices[1].Diffuse=dwColor;
	m_Vertices[2].Diffuse=dwColor;
	m_Vertices[3].Diffuse=dwColor;

	/* Create the vertex buffer by validate image. */
	if(!Validate(LG_NULL))
	{
		//L_safe_release(m_pTexture);
		m_pTexture=0;
		L_safe_release(m_pDevice);
		return LG_FALSE;
	}
	return LG_TRUE;
}

lg_bool CLImg2D::Render(float x, float y)
{
	HRESULT hr=0;
	D3DXMATRIX TempMatrix;

	if(!m_pVB)
		return FALSE;

	/* The start stop drawing function needs to be called
		prior to 2D rendering. */
	if(!g_bDrawingStarted)
		return LG_FALSE;

	
	D3DXMatrixIdentity(&TempMatrix);
	D3DXMatrixTranslation(&TempMatrix, x-(float)(g_ViewWidth/2), (float)(g_ViewHeight/2)-y, 0.0f);
	m_pDevice->SetTransform(D3DTS_WORLD, &TempMatrix);
	
	/* Set the texture. */
	//m_pDevice->SetTexture(0, (LPDIRECT3DBASETEXTURE9)m_pTexture);
	CLTMgr::TM_SetTexture(m_pTexture, 0);
	/* Render the image. */
	m_pDevice->SetStreamSource(0, m_pVB, 0, sizeof(LV2DIMGVERTEX));
	m_pDevice->DrawPrimitive(D3DPT_TRIANGLEFAN, 0, 2);
	return LG_TRUE;
}

lg_bool CLImg2D::Validate(void* pExtra)
{
	HRESULT hr=0;
	LPVOID lpVertices=LG_NULL;
	LF_FILE3 fin=LG_NULL;
	D3DSURFACE_DESC TexDesc;
	lg_rect* rcSrc=LG_NULL;

	if(m_pVB)
		return LG_FALSE;

	if(m_bFromTexture && pExtra)
	{
		memcpy(&m_pTexture, pExtra, sizeof(tm_tex));
		//m_pTexture=(tm_tex)pExtra;
		//IDirect3DTexture9_AddRef(m_pTexture);
	}

	/* Reload the texture. */
	if(!m_bIsColor)
	{
		if(!m_bFromTexture)
		{
			/*
			if(!Tex_Load(
				m_pDevice, 
				m_szFilename,  
				D3DPOOL_DEFAULT,
				m_dwTransparent,
				LG_TRUE,
				&m_pTexture))
				return LG_FALSE;
			*/
			m_pTexture=CLTMgr::TM_LoadTex(m_szFilename, CLTMgr::TM_FORCENOMIP);//Tex_Load2(m_szFilename, LG_TRUE);
			if(!m_pTexture)
				return LG_FALSE;
		}

		memset(&TexDesc, 0, sizeof(TexDesc));
		#if 0
		m_pTexture->GetLevelDesc(0, &TexDesc);

		if(m_dwHeight==-1)
			m_dwHeight=TexDesc.Height;

		if(m_dwWidth==-1)
			m_dwWidth=TexDesc.Width;

		if(m_rcSrc.bottom==0 && m_rcSrc.right==0)
			rcSrc=LG_NULL;
		else
			rcSrc=&m_rcSrc;
		#endif
		SetVertices(
			rcSrc,
			(float)TexDesc.Width, 
			(float)TexDesc.Height, 
			(float)m_dwWidth, 
			(float)m_dwHeight);
		
	}


	/* Create the vertex buffer. */
	hr=m_pDevice->CreateVertexBuffer(
		sizeof(LV2DIMGVERTEX)*4,
		0,
		LV2DIMGVERTEX_TYPE,
		D3DPOOL_DEFAULT,
		&m_pVB,
		NULL);

		if(LG_FAILED(hr))
		{
			//L_safe_release(m_pTexture);
			m_pTexture=0;
			return LG_FALSE;
		}

	/* Copy over the transformed vertices. */
	hr=m_pVB->Lock(0, sizeof(LV2DIMGVERTEX)*4, &lpVertices, 0);
	if(LG_FAILED(hr))
	{
		//L_safe_release(m_pTexture);
		m_pTexture=0;
		return LG_FALSE;
	}
	memcpy(lpVertices, &m_Vertices, sizeof(LV2DIMGVERTEX)*4);
	hr=m_pVB->Unlock();

	return LG_TRUE;
}

void CLImg2D::Invalidate()
{
	/* Release the vertex buffer and texture.*/
	//L_safe_release(m_pTexture);
	m_pTexture=0;
	L_safe_release(m_pVB);
}


__inline void CLImg2D::SetVertices(
	lg_rect* rcSrc,
	float fTexWidth,
	float fTexHeight,
	float fWidth, 
	float fHeight)
{
	/* If no src rectangle was passed we use
		the whole thing. Otherwise we adjust
		the texture coordinates appropriately.*/
	if(!rcSrc)
	{
		m_Vertices[0].tu=
		m_Vertices[3].tu=0.0f;

		m_Vertices[1].tu=
		m_Vertices[2].tu=1.0f;

		m_Vertices[2].tv=
		m_Vertices[3].tv=1.0f;

		m_Vertices[0].tv=
		m_Vertices[1].tv=0.0f;
	}
	else
	{	
		float fxscale=fTexWidth;
		m_Vertices[0].tu=
		m_Vertices[3].tu=(float)rcSrc->left/fTexWidth;

		m_Vertices[1].tu=
		m_Vertices[2].tu=(float)(rcSrc->right+rcSrc->left)/fTexWidth;

		m_Vertices[2].tv=
		m_Vertices[3].tv=(float)(rcSrc->bottom+rcSrc->top)/fTexHeight;

		m_Vertices[0].tv=
		m_Vertices[1].tv=(float)(rcSrc->top)/fTexHeight;
	}
	/* Set all the z values to 0.0f. */
	m_Vertices[0].z=
	m_Vertices[1].z=
	m_Vertices[2].z=
	m_Vertices[3].z=0.0f;
	/* Set all the diffuse to white. */
	m_Vertices[0].Diffuse=
	m_Vertices[1].Diffuse=
	m_Vertices[2].Diffuse=
	m_Vertices[3].Diffuse=0xFFFFFFFFl;


	/* Set up the initial vertices. */
	m_Vertices[0].x=0.0f;
	m_Vertices[0].y=0.0f;

	m_Vertices[1].x=fWidth;
	m_Vertices[1].y=0.0f;

	m_Vertices[2].x=fWidth;
	m_Vertices[2].y=-fHeight;

	m_Vertices[3].x=0.0f;
	m_Vertices[3].y=-fHeight;	
}


/* The StartStopDrawing function is used to start or stop drawing,
	the idea behind it is that it saves a few values, sets up the transformation
	matrices for use with 2d drawing, and when you stop drawing, it restores some
	of the saved values. */
lg_bool CLImg2D::StartStopDrawing(IDirect3DDevice9* lpDevice, lg_dword Width, lg_dword Height, lg_bool bStart)
{
	static lg_dword dwPrevFVF=0;
	static lg_dword dwOldCullMode=0;
	static lg_dword dwOldAA=0;
	//static LPDIRECT3DVERTEXSHADER9 lpOldVS=NULL;
	static lg_dword dwOldAlphaEnable=0, dwOldSrcBlend=0, dwOldDestBlend=0, dwOldAlphaA=0;
	static lg_dword dwOldZEnable=0;
	static lg_dword dwOldLighting=0;
	static lg_dword dwOldMipFilter=0;
	static lg_dword dwOldMinFilter=0;
	static lg_dword dwOldMagFilter=0;
	static lg_dword dwOldFill=0;

	if(bStart)
	{
		/* Initialize all the projection matrices for 2d drawing. */
		D3DXMATRIX TempMatrix;
		
		g_ViewWidth=Width;
		g_ViewHeight=Height;
			
		D3DXMatrixOrthoLH(&TempMatrix, (float)Width, (float)Height, 0.0f, 1.0f);
		//D3DXMatrixIdentity(&TempMatrix);
		lpDevice->SetTransform(D3DTS_PROJECTION, &TempMatrix);
		lpDevice->SetTransform(D3DTS_VIEW, D3DXMatrixIdentity(&TempMatrix));

		/* Save the old filter modes. */
		IDirect3DDevice9_GetSamplerState(lpDevice, 0, D3DSAMP_MAGFILTER, &dwOldMagFilter);
		IDirect3DDevice9_GetSamplerState(lpDevice, 0, D3DSAMP_MINFILTER, &dwOldMinFilter);
		IDirect3DDevice9_GetSamplerState(lpDevice, 0, D3DSAMP_MIPFILTER, &dwOldMipFilter);
		IDirect3DDevice9_SetSamplerState(lpDevice, 0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
		IDirect3DDevice9_SetSamplerState(lpDevice, 0, D3DSAMP_MINFILTER, D3DTEXF_NONE);
		IDirect3DDevice9_SetSamplerState(lpDevice, 0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);

		/* Save some rendering state values, they will be restored when this function
			is called with bStart set to FALSE. */
		lpDevice->GetRenderState(D3DRS_LIGHTING, &dwOldLighting);
		lpDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
		/* Get and set FVF. */
		lpDevice->GetFVF(&dwPrevFVF);
		lpDevice->SetFVF(LV2DIMGVERTEX_TYPE);
		/* Get and set vertex shader. */
		//lpDevice->lpVtbl->GetVertexShader(lpDevice, &lpOldVS);
		//lpDevice->lpVtbl->SetVertexShader(lpDevice, NULL);
		/* Get and set cull mode. */
		lpDevice->GetRenderState(D3DRS_CULLMODE, &dwOldCullMode);
		lpDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
		/* Set alpha blending. */	
		lpDevice->GetRenderState(D3DRS_ALPHABLENDENABLE, &dwOldAlphaEnable);
		lpDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);	

		lpDevice->GetRenderState(D3DRS_SRCBLEND, &dwOldSrcBlend);
		lpDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			
		lpDevice->GetRenderState(D3DRS_DESTBLEND, &dwOldDestBlend);
		lpDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
			
		lpDevice->GetTextureStageState(0, D3DTSS_ALPHAARG1, &dwOldAlphaA);
		lpDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);

		/* Get and set z-buffer status. */
		lpDevice->GetRenderState(D3DRS_ZENABLE, &dwOldZEnable);
		lpDevice->SetRenderState(D3DRS_ZENABLE, FALSE);

		lpDevice->GetRenderState(D3DRS_MULTISAMPLEANTIALIAS, &dwOldAA);
		lpDevice->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, LG_FALSE);

		lpDevice->GetRenderState(D3DRS_FILLMODE, &dwOldFill);
		IDirect3DDevice9_SetRenderState(lpDevice, D3DRS_FILLMODE, D3DFILL_SOLID);

		g_bDrawingStarted=LG_TRUE;
	}
	else
	{
		/* Restore all saved values. */
		lpDevice->SetFVF(dwPrevFVF);
		//lpDevice->SetVertexShader(lpOldVS);
		lpDevice->SetRenderState(D3DRS_CULLMODE, dwOldCullMode);

		/* Restore alpha blending state. */
		lpDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, dwOldAlphaEnable);	
		lpDevice->SetRenderState(D3DRS_SRCBLEND, dwOldSrcBlend);	
		lpDevice->SetRenderState(D3DRS_DESTBLEND, dwOldDestBlend);	
		lpDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, dwOldAlphaA);
		/* Restore Z-Buffering status. */
		lpDevice->SetRenderState(D3DRS_ZENABLE, dwOldZEnable);

		lpDevice->SetRenderState(D3DRS_LIGHTING, dwOldLighting);
		lpDevice->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, dwOldAA);
		IDirect3DDevice9_SetRenderState(lpDevice, D3DRS_FILLMODE, dwOldFill);

		IDirect3DDevice9_SetSamplerState(lpDevice, 0, D3DSAMP_MAGFILTER, dwOldMagFilter);
		IDirect3DDevice9_SetSamplerState(lpDevice, 0, D3DSAMP_MINFILTER, dwOldMinFilter);
		IDirect3DDevice9_SetSamplerState(lpDevice, 0, D3DSAMP_MIPFILTER, dwOldMipFilter);


		g_bDrawingStarted=LG_FALSE;
	}
	return LG_TRUE;
}
