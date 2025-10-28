//le_sys.cpp - The Legacy Entity class
//Copyright (c) 2007 Blaine Myers

#include "le_sys.h"
#include "lg_err.h"
#include "lw_sys.h"

//Reusable var temp vars.
ML_VEC3 CLEntity::s_v3Temp;
ML_MAT  CLEntity::s_matTemp;
ML_MAT  CLEntity::s_matTemp2;
	
//The static pointers can be used by all
//entities and need to be established before
//any entities are created.
CLWorldMap* CLEntity::s_pWorldMap=LG_NULL;
lg_float CLEntity::s_fGrav=-20.0f;


CLEntity::CLEntity():
	m_fYaw(0.0f),
	m_fPitch(0.0f),
	m_fRoll(0.0f),
	m_fMass(1.0f),
	m_nFlags(0),
	m_fVelFwd(0.0f),
	m_fVelStrafe(0.0f),
	m_nMovementType(MOVEMENT_UNKNOWN),
	m_nNumRegions(0),
	m_bIntelligent(LG_FALSE)
{
	m_v3Pos.x=m_v3Pos.y=m_v3Pos.z=0.0f;
	m_v3PhysVel.x=m_v3PhysVel.y=m_v3PhysVel.z=0.0f;
	m_v3PhysRot.x=m_v3PhysRot.y=m_v3PhysRot.z=0.0f;
}

CLEntity::~CLEntity()
{

}

void CLEntity::InitPhys(lg_bool bIntelligent)
{
	m_pPhysBody=CLWorld::s_LegacyPhysics.AddBody(&m_v3Pos, &m_aabbBase, m_fMass);
}

void CLEntity::ShutdownPhys()
{

}


void CLEntity::ProcessFrame()
{
	#if 1
	m_pPhysBody->GetVelocity(&m_v3PhysVel);
	ProcessAI();
	m_v3PhysVel.y+=s_fGrav*s_Timer.GetFElapsed();
	//Process friction.
	lg_float fFriction=9.8f*m_fMass*0.02f*s_Timer.GetFElapsed();
	
	m_v3PhysVel.x-=m_v3PhysVel.x*fFriction;
	m_v3PhysVel.z-=m_v3PhysVel.z*fFriction;
	
	m_pPhysBody->SetVelocity(&m_v3PhysVel);
	m_pPhysBody->GetPosition(&m_v3Pos);
	m_pPhysBody->GetAABB(&m_aabbCurrent);
	s_pWorldMap->CheckRegions(&m_aabbCurrent, m_nRegions, LW_MAX_O_REGIONS);
	m_fYaw+=m_v3PhysRot.y/50.0f;

	ML_MatRotationYawPitchRoll(
		&m_matOrient, 
		m_fYaw, 
		L_CHECK_FLAG(LENTITY_TRANSYAWONLY, m_nFlags)?0.0f:m_fPitch, 
		L_CHECK_FLAG(LENTITY_TRANSYAWONLY, m_nFlags)?0.0f:m_fRoll);
		
	m_matOrient._41=m_v3Pos.x;
	m_matOrient._42=m_v3Pos.y;
	m_matOrient._43=m_v3Pos.z;
	
	m_nNumRegions=s_pWorldMap->CheckRegions(&m_aabbCurrent, m_nRegions, LW_MAX_O_REGIONS);
	#endif
}

void CLEntity::CalculateVelXZ()
{
	//In a FPS based game movement will only be in two dimensions, except
	//for jumping and falling, but those velocities will be managed
	//by gravity and not by the object's movement.
	//The general calculation for velocity is x=vel*sin(yaw)+strafevel*sin(yaw+90deg (pi/2))
	//                                        z=vel*cos(yaw)+strafevel*cos(yaw+90deg (pi/2))
	
	
	m_v3PhysVel.x=m_fVelFwd*ML_sinf(m_fYaw)+m_fVelStrafe*ML_sinf(m_fYaw+ML_PI*0.5f);
	m_v3PhysVel.z=m_fVelFwd*ML_cosf(m_fYaw)+m_fVelStrafe*ML_cosf(m_fYaw+ML_PI*0.5f);
}

