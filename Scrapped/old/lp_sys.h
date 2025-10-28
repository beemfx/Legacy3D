//Legacy Physics (unusued)
#ifndef __LP_SYS_H__
#define __LP_SYS_H__

#include "lw_map.h"
#include "ML_lib.h"
#include "lg_types.h"

typedef enum _TRACE_RESULT{
	TRACE_INAIR=0,
	TRACE_HITGROUND,
	TRACE_ONGROUND,
	TRACE_HITCEILING,
}TRACE_RESULT;

class CLPhysBody
{
friend class CLPhysics;
private:
	ML_AABB m_aabbBase;     //Untransformed AABB
	ML_AABB m_aabbCurrent;  //Transformed AABB
	ML_AABB m_aabbCat;      //AABB which combines current aabb with proposed next aabb
	ML_AABB m_aabbNext;     //The next aabb
	
	ML_VEC3 m_v3Pos;
	ML_VEC3 m_v3PhysVel;
	ML_VEC3 m_v3ScaleVel;
	
	lg_float m_fMass;
	
	lg_dword m_nRegions[8];
	lg_dword m_nNumRegions;
	
	CLPhysBody* m_pNext;
	void CalculateAABBs();
	void WorldClip(CLWorldMap* pMap, lg_bool bSlide);
	void RegionClip(LW_REGION_EX* pRegion, lg_bool bSlide);
	lg_dword BlockClip(LW_GEO_BLOCK_EX* pBlock, lg_bool bSlide);
	lg_dword BlockCollision(
		LW_GEO_BLOCK_EX* pBlock, 
		const ML_VEC3* pVel, 
		lg_float* pColTime, 
		ML_PLANE* pHitPlane,
		lg_bool bAABBCheck=LG_FALSE);
	void ClipVel(ML_VEC3* pOut, ML_VEC3* pIn, ML_PLANE* pPlane, lg_float overbounce);

	
	static ML_VEC3 s_v3WishVel;
	static lg_dword s_nCollisionCount;
	static lg_bool s_bWasInBlock;
public:
	void GetPosition(ML_VEC3* v3Pos);
	void SetVelocity(ML_VEC3* v3Vel);
	void GetVelocity(ML_VEC3* v3Vel);
	void GetAABB(ML_AABB* aabb);
	void Process(lg_float fTime, CLWorldMap* pMap);
	void Update();
	void Collision(CLPhysBody* pBody, float fTime);
	
	//TRACE_RESULT GroundTrace(LW_REGION_EX* pRegion, ML_PLANE* pGround);
};

class CLPhysics
{
private:
	CLPhysBody* m_pFirstBody;
	CLWorldMap* m_pMap;
public:
	CLPhysics();
	~CLPhysics();
	CLPhysBody* AddBody(ML_VEC3* v3Pos, ML_AABB* aabb, lg_float fMass);
	void SetWorld(CLWorldMap* pMap);
	void Processes(lg_dword nTimeStep);
	void RemoveBody(CLPhysBody* pBody);
	void RemoveAllBodies();
};

#endif  __LP_SYS_H__