#include "le_test.h"
#include "lg_err.h"
#include "lg_sys.h"

#include "ld_sys.h"
#include "lg_func.h"

#include <stdio.h>

void CLJack::Initialize(ML_VEC3* v3Pos)
{
	CLBase3DEntity::Initialize(v3Pos);
	m_fMass=65.0f;
	m_pWalk=LG_LoadSkel("/dbase/skels/mb/mb_walk.lskl", 0);
	m_pStand=LG_LoadSkel("/dbase/skels/mb/mb_stand1.lskl", 0);
	m_pJump=LG_LoadSkel("/dbase/skels/mb/mb_jump.lskl", 0);
	
	LoadMesh("/dbase/meshes/jack/jack_lantern.xml", 0.023f, LOADMESH_BB45);
	
	if(m_pMeshNodes)
	{
		m_pMeshNodes[0].SetAnimation(m_pStand, 0, 2000, 0);	
	}

	InitPhys(LG_TRUE);
}

void CLJack::ProcessAI()
{
	//For this entity velocity represents the amount of units the entity
	//will move in one second.  (Since max payne entities are small
	//the velocity is small.)
	const float DEFAULT_VEL=2.0f;
	const float DEFAULT_ROT=ML_PI;
	
	m_fVelFwd=0.0f;
	m_fVelStrafe=0.0f;
	
	lg_bool bJump=LG_FALSE;
	
	if(s_pCmds[COMMAND_MOVEFORWARD].IsActive())
		m_fVelFwd=DEFAULT_VEL;
	if(s_pCmds[COMMAND_MOVELEFT].IsActive())
		m_fVelStrafe=-DEFAULT_VEL;
	if(s_pCmds[COMMAND_MOVERIGHT].IsActive())
		m_fVelStrafe=DEFAULT_VEL;
	if(s_pCmds[COMMAND_MOVEBACK].IsActive())
		m_fVelFwd=-DEFAULT_VEL;
	if(s_pCmds[COMMAND_TURNRIGHT].IsActive())
		m_fYaw+=DEFAULT_ROT*s_Timer.GetFElapsed();
	if(s_pCmds[COMMAND_TURNLEFT].IsActive())
		m_fYaw-=DEFAULT_ROT*s_Timer.GetFElapsed();
	if(s_pCmds[COMMAND_MOVEUP].Activated())
		bJump=LG_TRUE;
			
	if(s_pCmds[COMMAND_SPEED].IsActive())
	{
		m_fVelFwd*=2.0f;
		m_fVelStrafe*=2.0f;
	}
	
	m_fPitch+=s_pAxis->nY/100.0f;
	m_fPitch=LG_Clamp(m_fPitch, -(ML_HALFPI-0.001f), (ML_HALFPI-0.001f));
	
	m_v3PhysRot.y=s_pAxis->nX/2.5f;
	
	m_v3PhysVel.x+=m_fVelFwd*ML_sinf(m_fYaw)+m_fVelStrafe*ML_sinf(m_fYaw+ML_PI*0.5f);
	m_v3PhysVel.z+=m_fVelFwd*ML_cosf(m_fYaw)+m_fVelStrafe*ML_cosf(m_fYaw+ML_PI*0.5f);
	
	if(bJump)// && m_nMovementType==MOVEMENT_WALKING)
	{
		m_v3PhysVel.y=7.0f;	
		m_nMovementType=MOVEMENT_FALLING;
	}
	
	//Update the mesh times...
	if(!m_pMeshNodes)
		return;
		
	m_pMeshNodes[0].UpdateTime(s_Timer.GetTime());
	
	//Set animations as necessary...
	if(0)//m_nMovementType==MOVEMENT_FALLING)
		m_pMeshNodes[0].SetAnimation(m_pJump, 0, 1000, 1);
	else if((m_fVelFwd!=0.0f || m_fVelStrafe!=0.0f))
		m_pMeshNodes[0].SetAnimation(m_pWalk, 0, (m_fVelFwd<0.0f)?-2000:2000, 250);
	else
		m_pMeshNodes[0].SetAnimation(m_pStand, 0, 2000, 250);
		
	#if 1	
	Err_MsgPrintf(
		"Jack's Position: %s: (%.3f, %.3f, %.3f) Vel: (%f, %f, %f) %s", 
			(m_nNumRegions?s_pWorldMap->m_pRegions[m_nRegions[0]].szName:"NOWHERE"),
			m_v3Pos.x,
			m_v3Pos.y,
			m_v3Pos.z,
			m_v3PhysVel.x,
			m_v3PhysVel.y,
			m_v3PhysVel.z,
			m_nMovementType==MOVEMENT_WALKING?"ON GROUND":"IN AIR");
	#endif
}


void CLBlaineEnt::Initialize(ML_VEC3* v3Pos)
{
	CLBase3DEntity::Initialize(v3Pos);
	m_fMass=55.0f;
	m_pWalk=LG_LoadSkel("/dbase/skels/mb/mb_walk.lskl", 0);
	m_pStand=LG_LoadSkel("/dbase/skels/mb/mb_stand1.lskl", 0);
	LoadMesh("/dbase/meshes/blaine/blaine.xml", 0.023f, LOADMESH_BB45);
	//LoadMesh("/dbase/meshes/jack/jack_lantern.xml", 0.023f, LOADMESH_BB45);
	
	if(m_pMeshNodes)
	{
		m_pMeshNodes[0].SetAnimation(m_pStand, 0, 2000, 0);
	}
	m_nLastAIUpdate=s_Timer.GetTime();
	m_fRotation=ML_PI*0.25f;
	
	//Setup the sounds
	alGetError();
	alGenBuffers(1, &m_SndBuffer);
	alGenSources(1, &m_Snd);
	alSourcei(m_Snd, AL_LOOPING, AL_TRUE);
	CLSndMgr::LS_LoadSoundIntoBuffer(
		m_SndBuffer,
		"/dbase/sounds/speech/MyNameIsBlaine.ogg");
	alSourcei(m_Snd, AL_BUFFER, m_SndBuffer);
	alSourcePlay(m_Snd);
	//alSourcef(m_Snd, AL_MAX_DISTANCE, 10.0f);
	//alSourcef(m_Snd, AL_REFERENCE_DISTANCE, 5.0f);
	alSourcef(m_Snd, AL_ROLLOFF_FACTOR, 1.2f);
	
	InitPhys(LG_TRUE);
	
}

