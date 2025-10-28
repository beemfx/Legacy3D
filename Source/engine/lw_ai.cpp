#include "lw_ai.h"
#include <string.h>
#include "lg_err.h"


// Hack
#include "../game/test_game_main.cpp"
#include "../game/lw_ai_test.cpp"


CLWAIMgr::CLWAIMgr()
: m_hDllFile(LG_NULL)
{

}

CLWAIMgr::~CLWAIMgr()
{
#if 0
	if(m_hDllFile)
	{
		FreeLibrary(m_hDllFile);
		m_hDllFile=LG_NULL;
	}
#endif
}

void CLWAIMgr::LoadAI(lg_path szFilename)
{
	CloseAI();
	Err_Printf("= AI script: %s. =", szFilename);
	Err_IncTab();
	
#if 0
	m_hDllFile=LoadLibraryEx(szFilename, LG_NULL, 0);
	if(!m_hDllFile)
	{
		Err_Printf("Could not load the specified game file.");
		CloseAI();
	}
	
	//Setup all the methods:
	LWAI_OBTAIN_FUNC(Game_Init);
	LWAI_OBTAIN_FUNC(Game_ObtainAI);
	LWAI_OBTAIN_FUNC(Game_UpdateGlobals);
#endif

	Game_Init = TestGame_Game_Init;
	Game_ObtainAI = TestGame_Game_ObtainAI;
	Game_UpdateGlobals = TestGame_Game_UpdateGlobals;
	
	Game_Init();

	Err_DecTab();
	Err_Printf("=");
	
}

void CLWAIMgr::CloseAI()
{
#if 0
	if(m_hDllFile)
	{
		FreeLibrary(m_hDllFile);
		m_hDllFile=LG_NULL;
	}
#endif
	
	Game_ObtainAI=DEF_Game_ObtainAI;
}

void CLWAIMgr::SetGlobals(const GAME_AI_GLOBALS* pGlobals)
{
	Game_UpdateGlobals(pGlobals);
}

CLWAIBase* CLWAIMgr::GetAI(lg_cstr szName)
{
	CLWAIBase* pOut=Game_ObtainAI(szName);
	if(!pOut)
		pOut=&s_DefaultAI;
		
	return pOut;
}


CLWAIDefault CLWAIMgr::s_DefaultAI;

void GAME_FUNC CLWAIMgr::DEF_Game_Init()
{

}

void GAME_FUNC CLWAIMgr::DEF_Game_UpdateGlobals(const GAME_AI_GLOBALS* pIn)
{
	
}

CLWAIBase* GAME_FUNC CLWAIMgr::DEF_Game_ObtainAI(lg_cstr szName)
{
	return &s_DefaultAI;
}

/***************************
*** The Default AI class ***
***************************/

void CLWAIDefault::Init(LEntitySrv *pEnt)
{
}

void CLWAIDefault::PrePhys(LEntitySrv *pEnt)
{
}

void CLWAIDefault::PostPhys(LEntitySrv *pEnt)
{
}

lg_dword CLWAIDefault::PhysFlags()
{
	return 0;
}