#include <stdio.h>
#include "lv_con.h"
#include "lg_err.h"
#include "lt_sys.h"
#include "lg_cvars.h"
#include "lg_func.h"
#include "../lc_sys2/lc_sys2.h"

#define CONSOLE_WIDTH  640.0f
#define CONSOLE_HEIGHT 480.0f


/**********************************************
*** The Legacy Game Visual Console (CLVCon) ***
**********************************************/

CLVCon::CLVCon():
	m_lpFont(LG_NULL),
	m_dwViewHeight(0),
	m_dwFontHeight(0),
	//m_hConsole(LG_NULL),
	m_pDevice(LG_NULL),
	m_bActive(0),
	m_bDeactivating(0),
	m_fPosition(0.0f),
	m_nScrollPos(0),
	m_dwLastUpdate(0),
	m_bCaretOn(LG_FALSE),
	m_dwLastCaretChange(0),
	m_bFullMode(LG_FALSE)
{
	m_szMessage[0]=0;
}

CLVCon::CLVCon(IDirect3DDevice9* pDevice):
	m_lpFont(LG_NULL),
	m_dwViewHeight(0),
	m_dwFontHeight(0),
	m_pDevice(LG_NULL),
	m_bActive(0),
	m_bDeactivating(0),
	m_fPosition(0.0f),
	m_nScrollPos(0),
	m_dwLastUpdate(0),
	m_bCaretOn(LG_FALSE),
	m_dwLastCaretChange(0),
	m_bFullMode(LG_FALSE)
{
	m_szMessage[0]=0;
	Create(pDevice);
}

CLVCon::~CLVCon()
{
	Delete();
}

lg_bool CLVCon::IsActive()
{
	return m_bActive;
}

void CLVCon::SetMessage(lg_dword nSlot, lg_char* szMessage)
{
	lg_char* szTemp=nSlot==1?m_szMessage:m_szMessage2;
	
	if(szMessage==LG_NULL)
		szTemp[0]=0;
	else
		LG_strncpy(szTemp, szMessage, 127);
}

lg_bool CLVCon::Create(IDirect3DDevice9* pDevice)
{
	Delete();
	
	D3DVIEWPORT9 ViewPort;
	lg_pstr  szBGTexture=CV_Get(CVAR_lc_BG)->szValue;
	lg_dword dwFontWidth=0, dwFontHeight=0;

	/*We need to get the font size and convert it to a componet size.*/
	
	FontStringToSize(
		&dwFontHeight,
		&dwFontWidth,
		CV_Get(CVAR_lc_FontSize)->szValue);

	pDevice->GetViewport(&ViewPort);
	m_dwViewHeight=ViewPort.Height;
	m_dwFontHeight=dwFontHeight;

	/* Attempt to laod the background. */
	lg_bool bResult=m_Background.CreateFromFile(
		pDevice,
		szBGTexture,
		LG_NULL,
		(lg_dword)CONSOLE_WIDTH,
		(lg_dword)CONSOLE_HEIGHT,
		0x00000000);


	/* If we can't create a background from a texture,
		we attempt to do it from a color. */
	if(!bResult)
	{
		Err_Printf("Could not load texture from \"%s\" for console background, using color.", szBGTexture);
		bResult=m_Background.CreateFromColor(
			pDevice,
			(lg_dword)CONSOLE_WIDTH,
			(lg_dword)CONSOLE_HEIGHT,
			0xFF8080FF);
	}

	if(!bResult)
	{
		Err_Printf("Could not load console background, failing.");
		return LG_FALSE;
	}
	
	// Attemp to load a font for the console.
	if(0)//CVar_GetValue(cvars, "lc_UseLegacyFont", LG_NULL) || CVar_GetValue(cvars, "v_Force16BitTextures", LG_NULL))
	{
		Err_Printf("Using Legacy Font");
		lg_dword dwWidthInFile=0, dwHeightInFile=0;
		FontStringToSize(&dwHeightInFile, &dwWidthInFile, CV_Get(CVAR_lc_LegacyFontSizeInFile)->szValue);
		if(!dwFontWidth)
			dwFontWidth=dwFontHeight/2;
			
		/*
		m_pFont=Font_Create2(
			pDevice,
			CVar_Get(cvars, "lc_LegacyFont", LG_NULL),
			(lg_byte)dwFontWidth,
			(lg_byte)dwFontHeight,
			LG_FALSE,
			(lg_byte)dwWidthInFile,
			(lg_byte)dwHeightInFile,
			0);
		*/
		
		m_lpFont=CLFont::Create(
			pDevice,
			CV_Get(CVAR_lc_LegacyFont)->szValue,
			(lg_byte)dwFontWidth,
			(lg_byte)dwFontHeight,
			LG_FALSE,
			(lg_byte)dwWidthInFile,
			(lg_byte)dwHeightInFile,
			0);
		
	}
	else
	{
		
		m_lpFont=CLFont::Create(
			pDevice,
			"fixedsys",//CVar_Get(cvars, "lc_Font", LG_NULL),
			(lg_byte)dwFontWidth,
			(lg_byte)dwFontHeight,
			LG_TRUE,
			0,
			0,
			(0xFF<<24)|CV_Get(CVAR_lc_FontColor)->nValue);
		
		/*
		m_pFont=Font_Create2(
			pDevice,
			CVar_Get(cvars, "lc_Font", LG_NULL),
			(lg_byte)dwFontWidth,
			(lg_byte)dwFontHeight,
			LG_TRUE,
			0,
			0,
			(0xFF<<24)|(lg_dword)CVar_GetValue(cvars, "lc_FontColor", LG_NULL));
		*/
	}
	
	
	/*
	if(!m_lpFont)
	{
		Err_Printf("Could not load font for console, failing.");
		//L2DI_Delete(m_pBackground);
		m_Background.Delete();
		return LG_FALSE;
	}
	*/
	
	/* Attach the console */
	/* Set the last time the console was updated to now. */
	m_dwLastUpdate=LT_GetTimeMS();

	/* We need to set a few starting values for the console. */
	m_bCaretOn=LG_TRUE;
	m_dwLastCaretChange=LT_GetTimeMS();
	m_bActive=LG_FALSE;
	m_bDeactivating=LG_TRUE;
	m_fPosition=0.0f;
	m_bFullMode=LG_FALSE;

	m_pDevice=pDevice;
	pDevice->AddRef();
	return LG_TRUE;
}

