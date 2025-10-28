#include <d3dx9.h>
#include "lf_sys2.h"
#include "lg_err.h"
#include "lv_font.h"
#include "lg_func.h"
#include "lg_tmgr.h"

#define FONTRENDER_WIDTH  640.0f
#define FONTRENDER_HEIGHT 480.0f

class CLFontL: public CLFont
{
private:
	typedef struct _LVFONTVERTEX{
		float x, y, z;
		float tu, tv;
	}LVFONTVERTEX, *PLVFONTVERTEX;
	static const lg_dword LVFONTVERTEX_TYPE=D3DFVF_XYZ|D3DFVF_TEX1;
	static lg_bool SetResetStates(IDirect3DDevice9* lpDevice, lg_bool bStart);
	
private:
	tm_tex      m_lpFontTexture;
	IDirect3DVertexBuffer9* m_lpFontVB;
	lg_byte                  m_nCharWidthInFile;
	lg_byte                  m_nCharHeightInFile;
	LVFONTVERTEX            m_lpVertices[96*4];
	
public:
	CLFontL();
	~CLFontL();
	
	lg_bool Create(
		IDirect3DDevice9* lpDevice, 
		char* szFont, 
		lg_byte nCharWidth, 
		lg_byte nCharHeight,
		lg_byte nCharWidthInFile,
		lg_byte nCharHeightInFile);
		
	void Delete();
	lg_bool Validate();
	void Invalidate();
	void Begin();
	void End();
	void DrawString(char* szString, lg_long x, lg_long y);
	void GetDims(lg_byte* nWidth, lg_byte* nHeight, lg_void* pExtra);
private:
	void DrawChar(char c, lg_long x, lg_long y);
};

class CLFontD3D: public CLFont
{
private:
	ID3DXFont*         m_lpD3DXFont;
	ID3DXSprite*       m_lpD3DXSurface;
	lg_dword            m_dwD3DColor;
public:
	CLFontD3D();
	~CLFontD3D();
	
	lg_bool Create(
		IDirect3DDevice9* lpDevice, 
		char* szFont, 
		lg_byte nCharWidth, 
		lg_byte nCharHeight,
		lg_dword dwD3DColor);
		
	void Delete();
	lg_bool Validate();
	void Invalidate();
	void Begin();
	void End();
	void DrawString(char* szString, lg_long x, lg_long y);
	void GetDims(lg_byte* nWidth, lg_byte* nHeight, lg_void* pExtra);
	
};


/************************
*** CLFont Base Class ***
************************/
CLFont::CLFont():
m_pDevice(0),
m_nCharWidth(0),
m_nCharHeight(0),
m_bIsDrawing(0)
{
	memset(m_szFont, 0, sizeof(m_szFont));
}
CLFont::~CLFont()
{
	Delete();
}


CLFont* CLFont::Create(
		IDirect3DDevice9* lpDevice, 
		char* szFont, 
		lg_byte nCharWidth, 
		lg_byte nCharHeight,
		lg_bool bD3DXFont,
		lg_byte nCharWidthInFile,
		lg_byte nCharHeightInFile,
		lg_dword dwD3DColor)
{
	
	
	if(bD3DXFont)
	{
		CLFontD3D* pFont=new CLFontD3D();
		if(!pFont->Create(
			lpDevice,
			szFont,
			nCharWidth,
			nCharHeight,
			dwD3DColor))
		{
			delete pFont;
			return LG_NULL;
		}
		return pFont;
	}
	else
	{
		CLFontL* pFont=new CLFontL();
		if(!pFont->Create(
			lpDevice,
			szFont,
			nCharWidth,
			nCharHeight,
			nCharWidthInFile,
			nCharHeightInFile))
		{
			delete pFont;
			return LG_NULL;
		}
		return pFont;
	}
}


/******************************
*** CLFontL The Legacy Font ***
******************************/
CLFontL::CLFontL():
	CLFont(),
	m_lpFontTexture(0),
	m_lpFontVB(0),
	m_nCharWidthInFile(0),
	m_nCharHeightInFile(0)
{
	memset(m_lpVertices, 0, sizeof(m_lpVertices));
}
CLFontL::~CLFontL()
{
	Delete();
}

