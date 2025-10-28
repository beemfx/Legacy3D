//le_base.h - base structure for entities.
#ifndef __LW_ENTITY_H__
#define __LW_ENTITY_H__

#include "lw_ai.h"
#include "lg_types.h"
#include "ML_lib.h"
#include "lg_list_stack.h"

#include "lw_ent_sdk.h"

struct LEntityClnt: public CLListStack::LSItem{
	//Physical properties of the entity: (Position is gotten by server, AngVel and Thrust are sent to server)
	ML_MAT  m_matPos;   //Position of the entitiy.
	ML_VEC3 m_v3Vel;    //Velcoity (m/s).
	ML_VEC3 m_v3AngVel; //Angular velocity (radians/sec).
	ML_VEC3 m_v3Thrust; //Thrust force of object (Newtons) (Self propelled movement).
	ML_AABB m_aabbBody; //Square space occupied (volume: meters^3).
	
	//Some additional information
	//The following vector arrays are for At, Up, and Right vectors (non translated).
	ML_VEC3  m_v3Look[3]; //3 vectors describing what the entity is looking at.
	//The look is described by yaw pitch and roll
	lg_float m_fLook[3];
	
	//Animation information:
	lg_dword m_nAnimFlags1;
	lg_dword m_nAnimFlags2;
	
	//Additional information for the entity: (Updated by server, except m_nUIED which doesn't change).
	lg_dword m_nFlags1;     //32 slots for misc flags.
	lg_dword m_nFlags2;     //32 more slots for misc flags.
	lg_dword m_nMode1;      //32 bits for information about the mode the entity is in.
	lg_dword m_nMode2;      //32 more bits (a mode might be: walking, running, jumping, ducking, etc).
	lg_dword m_nUEID;  //The unique ID (should match server, and all other clients).
	
	//Information about the entity relative to the world: (might be updated by client)
	lg_dword m_nNumRegions; //The number of regions the entity is occupying.
	lg_dword m_nRegions[MAX_O_REGIONS]; //References to the regions the entity is occupying.
	
	//Raster information (excluding position matrix, see above): (Strictly for client use).
	lg_void* m_pRasterInfo; //Information used to rasterize entity.
};

typedef CLListStack LEntList;

typedef LEntitySrv  lg_srv_ent;
typedef LEntityClnt lg_clnt_ent;

#endif __LW_ENTITY_H__