void CLVCon::Delete()
{
	/* Delete the bg and the font. */
	m_Background.Delete();
		

	if(m_lpFont)
	{
		m_lpFont->Delete();
		delete m_lpFont;
		m_lpFont=LG_NULL;
	}
	/* Delete the console itself. */
	L_safe_release(m_pDevice);
	
	m_lpFont=LG_NULL;
	m_dwViewHeight=0;
	m_dwFontHeight=0;
	//m_hConsole=LG_NULL;
	m_pDevice=LG_NULL;
	m_bActive=0;
	m_bDeactivating=0;
	m_fPosition=0.0f;
	m_nScrollPos=0;
	m_dwLastUpdate=0;
	m_bCaretOn=LG_FALSE;
	m_dwLastCaretChange=0;
	m_bFullMode=LG_FALSE;
}

lg_bool CLVCon::Render()
{
	float fRenderPos=0;
	lg_dword i=0;
	lg_long nStart=0;
	lg_long nCharHeight=0;

	/* Update the position, and activity of the console. */
	UpdatePos();
	m_dwLastUpdate=LT_GetTimeMS();
	
	if(m_szMessage[0]!=0 || m_szMessage2!=0)
	{
		m_lpFont->Begin();
		m_lpFont->DrawString(m_szMessage, 0, 0);
		m_lpFont->DrawString(m_szMessage2, 0, 8);
		m_lpFont->End();
	}

	if(!m_bActive || !m_pDevice)
		return LG_TRUE;
	
	nCharHeight=m_dwFontHeight;//16;//lpVCon->lpFont->m_dwFontHeight;
	/* Render the background at the position. */
	CLImg2D::StartStopDrawing(m_pDevice, (lg_long)CONSOLE_WIDTH, (lg_long)CONSOLE_HEIGHT, LG_TRUE);
	m_Background.Render(0.0f, m_fPosition-CONSOLE_HEIGHT);
	CLImg2D::StartStopDrawing(m_pDevice, 0, 0, LG_FALSE);
	/* Now render all visible text. */
	/* First we render the active entry. */
	
	
	//Font_GetDims(m_pFont, &fontdims);
	lg_bool bD3DFont=LG_FALSE;
	m_lpFont->GetDims(LG_NULL, LG_NULL, &bD3DFont);
	if(bD3DFont)
		fRenderPos=m_fPosition*m_dwViewHeight/CONSOLE_HEIGHT-nCharHeight-8;
	else 
		fRenderPos=m_fPosition-nCharHeight-8;

	m_lpFont->Begin();
	{
	char szTemp[1024];
	_snprintf(szTemp, 1022, "]%s", LC_GetActiveLine());
	if(m_bCaretOn)
		strncat(szTemp, "_", 1);
	m_lpFont->DrawString(
		szTemp,
		8,
		(lg_long)fRenderPos);
	}
	fRenderPos-=nCharHeight;
	if(m_nScrollPos)
	{
		m_lpFont->DrawString(
			"^ ^ ^ ^",
			8,
			(lg_long)fRenderPos);
		nStart=2;
		fRenderPos-=nCharHeight;
	}
	else
		nStart=0;

	const lg_char* szLine=LC_GetLine(nStart+m_nScrollPos, LG_TRUE);
	while(fRenderPos>=(-nCharHeight) && szLine)
	{
		m_lpFont->DrawString(
			(char*)szLine,
			8,
			(lg_long)fRenderPos);
			
		szLine=LC_GetNextLine();
			
		fRenderPos-=nCharHeight;
	}
	m_lpFont->End();
	
	return LG_TRUE;
}


