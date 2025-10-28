/* Legacy3D.c - Core elements of Legacy3D engine.
	Copyright (c) 2006, Blaine Myers. */

#include "common.h"
#include <d3d9.h>
#include <d3dx9.h>
#include "lg_sys.h"
#include "lg_cmd.h"
#include "lg_init.h"
#include "lv_init.h"
#include "lv_reset.h"

#include <stdio.h>

L3DGame* g_game=L_null;

#define LG_SHUTDOWN 0x80000010l
#define LG_V_OR_S_DISABLED 0x80000020l


L_result LG_RenderGraphics(L3DGame* lpGame)
{
	D3DMATRIX matProjection, matView, matWorld;
	L_result nResult=0;
	IDirect3DDevice9* lpDevice=lpGame->v.m_lpDevice;

	/* Validate the device. */
	if(L_failed((nResult=LV_ValidateDevice(lpGame))))
	{
		if(nResult==LVERR_CANTRECOVER)
			return LG_SHUTDOWN;
		else return LG_V_OR_S_DISABLED;
	}

	lpDevice->lpVtbl->Clear(lpDevice,0,0,D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,0xFF5050FF,1.0f,0);
	lpDevice->lpVtbl->BeginScene(lpDevice);
	
	/* The following is all just test stuff to make sure rendering is
		done correctly.*/
	
	D3DXMatrixIdentity(&matProjection);
	D3DXMatrixIdentity(&matView);
	D3DXMatrixIdentity(&matWorld);
	D3DXMatrixPerspectiveFovLH(&matProjection, D3DX_PI/4.0f, 4.0f/3.0f, 1.0f, 1000.0f);
	lpDevice->lpVtbl->SetTransform(lpDevice, D3DTS_PROJECTION, &matProjection);
	lpDevice->lpVtbl->SetTransform(lpDevice, D3DTS_VIEW, &matView);
	lpDevice->lpVtbl->SetTransform(lpDevice, D3DTS_WORLD, &matView);

	//#define TESTLIGHT
	#ifdef TESTLIGHT
	{
	POINT ps;
	D3DLIGHT9 Light;
	D3DMATERIAL9 mtrl;
	GetCursorPos(&ps);
	ScreenToClient(lpGame->m_hwnd, &ps);
	ps.x=ps.x-(L_long)CVar_GetValue(lpGame->m_cvars, "v_Width", L_null)/2;
	ps.y=(L_long)CVar_GetValue(lpGame->m_cvars, "v_Height", L_null)/2-ps.y;
	memset(&Light, 0, sizeof(D3DLIGHT9));
	Light.Type=D3DLIGHT_POINT;
	Light.Diffuse.r=1.0f;
	Light.Diffuse.g=1.0f;
	Light.Diffuse.b=1.0f;
	Light.Diffuse.a=1.0f;
	Light.Specular.r=1.0f;
	Light.Specular.g=1.0f;
	Light.Specular.b=1.0f;
	Light.Specular.a=1.0f;
	Light.Ambient.r=1.0f;
	Light.Ambient.g=1.0f;
	Light.Ambient.b=1.0f;
	Light.Ambient.a=1.0f;
	Light.Position.x=(float)ps.x;
	Light.Position.y=(float)ps.y;
	Light.Position.z=-10.0f;
	Light.Range=200.0f;
	Light.Falloff=1.0f;
	Light.Attenuation0=0.0f;
	Light.Attenuation1=1.0f;
	Light.Attenuation2=0.0f;
	lpDevice->lpVtbl->SetLight(lpDevice, 0, &Light);
	lpDevice->lpVtbl->LightEnable(lpDevice, 0, L_true);

	memset( &mtrl, 0, sizeof(mtrl) );
	mtrl.Diffuse.r = mtrl.Ambient.r = 1.0f;
	mtrl.Diffuse.g = mtrl.Ambient.g = 1.0f;
	mtrl.Diffuse.b = mtrl.Ambient.b = 1.0f;
	mtrl.Diffuse.a = mtrl.Ambient.a = 1.0f;
	lpDevice->lpVtbl->SetMaterial(lpDevice, &mtrl );
	}
	#else
	lpDevice->lpVtbl->SetRenderState(lpDevice, D3DRS_LIGHTING, L_false);
	#endif
	LVT_Render(lpGame->v.m_lpTestObj, lpDevice);
	/* here is our 2d drawing demonstration. */
	VCon_Render(lpGame->v.m_lpVCon, lpGame->v.m_lpDevice);
	
	{
	static char szFPS[11];
	sprintf(szFPS, "%i", (L_long)lpGame->l_AverageFPS->value);
	Font_Begin2(lpGame->v.m_lpVCon->lpFont);
	if(lpGame->v.m_lpVCon && lpGame->v.m_lpVCon->lpFont)
		Font_DrawString2(lpGame->v.m_lpVCon->lpFont, szFPS, 0, 0);
	Font_End2(lpGame->v.m_lpVCon->lpFont);
	}
	
	lpGame->v.m_lpDevice->lpVtbl->EndScene(lpGame->v.m_lpDevice);
	lpGame->v.m_lpDevice->lpVtbl->Present(lpGame->v.m_lpDevice, NULL, NULL, NULL, NULL);
	return LG_OK;
}