void CLEntity::ProcessAI()
{
	m_v3PhysVel.x=m_v3PhysVel.y=m_v3PhysVel.z=0.0f;
}

class CLEntity* CLEntity::GetNext()
{
	return m_pNext;
}
class CLEntity* CLEntity::GetPrev()
{
	return m_pPrev;
}
void CLEntity::SetNext(class CLEntity* pEnt)
{
	m_pNext=pEnt;
}
void CLEntity::SetPrev(class CLEntity* pEnt)
{
	m_pPrev=pEnt;
}

void CLEntity::Update()
{
	//Update should only be called after all collisions with
	//the world and other entities have been adjusted for.
	
	//Add the velocity onto the current posisiton.
	//ML_Vec3Add(&m_v3Pos, &m_v3Pos, &m_v3Vel);
	
	//Update the current AABB (really this is only for rendering)
	//purposes
	//CalculateAABBs(CALC_AABB_CURRENTONLY);
	
	//Setup the transformation matrix with the objects rotation.
	ML_MatRotationYawPitchRoll(
		&m_matOrient, 
		L_CHECK_FLAG(LENTITY_RENDER180, m_nFlags)?m_fYaw+ML_PI:m_fYaw, 
		L_CHECK_FLAG(LENTITY_TRANSYAWONLY, m_nFlags)?0.0f:m_fPitch, 
		L_CHECK_FLAG(LENTITY_TRANSYAWONLY, m_nFlags)?0.0f:m_fRoll);
	//Put the entities position in the transformation matrix.
	m_matOrient._41=m_v3Pos.x;
	m_matOrient._42=m_v3Pos.y;
	m_matOrient._43=m_v3Pos.z;
}

void CLEntity::Render()
{

}

/***************************
	PhysX entity controller
***************************/

CLPhysXEntity::CLPhysXEntity():
	CLEntity(),
	m_pNxActor(LG_NULL)
{
}

void CLPhysXEntity::InitPhys(lg_bool bIntelligent)
{
	m_bIntelligent=bIntelligent;

	m_v3PhysVel.x=m_v3PhysVel.y=m_v3PhysVel.z=0.0f;
	m_v3PhysRot.x=m_v3PhysRot.y=m_v3PhysRot.z=0.0f;

	//Create the PhysX body.
	// Add a single-shape actor to the scene
	NxActorDesc actorDesc;
	NxBodyDesc bodyDesc;
	NxBoxShapeDesc boxDesc;
	
	boxDesc.dimensions.set(
		(m_aabbBase.v3Max.x-m_aabbBase.v3Min.x)/2.0f,
		(m_aabbBase.v3Max.y-m_aabbBase.v3Min.y)/2.0f,
		(m_aabbBase.v3Max.z-m_aabbBase.v3Min.z)/2.0f);
	
	ML_MAT matPos;	
	ML_MatIdentity(&matPos);
	//ML_MatRotationZ(&matPos, 0.5f*ML_PI);
	matPos._41=(m_aabbBase.v3Max.x+m_aabbBase.v3Min.x)/2.0f;
	matPos._42=(m_aabbBase.v3Max.y+m_aabbBase.v3Min.y)/2.0f;
	matPos._43=0.0f;//(m_aabbBase.v3Max.z+m_aabbBase.v3Min.z)/2.0f;
	boxDesc.localPose.setColumnMajor44((NxF32*)&matPos);
		
	actorDesc.shapes.pushBack(&boxDesc);

	bodyDesc.setToDefault();
	actorDesc.body = &bodyDesc;
	actorDesc.density = m_fMass;
	actorDesc.globalPose.t = NxVec3(m_v3Pos.x, m_v3Pos.y, m_v3Pos.z);
	m_pNxActor=s_pNxScene->createActor(actorDesc);
	
	m_v3PhysVel.x=m_v3PhysVel.y=m_v3PhysVel.z=0.0f;
	m_v3PhysRot.x=m_v3PhysRot.y=m_v3PhysRot.z=0.0f;
}

