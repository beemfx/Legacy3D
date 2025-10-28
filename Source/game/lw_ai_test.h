#pragma once
#include "lw_ai_sdk.h"

class CLWAIJack: public CLWAIBase
{
struct AIJACK_DATA{
	lg_dword nMode;
};
public:
	void Init(LEntitySrv* pEnt);
	void PrePhys(LEntitySrv* pEnt);
	void PostPhys(LEntitySrv* pEnt);
	lg_dword PhysFlags();
};

class CLWAIBlaine: public CLWAIBase
{
struct AIBLAINE_DATA{
	lg_float fElapsedTime;
	lg_bool  bForward;
	lg_float fCurThrust;
	lg_float fRotSpeed;
	lg_dword nTicks;
};
public:
	void Init(LEntitySrv* pEnt);
	void PrePhys(LEntitySrv* pEnt);
	void PostPhys(LEntitySrv* pEnt);
	lg_dword PhysFlags();
};