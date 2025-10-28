#ifndef __LW_AI_H__
#define __LW_AI_H__

#include "lg_types.h"
#include "lw_ai_sdk.h"
#include <windows.h>

struct LEntitySrv;

#define LWAI_DEC_LIB_FUNC(return_type, name) TYPE_##name##_FUNC name; \
				static return_type GAME_FUNC DEF_##name

class CLWAIDefault: public CLWAIBase{
public:
	virtual void Init(LEntitySrv* pEnt);
	virtual void PrePhys(LEntitySrv* pEnt);
	virtual void PostPhys(LEntitySrv* pEnt);
	virtual lg_dword PhysFlags();
};

class CLWAIMgr
{
private:
	static CLWAIDefault s_DefaultAI;
	//CLWAIBase*   m_pJackAI;
	//CLWAIBase*   m_pBlaineAI;
	
	HMODULE      m_hDllFile;
	
	LWAI_DEC_LIB_FUNC(CLWAIBase*, Game_ObtainAI)(lg_cstr szName);	
	LWAI_DEC_LIB_FUNC(void, Game_Init)();
	LWAI_DEC_LIB_FUNC(void, Game_UpdateGlobals)(const GAME_AI_GLOBALS* pIn);
	
public:
	CLWAIMgr();
	~CLWAIMgr();
	void LoadAI(lg_path szFilename);
	void CloseAI();
	void SetGlobals(const GAME_AI_GLOBALS* pGlobals);
	CLWAIBase* GetAI(lg_cstr szName);
	
};


#define LWAI_OBTAIN_FUNC(name) name=(TYPE_##name##_FUNC)GetProcAddress(m_hDllFile, #name); \
	if(!name){\
		Err_Printf(#name" function could not be loaded, using default."); \
		name=DEF_##name;}


#endif __LW_AI_H__