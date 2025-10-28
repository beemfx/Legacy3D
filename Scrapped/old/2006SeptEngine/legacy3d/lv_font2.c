#include "lv_font2.h"
#include <d3dx9.h>
#include <lf_sys.h>
#include "lg_err.h"
#include "lv_tex.h"

#define FONTRENDER_WIDTH  640.0f
#define FONTRENDER_HEIGHT 480.0f

#define LVFONTVERTEX_TYPE \
(                          \
	D3DFVF_XYZ|             \
	D3DFVF_TEX1             \
)

typedef struct _LVFONTVERTEX{
	float x, y, z;
	float tu, tv;
}LVFONTVERTEX, *PLVFONTVERTEX;


typedef struct _LVFONT2{
	IDirect3DDevice9*  m_lpDevice;
	L_byte             m_nCharWidth;
	L_byte             m_nCharHeight;
	L_bool             m_bD3DFont;
	L_bool             m_bIsDrawing;
	char               m_szFont[MAX_F_PATH];

	/*D3DXFont Specific*/
	ID3DXFont*         m_lpD3DXFont;
	ID3DXSprite*       m_lpD3DXSurface;
	L_dword            m_dwD3DColor;

	/*Legacy Font Specific*/
	IDirect3DTexture9*      m_lpFontTexture;
	IDirect3DVertexBuffer9* m_lpFontVB;
	L_byte                  m_nCharWidthInFile;
	L_byte                  m_nCharHeightInFile;
	//LV2DIMAGE*         m_lpChars[96];
	LVFONTVERTEX            m_lpVertices[96*4];
}LVFONT2, *PLVFONT2;


HLVFONT2 Font_Create2(
	IDirect3DDevice9* lpDevice, 
	char* szFont, 
	L_byte nCharWidth, 
	L_byte nCharHeight,
	L_bool bD3DXFont,
	L_byte nCharWidthInFile,
	L_byte nCharHeightInFile,
	L_dword dwD3DColor)
{
	L_result nResult=0;
	LVFONT2* lpFont=L_null;
	/* Allocate memory for the font. */
	lpFont=malloc(sizeof(LVFONT2));
	if(!lpFont)
	{
		Err_Printf("Could not allocate memory for \"%s\" font.", szFont);
		return L_null;
	}

	/* Nullify everything in the font.*/
	memset(lpFont, 0, sizeof(LVFONT2));
	lpFont->m_lpDevice=lpDevice;
	lpDevice->lpVtbl->AddRef(lpDevice);
	lpFont->m_bD3DFont=bD3DXFont;
	lpFont->m_nCharWidth=nCharWidth;
	lpFont->m_nCharHeight=nCharHeight;
	lpFont->m_dwD3DColor=dwD3DColor;
	lpFont->m_nCharWidthInFile=nCharWidthInFile;
	lpFont->m_nCharHeightInFile=nCharHeightInFile;
	L_strncpy(lpFont->m_szFont, szFont, MAX_F_PATH-1);

	if(bD3DXFont)
	{
		D3DXFONT_DESC fdesc;

		nResult=D3DXCreateFont(
			lpDevice,
			nCharHeight,
			nCharWidth,
			FW_DONTCARE,
			1,
			L_false,
			ANSI_CHARSET,
			OUT_DEFAULT_PRECIS,
			DEFAULT_QUALITY,
			DEFAULT_PITCH,
			szFont,
			&lpFont->m_lpD3DXFont);

		if(L_failed(nResult))
		{
			Err_Printf("Could not create \"%s\" font.", szFont);
			Err_PrintDX("   D3DXCreateFont", nResult);
			lpFont->m_lpDevice->lpVtbl->Release(lpFont->m_lpDevice);
			free(lpFont);
			return L_null;	
		}

		nResult=D3DXCreateSprite(lpDevice, &lpFont->m_lpD3DXSurface);
		if(L_failed(nResult))
		{
			Err_Printf("Failed to create font surface for writing, expect slower font drawing.");
			lpFont->m_lpD3DXSurface=L_null;
		}

		memset(&fdesc, 0, sizeof(fdesc));
		lpFont->m_lpD3DXFont->lpVtbl->GetDescA(lpFont->m_lpD3DXFont, &fdesc);
		lpFont->m_nCharHeight=fdesc.Height;
		lpFont->m_nCharWidth=fdesc.Width;
		lpFont->m_bIsDrawing=L_false;
		/*And below the font will be returned.*/
	}
	else
	{
		if(!Font_Validate2(lpFont))
		{
			L_safe_release(lpFont->m_lpDevice);
			free(lpFont);
			return L_null;
		}
	}
	return lpFont;
}

