/* lv_con.h - the graphical console. */
#ifndef __LV_CON_H__
#define __LV_CON_H__
#include "common.h"
#include "lv_font2.h"
#include "lv_img2d.h"
#include <lc_sys.h>

#ifdef __cplusplus
extern "C"{
#endif /* __cplusplus */

typedef struct _LVCON{
	LV2DIMAGE*        lpBackground; /* The background image. */
	void*             lpFont;       /* The font. */
	L_dword           dwViewHeight;  /* The view height. */
	L_dword           dwFontHeight;
	HLCONSOLE         hConsole;     /* Pointer to the console. */
	IDirect3DDevice9* lpDevice;     /* The device that created teh console. */
	L_bool            bActive;      /* Whether or not the console is receiving input. */
	L_bool            bDeactivating;/* If the console is in the process of deactivating (scrolling up).*/
	float             fPosition;    /* The position of the console. */
	long              nScrollPos;  /* Where the console is rendering output. */
	L_dword           dwLastUpdate; /* The last time the console position was updated. */
	L_bool            bCaretOn;
	L_dword           dwLastCaretChange;
	L_bool            bFullMode;
}LVCON, *LPLVCON;

LVCON* VCon_Create(
	HLCONSOLE hConsole,
	HCVARLIST cvars,
	IDirect3DDevice9* lpDevice);

L_bool VCon_Delete(
	LVCON* lpVCon);

L_bool VCon_Render(
	LVCON* lpVCon,
	IDirect3DDevice9* lpDevice);

L_bool VCon_Validate(
	LVCON* lpVCon,
	IDirect3DDevice9* lpDevice);

L_bool VCon_Invalidate(
	LVCON* lpVCon);

L_bool VCon_OnChar(
	LVCON* lpVCon,
	char c);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif __LV_CON_H__