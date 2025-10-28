#include "common.h"
#include <stdio.h>
#include "lv_con.h"
#include "lg_sys.h"
#include <lf_sys.h>

#define CONSOLE_WIDTH  640.0f
#define CONSOLE_HEIGHT 480.0f


L_bool VCon_UpdatePos(LVCON* lpVCon)
{
	float fElapsedTime=0;
	float fScrollDist=0.0f;

	if(!lpVCon)
		return L_false;

	if(!lpVCon->bActive)
		return L_false;

	if((timeGetTime()-lpVCon->dwLastCaretChange)>GetCaretBlinkTime())
	{
		lpVCon->bCaretOn=!lpVCon->bCaretOn;
		lpVCon->dwLastCaretChange=timeGetTime();
	}

	/* In case the console is cleared we make sure teh current console position
		is allowed. */
	if(lpVCon->nScrollPos>(L_long)Con_GetNumEntries(lpVCon->hConsole))
		lpVCon->nScrollPos=0;

	fElapsedTime=(float)(timeGetTime()-lpVCon->dwLastUpdate);
	fScrollDist=fElapsedTime/1.5f;
	if(lpVCon->bDeactivating)
	{
		if(lpVCon->fPosition<=0.0f)
		{
			lpVCon->fPosition=0.0f;
			lpVCon->bActive=L_false;
			return L_true;
		}
		lpVCon->fPosition-=fScrollDist;
	}
	else
	{
		lpVCon->fPosition+=fScrollDist;
		if(lpVCon->fPosition>(CONSOLE_HEIGHT/(2-lpVCon->bFullMode)))
			lpVCon->fPosition=(CONSOLE_HEIGHT/(2-lpVCon->bFullMode));
	}
	return L_true;
}

__inline void VCon_FontStringToSize(L_dword* pHeight, L_dword* pWidth, L_pstr szString)
{
	L_dword dwLen=L_strlen(szString);
	L_bool bFoundHeight=L_false;
	char szHeight[32];
	L_pstr szWidth=L_null;
	L_dword i=0;
	if(dwLen<1)
	{
		*pWidth=0;
		*pHeight=16;
	}
	for(i=0; i<dwLen; i++)
	{
		if(!bFoundHeight)
		{
			if(szString[i]=='X' || szString[i]=='x')
			{
				bFoundHeight=L_true;
				szHeight[i]=0;
				szWidth=&szString[i+1];
				break;
			}
			szHeight[i]=szString[i];
		}
	}

	*pHeight=L_atol(szHeight);
	*pWidth=L_atol(szWidth);
	return;
}

