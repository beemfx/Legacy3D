/* lp_legacy.h - The basic Legacy Physics engine, not as
	robust as the Newton or PhysX physics engines, but a
	demonstration of the development process.
	
	Copyright (c) 2007 Blaine Myers
*/
#ifndef __LP_LEGACY_H__
#define __LP_LEGACY_H__
#include "lp_sys2.h"
#include "lg_list.h"
#include "lg_stack.h"
#include "lg_list_stack.h"

//#define FORCE_METHOD

class CLPhysLegacy: CLPhys
{
//Constants:
private:
	static const lg_dword LP_MAX_O_REGIONS=8;
private:	
	LG_CACHE_ALIGN class CLPhysBody: public CLListStack::LSItem
	{
	public:
		ML_MAT   m_matPos; //Position of body.
		
		ML_VEC3  m_v3Thrust;
		ML_VEC3  m_v3Vel;
		
		ML_VEC3  m_v3Torque;
		ML_VEC3  m_v3AngVel;
		lg_float m_fMass;  //Mass in KG.
		ML_VEC3  m_v3Center;
		
		ML_AABB  m_aabbBase;    //The volume of the object in meters.
		ML_AABB  m_aabbPreMove; //The current location of the volume.
		ML_AABB  m_aabbPostMove;  //The future location of the volume.
		ML_AABB  m_aabbMove;    //The volume occupied while the body was moving.
		#ifdef FORCE_METHOD
		ML_VEC3  m_v3ClsnForces; //Forces that were applied during collsion detection:
		#endif
		//Regions that the body is occupying:
		lg_dword m_nNumRegions;
		lg_dword m_nRegions[LP_MAX_O_REGIONS];
		
		//Additional data:
		lg_float m_fSpeed;    //The linear speed of the object.  (m/s)
		lg_float m_fAngSpeed; //The angular speed of the object. (rad/s)
		
		//Flags:
		//BITS 0-15 (Createion flags, see lp_body_info structure for mor info).
		//BIT 16: Awake Asleep Flag (1) Awake (0) Asleep
		lg_dword m_nFlags;
		
		lg_void* m_pEnt;     //The entity attached to the body.
		
		//A pointer to a body that the entity is colliding with,
		//for inelastic collisions.
		lg_dword m_nColBody;
	public:
		//Constants
		static const lg_dword PHYS_BODY_AWAKE= 0x00010000;
	};
	
	//Class to describe the map geometry:
	LG_CACHE_ALIGN class CLPhysHull
	{
	public:
		lg_dword  m_nFirstFace;
		lg_dword  m_nNumFaces;
		ml_aabb   m_aabbHull;
		ml_plane* m_pFaces;
	};
	
private:
	lg_dword m_nMaxBodies;      //The maximum number of bodies allowed.
	
	CLListStack m_AsleepBodies; //Asleep bodies
	CLListStack m_AwakeBodies;  //Awake bodies
	CLListStack m_UnusedBodies; //Bodies available to be created.
	CLPhysBody* m_pBodyList;    //List of available bodies.
	
	//Global Settings
	ML_VEC3  m_v3Grav;//Gravity force (Newtons).
	lg_float m_fDrag; //Drag coefficient.
	
	//Frame stuff:
	lg_float m_fTimeStep;
	
	//World geometry:
	//In the future it should be divided into regions:
	lg_dword    m_nHullCount;
	ml_plane*   m_pGeoPlaneList;
	CLPhysHull* m_pGeoHulls;
	
	//Some vars that are used temporarily:
	lg_dword m_nClsns;
	
//Internal methods:
private:
	__inline void ProcessAwakeBodies();    
	
	__inline void CollisionCheck();
	__inline void BodyClsnCheck(CLPhysBody* pBody1, CLPhysBody* pBody2);
	__inline void HullClsnCheck(CLPhysBody* pBody, CLPhysHull* pHull);
	
	__inline void ApplyWorldForces(CLPhysBody* pBody);
	
	__inline static lg_dword AABBIntersectHull(const ml_aabb* pAABB, const CLPhysHull* pHull, const ml_vec3* pVel, ml_plane* pIsctPlane);
	
	/*
	__inline void WorldCheck(CLPhysBody* pBody);
	__inline void RegionCheck(CLPhysBody* pBody, lg_dword nRegion);
	__inline void BlockClip(CLPhysBody* pBody, LW_GEO_BLOCK_EX* pBlock);
	*/
	__inline void SetPreMoveAABB(CLPhysBody* pBody);
	__inline void SetPostMoveAABB(CLPhysBody* pBody);
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
	
	__inline void UpdateFromSrv(CLPhysBody* pPhysBody, lg_void* pEntSrv);
	__inline void UpdateToSrv(CLPhysBody* pPhysBody, lg_void* pEntSrv);
};
#endif __LP_LEGACY_H__