L_result LG_ProcessGame(L3DGame* lpGame)
{
	static L_dword dwLastFUpdate=0;
	static L_dword dwFrame=0;

	if(LG_RenderGraphics(lpGame)==LG_SHUTDOWN)
		return LG_SHUTDOWN;

	//lpGame->l_AverageFPS->value=(timeGetTime()-lpGame->dwLastUpdate)*1000;
	lpGame->m_dwLastUpdate=timeGetTime();

	if((lpGame->m_dwLastUpdate-dwLastFUpdate)>250)
	{
		lpGame->l_AverageFPS->value=(float)dwFrame*4.0f;
		dwFrame=0;
		dwLastFUpdate=timeGetTime();
	}
	dwFrame++;
	return LG_OK;
}


/* The game loop everything in the game controlled from here, or at least
	by one of the functions called from here.  The parameter hwnd, is the handle
	of the games window, the nShutdownMsg is used to determine if windows is
	telling the game it should shutdown. GameLoop returns TRUE if the game is
	still running, if the game is not running it returns FALSE in which case
	the application should close itself.*/
L_bool LG_GameLoop(HWND hwnd, L_bool bShutdownMsg, L_bool bActive)
{
	static L3DGame* lpGame=NULL;
	static GAMESTATE nGameState=GAMESTATE_NOTSTARTED;


	switch(nGameState)
	{
	case GAMESTATE_NOTSTARTED:
	{	
		/* By default the game status will be not started, when we first start
			(see the declaration of GAMESTATE. So we need to start it up.*/
		if(!(lpGame=LG_GameInit(".\\base", hwnd)))
		{
			g_game=L_null;
			nGameState=GAMESTATE_SHUTDOWN;
			return L_false;	
		}
		nGameState=GAMESTATE_RUNNING;
		/* Fall through and start running the game. */
	}
	case GAMESTATE_RUNNING:
	{
		if(!bActive)
			break;

		if(LG_ProcessGame(lpGame)==LG_SHUTDOWN)
			bShutdownMsg=L_true;
		break;
	}
	case GAMESTATE_SHUTDOWN:
		return L_false;
	default:
		break;
	}

	if(bShutdownMsg || lpGame->l_ShouldQuit->value)
	{	
		L_result nResult=0;
		nResult=LG_GameShutdown(lpGame);
		lpGame=L_null;
		g_game=L_null;
		nGameState=GAMESTATE_SHUTDOWN;
		return L_false;
	}

	return L_true;
}

L_bool LG_OnChar(char c)
{
	if(!g_game)
		return L_false;
	
	return VCon_OnChar(g_game->v.m_lpVCon, c);
}

L3DGame* LG_GetGame()
{
	return g_game;
}