void CLBlaineEnt::ProcessAI()
{
	//For this entity velocity represents the amount of units the entity
	//will move in one second.
	const float DEFAULT_VEL=2.5f;
	const float DEFAULT_ROT=ML_PI;
	
	m_fVelFwd=0.0f;
	m_fVelStrafe=0.0f;

	m_v3PhysRot.y=m_fRotation;
	//m_fYaw+=m_fRotation*s_Timer.GetFElapsed();
	m_fVelFwd=DEFAULT_VEL*0.5f;
	
	if((s_Timer.GetTime()-m_nLastAIUpdate)>5000)
	{
		m_fRotation=LG_RandomFloat(-ML_HALFPI, ML_HALFPI);
		//m_fRotation=-m_fRotation;
		m_nLastAIUpdate=s_Timer.GetTime();
	}
	
	
	m_v3PhysVel.x+=m_fVelFwd*ML_sinf(m_fYaw)+m_fVelStrafe*ML_sinf(m_fYaw+ML_PI*0.5f);
	m_v3PhysVel.z+=m_fVelFwd*ML_cosf(m_fYaw)+m_fVelStrafe*ML_cosf(m_fYaw+ML_PI*0.5f);
	
	
	//Update the meshes...
	if(m_pMeshNodes)
	{
		m_pMeshNodes[0].UpdateTime(s_Timer.GetTime());
		
		if((m_fVelFwd!=0.0f || m_fVelStrafe!=0.0f))
			m_pMeshNodes[0].SetAnimation(m_pWalk, 0, (m_fVelFwd<0.0f)?-2000:2000, 250);
		else
			m_pMeshNodes[0].SetAnimation(m_pStand, 0, 2000, 250);
	}
	
	//alSourcefv(m_Snd, AL_POSITION, (ALfloat*)&m_v3Pos);
	alSource3f(m_Snd, AL_POSITION, m_v3Pos.x, m_v3Pos.y+1.5f, -m_v3Pos.z);
	alSource3f(m_Snd, AL_VELOCITY, m_v3PhysVel.x, m_v3PhysVel.y, -m_v3PhysVel.z);
}

#if 0
void CLMonaEnt::Initialize(ML_VEC3* v3Pos)
{
	CLBase3DEntity::Initialize(v3Pos);
	m_pSkel=LG_LoadSkel("/dbase/meshes/max payne/p2_walk_stand.lskl", 0);
	LoadMesh("/dbase/meshes/max payne/hooker.xml", 1.0f, LOADMESH_BB45);
	
	if(m_pMeshNodes)
	{
		m_pMeshNodes[0].SetAnimation(m_pSkel, 0, 2000, 0);
	}
}

void CLMonaEnt::ProcessAI()
{
	//For this entity velocity represents the amount of units the entity
	//will move in one second.
	const float DEFAULT_VEL=2.5f;
	const float DEFAULT_ROT=ML_PI;
	
	m_fVelFwd=0.0f;
	m_fVelStrafe=0.0f;

	m_fYaw+=(DEFAULT_ROT/4.0f)*s_Timer.GetFElapsed();
	m_fVelFwd=DEFAULT_VEL*0.5f;
			
	while(m_fYaw>ML_2PI)
	{
		m_fYaw-=ML_2PI;
	}
	
	while(m_fYaw<0.0f)
	{
		m_fYaw+=ML_2PI;
	}
	
	
	CalculateVelXZ();
	
	//Update the meshes...
	if(m_pMeshNodes)
	{
		m_pMeshNodes[0].UpdateTime(s_Timer.GetTime());
		
		if((m_fVelFwd!=0.0f || m_fVelStrafe!=0.0f))
			m_pMeshNodes[0].SetAnimation(m_pSkel, 1, (m_fVelFwd<0.0f)?-2000:2000, 250);
		else
			m_pMeshNodes[0].SetAnimation(m_pSkel, 0, 2000, 250);
	}
}
#endif

void CLBarrelEnt::Initialize(ML_VEC3* v3Pos)
{
	CLBase3DEntity::Initialize(v3Pos);
	m_fMass=100.0f;
	LoadMesh("/dbase/meshes/objects/barrel.xml", 0.13f, 0);
	InitPhys(LG_FALSE);
	/*
	::NewtonBodySetMaterialGroupID(m_pNewtonBody, MTR_OBJECT);
	ML_VEC3 v3={
		m_aabbBase.v3Min.x+(m_aabbBase.v3Max.x-m_aabbBase.v3Min.x)/2,
		m_aabbBase.v3Min.y+(m_aabbBase.v3Max.y-m_aabbBase.v3Min.y)/2,
		m_aabbBase.v3Min.z+(m_aabbBase.v3Max.z-m_aabbBase.v3Min.z)/2};
	NewtonBodySetCentreOfMass(m_pNewtonBody, (dFloat*)&v3);
	*/
}

void CLBarrelEnt::ProcessAI()
{

}