L_bool Font_Delete2(HLVFONT2 hFont)
{
	LVFONT2* lpFont=hFont;
	if(!lpFont)return L_false;

	if(lpFont->m_bD3DFont)
	{
		L_safe_release(lpFont->m_lpD3DXFont);
		L_safe_release(lpFont->m_lpD3DXSurface);
	}
	else
	{
		L_safe_release(lpFont->m_lpFontTexture);
		L_safe_release(lpFont->m_lpFontVB);
	}
	L_safe_release(lpFont->m_lpDevice);
	L_safe_free(lpFont);
	return L_true;
}

L_bool Font_GetDims(HLVFONT2 hFont, LVFONT_DIMS* pDims)
{
	LVFONT2* lpFont=hFont;
	if(!lpFont)return L_false;

	pDims->bD3DXFont=lpFont->m_bD3DFont;
	pDims->dwD3DColor=lpFont->m_dwD3DColor;
	pDims->nCharHeight=lpFont->m_nCharHeight;
	pDims->nCharWidth=lpFont->m_nCharWidth;
	return L_true;
}

L_bool Font_SetResetStates(IDirect3DDevice9* lpDevice, L_bool bStart)
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
		lpDevice->lpVtbl->SetFVF(lpDevice, LVFONTVERTEX_TYPE);
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
		lpDevice->lpVtbl->SetRenderState(lpDevice, D3DRS_MULTISAMPLEANTIALIAS, FALSE);

		lpDevice->lpVtbl->GetRenderState(lpDevice, D3DRS_FILLMODE, &dwOldFill);
		IDirect3DDevice9_SetRenderState(lpDevice, D3DRS_FILLMODE, D3DFILL_SOLID);

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
		IDirect3DDevice9_SetSamplerState(lpDevice, 0, D3DSAMP_MAGFILTER, dwOldMagFilter);
		IDirect3DDevice9_SetSamplerState(lpDevice, 0, D3DSAMP_MINFILTER, dwOldMinFilter);
		IDirect3DDevice9_SetSamplerState(lpDevice, 0, D3DSAMP_MIPFILTER, dwOldMipFilter);
		IDirect3DDevice9_SetRenderState(lpDevice, D3DRS_FILLMODE, dwOldFill);

		//lpDevice->lpVtbl->SetVertexShader(lpDevice, lpOldVS);
		//L_safe_release(lpOldVS);
	}

	return L_true;
}
	


L_bool Font_Begin2(HLVFONT2 hFont)
{
	LVFONT2* lpFont=hFont;
	if(!lpFont)return L_false;

	if(lpFont->m_bIsDrawing)
		return L_true;

	if(lpFont->m_bD3DFont)
	{
		if(lpFont->m_lpD3DXSurface)
			lpFont->m_lpD3DXSurface->lpVtbl->Begin(lpFont->m_lpD3DXSurface, D3DXSPRITE_ALPHABLEND);
	}
	else
	{
		D3DMATRIX TempMatrix;	
		/*Set some render states.*/
		D3DXMatrixOrthoLH(&TempMatrix, FONTRENDER_WIDTH, FONTRENDER_HEIGHT, 0.0f, 1.0f);
		IDirect3DDevice9_SetTransform(lpFont->m_lpDevice, D3DTS_PROJECTION, &TempMatrix);
		IDirect3DDevice9_SetTransform(lpFont->m_lpDevice, D3DTS_VIEW, D3DXMatrixIdentity(&TempMatrix));
		Font_SetResetStates(lpFont->m_lpDevice, L_true);
		/* Set the texture and vertex buffer. */
		IDirect3DDevice9_SetTexture(lpFont->m_lpDevice, 0, (LPDIRECT3DBASETEXTURE9)lpFont->m_lpFontTexture);
		IDirect3DDevice9_SetStreamSource(lpFont->m_lpDevice, 0, lpFont->m_lpFontVB, 0, sizeof(LVFONTVERTEX));
	}
	lpFont->m_bIsDrawing=L_true;
	return L_true;
}