lg_bool CLFontL::Create(
	IDirect3DDevice9* lpDevice, 
	char* szFont, 
	lg_byte nCharWidth, 
	lg_byte nCharHeight,
	lg_byte nCharWidthInFile,
	lg_byte nCharHeightInFile)
{
	Delete();
	
	lg_result nResult=0;

	/* Nullify everything in the font.*/
	m_pDevice=lpDevice;
	lpDevice->AddRef();
	m_nCharWidth=nCharWidth;
	m_nCharHeight=nCharHeight;
	m_nCharWidthInFile=nCharWidthInFile;
	m_nCharHeightInFile=nCharHeightInFile;
	LG_strncpy(m_szFont, szFont, LF_MAX_PATH);

	if(!Validate())
	{
		L_safe_release(m_pDevice);
		return LG_FALSE;
	}
	return LG_TRUE;
}

void CLFontL::GetDims(lg_byte* nWidth, lg_byte* nHeight, lg_void* pExtra)
{
	if(nWidth)
		*nWidth=m_nCharWidth;
	if(nHeight)
		*nHeight=m_nCharHeight;
	if(pExtra)
		*((lg_bool*)pExtra)=LG_FALSE;
}

void CLFontL::Delete()
{
	m_lpFontTexture=0;//L_safe_release(m_lpFontTexture);
	L_safe_release(m_lpFontVB);
	L_safe_release(m_pDevice);
}

lg_bool CLFontL::Validate()
{
	lg_result nResult=0;
	
	if(m_lpFontTexture || m_lpFontVB)
		return LG_TRUE;

	D3DSURFACE_DESC desc;
	int i=0, j=0, c=0, a=0, nCharsPerLine=0;
	LVFONTVERTEX* v;

	/* We need a texture for the font.*/
	m_lpFontTexture=CLTMgr::TM_LoadTex(m_szFont, CLTMgr::TM_FORCENOMIP);
	if(!m_lpFontTexture)
	{
		Err_Printf("Font_Create2 Error: Could not create texture from \"%s\".", m_szFont);
		return LG_FALSE;
	}

	/* Create the vertex buffer.*/
	nResult=IDirect3DDevice9_CreateVertexBuffer(
		m_pDevice,
		sizeof(LVFONTVERTEX)*96*4,
		D3DUSAGE_WRITEONLY,
		LVFONTVERTEX_TYPE,
		D3DPOOL_DEFAULT,
		&m_lpFontVB,
		LG_NULL);

	if(LG_FAILED(nResult))
	{
		Err_Printf("Font_Create2 Error: Could not create vertex buffer.");
		Err_PrintDX("   IDirect3DDevice9::CreateVertexBuffer", nResult);
		m_lpFontTexture=0;//L_safe_release(m_lpFontTexture);
		return LG_FALSE;
	}
		
	//m_lpFontTexture->GetLevelDesc(0, &desc);
	desc.Width=128;
	/* Now find each individual letter in the texture.*/
	nCharsPerLine=desc.Width/m_nCharWidthInFile;
	if((desc.Width%m_nCharWidthInFile)>0)
		nCharsPerLine++;//+((desc.Width%m_nCharWidthInFile)>0)?1:0;

	
	IDirect3DVertexBuffer9_Lock(m_lpFontVB, 0, sizeof(LVFONTVERTEX)*96*4, (void**)&v, 0);
		
	i=0;j=0;
	for(c=0, a=0; c<96; c++, a+=4)
	{
		if(j>nCharsPerLine){j=0; i++;}
		v[a].x=0.0f;
		v[a].y=0.0f;
		v[a+1].x=(float)m_nCharWidth;
		v[a+1].y=0.0f;
		v[a+2].x=(float)m_nCharWidth;
		v[a+2].y=-(float)m_nCharHeight;
		v[a+3].x=0.0f;
		v[a+3].y=-(float)m_nCharHeight;
		v[a].z=v[a+1].z=v[a+2].z=v[a+3].z=0.0f;
			
		/*
		v[a].Diffuse=
		v[a+1].Diffuse=
		v[a+2].Diffuse=
		v[a+3].Diffuse=0xFFFFFFFF;
		*/


	}

	for(i=0, a=0; a<(96*4); i++)
	{
		for(j=0; j<nCharsPerLine && a<(96*4); j++, a+=4)
		{
			v[a].x=0.0f;
			v[a].y=0.0f;
			v[a+1].x=(float)m_nCharWidth;
			v[a+1].y=0.0f;
			v[a+2].x=(float)m_nCharWidth;
			v[a+2].y=-(float)m_nCharHeight;
			v[a+3].x=0.0f;
			v[a+3].y=-(float)m_nCharHeight;
			v[a].z=v[a+1].z=v[a+2].z=v[a+3].z=0.0f;
			
			/*
			v[a].Diffuse=
			v[a+1].Diffuse=
			v[a+2].Diffuse=
			v[a+3].Diffuse=0xFFFFFFFF;
			*/
			
			v[a].tu=
			v[a+3].tu=(float)j*(float)m_nCharWidthInFile/(float)desc.Width;

			v[a+1].tu=
			v[a+2].tu=(float)(j+1)*(float)m_nCharWidthInFile/(float)desc.Width;

			v[a+2].tv=
			v[a+3].tv=(float)(i+1)*(float)m_nCharHeightInFile/(float)desc.Height;

			v[a].tv=
			v[a+1].tv=(float)i*(float)m_nCharHeightInFile/(float)desc.Height;
		}
	}
	IDirect3DVertexBuffer9_Unlock(m_lpFontVB);

	return LG_TRUE;
}

