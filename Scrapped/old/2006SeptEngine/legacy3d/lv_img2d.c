#include "common.h"
#include <d3dx9.h>
#include "lv_tex.h"
#include "lv_img2d.h"
#include <lf_sys.h>

L_dword g_ViewWidth=0, g_ViewHeight=0;
L_bool g_bDrawingStarted=L_false;

/* The L2DI_StartStopDrawing function is used to start or stop drawing,
	the idea behind it is that it saves a few values, sets up the transformation
	matrices for use with 2d drawing, and when you stop drawing, it restores some
	of the saved values. */
L_bool L2DI_StartStopDrawing(IDirect3DDevice9* lpDevice, L_dword Width, L_dword Height, L_bool bStart)
{
	static L_dword dwPrevFVF=0;
	static L_dword dwOldCullMode=0;
	static L_dword dwOldAA=0;
	//static LPDIRECT3DVERTEXSHADER9 lpOldVS=NULL;
	static L_dword dwOldAlphaEnable=0, dwOldSrcBlend=0, dwOldDestBlend=0, dwOldAlphaA=0;
	static L_dword dwOldZEnable=0;
	static L_dword dwOldLighting=0;
	static L_dword dwOldMipFilter=0;
	static L_dword dwOldMinFilter=0;
	static L_dword dwOldMagFilter=0;
	static L_dword dwOldFill=0;

	if(bStart)
	{
		/* Initialize all the projection matrices for 2d drawing. */
		D3DMATRIX TempMatrix;
		
		g_ViewWidth=Width;
		g_ViewHeight=Height;
			
		D3DXMatrixOrthoLH(&TempMatrix, (float)Width, (float)Height, 0.0f, 1.0f);
		lpDevice->lpVtbl->SetTransform(lpDevice, D3DTS_PROJECTION, &TempMatrix);
		lpDevice->lpVtbl->SetTransform(lpDevice, D3DTS_VIEW, D3DXMatrixIdentity(&TempMatrix));

		/* Save the old filter modes. */
		IDirect3DDevice9_GetSamplerState(lpDevice, 0, D3DSAMP_MAGFILTER, &dwOldMagFilter);
		IDirect3DDevice9_GetSamplerState(lpDevice, 0, D3DSAMP_MINFILTER, &dwOldMinFilter);
		IDirect3DDevice9_GetSamplerState(lpDevice, 0, D3DSAMP_MIPFILTER, &dwOldMipFilter);
		IDirect3DDevice9_SetSamplerState(lpDevice, 0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
		IDirect3DDevice9_SetSamplerState(lpDevice, 0, D3DSAMP_MINFILTER, D3DTEXF_NONE);
		IDirect3DDevice9_SetSamplerState(lpDevice, 0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);

		/* Save some rendering state values, they will be restored when this function
			is called with bStart set to FALSE. */
		lpDevice->lpVtbl->GetRenderState(lpDevice, D3DRS_LIGHTING, &dwOldLighting);
		lpDevice->lpVtbl->SetRenderState(lpDevice, D3DRS_LIGHTING, FALSE);
		/* Get and set FVF. */
		lpDevice->lpVtbl->GetFVF(lpDevice, &dwPrevFVF);
		lpDevice->lpVtbl->SetFVF(lpDevice, LV2DIMGVERTEX_TYPE);
		/* Get and set vertex shader. */
		//lpDevice->lpVtbl->GetVertexShader(lpDevice, &lpOldVS);
		//lpDevice->lpVtbl->SetVertexShader(lpDevice, NULL);
		/* Get and set cull mode. */
		lpDevice->lpVtbl->GetRenderState(lpDevice, D3DRS_CULLMODE, &dwOldCullMode);
		lpDevice->lpVtbl->SetRenderState(lpDevice, D3DRS_CULLMODE, D3DCULL_CCW);
		/* Set alpha blending. */	
		lpDevice->lpVtbl->GetRenderState(lpDevice, D3DRS_ALPHABLENDENABLE, &dwOldAlphaEnable);
		lpDevice->lpVtbl->SetRenderState(lpDevice, D3DRS_ALPHABLENDENABLE, TRUE);	

		lpDevice->lpVtbl->GetRenderState(lpDevice, D3DRS_SRCBLEND, &dwOldSrcBlend);
		lpDevice->lpVtbl->SetRenderState(lpDevice, D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			
		lpDevice->lpVtbl->GetRenderState(lpDevice, D3DRS_DESTBLEND, &dwOldDestBlend);
		lpDevice->lpVtbl->SetRenderState(lpDevice, D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
			
		lpDevice->lpVtbl->GetTextureStageState(lpDevice, 0, D3DTSS_ALPHAARG1, &dwOldAlphaA);
		lpDevice->lpVtbl->SetTextureStageState(lpDevice, 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);

		/* Get and set z-buffer status. */
		lpDevice->lpVtbl->GetRenderState(lpDevice, D3DRS_ZENABLE, &dwOldZEnable);
		lpDevice->lpVtbl->SetRenderState(lpDevice, D3DRS_ZENABLE, FALSE);

		lpDevice->lpVtbl->GetRenderState(lpDevice, D3DRS_MULTISAMPLEANTIALIAS, &dwOldAA);
		lpDevice->lpVtbl->SetRenderState(lpDevice, D3DRS_MULTISAMPLEANTIALIAS, L_false);

		lpDevice->lpVtbl->GetRenderState(lpDevice, D3DRS_FILLMODE, &dwOldFill);
		IDirect3DDevice9_SetRenderState(lpDevice, D3DRS_FILLMODE, D3DFILL_SOLID);

		g_bDrawingStarted=L_true;
	}
	else
	{
		/* Restore all saved values. */
		lpDevice->lpVtbl->SetFVF(lpDevice, dwPrevFVF);
		//lpDevice->lpVtbl->SetVertexShader(lpDevice, lpOldVS);
		lpDevice->lpVtbl->SetRenderState(lpDevice, D3DRS_CULLMODE, dwOldCullMode);

		/* Restore alpha blending state. */
		lpDevice->lpVtbl->SetRenderState(lpDevice, D3DRS_ALPHABLENDENABLE, dwOldAlphaEnable);	
		lpDevice->lpVtbl->SetRenderState(lpDevice, D3DRS_SRCBLEND, dwOldSrcBlend);	
		lpDevice->lpVtbl->SetRenderState(lpDevice, D3DRS_DESTBLEND, dwOldDestBlend);	
		lpDevice->lpVtbl->SetTextureStageState(lpDevice, 0, D3DTSS_ALPHAARG1, dwOldAlphaA);
		/* Restore Z-Buffering status. */
		lpDevice->lpVtbl->SetRenderState(lpDevice, D3DRS_ZENABLE, dwOldZEnable);

		lpDevice->lpVtbl->SetRenderState(lpDevice, D3DRS_LIGHTING, dwOldLighting);
		lpDevice->lpVtbl->SetRenderState(lpDevice, D3DRS_MULTISAMPLEANTIALIAS, dwOldAA);
		IDirect3DDevice9_SetRenderState(lpDevice, D3DRS_FILLMODE, dwOldFill);

		IDirect3DDevice9_SetSamplerState(lpDevice, 0, D3DSAMP_MAGFILTER, dwOldMagFilter);
		IDirect3DDevice9_SetSamplerState(lpDevice, 0, D3DSAMP_MINFILTER, dwOldMinFilter);
		IDirect3DDevice9_SetSamplerState(lpDevice, 0, D3DSAMP_MIPFILTER, dwOldMipFilter);


		g_bDrawingStarted=L_false;
	}
	return L_true;
}

__inline void L2DI_SetVertices(
	LV2DIMAGE* lpImage, 
	L_rect* rcSrc,
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
		lpImage->Vertices[0].tu=
		lpImage->Vertices[3].tu=0.0f;

		lpImage->Vertices[1].tu=
		lpImage->Vertices[2].tu=1.0f;

		lpImage->Vertices[2].tv=
		lpImage->Vertices[3].tv=1.0f;

		lpImage->Vertices[0].tv=
		lpImage->Vertices[1].tv=0.0f;
	}
	else
	{	
		float fxscale=fTexWidth;
		lpImage->Vertices[0].tu=
		lpImage->Vertices[3].tu=(float)rcSrc->left/fTexWidth;

		lpImage->Vertices[1].tu=
		lpImage->Vertices[2].tu=(float)(rcSrc->right+rcSrc->left)/fTexWidth;

		lpImage->Vertices[2].tv=
		lpImage->Vertices[3].tv=(float)(rcSrc->bottom+rcSrc->top)/fTexHeight;

		lpImage->Vertices[0].tv=
		lpImage->Vertices[1].tv=(float)(rcSrc->top)/fTexHeight;
	}
	/* Set all the z values to 0.0f. */
	lpImage->Vertices[0].z=
	lpImage->Vertices[1].z=
	lpImage->Vertices[2].z=
	lpImage->Vertices[3].z=0.0f;
	/* Set all the diffuse to white. */
	lpImage->Vertices[0].Diffuse=
	lpImage->Vertices[1].Diffuse=
	lpImage->Vertices[2].Diffuse=
	lpImage->Vertices[3].Diffuse=0xFFFFFFFFl;


	/* Set up the initial vertices. */
	lpImage->Vertices[0].x=0.0f;
	lpImage->Vertices[0].y=0.0f;

	lpImage->Vertices[1].x=fWidth;
	lpImage->Vertices[1].y=0.0f;

	lpImage->Vertices[2].x=fWidth;
	lpImage->Vertices[2].y=-fHeight;

	lpImage->Vertices[3].x=0.0f;
	lpImage->Vertices[3].y=-fHeight;	
}

/*
LV2DIMAGE* L2DI_CreateFromImage(
	LV2DIMAGE* lpImageSrc, 
	L_rect* rsrc, 
	L_dword dwWidth, 
	L_dword dwHeight,
	L_dword dwTransparent)
{
	if(!lpImageSrc)
		return NULL;

	return L2DI_CreateFromTexture(
		lpImageSrc->lpDevice,
		rsrc, 
		lpImageSrc->dwWidth, 
		lpImageSrc->dwHeight, 
		lpImageSrc->lpTexture,
		lpImageSrc);
}
*/

LV2DIMAGE* L2DI_CreateFromFile(
	IDirect3DDevice9* lpDevice,
	char* szFilename,
	L_rect* rcSrc,
	L_dword dwWidth,
	L_dword dwHeight,
	L_dword dwTransparent)
{
	HRESULT hr=0;
	LV2DIMAGE* lpImage=NULL;


	if(!lpDevice)
		return NULL;

	lpImage=malloc(sizeof(LV2DIMAGE));
	if(lpImage==NULL)
		return NULL;

	memset(lpImage, 0, sizeof(LV2DIMAGE));
	lpImage->lpTexture=NULL;
	lpImage->lpVB=NULL;
	lpImage->lpCreate=L_null;
	lpImage->dwWidth=dwWidth;
	lpImage->dwHeight=dwHeight;
	L_strncpy(lpImage->szFilename, szFilename, 259);
	lpImage->dwTransparent=dwTransparent;
	if(rcSrc)
		memcpy(&lpImage->rcSrc, rcSrc, sizeof(L_rect));
	else
		memset(&lpImage->rcSrc, 0, sizeof(L_rect));
	lpImage->lpDevice=lpDevice;
	lpImage->lpDevice->lpVtbl->AddRef(lpImage->lpDevice);
	lpImage->bIsColor=L_false;
	lpImage->bFromTexture=L_false;

	/* Create the texture and vertex buffer by validating the image.*/
	if(!L2DI_Validate(lpImage, lpDevice, L_null))
	{
		L_safe_release( (lpImage->lpDevice) );
		L_safe_free(lpImage);
		return L_null;
	}
	return lpImage;
}


LV2DIMAGE* L2DI_CreateFromTexture(
	IDirect3DDevice9* lpDevice,
	L_rect* rcsrc,
	L_dword dwWidth,
	L_dword dwHeight,
	IDirect3DTexture9* lpTexture,
	L_pvoid* pExtra)
{
	HRESULT hr=0;
	D3DSURFACE_DESC TexDesc;
	LV2DIMAGE* lpImage=NULL;
	if(!lpDevice)
		return NULL;

	lpImage=malloc(sizeof(LV2DIMAGE));
	if(lpImage==NULL)
		return NULL;
	ZeroMemory(&TexDesc, sizeof(D3DSURFACE_DESC));
	lpImage->lpTexture=NULL;
	lpImage->lpVB=NULL;
	lpImage->dwHeight=dwHeight;
	lpImage->dwWidth=dwWidth;
	if(rcsrc)
		lpImage->rcSrc=*rcsrc;

	// Set the texture.
	//lpTexture->lpVtbl->AddRef(lpTexture);

	lpImage->lpTexture=lpTexture;
	lpImage->lpDevice=lpDevice;
	lpImage->lpDevice->lpVtbl->AddRef(lpImage->lpDevice);
	lpImage->bIsColor=L_false;
	lpImage->bFromTexture=L_true;


	//lpImage->lpCreate=(LV2DIMAGE*)pExtra;

	// Create the vertex buffer by validating image.
	if(!L2DI_Validate(lpImage, lpDevice, lpTexture))
	{
		L_safe_release( (lpImage->lpTexture) );
		L_safe_release( (lpImage->lpDevice) );
		L_safe_free(lpImage);
		return NULL;
	}
	
	return lpImage;
}


LV2DIMAGE* L2DI_CreateFromColor(
	IDirect3DDevice9* lpDevice,
	L_dword dwWidth,
	L_dword dwHeight,
	L_dword dwColor)
{
	HRESULT hr=0;
	LV2DIMAGE* lpImage=NULL;
	if(!lpDevice)
		return NULL;

	lpImage=malloc(sizeof(LV2DIMAGE));
	lpImage->lpTexture=NULL;
	lpImage->lpVB=NULL;


	lpImage->dwHeight=dwHeight;
	lpImage->dwWidth=dwWidth;

	lpImage->bIsColor=TRUE;

	L2DI_SetVertices(lpImage, L_null, 0.0f, 0.0f, (float)dwWidth, (float)dwHeight);

	lpImage->lpDevice=lpDevice;
	lpImage->lpDevice->lpVtbl->AddRef(lpImage->lpDevice);

	/* Set the color. */
	lpImage->Vertices[0].Diffuse=dwColor;
	lpImage->Vertices[1].Diffuse=dwColor;
	lpImage->Vertices[2].Diffuse=dwColor;
	lpImage->Vertices[3].Diffuse=dwColor;

	/* Create the vertex buffer by validate image. */
	if(!L2DI_Validate(lpImage, lpDevice, L_null))
	{
		L_safe_release( (lpImage->lpTexture) );
		L_safe_release( (lpImage->lpDevice) );
		L_safe_free(lpImage);
		return L_null;
	}

	return lpImage;
}

BOOL L2DI_Invalidate(LV2DIMAGE* lpImage)
{
	if(!lpImage)
		return FALSE;
	/* Release the vertex buffer.
		The texture is managed so we don't 
		need to release it. */
	L_safe_release( (lpImage->lpTexture) );
	L_safe_release( (lpImage->lpVB) );
	return TRUE;
}

L_bool L2DI_Validate(LV2DIMAGE* lpImage, IDirect3DDevice9* lpDevice, void* pExtra)
{
	HRESULT hr=0;
	LPVOID lpVertices=L_null;
	LF_FILE2 fin=L_null;
	D3DSURFACE_DESC TexDesc;
	L_rect* rcSrc=L_null;

	if(!lpImage)
		return FALSE;

	if( (((LV2DIMAGE*)lpImage)->lpVB) )
		return FALSE;

	if(lpImage->bFromTexture && pExtra)
	{
		lpImage->lpTexture=pExtra;
		IDirect3DTexture9_AddRef(lpImage->lpTexture);
	}

	/* Reload the texture. */
	if(!lpImage->bIsColor)
	{
		if(!lpImage->bFromTexture)
		{
			if(!Tex_Load(
				lpDevice, 
				lpImage->szFilename,  
				D3DPOOL_DEFAULT,
				lpImage->dwTransparent,
				L_true,
				&lpImage->lpTexture))
				return L_false;
		}

		memset(&TexDesc, 0, sizeof(TexDesc));
		lpImage->lpTexture->lpVtbl->GetLevelDesc(lpImage->lpTexture, 0, &TexDesc);

		if(lpImage->dwHeight==-1)
			lpImage->dwHeight=TexDesc.Height;

		if(lpImage->dwWidth==-1)
			lpImage->dwWidth=TexDesc.Width;

		if(lpImage->rcSrc.bottom==0 && lpImage->rcSrc.right==0)
			rcSrc=L_null;
		else
			rcSrc=&lpImage->rcSrc;

		L2DI_SetVertices(
			lpImage, 
			rcSrc,
			(float)TexDesc.Width, 
			(float)TexDesc.Height, 
			(float)lpImage->dwWidth, 
			(float)lpImage->dwHeight);
		
	}


	/* Create the vertex buffer. */
	hr=lpImage->lpDevice->lpVtbl->CreateVertexBuffer(
		lpImage->lpDevice,
		sizeof(LV2DIMGVERTEX)*4,
		0,
		LV2DIMGVERTEX_TYPE,
		D3DPOOL_DEFAULT,
		&lpImage->lpVB,
		NULL);

		if(L_failed(hr))
		{
			L_safe_release(lpImage->lpTexture);
			return L_false;
		}

	/* Copy over the transformed vertices. */
	hr=lpImage->lpVB->lpVtbl->Lock(lpImage->lpVB, 0, sizeof(LV2DIMGVERTEX)*4, &lpVertices, 0);
	if(L_failed(hr))
	{
		L_safe_release(lpImage->lpTexture);
		return L_false;
	}
	memcpy(lpVertices, &lpImage->Vertices, sizeof(LV2DIMGVERTEX)*4);
	hr=lpImage->lpVB->lpVtbl->Unlock(lpImage->lpVB);

	return L_true;
}

BOOL L2DI_Delete(LV2DIMAGE* lpImage)
{
	if(!lpImage)
		return FALSE;
	/* Release the texture and VB. */
	L_safe_release( (lpImage->lpTexture) );
	L_safe_release( (lpImage->lpVB) );
	L_safe_release( (lpImage->lpDevice) );
	L_safe_free(lpImage);
	return TRUE;
}

BOOL L2DI_Render(
	LV2DIMAGE* lpImage,
	IDirect3DDevice9* lpDevice,
	float x,
	float y)
{
	HRESULT hr=0;
	D3DMATRIX TempMatrix;

	if(!lpImage)
		return FALSE;

	if(!lpImage->lpVB)
		return FALSE;

	/* The start stop drawing function needs to be called
		prior to 2D rendering. */
	if(!g_bDrawingStarted)
		return L_false;

	
	D3DXMatrixIdentity(&TempMatrix);
	D3DXMatrixTranslation(&TempMatrix, x-(float)(g_ViewWidth/2), (float)(g_ViewHeight/2)-y, 0.0f);
	lpDevice->lpVtbl->SetTransform(lpDevice, D3DTS_WORLD, &TempMatrix);
	
	/* Set the texture. */
	lpDevice->lpVtbl->SetTexture(lpDevice, 0, (LPDIRECT3DBASETEXTURE9)lpImage->lpTexture);
	/* Render the image. */
	lpDevice->lpVtbl->SetStreamSource(lpDevice, 0, lpImage->lpVB, 0, sizeof(LV2DIMGVERTEX));
	lpDevice->lpVtbl->DrawPrimitive(lpDevice, D3DPT_TRIANGLEFAN, 0, 2);

	return L_true;
}

/*
BOOL L2DI_RenderEx(
	LV2DIMAGE* lpImage,
	L_rect* rcDest,
	L_rect* rcSrc)
{
	LV2DIMAGE FinalImage;
	float fSrcWidth=0.0f;
	float fSrcHeight=0.0f;

	if(!lpImage)
		return FALSE;

	memset(&FinalImage, 0, sizeof(LV2DIMAGE));
	fSrcWidth=(float)(rcSrc->right-rcSrc->left);
	fSrcHeight=(float)(rcSrc->bottom-rcSrc->top);

	FinalImage=*lpImage;
	FinalImage.dwWidth=rcDest->right-rcDest->left;
	FinalImage.dwHeight=rcDest->bottom-rcDest->top;
	
	FinalImage.Vertices[0].x=(float)FinalImage.dwWidth;
	FinalImage.Vertices[0].y=(float)FinalImage.dwHeight;
	FinalImage.Vertices[1].y=(float)FinalImage.dwHeight;
	FinalImage.Vertices[3].x=(float)FinalImage.dwWidth;

	// Set the texture coordinates. 
	FinalImage.Vertices[0].tu=(fSrcWidth+rcSrc->left)/lpImage->dwWidth;
	FinalImage.Vertices[0].tv=(fSrcHeight+rcSrc->top)/lpImage->dwHeight;

	FinalImage.Vertices[1].tu=(float)rcSrc->left/(float)lpImage->dwWidth;
	FinalImage.Vertices[1].tv=(float)(fSrcHeight+rcSrc->top)/(float)lpImage->dwHeight;

	FinalImage.Vertices[2].tu=(float)rcSrc->left/(float)lpImage->dwWidth;
	FinalImage.Vertices[2].tv=(float)rcSrc->top/(float)lpImage->dwHeight;

	FinalImage.Vertices[3].tu=(float)(fSrcWidth+rcSrc->left)/lpImage->dwWidth;
	FinalImage.Vertices[3].tv=(float)rcSrc->top/(float)lpImage->dwHeight;
	return L2DI_Render(&FinalImage, rcDest->left, rcDest->top);
}
*/
/*
BOOL L2DI_RenderRelativeEx(
	LV2DIMAGE* lpImage,
	float fXDest,
	float fYDest,
	float fWidthDest,
	float fHeightDest,
	LONG nXSrc,
	LONG nYSrc,
	LONG nWidthSrc,
	LONG nHeightSrc)
{
	LV2DIMAGE FinalImage;
	LONG dwScreenWidth=0, dwScreenHeight=0;
	D3DVIEWPORT9 ViewPort;

	if(!lpImage)
		return FALSE;

	ZeroMemory(&FinalImage, sizeof(LV2DIMAGE));
	ZeroMemory(&ViewPort, sizeof(D3DVIEWPORT9));

	((LV2DIMAGE*)lpImage)->lpDevice->lpVtbl->GetViewport(((LV2DIMAGE*)lpImage)->lpDevice, &ViewPort);
	dwScreenWidth=ViewPort.Width;
	dwScreenHeight=ViewPort.Height;

	FinalImage=*(LV2DIMAGE*)lpImage;

	
	// Scale down the dest values. 
	fWidthDest/=100.0f;
	fHeightDest/=100.0f;
	fXDest/=100.0f;
	fYDest/=100.0f;

	FinalImage.dwWidth=(LONG)(fWidthDest*(float)dwScreenWidth);
	FinalImage.dwHeight=(LONG)(fHeightDest*(float)dwScreenHeight);

	FinalImage.Vertices[0].x=(float)FinalImage.dwWidth;
	FinalImage.Vertices[0].y=(float)FinalImage.dwHeight;
	FinalImage.Vertices[1].y=(float)FinalImage.dwHeight;
	FinalImage.Vertices[3].x=(float)FinalImage.dwWidth;

	

	// Discover the proper texture coords.
	FinalImage.Vertices[0].tu=(float)(nWidthSrc+nXSrc)/(float)lpImage->dwWidth;
	FinalImage.Vertices[0].tv=(float)(nHeightSrc+nYSrc)/(float)lpImage->dwHeight;

	FinalImage.Vertices[1].tu=(float)nXSrc/(float)lpImage->dwWidth;
	FinalImage.Vertices[1].tv=(float)(nHeightSrc+nYSrc)/(float)lpImage->dwHeight;

	FinalImage.Vertices[2].tu=(float)nXSrc/(float)lpImage->dwWidth;
	FinalImage.Vertices[2].tv=(float)nYSrc/(float)lpImage->dwHeight;

	FinalImage.Vertices[3].tu=(float)(nWidthSrc+nXSrc)/(float)lpImage->dwWidth;
	FinalImage.Vertices[3].tv=(float)nYSrc/(float)lpImage->dwHeight;
	
	return L2DI_Render(&FinalImage, (L_long)(fXDest*dwScreenWidth), (L_long)(fYDest*dwScreenHeight));
}
*/