L_bool Font_End2(HLVFONT2 hFont)
{
	LVFONT2* lpFont=hFont;
	if(!lpFont)return L_false;

	if(!lpFont->m_bIsDrawing)
		return L_true;

	if(lpFont->m_bD3DFont)
	{
		if(lpFont->m_lpD3DXSurface)
			lpFont->m_lpD3DXSurface->lpVtbl->End(lpFont->m_lpD3DXSurface);
	}
	else
	{
		Font_SetResetStates(lpFont->m_lpDevice, L_false);
	}
	lpFont->m_bIsDrawing=L_false;
	return L_true;
}

L_bool Font_DrawChar(HLVFONT2 hFont, char c, L_long x, L_long y)
{
	D3DMATRIX TempMatrix;
	LVFONT2* lpFont=hFont;
	if(!lpFont)return L_false;

	if(c<' ' || c>'~')
		c=' ';
	D3DXMatrixIdentity(&TempMatrix);
	D3DXMatrixTranslation(&TempMatrix, x-FONTRENDER_WIDTH/2.0f, FONTRENDER_HEIGHT/2.0f-y, 0.0f);
	IDirect3DDevice9_SetTransform(lpFont->m_lpDevice, D3DTS_WORLD, &TempMatrix);
	
	/* Set the texture. */
	IDirect3DDevice9_DrawPrimitive(lpFont->m_lpDevice, D3DPT_TRIANGLEFAN, (c-' ')*4, 2);

	return L_true;
}

L_bool Font_DrawString2(HLVFONT2 hFont, char* szString, L_long x, L_long y)
{
	LVFONT2* lpFont=hFont;
	if(!lpFont || !szString)return L_false;

	if(lpFont->m_bD3DFont)
	{
		L_result nResult=0;
		L_long nWidth = 1024;
		L_rect rcDest={x, y, x+nWidth, y+lpFont->m_nCharHeight};

		if(!szString)
			return L_false;

		/* Note in parameter 2 we test to see if drawing has
			started, if it hasn't we pass null for the surface,
			so the text will be drawn anyway, but this will be
			slower.*/
		lpFont->m_lpD3DXFont->lpVtbl->DrawText(
			lpFont->m_lpD3DXFont,
			lpFont->m_bIsDrawing?lpFont->m_lpD3DXSurface:L_null,
			szString,
			-1, 
			(void*)&rcDest, 
			DT_LEFT, 
			lpFont->m_dwD3DColor);
	}
	else
	{
		L_dword dwLen=L_strlen(szString), i=0;
		for(i=0; i<dwLen; i++)
		{
			Font_DrawChar(lpFont, szString[i], x+(i*lpFont->m_nCharWidth), y);
		}
	}
	return L_true;
}

