/*****************************************************
*** lw_ai_sdk.h - Copyright (c) 2008 Beem Software ***
***                                                ***
***                 DO NOT MODIFY                  ***
*** This file is a template for structures and     ***
*** items, that are used within the game, if these ***
*** structures are modified in this file, then     ***
*** they will be incompatible with the game, and   ***
*** it is likely that the software will crash.     ***
*****************************************************/
#ifndef __LW_AI_SDK_H__
#define __LW_AI_SDK_H__

#include "lg_types.h"
struct LEntitySrv;

#define GAME_EXPORT extern "C" __declspec(dllexport)
#define GAME_FUNC   __cdecl


/* AI Functions - These functions can be called within the PrePhys
	methods of an entity, the result (if any) will be avaialable in
	the post phys method.
*/

#define LWAI_QueueFunc(ent, func) LG_SetFlag(ent->m_nFuncs, func)

//Do a ray trace, based on the look vector (the ray trace is carried
//out from the objects center of gravity.  The result is the UEID of
//the entity that was hit (or level geometry, or null if nothing was 
//hit), and the coordinates of the trace.
const lg_dword AI_FUNC_RAY_TRACE_LOOK=0x00000001;

//Spawn entity, spawns an entity, based on the input given (useful for
//missles being fired, etc.
const lg_dword AI_FUNC_SPAWN_ENT=0x00000002;

//Delete's the current entity, useful if the entity was killed, blown
//up, or has expired.
const lg_dword AI_FUNC_DELETE=0x00000004;
const lg_dword AI_FUNC_REMOVE=0x00000004;

//Do a ground trace, which gets the results of what the entity
//is standing on (surface plane, in the air etc.)
const lg_dword AI_FUNC_GROUND_TRACE=0x00000008;



//Here are the physics flags, they are gotten by the physics engine to
//decide certain things about how the body reacts.

//The object is intelligent.
const lg_dword AI_PHYS_INT=0x00000001;

//m_v3Thrust represents and actual 
//movement distance and not a force.
const lg_dword AI_PHYS_DIRECT_LINEAR=0x00000002;

//m_v3Torque represents an actual
//rotation and not a torque.
const lg_dword AI_PHYS_DIRECT_ANGULAR=0x00000004;

class CLWAIBase
{
public:
	virtual void Init(LEntitySrv* pEnt)=0;
	virtual void PrePhys(LEntitySrv* pEnt)=0;
	virtual void PostPhys(LEntitySrv* pEnt)=0;
	virtual lg_dword PhysFlags()=0;
};

typedef struct _GAME_AI_GLOBAL{
	lg_float fTimeStep; //Time step in seconds (since the last update).
}GAME_AI_GLOBALS;

#define LWAI_LIB_FUNC(return_type, name) typedef return_type (GAME_FUNC * TYPE_##name##_FUNC)

LWAI_LIB_FUNC(CLWAIBase*, Game_ObtainAI)(lg_cstr szName);
LWAI_LIB_FUNC(void, Game_Init)();
LWAI_LIB_FUNC(void, Game_UpdateGlobals)(const GAME_AI_GLOBALS* pIn);


/*
Command macros, translates flags into commands:
*/

#define LWAI_CmdActive(ent, cmd) LG_CheckFlag(pEnt->m_nCmdsActive, 1<<cmd)
#define LWAI_CmdPressed(ent, cmd) LG_CheckFlag(pEnt->m_nCmdsPressed, 1<<cmd)


/*
Animation information:  The infomation for an animation
changes is stored within a single 32 bit dword, that way
it can transfer over the network more quickly.

Use the LWAI_SetAnim macro to set the animation:
flag:
	value: Needs to be the entity's animation flag.
node:
   value range: 0 to 15 
   notes: specifies which node the animation will effect.
skel:
   value range: 0 to 15
   notes: specifies which skeleton will be used for the
   animation.
anim:
   value range: 0 to 31
   notes: specifes which animation within the skel will
   be used.
speed:
   value range: 0.0f to 10.0f
   notes: specifies the speed of the animation, the value
   represents how many seconds it takes to complete the
   animation (0 to 10 seconds).
trans:
   value range: 0f to 10.0f
   notes: computed as above, but represents the time it
   takes to transition from the current animation to the
   next.
*/
const lg_dword LWAI_ANIM_CHANGE=0x00000001;

#define LWAI_SetAnim(flag, node, skel, anim, speed, trans) { \
	flag=0x00000001; \
	flag|=(0xF&(node))<<1;\
	flag|=(0xF&(skel))<<5;\
	flag|=(0x1F&(anim))<<9; \
	flag|=(0x1FF&((lg_dword)((speed)*51.1f)))<<14;\
	flag|=(0x1FF&((lg_dword)((trans)*51.1f)))<<23;\
	}
	
//Checks to see if the animation is as specified.
#define LWAI_IsAnim(flag, node, skel, anim) \
	(flag&((node<<1)|(skel<<5)|(anim<<9)))\
	

//These are used internally and probably don't need to be
//called for ai functions.		
#define LWAI_GetAnimNode(flag)   ((0x0000001E&flag)>>1)
#define LWAI_GetAnimSkel(flag)   ((0x000001E0&flag)>>5)
#define LWAI_GetAnimAnim(flag)   ((0x00003E00&flag)>>9)
#define LWAI_GetAnimSpeed(flag) (((0x007FC000&flag)>>14)/51.1f)
#define LWAI_GetAnimTrans(flag) (((0xFF800000&flag)>>23)/51.1f)


#endif __LW_AI_SDK_H__