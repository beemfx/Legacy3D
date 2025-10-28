#ifndef __LV_FONT2_H__
#define __LV_FONT2_H__
#include <d3d9.h>
#include "common.h"
#ifdef __cplusplus
extern "C"{
#endif /*__cplusplus*/

typedef void* HLVFONT2;
typedef struct _LVFONT_DIMS{
	L_byte nCharWidth;
	L_byte nCharHeight;
	L_bool bD3DXFont;
	L_dword dwD3DColor;
}LVFONT_DIMS;

HLVFONT2 Font_Create2(
	IDirect3DDevice9* lpDevice, 
	char* szFont, 
	L_byte dwWidth, 
	L_byte dwHeight,
	L_bool bD3DXFont,
	L_byte dwWidthInFile,
	L_byte dwHeightInFile,
	L_dword dwD3DColor);
L_bool Font_Delete2(HLVFONT2 hFont);

L_bool Font_GetDims(HLVFONT2 hFont, LVFONT_DIMS* pDims);

L_bool Font_Begin2(HLVFONT2 hFont);
L_bool Font_End2(HLVFONT2 hFont);

L_bool Font_DrawString2(HLVFONT2 hFont, char* szString, L_long x, L_long y);

L_bool Font_Validate2(HLVFONT2 hFont);
L_bool Font_Invalidate2(HLVFONT2 hFont);


#ifdef __cplusplus
}
#endif /*__cplusplus*/
#endif /*__LV_FONT2_H__*/