void CLFontL::Invalidate()
{
	L_safe_release(m_lpFontVB);
	//L_safe_release(m_lpFontTexture);
	m_lpFontTexture=0;
}

void CLFontL::Begin()
{
	if(m_bIsDrawing || !m_lpFontVB)
		return;

	D3DXMATRIX TempMatrix;	
	//Set some render states.
	D3DXMatrixOrthoLH(&TempMatrix, FONTRENDER_WIDTH, FONTRENDER_HEIGHT, 0.0f, 1.0f);
	IDirect3DDevice9_SetTransform(m_pDevice, D3DTS_PROJECTION, &TempMatrix);
	IDirect3DDevice9_SetTransform(m_pDevice, D3DTS_VIEW, D3DXMatrixIdentity(&TempMatrix));
	SetResetStates(m_pDevice, LG_TRUE);
	// Set the texture and vertex buffer.
	//IDirect3DDevice9_SetTexture(m_pDevice, 0, (LPDIRECT3DBASETEXTURE9)m_lpFontTexture);
	CLTMgr::TM_SetTexture(m_lpFontTexture, 0);
	IDirect3DDevice9_SetStreamSource(m_pDevice, 0, m_lpFontVB, 0, sizeof(LVFONTVERTEX));
	m_bIsDrawing=LG_TRUE;
}

void CLFontL::End()
{
	if(!m_bIsDrawing)
		return;


	SetResetStates(m_pDevice, LG_FALSE);
	m_bIsDrawing=LG_FALSE;
}

void CLFontL::DrawString(char* szString, lg_long x, lg_long y)
{
	if(!szString)
		return;
	lg_dword dwLen=L_strlen(szString), i=0;
	for(i=0; i<dwLen; i++)
	{
		DrawChar(szString[i], x+(i*m_nCharWidth), y);
	}
}
void CLFontL::DrawChar(char c, lg_long x, lg_long y)
{
	D3DXMATRIX TempMatrix;
	if(c<' ' || c>'~')
		c=' ';
	D3DXMatrixIdentity(&TempMatrix);
	D3DXMatrixTranslation(&TempMatrix, x-FONTRENDER_WIDTH/2.0f, FONTRENDER_HEIGHT/2.0f-y, 0.0f);
	IDirect3DDevice9_SetTransform(m_pDevice, D3DTS_WORLD, &TempMatrix);
	/* Set the texture.*/
	IDirect3DDevice9_DrawPrimitive(m_pDevice, D3DPT_TRIANGLEFAN, (c-' ')*4, 2);
}


