#include "common.h"
#include <d3d9.h>
#include <d3dx9.h>
#include "lg_err.h"
#include "libs\lf_sys.h"
#include "lv_d3dfont.h"

LVFONT* Font_Create(
	IDirect3DDevice9* lpDevice,
	char* szFontName,
	L_dword dwWidth,
	L_dword dwHeight,
	L_dword dwWeight)
{
	LVFONT* lpNewFont=L_null;
	L_result nResult=0;
	D3DXFONT_DESC fdesc;

	lpNewFont=malloc(sizeof(LVFONT));
	if(!lpNewFont)
	{
		Err_Printf("Could not allocate memory to create font.");
		return L_null;
	}

	memset(lpNewFont, 0, sizeof(LVFONT));

	nResult=D3DXCreateFont(
		lpDevice,
		dwWidth,
		dwHeight,
		dwWeight,
		1, //D3DX_DEFAULT,
		L_false,
		ANSI_CHARSET,
		OUT_DEFAULT_PRECIS,
		DEFAULT_QUALITY,
		DEFAULT_PITCH,
		szFontName,
		&lpNewFont->m_lpFont);

	if(L_failed(nResult))
	{
		Err_Printf("Could not create \"%s\" font.", szFontName);
		Err_PrintDX("   D3DXCreateFont", nResult);
		return L_null;
	}
	nResult=D3DXCreateSprite(lpDevice, &lpNewFont->m_lpSurface);
	if(L_failed(nResult))
	{
		Err_Printf("Failed to create surface for font writing, writing will be slower.");
		lpNewFont->m_lpSurface=L_null;
	}

	memset(&fdesc, 0, sizeof(fdesc));
	lpNewFont->m_lpFont->lpVtbl->GetDescA(lpNewFont->m_lpFont, &fdesc);
	lpNewFont->m_dwFontHeight=fdesc.Height;
	lpNewFont->m_dwFontWidth=fdesc.Width;
	lpNewFont->m_bDrawing=L_false;

	return lpNewFont;
}

L_bool Font_Delete(LVFONT* lpFont)
{
	if(!lpFont)
		return L_false;

	L_safe_release(lpFont->m_lpSurface);
	L_safe_release(lpFont->m_lpFont);
	L_safe_free(lpFont);
	return L_true;
}

L_bool Font_Begin(LVFONT* lpFont)
{
	if(!lpFont)
		return L_false;

	if(lpFont->m_lpSurface)
		lpFont->m_lpSurface->lpVtbl->Begin(lpFont->m_lpSurface, D3DXSPRITE_ALPHABLEND);

	lpFont->m_bDrawing=L_true;
	return L_true;
}
L_bool Font_End(LVFONT* lpFont)
{
	if(!lpFont)
		return L_false;

	if(lpFont->m_lpSurface)
		lpFont->m_lpSurface->lpVtbl->End(lpFont->m_lpSurface);

	lpFont->m_bDrawing=L_false;
	return L_true;
}


L_bool Font_DrawString(
	LVFONT* lpFont, 
	char* szString, 
	L_long x, 
	L_long y)
{
	L_result nResult=0;
	L_long nWidth = 1024;
	L_rect rcDest={x, y, x+nWidth, y+(L_long)lpFont->m_dwFontHeight};

	if(!szString)
		return L_false;

	/* Note in parameter 2 we test to see if drawing has
		started, if it hasn't we pass null for the surface,
		so the text will be drawn anyway, but this will be
		slower.*/
	lpFont->m_lpFont->lpVtbl->DrawText(
		lpFont->m_lpFont,
		lpFont->m_bDrawing?lpFont->m_lpSurface:L_null,
		szString, 
		-1, 
		(RECT*)&rcDest, 
		DT_LEFT, 
		0xFFFFFF00);
	return L_true;
}


L_bool Font_Validate(LVFONT* lpFont, IDirect3DDevice9* lpDevice)
{
	L_result nResult=0;
	nResult=lpFont->m_lpFont->lpVtbl->OnResetDevice(lpFont->m_lpFont);
	nResult=D3DXCreateSprite(lpDevice, &lpFont->m_lpSurface);
	if(L_failed(nResult))
	{
		Err_Printf("Failed to create surface for font writing, writing will be slower.");
		lpFont->m_lpSurface=L_null;
	}
	return L_succeeded(nResult);
}

L_bool Font_Invalidate(LVFONT* lpFont)
{
	L_result nResult=0;
	nResult=lpFont->m_lpFont->lpVtbl->OnLostDevice(lpFont->m_lpFont);
	L_safe_release(lpFont->m_lpSurface);
	return L_succeeded(nResult);
}