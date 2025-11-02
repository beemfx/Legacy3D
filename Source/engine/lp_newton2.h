#if L3D_WITH_NEWTONGD_PHYSICS

#ifndef __NEWTON2_H__
#define __NEWTON2_H__
#include "lp_sys2.h"
#include "dgNewton/Newton.h"

class CLPhysNewton: CLPhys
{
private:  
	NewtonWorld* m_pNewtonWorld;
	NewtonBody*  m_pWorldBody;
	lg_int MTR_DEFAULT;
	lg_int MTR_LEVEL;
	lg_int MTR_CHARACTER;
	lg_int MTR_OBJECT;
	
	ML_VEC3 m_v3Grav;
	lg_float m_fTimeStep;
public:
	virtual void Init(lg_dword nMaxBodies);
	virtual void Shutdown();
	virtual lg_void* AddBody(lg_void* pEnt, lp_body_info* pInfo);
	virtual void RemoveBody(lg_void* pBody);
	virtual void SetupWorld(lg_void* pWorldMap);
	virtual void SetGravity(ML_VEC3* pGrav);
	virtual void Simulate(lg_float fTimeStepSec);
	virtual void SetBodyFlag(lg_void* pBody, lg_dword nFlagID, lg_dword nValue);
	virtual void SetBodyVector(lg_void* pBody, lg_dword nVecID, ML_VEC3* pVec);
	virtual void SetBodyFloat(lg_void* pBody, lg_dword nFloatID, lg_float fValue);
	virtual void SetBodyPosition(lg_void* pBody, ML_MAT* pMat);
	
	virtual lg_void* GetBodySaveInfo(lg_void* pBody, lg_dword* pSize);
	virtual lg_void* LoadBodySaveInfo(lg_void* pData, lg_dword nSize);
	
private:
	static void UpdateFromSrv(const NewtonBody* const body, dFloat timestep, int threadIndex);
	static void UpdateToSrv(const NewtonBody* const body, const dFloat* const matrix, int threadIndex);
	/*typedef void (*NewtonBodyIterator) (const NewtonBody* body);*/
	
	static CLPhysNewton* s_pPhys;
};

#endif __NEWTON2_H__

#endif
