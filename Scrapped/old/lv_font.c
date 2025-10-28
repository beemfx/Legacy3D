#include <d3d9.h>
#include <d3dx9.h>
#include "common.h"
#include "lf_sys.h"
#include "lv_font.h"

LVFONT* Font_Create(
	IDirect3DDevice9* lpDevice,
	char* szFilename, 
	float fCharWidth, 
	float fCharHeight,
	L_dword dwFontWidth,
	L_dword dwFontHeight)
{
	LVFONT* lpNewFont=L_null;
	LF_FILE* fin=L_null;
	L_result nResult=0;
	D3DSURFACE_DESC TexDesc;

	if(!lpDevice || !szFilename)
		return L_null;

	/* Allocate memory for the font. */
	lpNewFont=malloc(sizeof(LVFONT));
	if(!lpNewFont)
		return L_null;
	
	memset(&TexDesc, 0, sizeof(TexDesc));
	memset(lpNewFont, 0, sizeof(LVFONT));
	lpNewFont->fCharHeight=(float)dwFontHeight;
	lpNewFont->fCharWidth=(float)dwFontWidth;

	/* Attempt to load the texture. */
	fin=File_Open(szFilename, 0, LFF_READ);
	if(!fin)
		return L_null;

	/* Load the texture.  Because of the way
		the Legacy Filesystem works we can pass
		the data directly to the function. */
	nResult=D3DXCreateTextureFromFileInMemoryEx(
		lpDevice, 
		fin->pFileData,
		fin->FileSize,
		D3DX_DEFAULT,
		D3DX_DEFAULT,
		D3DX_DEFAULT,
		0,
		D3DFMT_UNKNOWN,
		D3DPOOL_MANAGED,
		D3DX_FILTER_POINT,
		D3DX_FILTER_POINT,
		0xFFFF00FF,
		L_null,
		L_null,
		&lpNewFont->lpTexture);

	File_Close(fin);

	if(L_failed(nResult))
	{
		L_safe_free(lpNewFont);
		return L_null;
	}
	
	/* Save a copy of the device, and add a reference to it. */
	lpNewFont->lpDevice=lpDevice;
	lpNewFont->lpDevice->lpVtbl->AddRef(lpNewFont->lpDevice);
	lpNewFont->lpTexture->lpVtbl->GetLevelDesc(lpNewFont->lpTexture, 0, &TexDesc);


	{
	float fCharsPerLine=0.0f;
	float fNumLines=0.0f;
	int i=0, j=0, a=0;
	L_rect rcSrc={0, 0, 0, 0};

	fCharsPerLine=(float)TexDesc.Width/fCharWidth;
	rcSrc.right=(L_long)fCharWidth;
	rcSrc.bottom=(L_long)fCharHeight;
	for(i=0, a=0; i<fNumLines, a<96; i++)
	{
		for(j=0; j<fCharsPerLine && a<96; j++, a++)
		{
			rcSrc.left=j*(L_long)fCharWidth;
			rcSrc.top=i*(L_long)fCharHeight;

			lpNewFont->lpChars[a]=L2DI_CreateFromTexture(
				lpDevice,
				&rcSrc,
				dwFontWidth,
				dwFontHeight,
				lpNewFont->lpTexture,
				L_null);
		}
	}

	}


	return lpNewFont;
}

L_bool Font_Delete(LVFONT* lpFont)
{
	int i=0;

	if(!lpFont)
		return L_false;

	for(i=0; i<96; i++)
	{
		L2DI_Delete(lpFont->lpChars[i]);
	}

	lpFont->lpTexture->lpVtbl->Release(lpFont->lpTexture);
	lpFont->lpDevice->lpVtbl->Release(lpFont->lpDevice);
	L_safe_free(lpFont);
	return L_true;
}

L_bool Font_DrawChar(
	LVFONT* lpFont, 
	IDirect3DDevice9* lpDevice, 
	char c, 
	float x, 
	float y)
{
	/* Make sure the character is renderable. */
	if(!lpFont || !lpDevice)
		return L_false;
	if(c<' ' || c>'~')
		c=' ';

	L2DI_Render(lpFont->lpChars[c-' '], lpDevice, x, y);
	return L_true;
}

L_bool Font_DrawString(
	LVFONT* lpFont, 
	IDirect3DDevice9* lpDevice, 
	char* szString, 
	float x, 
	float y)
{
	L_dword dwLen=0;
	L_dword i=0;

	if(!lpFont)
		return L_false;

	if(!szString)
		return L_false;

	dwLen=L_strlen(szString);
	for(i=0; i<dwLen; i++)
	{
		Font_DrawChar(lpFont, lpDevice, szString[i], x+(i*lpFont->fCharWidth), y);
	}
	return L_true;
}

L_bool Font_Validate(LVFONT* lpFont, IDirect3DDevice9* lpDevice)
{
	int i=0;
	if(!lpFont)
		return L_false;
	for(i=0; i<96; i++)
	{
		L2DI_Validate(lpFont->lpChars[i]);
	}
	return L_true;
}

L_bool Font_Invalidate(LVFONT* lpFont)
{
	int i=0;
	if(!lpFont)
		return L_false;
	for(i=0; i<96; i++)
	{
		L2DI_Invalidate(lpFont->lpChars[i]);
	}
	return L_true;
}