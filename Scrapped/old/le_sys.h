// le_sys.h - The Legacy Entity class
//Copyright (c) 2007 Blaine Myers
#ifndef __LE_SYS_H__
#define __LE_SYS_H__

#include "Newton/Newton.h"
#include "ML_lib.h"
#include "common.h"
#include "lw_map.h"
#include "lt_sys.h"
#include "lp_sys.h"
#include "li_sys.h"
#include "lp_physx.h"
#include "lp_newton.h"

//This is the maximum number of rooms that an 
//entity can occupy at one time.
#define LW_MAX_O_REGIONS 8


typedef enum _ENTITY_MOVEMENT{
	MOVEMENT_WALKING=0,
	MOVEMENT_FALLING,
	MOVEMENT_FLYING,
	MOVEMENT_SWIMMING,
	MOVEMENT_UNKNOWN,
}ENTITY_MOVEMENT;

class CLEntity: public CElementInput, public CElementTimer
{
friend class CLCamera;
friend class CLEntity;
friend class CLWorld;

protected:
	static lg_float s_fGrav;
//Static reusabel and global variables:
protected:
	//Temp reusable vars.
	static ML_VEC3 s_v3Temp;
	static ML_MAT  s_matTemp;
	static ML_MAT  s_matTemp2;
protected:	
	static CLWorldMap* s_pWorldMap;

protected:
	class CLEntity* m_pPrev;
	class CLEntity* m_pNext;
	//The position of the entity.
	ML_VEC3 m_v3Pos;
	//The velocity of the entity. (m/s).
	ML_VEC3 m_v3PhysVel;
	//The rotation of the entity (rad/s).
	ML_VEC3 m_v3PhysRot;
	//The orientation of the entity.
	float m_fYaw, m_fPitch, m_fRoll;
	
	float m_fVelFwd;
	float m_fVelStrafe;
	
	ML_MAT m_matOrient;
	
	ML_AABB m_aabbBase;     //Untransformed AABB
	ML_AABB m_aabbCurrent;  //Transformed AABB
	
	//The legacy physics body (unused)...
	CLPhysBody* m_pPhysBody;
	
	lg_float m_fMass;
	
	lg_bool m_bIntelligent;
	
	lg_dword m_nNumRegions;                //The number of rooms that the entity is in.
	lg_dword m_nRegions[LW_MAX_O_REGIONS]; //The rooms that the entity is in
	
	static const lg_dword LENTITY_RENDER180=0x00000001;
	static const lg_dword LENTITY_TRANSYAWONLY=0x00000002;
	static const lg_dword LENTITY_NOWORLDCLIP=0x00000004;
	static const lg_dword LENTITY_NOENTITYCOLLIDE=0x00000008;
	
	ENTITY_MOVEMENT m_nMovementType;
	lg_dword m_nFlags;
	
public:
	CLEntity();
	~CLEntity();
	void Update();
	virtual void Render();
	virtual void InitPhys(lg_bool bIntelligent);
	virtual void ShutdownPhys();
	virtual void ProcessFrame();
	virtual void ProcessAI();
	void CalculateVelXZ();
	
	class CLEntity* GetNext();
	class CLEntity* GetPrev();
	void SetNext(class CLEntity* pEnt);
	void SetPrev(class CLEntity* pEnt);
};

class CLPhysXEntity: public CLEntity, public CElementPhysX
{
private:
	NxActor* m_pNxActor;
public:
	CLPhysXEntity();
	virtual void InitPhys(lg_bool bIntelligent);
	virtual void ShutdownPhys();
	virtual void ProcessFrame();
};

class CLNewtonEntity: public CLEntity, public CElementNewton
{
private:
	//The newton body...
	NewtonBody*  m_pNewtonBody;
	NewtonJoint* m_pUpVec;
public:
	CLNewtonEntity();
	virtual void InitPhys(lg_bool bIntelligent);
	virtual void ShutdownPhys();
	virtual void ProcessFrame();
};

#endif __LE_SYS_H__