LVCON* VCon_Create(
	HLCONSOLE hConsole,
	HCVARLIST cvars,
	IDirect3DDevice9* lpDevice)
{
	LVCON* lpNewCon=L_null;
	D3DVIEWPORT9 ViewPort;
	//L_byte dwFontSize=(L_byte)CVar_GetValue(cvars, "lc_FontSize", L_null);
	L_pstr  szBGTexture=CVar_Get(cvars, "lc_BG", L_null);
	//L_pstr  szFontSize=L_null;
	L_dword dwFontWidth=0, dwFontHeight=0;

	//szFontSize=CVar_Get(cvars, "lc_FontSize", L_null);
	/*We need to get the font size and convert it to a componet size.*/
	VCon_FontStringToSize(&dwFontHeight, &dwFontWidth, CVar_Get(cvars, "lc_FontSize", L_null));

	/* Allocate memory for the graphic console. */
	lpNewCon=malloc(sizeof(LVCON));
	if(!lpNewCon)
	{
		Err_Printf("Could not allocate memory for graphic console.");
		return L_null;
	}

	memset(lpNewCon, 0, sizeof(LVCON));

	lpDevice->lpVtbl->GetViewport(lpDevice, &ViewPort);
	lpNewCon->dwViewHeight=ViewPort.Height;
	lpNewCon->dwFontHeight=dwFontHeight;

	/*
	{
	char szTemp[100];
	sprintf(szTemp, "Width: %i Height: %i", dwFontWidth, dwFontHeight);
	MessageBox(0, szTemp, 0, 0);
	}
	*/

	/* Attempt to laod the background. */
	lpNewCon->lpBackground=L2DI_CreateFromFile(
		lpDevice,
		szBGTexture,
		L_null,
		(L_dword)CONSOLE_WIDTH,
		(L_dword)CONSOLE_HEIGHT,
		0x00000000);


	/* If we can't create a background from a texture,
		we attempt to do it from a color. */
	if(!lpNewCon->lpBackground)
	{
		Err_Printf("Could not load texture from \"%s\" for console background, using color.", szBGTexture);
		lpNewCon->lpBackground=L2DI_CreateFromColor(
			lpDevice,
			(L_dword)CONSOLE_WIDTH,
			(L_dword)CONSOLE_HEIGHT,
			0xFF8080FF);
	}

	if(!lpNewCon->lpBackground)
	{
		Err_Printf("Could not load console background, failing.");
		L_safe_free(lpNewCon);
		return L_null;
	}

	/* Attemp to load a font for the console. */
	if(CVar_GetValue(cvars, "lc_UseLegacyFont", L_null) || CVar_GetValue(cvars, "v_Force16BitTextures", L_null))
	{
		L_dword dwWidthInFile=0, dwHeightInFile=0;
		VCon_FontStringToSize(&dwHeightInFile, &dwWidthInFile, CVar_Get(cvars, "lc_LegacyFontSizeInFile", L_null));
		if(!dwFontWidth)
			dwFontWidth=dwFontHeight/2;
		lpNewCon->lpFont=Font_Create2(
			lpDevice,
			CVar_Get(cvars, "lc_LegacyFont", L_null),
			(L_byte)dwFontWidth,
			(L_byte)dwFontHeight,
			L_false,
			(L_byte)dwWidthInFile,
			(L_byte)dwHeightInFile,
			0);
	}
	else
	{
		lpNewCon->lpFont=Font_Create2(
			lpDevice,
			CVar_Get(cvars, "lc_Font", L_null),
			(L_byte)dwFontWidth,
			(L_byte)dwFontHeight,
			L_true,
			0,
			0,
			(0xFF<<24)|(L_dword)CVar_GetValue(cvars, "lc_FontColor", L_null));
	}
	
	
	if(!lpNewCon->lpFont)
	{
		Err_Printf("Could not load font for console, failing.");
		L2DI_Delete(lpNewCon->lpBackground);
		L_safe_free(lpNewCon);
		return L_null;
	}
	
	/* Attach the console */
	lpNewCon->hConsole=hConsole;
	/* Set the last time the console was updated to now. */
	lpNewCon->dwLastUpdate=timeGetTime();

	/* We need to set a few starting values for the console. */
	lpNewCon->bCaretOn=L_true;
	lpNewCon->dwLastCaretChange=timeGetTime();
	lpNewCon->bActive=L_false;
	lpNewCon->bDeactivating=L_true;
	lpNewCon->fPosition=0;
	lpNewCon->bFullMode=L_false;

	return lpNewCon;
}

L_bool VCon_Delete(
	LVCON* lpVCon)
{
	if(!lpVCon)
		return L_false;

	/* Delete the bg and the font. */
	L2DI_Delete(lpVCon->lpBackground);
	Font_Delete2(lpVCon->lpFont);
	/* Delete the console itself. */
	L_safe_free(lpVCon);
	return L_true;
}