lg_bool CLFontL::SetResetStates(IDirect3DDevice9* lpDevice, lg_bool bStart)
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
		
		// Save the old filter modes.
		IDirect3DDevice9_GetSamplerState(lpDevice, 0, D3DSAMP_MAGFILTER, &dwOldMagFilter);
		IDirect3DDevice9_GetSamplerState(lpDevice, 0, D3DSAMP_MINFILTER, &dwOldMinFilter);
		IDirect3DDevice9_GetSamplerState(lpDevice, 0, D3DSAMP_MIPFILTER, &dwOldMipFilter);
		IDirect3DDevice9_SetSamplerState(lpDevice, 0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
		IDirect3DDevice9_SetSamplerState(lpDevice, 0, D3DSAMP_MINFILTER, D3DTEXF_NONE);
		IDirect3DDevice9_SetSamplerState(lpDevice, 0, D3DSAMP_MIPFILTER, D3DTEXF_NONE);

		// Save some rendering state values, they will be restored when this function
		//	is called with bStart set to FALSE.
		lpDevice->GetRenderState(D3DRS_LIGHTING, &dwOldLighting);
		lpDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
		//* Get and set FVF.
		lpDevice->GetFVF(&dwPrevFVF);
		lpDevice->SetFVF(LVFONTVERTEX_TYPE);
		//* Get and set vertex shader.
		//lpDevice->GetVertexShader(&lpOldVS);
		//lpDevice->SetVertexShader(NULL);
		//* Get and set cull mode.
		lpDevice->GetRenderState(D3DRS_CULLMODE, &dwOldCullMode);
		lpDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
		//* Set alpha blending.
		lpDevice->GetRenderState(D3DRS_ALPHABLENDENABLE, &dwOldAlphaEnable);
		lpDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);	

		lpDevice->GetRenderState(D3DRS_SRCBLEND, &dwOldSrcBlend);
		lpDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			
		lpDevice->GetRenderState(D3DRS_DESTBLEND, &dwOldDestBlend);
		lpDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
			
		lpDevice->GetTextureStageState(0, D3DTSS_ALPHAARG1, &dwOldAlphaA);
		lpDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);

		// Get and set z-buffer status.
		lpDevice->GetRenderState(D3DRS_ZENABLE, &dwOldZEnable);
		lpDevice->SetRenderState(D3DRS_ZENABLE, FALSE);

		lpDevice->GetRenderState(D3DRS_MULTISAMPLEANTIALIAS, &dwOldAA);
		lpDevice->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, FALSE);

		lpDevice->GetRenderState(D3DRS_FILLMODE, &dwOldFill);
		IDirect3DDevice9_SetRenderState(lpDevice, D3DRS_FILLMODE, D3DFILL_SOLID);
	}
	else
	{
		// Restore all saved values.
		lpDevice->SetFVF(dwPrevFVF);
		//lpDevice->SetVertexShader(lpOldVS);
		lpDevice->SetRenderState(D3DRS_CULLMODE, dwOldCullMode);
		// Restore alpha blending state.
		lpDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, dwOldAlphaEnable);	
		lpDevice->SetRenderState(D3DRS_SRCBLEND, dwOldSrcBlend);	
		lpDevice->SetRenderState(D3DRS_DESTBLEND, dwOldDestBlend);	
		lpDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, dwOldAlphaA);
		// Restore Z-Buffering status.
		lpDevice->SetRenderState(D3DRS_ZENABLE, dwOldZEnable);
		lpDevice->SetRenderState(D3DRS_LIGHTING, dwOldLighting);
		lpDevice->SetRenderState(D3DRS_MULTISAMPLEANTIALIAS, dwOldAA);
		IDirect3DDevice9_SetSamplerState(lpDevice, 0, D3DSAMP_MAGFILTER, dwOldMagFilter);
		IDirect3DDevice9_SetSamplerState(lpDevice, 0, D3DSAMP_MINFILTER, dwOldMinFilter);
		IDirect3DDevice9_SetSamplerState(lpDevice, 0, D3DSAMP_MIPFILTER, dwOldMipFilter);
		IDirect3DDevice9_SetRenderState(lpDevice, D3DRS_FILLMODE, dwOldFill);

		//lpDevice->lpVtbl->SetVertexShader(lpDevice, lpOldVS);
		//L_safe_release(lpOldVS);
	}
	return LG_TRUE;
}
	
/**************************************************
*** CLFontD3D The Legacy Font Based on D3DXFont ***
**************************************************/
CLFontD3D::CLFontD3D():
	CLFont(),
	m_lpD3DXFont(0),
	m_lpD3DXSurface(0),
	m_dwD3DColor(0)
{
}

CLFontD3D::~CLFontD3D()
{
	Delete();
}

