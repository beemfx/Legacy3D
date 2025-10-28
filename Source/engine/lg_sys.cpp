#include <d3dx9.h>
#include <stdio.h>
#include "lg_sys.h"
#include "ML_lib.h"
#include "lt_sys.h"

CLGame::CLGame():
	m_nGameState(LGSTATE_NOTSTARTED),
	m_nGameType(LGTYPE_SERVER_CLIENT),
	m_hwnd(LG_NULL),
	
	#ifdef OLD_WORLD
	m_pMeshMgr(LG_NULL),
	#endif OLD_WORLD
	
	#ifdef OLD_WORLD
	m_pWorld(LG_NULL),
	#endif
	m_WorldSrv(),
	m_WorldCln(),
	
	//m_pMMgr(LG_NULL)
	CLMgrs()
{

}

CLGame::~CLGame()
{
}

/* The game loop everything in the game controlled from here, or at least
	by one of the functions called from here.  The parameter hwnd, is the handle
	of the games window, the nShutdownMsg is used to determine if windows is
	telling the game it should shutdown. GameLoop returns TRUE if the game is
	still running, if the game is not running it returns FALSE in which case
	the application should close itself.*/
lg_bool CLGame::LG_GameLoop()
{
	if(m_nGameState!=LGSTATE_RUNNING)
		return LG_FALSE;
	
	LG_ProcessGame();
	
	if(cv_l_ShouldQuit->nValue)//l_ShouldQuit->value)
	{	
		LG_GameShutdown();
		return LG_FALSE;
	}
	return LG_TRUE;
}

void CLGame::LG_ProcessGame()
{
	//First thing we should do is update the timer
	//Note that the timer should only be updated here as
	//the whole frame should be processed as if it were the
	//same time, even though some time will pass durring
	//the processing.
	s_Timer.Update();
	
	
	//Now update input (if the console is active
	//we'll let windows handle keyboard input to
	//the console (see win_sys.cpp).
	if(!m_VCon.IsActive())
	{
		if(m_Input.LI_Update(LI_UPDATE_ALL)==LIS_CONSOLE)
			m_VCon.Toggle();
	}
	else
		m_Input.LI_Update(LI_UPDATE_MOUSE|LI_UPDATE_GP);
		
	//Should process AI.
	#ifdef OLD_WORLD
	m_pWorld->ProcessEntities();
	#endif
	m_WorldSrv.ProcessServer();
	m_WorldCln.Update();
	
	//The render everything...
	LG_RenderGraphics();
	//Now update the sound engine...
	m_SndMgr.Update();
	
	
	//Here is some week framerate code.
	static lg_dword dwLastTick=s_Timer.GetTime();
	static lg_dword dwFrame=0;
	
	if((s_Timer.GetTime()-dwLastTick)>250)
	{
		CV_Set_l(cv_l_AverageFPS, dwFrame*4);
		dwFrame=0;
		dwLastTick=s_Timer.GetTime();
		m_VCon.SetMessage(1, cv_l_AverageFPS->szValue);
	}
	dwFrame++;
	
}
//LG_OnChar is only for console input,
//it is called from the windows messaging
//system, all other keyboard input is 
//managed by the CLInput class.
void CLGame::LG_OnChar(char c)
{
	m_VCon.OnCharA(c);
}

void CLGame::LG_SendCommand(lg_cstr szCommand)
{
	LC_SendCommand(szCommand);
}

lg_result CLGame::LG_RenderGraphics()
{
	D3DXMATRIX matT;
	lg_result nResult=0;

	/* Make sure the device can still render. If not
		call LV_ValidateDevice() until the device can
		render again.*/
	if(LG_FAILED(s_pDevice->TestCooperativeLevel()))
		return LV_ValidateDevice();
		
	D3DVIEWPORT9 vp;

	s_pDevice->Clear(0,0,D3DCLEAR_ZBUFFER|D3DCLEAR_TARGET,0xFF5050FF,1.0f,0);
	s_pDevice->BeginScene();
	
	
	#ifdef OLD_WORLD
	m_pWorld->Render();
	#endif
	//Render the client.
	SetWorldRS();
	m_WorldCln.Render();
	SetWindowRS();
	m_WndMgr.Render();
	SetConsoleRS();
	m_VCon.Render();
	
	s_pDevice->EndScene();
	s_pDevice->Present(NULL, NULL, NULL, NULL);
	return LG_OK;
}

void CLGame::SetWorldRS()
{
}

void CLGame::SetWindowRS()
{
}

void CLGame::SetConsoleRS()
{
}
