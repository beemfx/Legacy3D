#ifndef __LW_ENT_SDK_H__
#define __LW_ENT_SDK_H__


#include "lw_ai_sdk.h"
#include "ML_lib.h"
#ifdef __LEGACY_GAME__
#include "lg_list_stack.h"
#endif __LEGACY_GAME__

//Definitions
//MAX_O_REGIONS is the number of regions that
//an entity can occupy at any given time.
//Maps should be designed so that no more
//than two regions are occupied at a time,
//but since it is possible that more regions
//can be occupied we set a higher limit.
#define MAX_O_REGIONS 8

struct LEntitySrv
#ifdef __LEGACY_GAME__
: public CLListStack::LSItem{
#else !__LEGACY_GAME__
{
	lg_dword  m_nLSItemSpace[4]; //Filler space
#endif __LEGACY_GAME__
	//Physical properties of the entity:
	ML_MAT   m_matPos;   //Position/orientation matrix (position: meters. rotation: radians.).
	ML_VEC3  m_v3Vel;    //Velocity (meters/sec).
	ML_VEC3  m_v3AngVel; //Angular velocity (radians/sec).
	ML_VEC3  m_v3Torque; //Torque vector (N m) (Technically meters is ignored by the engine and a default of 1 m is assumed).
	ML_VEC3  m_v3Thrust; //Thrust force of object (Newtons) (Self propelled movement).
	ML_VEC3  m_v3Impulse;//Impulses applied to the object (self propelled (jumping), or from being shot by a bullet for example).
	ML_VEC3  m_v3ExtFrc; //External forces acting on object (Newtons).
	lg_float m_fMass;    //Mass (Kilograms).
	ML_AABB  m_aabbBody; //Square space occupied (volume: meters^3).
	lg_dword m_nPhysFlags; //Flags relating to the physical body (shape, etc).
	
	//Some additional information
	//The following vector arrays are for At, Up, and Right vectors (non translated).
	ML_VEC3  m_v3Face[3]; //3 vectors describing where the entity is facing (obtained from m_matPos).
	ML_VEC3  m_v3Look[3]; //3 vectors describing what the entity is looking at.
	//The look is described by yaw pitch and roll
	lg_float m_fLook[3];
	
	//Animation information
	lg_dword m_nAnimFlags1;
	lg_dword m_nAnimFlags2;
	
	//Information about the entity relative to the world:
	lg_dword m_nNumRegions; //The number of regions the entity is occupying.
	lg_dword m_nRegions[MAX_O_REGIONS]; //References to the regions the entity is occupying.
	
	//The physics engine body of the entity:
	lg_void* m_pPhysBody; //Physics engine body.
	
	//Additional information for entity:
	//Flags 1:
	//BIT 0: 0 inert, 1 intelligent.
	lg_dword m_nFlags1;     //32 slots for misc flags.
	lg_dword m_nFlags2;     //32 more slots for misc flags.
	lg_dword m_nMode1;      //32 bits for information about the mode the entity is in.
	lg_dword m_nMode2;      //32 more bits (a mode might be: walking, running, jumping, ducking, etc).
	lg_dword m_nUEID;       //The unique ID of this entity (accross all clients).
	lg_byte  m_pData[1024]; //1 Kilobyte for all additional information.
	
	lg_dword m_nCmdsActive;
	lg_dword m_nCmdsPressed;
	lg_float m_fAxis[2];
	//The AI functionality:
	CLWAIBase* m_pAI;
	
	//Entity function stuff
	lg_dword m_nFuncs;
	
	//The object script
	lg_path m_szObjScript;
	
	//Ent Flags:
	static const lg_dword EF_1_INERT=0x00000000;
	static const lg_dword EF_1_INT  =0x00000001;
	static const lg_dword EF_1_BLANK=0x00000002;
	
	//Physics Flags:
	static const lg_dword EF_PHYS_SPHERE=  0x00000001;
	static const lg_dword EF_PHYS_CYLINDER=0x00000002;
	static const lg_dword EF_PHYS_BOX=     0x00000004;
	static const lg_dword EF_PHYS_CAPSULE= 0x00000008;
};

#endif __LW_ENT_SDK_H__