void CLPhysXEntity::ShutdownPhys()
{
	
}

void CLPhysXEntity::ProcessFrame()
{
	if(m_bIntelligent)
	{
		NxVec3 v3Vel=m_pNxActor->getLinearVelocity();
		m_v3PhysVel.x=v3Vel.x;
		m_v3PhysVel.y=v3Vel.y;
		m_v3PhysVel.z=v3Vel.z;
		v3Vel=m_pNxActor->getAngularVelocity();
		m_v3PhysRot.x=v3Vel.x;
		m_v3PhysRot.y=v3Vel.y;
		m_v3PhysRot.z=v3Vel.z;
		
		
		ProcessAI();
	}
	m_pNxActor->getGlobalPose().getColumnMajor44((NxF32*)&m_matOrient);
	m_v3Pos.x=m_matOrient._41;
	m_v3Pos.y=m_matOrient._42;
	m_v3Pos.z=m_matOrient._43;
	
	NxVec3 force(m_v3PhysVel.x, m_v3PhysVel.y, m_v3PhysVel.z);
	m_pNxActor->setLinearVelocity((NxVec3)force);
	force.set(m_v3PhysRot.x, m_v3PhysRot.y, m_v3PhysRot.z);
	m_pNxActor->setAngularVelocity(force);
	
	ML_VEC3 v3={0.0f, 0.0f, 1.0f};
	ML_Vec3TransformNormal(&v3, &v3, &m_matOrient);
	m_fYaw=v3.z>0.0f?ML_asinf(v3.x):ML_asinf(-v3.x)+ML_PI;
	
	m_pNxActor->getShapes()[0]->getWorldBounds((NxBounds3&)m_aabbCurrent);
	
	m_nNumRegions=s_pWorldMap->CheckRegions(&m_aabbCurrent, m_nRegions, LW_MAX_O_REGIONS);
}


/***************************
	Newton Game Dynamics entity controller
***************************/

CLNewtonEntity::CLNewtonEntity():
	CLEntity(),
	m_pNewtonBody(LG_NULL)
{
}

void CLNewtonEntity::InitPhys(lg_bool bIntelligent)
{
	m_bIntelligent=bIntelligent;
	//Create the newton body...
	ML_MAT matPos;
	ML_MatIdentity(&matPos);
	ML_MatRotationZ(&matPos, 0.5f*ML_PI);
	matPos._41=(m_aabbBase.v3Max.x+m_aabbBase.v3Min.x)/2.0f;
	matPos._42=(m_aabbBase.v3Max.y+m_aabbBase.v3Min.y)/2.0f;
	matPos._43=0.0f;//(m_aabbBase.v3Max.z+m_aabbBase.v3Min.z)/2.0f;

	NewtonCollision* pCollision=NewtonCreateCylinder(
		s_pNewtonWorld,
		(m_aabbBase.v3Max.x-m_aabbBase.v3Min.x)*0.5f,
		(m_aabbBase.v3Max.y-m_aabbBase.v3Min.y),
		(dFloat*)&matPos);
	/*
	NewtonCollision* pCollision=NewtonCreateBox(
		s_pNewtonWorld,
		m_aabbBase.v3Max.x-m_aabbBase.v3Min.x,
		m_aabbBase.v3Max.y-m_aabbBase.v3Min.y,
		m_aabbBase.v3Max.z-m_aabbBase.v3Min.z,
		(dFloat*)&matPos);
	*/
	m_pNewtonBody=NewtonCreateBody(s_pNewtonWorld, pCollision);
	
	NewtonReleaseCollision(s_pNewtonWorld, pCollision);
	
	
	ML_MatIdentity(&matPos);
	matPos._41=m_v3Pos.x;
	matPos._42=m_v3Pos.y;
	matPos._43=m_v3Pos.z;
	NewtonBodySetMatrix(m_pNewtonBody, (dFloat*)&matPos);
	
	ML_VEC3 v3={
		m_aabbBase.v3Min.x+(m_aabbBase.v3Max.x-m_aabbBase.v3Min.x)/2.0f,
		m_aabbBase.v3Min.y+(m_aabbBase.v3Max.y-m_aabbBase.v3Min.y)/2.0f,
		m_aabbBase.v3Min.z+(m_aabbBase.v3Max.z-m_aabbBase.v3Min.z)/2.0f};
	NewtonBodySetCentreOfMass(m_pNewtonBody, (dFloat*)&v3);
	
	
	m_v3PhysVel.x=m_v3PhysVel.y=m_v3PhysVel.z=0.0f;
	m_v3PhysRot.x=m_v3PhysRot.y=m_v3PhysRot.z=0.0f;
	
	NewtonBodySetMassMatrix(m_pNewtonBody, m_fMass, m_fMass, m_fMass, m_fMass);
	
	
	if(m_bIntelligent)
	{
		NewtonBodySetMaterialGroupID(m_pNewtonBody, MTR_CHARACTER);
		//NewtonBodySetMassMatrix(m_pNewtonBody, m_fMass, 1e30f, 1e30f, 1e30f);
		NewtonBodySetAutoFreeze(m_pNewtonBody, 0);
		NewtonWorldUnfreezeBody(s_pNewtonWorld, m_pNewtonBody);
		// add and up vector constraint to help in keeping the body upright
		ML_VEC3 upDirection ={0.0f, 1.0f, 0.0f};
		m_pUpVec = NewtonConstraintCreateUpVector (s_pNewtonWorld, (dFloat*)&upDirection, m_pNewtonBody); 
	}
	else
	{
		//NewtonBodySetMassMatrix(m_pNewtonBody, m_fMass, m_fMass, m_fMass, m_fMass);
		NewtonBodySetAutoFreeze(m_pNewtonBody, LG_TRUE);
		NewtonWorldFreezeBody(s_pNewtonWorld, m_pNewtonBody);
		NewtonBodySetMaterialGroupID(m_pNewtonBody, MTR_OBJECT);
		m_pUpVec=LG_NULL;
	}
}

