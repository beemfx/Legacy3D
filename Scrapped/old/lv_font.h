/* lv_font.h - Header for the legacy font functionality. */
#ifndef __LV_FONT_H__
#define __LV_FONT_H__

#include "lv_img2d.h"

#ifdef __cplusplus
extern "C"{
#endif /* __cplusplus */

typedef struct _LVFONT{
	IDirect3DDevice9* lpDevice;
	IDirect3DTexture9* lpTexture;
	LV2DIMAGE* lpChars[96]; /* Our font supports 96 characters. */
	float fCharWidth;
	float fCharHeight;
}LVFONT, *LPLVFONT;

LVFONT* Font_Create(
	IDirect3DDevice9* lpDevice, 
	char* szFilename, 
	float fCharWidth, 
	float fCharHeight,
	L_dword dwFontWidth,
	L_dword dwFontHeight);
L_bool Font_Delete(LVFONT* lpFont);

L_bool Font_DrawChar(LVFONT* lpFont, IDirect3DDevice9* lpDevice, char c, float x, float y);
L_bool Font_DrawString(LVFONT* lpFont, IDirect3DDevice9* lpDevice, char* szString, float x, float y);

L_bool Font_Validate(LVFONT* lpFont, IDirect3DDevice9* lpDevice);
L_bool Font_Invalidate(LVFONT* lpFont);


#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /*__LV_FONT_H__*/