L_bool VCon_Render(
	LVCON* lpVCon,
	IDirect3DDevice9* lpDevice)
{
	float fRenderPos=0;
	L_dword i=0;
	L_long nStart=0;
	L_long nCharHeight=0;
	LVFONT_DIMS fontdims;

	if(!lpVCon)
		return L_false;

	/* Update the position, and activity of the console. */
	VCon_UpdatePos(lpVCon);
	lpVCon->dwLastUpdate=timeGetTime();

	if(!lpVCon->bActive)
		return L_true;
	
	nCharHeight=lpVCon->dwFontHeight;//16;//lpVCon->lpFont->m_dwFontHeight;
	/* Render the background at the position. */
	L2DI_StartStopDrawing(lpDevice, (L_long)CONSOLE_WIDTH, (L_long)CONSOLE_HEIGHT, L_true);
	L2DI_Render(lpVCon->lpBackground, lpDevice, 0.0f, lpVCon->fPosition-CONSOLE_HEIGHT);
	L2DI_StartStopDrawing(lpDevice, 0, 0, L_false);
	/* Now render all visible text. */
	/* First we render the active entry. */
	Font_GetDims(lpVCon->lpFont, &fontdims);
	if(fontdims.bD3DXFont)
		fRenderPos=lpVCon->fPosition*lpVCon->dwViewHeight/CONSOLE_HEIGHT-nCharHeight-8;//800.0f-nCharHeight;//lpVCon->fPosition*800.0f/CONSOLE_HEIGHT;
	else 
		fRenderPos=lpVCon->fPosition-nCharHeight-8;//CONSOLE_HEIGHT/2-nCharHeight-8;
	//fRenderPos-=(nCharHeight*nStart)-8.0f;

	Font_Begin2(lpVCon->lpFont);
	{
	char szTemp[1024];
	_snprintf(szTemp, 1022, "]%s", Con_GetEntry(lpVCon->hConsole, 1, L_null));
	if(lpVCon->bCaretOn)
		strncat(szTemp, "_", 1);
	Font_DrawString2(
		lpVCon->lpFont, 
		szTemp, 
		8, 
		(L_long)fRenderPos);
	}
	fRenderPos-=nCharHeight;
	if(lpVCon->nScrollPos)
	{
		Font_DrawString2(
			lpVCon->lpFont,
			"^ ^ ^ ^",
			8,
			(L_long)fRenderPos);
		nStart=3;
		fRenderPos-=nCharHeight;
	}
	else
		nStart=2;

	for(i=nStart+lpVCon->nScrollPos; fRenderPos>=(-nCharHeight); fRenderPos-=nCharHeight, i++)
	{
		if(!Font_DrawString2(
			lpVCon->lpFont,
			Con_GetEntry(lpVCon->hConsole, i, L_null),
			8,
			(L_long)fRenderPos))
			break;
	}
	
	Font_End2(lpVCon->lpFont);
	
	return L_true;
}


L_bool VCon_Validate(
	LVCON* lpVCon,
	IDirect3DDevice9* lpDevice)
{
	D3DVIEWPORT9 ViewPort;
	if(!lpVCon || !lpDevice)
		return L_false;

	L2DI_Validate(lpVCon->lpBackground, lpDevice, L_null);
	Font_Validate2(lpVCon->lpFont);

	lpDevice->lpVtbl->GetViewport(lpDevice, &ViewPort);
	lpVCon->dwViewHeight=ViewPort.Height;
	return L_true;
}

L_bool VCon_Invalidate(
	LVCON* lpVCon)
{
	if(!lpVCon)
		return L_false;

	L2DI_Invalidate(lpVCon->lpBackground);
	Font_Invalidate2(lpVCon->lpFont);
	return L_true;
}

/* VCon_OnChar - processes input through the graphical console,
	it checks to see if it should activate/deactive and then it
	checks to see if it is active.  If it isn't active it won't
	do anything, if it is, it can scoll up the entry list, and
	it will pass any other data to the actual console. */

L_bool VCon_OnChar(
	LVCON* lpVCon,
	char c)
{
	if(!lpVCon)
		return L_false;

	if(c=='`' || c=='~')
	{
		lpVCon->bDeactivating=!lpVCon->bDeactivating;
		if(!lpVCon->bDeactivating)
			lpVCon->bActive=L_true;

		return L_true;
	}

	if(!lpVCon->bActive)
		return L_false;


	/* There are two special coes, LK_PAGEUP and LK_PAGEDOWN that are passed from
		win_sys.c when the pageup and pagedown keys are pressed, that way the console
		can know if it should scroll up or down. */
	switch(c)
	{
	case LK_PAGEUP:
		lpVCon->nScrollPos++;
		if(lpVCon->nScrollPos>=((L_long)Con_GetNumEntries(lpVCon->hConsole)-2))
			lpVCon->nScrollPos=Con_GetNumEntries(lpVCon->hConsole)-2;
		break;
	case LK_PAGEDOWN:
		lpVCon->nScrollPos--;
		if(lpVCon->nScrollPos<0)
			lpVCon->nScrollPos=0;
		break;
	case LK_END:
		lpVCon->nScrollPos=0;
		break;
	default:
		return Con_OnChar(lpVCon->hConsole, c);
	}

	return L_true;
}