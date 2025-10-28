/* lv_font.h - Header for the legacy font functionality. */
#ifndef __LV_D3DFONT_H__
#define __LV_D3DFONT_H__

#include <d3dx9.h>
#include "lv_img2d.h"

#ifdef __cplusplus
extern "C"{
#endif /* __cplusplus */

typedef struct _LVFONT{
	ID3DXFont*   m_lpFont;
	ID3DXSprite* m_lpSurface;
	L_dword      m_dwFontWidth;
	L_dword      m_dwFontHeight;
	L_bool       m_bDrawing;
}LVFONT, *LPLVFONT;

LVFONT* Font_Create(
	IDirect3DDevice9* lpDevice,
	char* szFontName,
	L_dword dwWidth,
	L_dword dwHeight,
	L_dword dwWeight);

L_bool Font_Delete(LVFONT* lpFont);

L_bool Font_DrawString(
	LVFONT* lpFont,
	char* szString, 
	L_long x, 
	L_long y);

L_bool Font_Validate(LVFONT* lpFont, IDirect3DDevice9* lpDevice);
L_bool Font_Invalidate(LVFONT* lpFont);
L_bool Font_Begin(LVFONT* lpFont);
L_bool Font_End(LVFONT* lpFont);


#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /*__LV_D3DFONT_H__*/