lg_bool CLVCon::Validate()
{
	if(!m_pDevice)
		return LG_FALSE;

	m_Background.Validate(LG_NULL);
	m_lpFont->Validate();

	D3DVIEWPORT9 ViewPort;
	m_pDevice->GetViewport(&ViewPort);
	m_dwViewHeight=ViewPort.Height;
	return LG_TRUE;
}

lg_bool CLVCon::Invalidate()
{
	m_Background.Invalidate();
	m_lpFont->Invalidate();
	return LG_TRUE;
}

void CLVCon::Toggle()
{
	m_bDeactivating=!m_bDeactivating;
		if(!m_bDeactivating)
			m_bActive=LG_TRUE;
}

/* VCon_OnChar - processes input through the graphical console,
	it checks to see if it should activate/deactive and then it
	checks to see if it is active.  If it isn't active it won't
	do anything, if it is, it can scoll up the entry list, and
	it will pass any other data to the actual console. */

lg_bool CLVCon::OnCharA(char c)
{
	if(!m_bActive)
		return LG_TRUE;
	
	if(c==CON_KEY1 || c==CON_KEY2)
	{
		Toggle();
		return LG_TRUE;
	}

	/* There are two special coes, LK_PAGEUP and LK_PAGEDOWN that are passed from
		win_sys.c when the pageup and pagedown keys are pressed, that way the console
		can know if it should scroll up or down. */
	switch(c)
	{
	case LK_PAGEUP:
		m_nScrollPos++;
		if(m_nScrollPos>=(lg_long)LC_GetNumLines()-2)//((lg_long)Con_GetNumEntries(m_hConsole)-2))
			m_nScrollPos=(lg_long)LC_GetNumLines()-2;//Con_GetNumEntries(m_hConsole)-2;
		break;
	case LK_PAGEDOWN:
		m_nScrollPos--;
		if(m_nScrollPos<0)
			m_nScrollPos=0;
		break;
	case LK_END:
		m_nScrollPos=0;
		break;
	default:
		LC_OnChar(c);
		break;
	}

	return LG_TRUE;
}

lg_bool CLVCon::UpdatePos()
{
	float fElapsedTime=0;
	float fScrollDist=0.0f;

	if(!m_bActive)
		return LG_FALSE;

	if((LT_GetTimeMS()-m_dwLastCaretChange)>GetCaretBlinkTime())
	{
		m_bCaretOn=!m_bCaretOn;
		m_dwLastCaretChange=LT_GetTimeMS();
	}

	/* In case the console is cleared we make sure teh current console position
		is allowed. */
	if(m_nScrollPos>(lg_long)LC_GetNumLines())
		m_nScrollPos=0;

	fElapsedTime=(float)(LT_GetTimeMS()-m_dwLastUpdate);
	fScrollDist=fElapsedTime/1.5f;
	if(m_bDeactivating)
	{
		if(m_fPosition<=0.0f)
		{
			m_fPosition=0.0f;
			m_bActive=LG_FALSE;
			return LG_TRUE;
		}
		m_fPosition-=fScrollDist;
	}
	else
	{
		m_fPosition+=fScrollDist;
		if(m_fPosition>(CONSOLE_HEIGHT/(2-m_bFullMode)))
			m_fPosition=(CONSOLE_HEIGHT/(2-m_bFullMode));
	}
	return LG_TRUE;
}


__inline void CLVCon::FontStringToSize(lg_dword* pHeight, lg_dword* pWidth, lg_pstr szString)
{
	lg_dword dwLen=L_strlen(szString);
	lg_bool bFoundHeight=LG_FALSE;
	char szHeight[32];
	lg_pstr szWidth=LG_NULL;
	lg_dword i=0;
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
				bFoundHeight=LG_TRUE;
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
