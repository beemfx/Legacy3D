/* lp_phys2.h - Physics egine base class.
	Copyright (c) 2007 Blaine Myers
*/
#ifndef __LP_SYS2_H__
#define __LP_SYS2_H__
#include "common.h"
#include "ML_lib.h"

/*
	This is just a list of supposedly available physics engines.
*/
typedef enum _PHYS_ENGINE{
	PHYS_ENGINE_NULL=0,
	PHYS_ENGINE_LEGACY,
	PHYS_ENGINE_NEWTON,
#if 0
	PHYS_ENGINE_PHYSX
#endif
}PHYS_ENGINE;

typedef enum _PHYS_SHAPE{
	PHYS_SHAPE_UNKNOWN=0,
	PHYS_SHAPE_BOX,
	PHYS_SHAPE_CAPSULE,
	PHYS_SHAPE_SPHERE,
	PHYS_SHAPE_CYLINDER
}PHYS_SHAPE;

typedef struct _lp_body_info{
	ML_MAT   m_matPos;
	ML_AABB  m_aabbBody;
	lg_float m_fMass;
	lg_float m_fMassCenter[3];
	lg_float m_fShapeOffset[3];
	PHYS_SHAPE m_nShape;
	//Flags gotten from the AI routine.
	lg_dword m_nAIPhysFlags;
}lp_body_info;

class CLPhys{
//Some Global variables:
protected:
	static const ML_VEC3 s_v3StdFace[3]; //Standard face vectors (see declaration).
	static const ML_VEC3 s_v3Zero;       //Zero vector
	
//Methods that need to be overriddent:
public:
	virtual void Init(lg_dword nMaxBodies)=0;
	virtual void Shutdown()=0;
	virtual lg_void* AddBody(lg_void* pEnt, lp_body_info* pInfo)=0;
	virtual void RemoveBody(lg_void* pBody)=0;
	virtual void SetupWorld(lg_void* pWorldMap)=0;
	virtual void SetGravity(ML_VEC3* pGrav)=0;
	virtual void Simulate(lg_float fTimeStepSec)=0;
	
	virtual void SetBodyFlag(lg_void* pBody, lg_dword nFlagID, lg_dword nValue)=0;
	virtual void SetBodyVector(lg_void* pBody, lg_dword nVecID, ML_VEC3* pVec)=0;
	virtual void SetBodyFloat(lg_void* pBody, lg_dword nFloatID, lg_float fValue)=0;
	virtual void SetBodyPosition(lg_void* pBody, ML_MAT* pMat)=0;
	
	virtual lg_void* GetBodySaveInfo(lg_void* pBody, lg_dword* pSize)=0;
	virtual lg_void* LoadBodySaveInfo(lg_void* pData, lg_dword nSize)=0;
};

#endif  __LP_SYS2_H__