lg_bool CLFontD3D::Create(
	IDirect3DDevice9* lpDevice, 
	char* szFont, 
	lg_byte nCharWidth, 
	lg_byte nCharHeight,
	lg_dword dwD3DColor)
{
	Delete();
	
	lg_result nResult=0;

	/* Nullify everything in the font.*/
	m_pDevice=lpDevice;
	lpDevice->AddRef();
	m_nCharWidth=nCharWidth;
	m_nCharHeight=nCharHeight;
	m_dwD3DColor=dwD3DColor;
	LG_strncpy(m_szFont, szFont, LG_MAX_PATH);


	D3DXFONT_DESC fdesc;

	nResult=D3DXCreateFont(
		lpDevice,
		nCharHeight,
		nCharWidth,
		FW_DONTCARE,
		1,
		LG_FALSE,
		ANSI_CHARSET,
		OUT_DEFAULT_PRECIS,
		DEFAULT_QUALITY,
		DEFAULT_PITCH,
		szFont,
		&m_lpD3DXFont);

	if(LG_FAILED(nResult))
	{
		Err_Printf("Could not create \"%s\" font.", szFont);
		Err_PrintDX("   D3DXCreateFont", nResult);
		m_pDevice->Release();
		return LG_NULL;	
	}

	nResult=D3DXCreateSprite(lpDevice, &m_lpD3DXSurface);
	if(LG_FAILED(nResult))
	{
		Err_Printf("Failed to create font surface for writing, expect slower font drawing.");
		m_lpD3DXSurface=LG_NULL;
	}
	memset(&fdesc, 0, sizeof(fdesc));
	m_lpD3DXFont->GetDescA(&fdesc);
	m_nCharHeight=fdesc.Height;
	m_nCharWidth=fdesc.Width;
	m_bIsDrawing=LG_FALSE;
	
	return LG_TRUE;
}

void CLFontD3D::Delete()
{
	L_safe_release(m_lpD3DXFont);
	L_safe_release(m_lpD3DXSurface);
	L_safe_release(m_pDevice);
}

lg_bool CLFontD3D::Validate()
{
	lg_result nResult=0;
	nResult=m_lpD3DXFont->OnResetDevice();
	if(LG_FAILED(D3DXCreateSprite(m_pDevice, &m_lpD3DXSurface)))
	{
		Err_Printf("Failed to create font surface for writing, expect slower font drawing.");
		m_lpD3DXSurface=LG_NULL;
	}
	return LG_SUCCEEDED(nResult);
}

void CLFontD3D::Invalidate()
{
	lg_result nResult=0;
	nResult=m_lpD3DXFont->OnLostDevice();
	L_safe_release(m_lpD3DXSurface);
}

void CLFontD3D::Begin()
{
	if(m_bIsDrawing)
		return;
		
	if(m_lpD3DXSurface)
		m_lpD3DXSurface->Begin(D3DXSPRITE_ALPHABLEND);
	
	m_bIsDrawing=LG_TRUE;
}
void CLFontD3D::End()
{
	if(!m_bIsDrawing)
		return;

	if(m_lpD3DXSurface)
		m_lpD3DXSurface->End();
		
	m_bIsDrawing=LG_FALSE;
}
void CLFontD3D::DrawString(char* szString, lg_long x, lg_long y)
{
	if(!szString)
		return;
		
	lg_result nResult=0;
	lg_long nWidth = 1024;
	lg_rect rcDest={x, y, x+nWidth, y+m_nCharHeight};
	/* Note in parameter 2 we test to see if drawing has
		started, if it hasn't we pass null for the surface,
		so the text will be drawn anyway, but this will be
		slower.*/
	m_lpD3DXFont->DrawText(
		m_bIsDrawing?m_lpD3DXSurface:LG_NULL,
		szString,
		-1, 
		(LPRECT)&rcDest, 
		DT_LEFT, 
		m_dwD3DColor);
}

void CLFontD3D::GetDims(lg_byte* nWidth, lg_byte* nHeight, lg_void* pExtra)
{
	if(nWidth)
		*nWidth=m_nCharWidth;
	if(nHeight)
		*nHeight=m_nCharHeight;
	if(pExtra)
		*((lg_bool*)pExtra)=LG_TRUE;
}