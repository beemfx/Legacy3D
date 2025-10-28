#ifndef __LEGACY3D_H__
#define __LEGACY3D_H__

#ifdef __cplusplus
extern "C"{
#endif

#include <d3d9.h>
#include <dsound.h>
#include "lg_err.h"
#include "lv_con.h"
#include "lv_font2.h"
#include <lc_sys.h>
#include "common.h"

#include "lv_test.h"

#define LGAME_NAME    "Legacy 3D Game Engine"
#define LGAME_VERSION "1.00"

typedef enum tagGAMESTATE{
	GAMESTATE_UNKNOWN    =0, 
	GAMESTATE_NOTSTARTED =1, 
	GAMESTATE_RUNNING    =2, 
	GAMESTATE_SHUTDOWN   =3,
	GAMESTATE_FORCE_DWORD=0xFFFFFFFFl
} GAMESTATE, *PGAMESTATE;

typedef struct tagL3DVIDEO{
	IDirect3D9*        m_lpD3D;
	IDirect3DDevice9*  m_lpDevice;
	IDirect3DSurface9* m_lpBackSurface;
	L_uint             m_nAdapterID;
	D3DDEVTYPE         m_nDeviceType;
	D3DFORMAT          m_nBackBufferFormat;
	LVCON*             m_lpVCon;

	LVT_OBJ*           m_lpTestObj;
}L3DVIDEO;

typedef struct tagL3DSOUND{
	L_bool               m_bAvailable;
	IDirectSound8*       m_lpDS;
	IDirectSoundBuffer*  m_lpPrimaryBuffer;

	IDirectSoundBuffer8* m_lpTestSound;
}L3DSOUND;

typedef struct tagL3DGame{
	//GAMESTATE m_nGamestate;
	HWND      m_hwnd;
	HLCONSOLE m_Console;
	HCVARLIST m_cvars;
	L3DVIDEO  v;
	L3DSOUND  s;
	CVar*     l_ShouldQuit;
	CVar*     l_AverageFPS;
	L_dword   m_dwLastUpdate;
}L3DGame, *LPL3DGame;

#define LG_OK  (0x00000000l)
#define LG_ERR (0x80000000l)
#define LG_FAIL LG_ERR


L_bool LG_GameLoop(HWND hwnd, L_bool bShutdownMsg, L_bool bActive);
L3DGame* LG_GetGame();

#define LK_PAGEUP   1
#define LK_PAGEDOWN 2
#define LK_END      3
L_bool LG_OnChar(char c);

#ifdef __cplusplus
}
#endif

#endif /* __LEGACY3D_H__ */