void CLNewtonEntity::ShutdownPhys()
{
	if(m_pUpVec)
		NewtonDestroyJoint(s_pNewtonWorld, m_pUpVec);
	if(m_pNewtonBody)
		NewtonDestroyBody(s_pNewtonWorld, m_pNewtonBody);
}

void CLNewtonEntity::ProcessFrame()
{
	NewtonBodyGetVelocity(m_pNewtonBody, (dFloat*)&m_v3PhysVel);
	NewtonBodyGetOmega(m_pNewtonBody, (dFloat*)&m_v3PhysRot);
	
	ProcessAI();
	
	//Apply gravity, this really only needs to be applied if
	//the object is a falling object (which most are), but flying
	//objects should not have gravity applied.
	if(NewtonBodyGetSleepingState(m_pNewtonBody))
		m_v3PhysVel.y+=s_fGrav*s_Timer.GetFElapsed();
	
	//NewtonWorldUnfreezeBody(s_pNewtonWorld, m_pNewtonBody);
	NewtonBodySetVelocity(m_pNewtonBody, (dFloat*)&m_v3PhysVel);
	NewtonBodySetOmega(m_pNewtonBody, (dFloat*)&m_v3PhysRot);
	NewtonBodyGetMatrix(m_pNewtonBody, (dFloat*)&m_matOrient);
	m_v3Pos.x=m_matOrient._41;
	m_v3Pos.y=m_matOrient._42;
	m_v3Pos.z=m_matOrient._43;
	
	NewtonBodyGetAABB(m_pNewtonBody, (dFloat*)&m_aabbCurrent.v3Min, (dFloat*)&m_aabbCurrent.v3Max);
	
	ML_VEC3 v3={0.0f, 0.0f, 1.0f};
	ML_Vec3TransformNormal(&v3, &v3, &m_matOrient);
	m_fYaw=v3.z>0.0f?ML_asinf(v3.x):ML_asinf(-v3.x)+ML_PI;
	
	m_nNumRegions=s_pWorldMap->CheckRegions(&m_aabbCurrent, m_nRegions, LW_MAX_O_REGIONS);
	
}