L_bool Font_Validate2(HLVFONT2 hFont)
{
	LVFONT2* lpFont=hFont;
	if(!lpFont)return L_false;

	if(lpFont->m_bD3DFont)
	{
		L_result nResult=0;
		nResult=lpFont->m_lpD3DXFont->lpVtbl->OnResetDevice(lpFont->m_lpD3DXFont);
		if(L_failed(D3DXCreateSprite(lpFont->m_lpDevice, &lpFont->m_lpD3DXSurface)))
		{
			Err_Printf("Failed to create font surface for writing, expect slower font drawing.");
			lpFont->m_lpD3DXSurface=L_null;
		}
		return L_succeeded(nResult);
	}
	else
	{
		L_result nResult=0;

		D3DSURFACE_DESC desc;
		int i=0, j=0, c=0, a=0, nCharsPerLine=0;
		LVFONTVERTEX* v;

		/* We need a texture for the font.*/
		if(!Tex_Load(lpFont->m_lpDevice, lpFont->m_szFont, D3DPOOL_DEFAULT, 0xFFFFFFFF, L_true, &lpFont->m_lpFontTexture))
		{
			Err_Printf("Font_Create2 Error: Could not create texture from \"%s\".", lpFont->m_szFont);
			//L_safe_release(lpFont->m_lpDevice);
			//L_safe_free(lpFont);
			return L_false;
		}

		/* Create the vertex buffer. */
		nResult=IDirect3DDevice9_CreateVertexBuffer(
			lpFont->m_lpDevice,
			sizeof(LVFONTVERTEX)*96*4,
			D3DUSAGE_WRITEONLY,
			LVFONTVERTEX_TYPE,
			D3DPOOL_DEFAULT,
			&lpFont->m_lpFontVB,
			L_null);

		if(L_failed(nResult))
		{
			Err_Printf("Font_Create2 Error: Could not create vertex buffer.");
			Err_PrintDX("   IDirect3DDevice9::CreateVertexBuffer", nResult);
			L_safe_release(lpFont->m_lpFontTexture);
			return L_false;
		}
		
		lpFont->m_lpFontTexture->lpVtbl->GetLevelDesc(lpFont->m_lpFontTexture, 0, &desc);
		/* Now find each individual letter in the texture.*/
		nCharsPerLine=desc.Width/lpFont->m_nCharWidthInFile;
		if((desc.Width%lpFont->m_nCharWidthInFile)>0)
			nCharsPerLine++;//+((desc.Width%lpFont->m_nCharWidthInFile)>0)?1:0;

	
		IDirect3DVertexBuffer9_Lock(lpFont->m_lpFontVB, 0, sizeof(LVFONTVERTEX)*96*4, &v, 0);
		
		i=0;j=0;
		for(c=0, a=0; c<96; c++, a+=4)
		{
			if(j>nCharsPerLine){j=0; i++;}
			v[a].x=0.0f;
			v[a].y=0.0f;
			v[a+1].x=(float)lpFont->m_nCharWidth;
			v[a+1].y=0.0f;
			v[a+2].x=(float)lpFont->m_nCharWidth;
			v[a+2].y=-(float)lpFont->m_nCharHeight;
			v[a+3].x=0.0f;
			v[a+3].y=-(float)lpFont->m_nCharHeight;
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
				v[a+1].x=(float)lpFont->m_nCharWidth;
				v[a+1].y=0.0f;
				v[a+2].x=(float)lpFont->m_nCharWidth;
				v[a+2].y=-(float)lpFont->m_nCharHeight;
				v[a+3].x=0.0f;
				v[a+3].y=-(float)lpFont->m_nCharHeight;
				v[a].z=v[a+1].z=v[a+2].z=v[a+3].z=0.0f;
				
				/*
				v[a].Diffuse=
				v[a+1].Diffuse=
				v[a+2].Diffuse=
				v[a+3].Diffuse=0xFFFFFFFF;
				*/
				
				v[a].tu=
				v[a+3].tu=(float)j*(float)lpFont->m_nCharWidthInFile/(float)desc.Width;

				v[a+1].tu=
				v[a+2].tu=(float)(j+1)*(float)lpFont->m_nCharWidthInFile/(float)desc.Width;

				v[a+2].tv=
				v[a+3].tv=(float)(i+1)*(float)lpFont->m_nCharHeightInFile/(float)desc.Height;

				v[a].tv=
				v[a+1].tv=(float)i*(float)lpFont->m_nCharHeightInFile/(float)desc.Height;
			}
		}

		IDirect3DVertexBuffer9_Unlock(lpFont->m_lpFontVB);

	}
	return L_true;
}

L_bool Font_Invalidate2(HLVFONT2 hFont)
{
	LVFONT2* lpFont=hFont;
	if(!lpFont)return L_false;

	if(lpFont->m_bD3DFont)
	{
		L_result nResult=0;
		nResult=lpFont->m_lpD3DXFont->lpVtbl->OnLostDevice(lpFont->m_lpD3DXFont);
		L_safe_release(lpFont->m_lpD3DXSurface);
		return L_succeeded(nResult);
	}
	else
	{
		int i=0;
		L_safe_release(lpFont->m_lpFontVB);
		L_safe_release(lpFont->m_lpFontTexture);
	}
	return L_true;
}