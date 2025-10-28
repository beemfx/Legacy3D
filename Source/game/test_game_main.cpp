
#include <string.h>
#include <ML_lib.h>

#include "lw_ai_test.h"
#include "lw_ai_sdk.h"

GAME_AI_GLOBALS g_Globals;

CLWAIJack   AI_Jack;
CLWAIBlaine AI_Blaine;

GAME_EXPORT void GAME_FUNC TestGame_Game_Init()
{
	ML_Init(ML_INSTR_F);
	memset(&g_Globals, 0, sizeof(g_Globals));
}

GAME_EXPORT void GAME_FUNC TestGame_Game_UpdateGlobals(const GAME_AI_GLOBALS* pIn)
{
	g_Globals=*pIn;
}

GAME_EXPORT CLWAIBase*  GAME_FUNC TestGame_Game_ObtainAI(lg_cstr szName)
{
	CLWAIBase* m_pOut=LG_NULL;
	
	if(_stricmp(szName, "jack_basic_ai")==0)
		m_pOut=&AI_Jack;
	else if(_stricmp(szName, "blaine_basic_ai")==0)
		m_pOut=&AI_Blaine;
		
	return m_pOut;
}