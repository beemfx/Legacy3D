#ifndef __LP_NEWTON_H__
#define __LP_NEWTON_H__
#include "Newton/Newton.h"
#include "lg_types.h"
#include "lw_map.h"

class CElementNewton
{
private:
	//The Newton Game Dynamics physics model for the world...
	static NewtonBody* s_pWorldBody;
public:
	static NewtonWorld* s_pNewtonWorld;
	static void Initialize();
	static void Shutdown();
	static void Simulate(lg_float fSeconds);
	static void SetupWorld(CLWorldMap* pMap);
	
	static lg_int MTR_DEFAULT;
	static lg_int MTR_LEVEL;
	static lg_int MTR_CHARACTER;
	static lg_int MTR_OBJECT;
};

#endif  __LP_NEWTON_H__