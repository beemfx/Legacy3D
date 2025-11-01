#include "lp_newton2.h"
#include "lg_err_ex.h"
#include "lg_err.h"
#include "lg_func.h"
#include "lw_entity.h"
#include "lw_map.h"

CLPhysNewton* CLPhysNewton::s_pPhys=LG_NULL;

void CLPhysNewton::Init(lg_dword nMaxBodies)
{
	s_pPhys=this;
	m_fTimeStep=0.0f;
	Err_Printf("Initializing Newton Game Dynamics physics engine...");
	m_pNewtonWorld=NewtonCreate();//(NewtonAllocMemory)LG_Malloc, (NewtonFreeMemory)LG_Free);
	if(!m_pNewtonWorld)
		throw CLError(LG_ERR_DEFAULT, __FILE__, __LINE__, "Newton Initialization: Could not create physics model.");
	Err_Printf("Newton World version %i.", NewtonWorldGetVersion());
	Err_Printf("Setting up basic materials.");
	MTR_DEFAULT=0;
	MTR_DEFAULT=NewtonMaterialGetDefaultGroupID(m_pNewtonWorld);
	
	NewtonMaterialSetDefaultSoftness(m_pNewtonWorld, MTR_DEFAULT, MTR_DEFAULT, 1.0f);
	NewtonMaterialSetDefaultElasticity(m_pNewtonWorld, MTR_DEFAULT, MTR_DEFAULT, 0.0f);
	NewtonMaterialSetDefaultCollidable(m_pNewtonWorld, MTR_DEFAULT, MTR_DEFAULT, 1);
	NewtonMaterialSetDefaultFriction(m_pNewtonWorld, MTR_DEFAULT, MTR_DEFAULT, 0.0f, 0.0f);
	//NewtonMaterialSetCollisionCallback (m_pNewtonWorld, MTR_DEFAULT, MTR_DEFAULT, &wood_wood, GenericContactBegin, GenericContactProcess, GenericContactEnd);  

	MTR_LEVEL=NewtonMaterialCreateGroupID(m_pNewtonWorld);
	MTR_CHARACTER=NewtonMaterialCreateGroupID(m_pNewtonWorld);
	MTR_OBJECT=NewtonMaterialCreateGroupID(m_pNewtonWorld);
	
	NewtonMaterialSetDefaultSoftness(m_pNewtonWorld, MTR_CHARACTER, MTR_LEVEL, 1.0f);
	NewtonMaterialSetDefaultElasticity(m_pNewtonWorld, MTR_CHARACTER, MTR_LEVEL, 0.0f);
	NewtonMaterialSetDefaultCollidable(m_pNewtonWorld, MTR_CHARACTER, MTR_LEVEL, 1);
	NewtonMaterialSetDefaultFriction(m_pNewtonWorld, MTR_CHARACTER, MTR_LEVEL, 0.0f, 0.0f);
	
	NewtonMaterialSetDefaultSoftness(m_pNewtonWorld, MTR_OBJECT, MTR_LEVEL, 1.0f);
	NewtonMaterialSetDefaultElasticity(m_pNewtonWorld, MTR_OBJECT, MTR_LEVEL, 0.0f);
	NewtonMaterialSetDefaultCollidable(m_pNewtonWorld, MTR_OBJECT, MTR_LEVEL, 1);
	NewtonMaterialSetDefaultFriction(m_pNewtonWorld, MTR_OBJECT, MTR_LEVEL, 0.0f, 0.0f);
	
	NewtonMaterialSetDefaultSoftness(m_pNewtonWorld, MTR_OBJECT, MTR_CHARACTER, 0.5f);
	NewtonMaterialSetDefaultElasticity(m_pNewtonWorld, MTR_OBJECT, MTR_CHARACTER, 0.0f);
	NewtonMaterialSetDefaultCollidable(m_pNewtonWorld, MTR_OBJECT, MTR_CHARACTER, 1);
	NewtonMaterialSetDefaultFriction(m_pNewtonWorld, MTR_OBJECT, MTR_CHARACTER, 1.0f, 0.5f);
	
	NewtonMaterialSetDefaultSoftness(m_pNewtonWorld, MTR_CHARACTER, MTR_CHARACTER, 0.5f);
	NewtonMaterialSetDefaultElasticity(m_pNewtonWorld, MTR_CHARACTER, MTR_CHARACTER, 0.0f);
	NewtonMaterialSetDefaultCollidable(m_pNewtonWorld, MTR_CHARACTER, MTR_CHARACTER, 1);
	NewtonMaterialSetDefaultFriction(m_pNewtonWorld, MTR_CHARACTER, MTR_CHARACTER, 1.0f, 0.5f);
	
	NewtonMaterialSetDefaultSoftness(m_pNewtonWorld, MTR_OBJECT, MTR_OBJECT, 0.5f);
	NewtonMaterialSetDefaultElasticity(m_pNewtonWorld, MTR_OBJECT, MTR_OBJECT, 0.0f);
	NewtonMaterialSetDefaultCollidable(m_pNewtonWorld, MTR_OBJECT, MTR_OBJECT, 1);
	NewtonMaterialSetDefaultFriction(m_pNewtonWorld, MTR_OBJECT, MTR_OBJECT, 1.0f, 0.5f);
	
	//Setup some base values
	m_v3Grav=s_v3Zero;
	m_pWorldBody=LG_NULL;
}

/* Shutdown
	PRE: Init should have been called.
	POST: Memory allocated to the physics engine is released,
	 after this method is called any created bodies that might
	 still ahve references on the server should not be used.
*/
void CLPhysNewton::Shutdown()
{
	NewtonDestroyAllBodies(m_pNewtonWorld);
	NewtonMaterialDestroyAllGroupID(m_pNewtonWorld);
	NewtonDestroy(m_pNewtonWorld);
}

/* AddBody
	PRE: pEnt should be an entity in the server world,
	 pInfo should be set with all information about the
	 body to be created.
	POST: A new body is added to the world (or if a new
	 body could not be added, null is returned.  Note that
	 the return value is a pointer that should be used
	 whenever updates by the server need to be applied
	 to the physics engine.
*/
lg_void* CLPhysNewton::AddBody(lg_void* pEnt, lp_body_info* pInfo)
{
	ML_MAT matPos;
	ML_MatIdentity(&matPos);
	matPos._41=(pInfo->m_aabbBody.v3Max.x+pInfo->m_aabbBody.v3Min.x)*0.5f;
	matPos._42=(pInfo->m_aabbBody.v3Max.y+pInfo->m_aabbBody.v3Min.y)*0.5f;
	matPos._43=(pInfo->m_aabbBody.v3Max.z+pInfo->m_aabbBody.v3Min.z)*0.5f;
	
	NewtonCollision* pClsn=NewtonCreateBox(
		m_pNewtonWorld, 
		pInfo->m_aabbBody.v3Max.x-pInfo->m_aabbBody.v3Min.x,
		pInfo->m_aabbBody.v3Max.y-pInfo->m_aabbBody.v3Min.y,
		pInfo->m_aabbBody.v3Max.z-pInfo->m_aabbBody.v3Min.z,
		0,
		(dFloat*)&matPos);
		
	NewtonBody* pBdy= NewtonCreateDynamicBody(
		m_pNewtonWorld,
		pClsn, (dFloat*)&pInfo->m_matPos);
	NewtonDestroyCollision(pClsn);
	
	#if 0
	ML_VEC3 v3={
		pInfo->m_aabbBody.v3Min.x+(pInfo->m_aabbBody.v3Max.x-pInfo->m_aabbBody.v3Min.x)*0.5f,
		pInfo->m_aabbBody.v3Min.y+(pInfo->m_aabbBody.v3Max.y-pInfo->m_aabbBody.v3Min.y)*0.5f,
		pInfo->m_aabbBody.v3Min.z+(pInfo->m_aabbBody.v3Max.z-pInfo->m_aabbBody.v3Min.z)*0.5f};
	NewtonBodySetCentreOfMass(pBdy, (dFloat*)&v3);
	#else
	NewtonBodySetCentreOfMass(pBdy, (dFloat*)pInfo->m_fMassCenter);
	#endif
	NewtonBodySetMassMatrix(pBdy, pInfo->m_fMass, pInfo->m_fMass, pInfo->m_fMass, pInfo->m_fMass);
	NewtonBodySetMaterialGroupID(pBdy, MTR_OBJECT);
	// NewtonBodySetAutoFreeze(pBdy, 0);
	NewtonBodySetUserData(pBdy, pEnt);
	//Set the force and torque callback to the update from srv.
	NewtonBodySetForceAndTorqueCallback(pBdy, UpdateFromSrv);
	//Set the transform callback, wich sends data back to the srv.
	NewtonBodySetTransformCallback(pBdy, UpdateToSrv);
	if(LG_CheckFlag(pInfo->m_nAIPhysFlags, AI_PHYS_INT))
	{
		//Err_Printf("Created UPRIGHT object!");
		// add and up vector constraint to help in keeping the body upright
		ML_VEC3 upDirection ={0.0f, 1.0f, 0.0f};
		NewtonJoint* pUpVec = NewtonConstraintCreateUpVector(m_pNewtonWorld, (dFloat*)&upDirection, pBdy); 
	}
	
	UpdateToSrv(pBdy, (dFloat*)&pInfo->m_matPos, 0);
	
	return pBdy;
	#if 0		
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
	#endif
}

void CLPhysNewton::RemoveBody(lg_void* pBody)
{
	NewtonBody* pPhysBody=(NewtonBody*)pBody;
	NewtonDestroyBody(pPhysBody);
}

void CLPhysNewton::SetupWorld(lg_void* pWorldMap)
{
	if(m_pWorldBody)
	{
		NewtonDestroyBody(m_pWorldBody);
		m_pWorldBody=LG_NULL;
	}
	
	if(!pWorldMap)
		return;
		
	CLMap* pMap=(CLMap*)pWorldMap;
	
	NewtonCollision* pCol=NewtonCreateTreeCollision(m_pNewtonWorld, LG_NULL);
	if(!pCol)
	{
		Err_Printf("NEWTON GAME: SetupWorld: Could not create Newton collision.");
		return;
	}
	
	NewtonTreeCollisionBeginBuild(pCol);
	for(lg_dword i=0; i<pMap->m_nVertexCount/3; i++)
	{
		NewtonTreeCollisionAddFace(
			pCol,
			3,
			(dFloat*)&pMap->m_pVertexes[i*3],
			sizeof(LW_VERTEX),
			MTR_LEVEL);
	}
	
	NewtonTreeCollisionEndBuild(pCol, LG_TRUE);
	
	ML_MATRIX Identity;
	ML_MatIdentity(&Identity);
	m_pWorldBody=NewtonCreateKinematicBody(m_pNewtonWorld, pCol, (const dFloat*)&Identity);
	NewtonDestroyCollision(pCol);
	
	NewtonBodySetMaterialGroupID(m_pWorldBody, MTR_LEVEL);
	/*
	NewtonSetWorldSize(
		m_pNewtonWorld, 
		(dFloat*)&pMap->m_aabbMapBounds.v3Min, 
		(dFloat*)&pMap->m_aabbMapBounds.v3Max);
	*/
}

void CLPhysNewton::SetGravity(ML_VEC3* pGrav)
{
	m_v3Grav=*pGrav;
}

/* Simulate
	PRE: Everything needs to be initialized, fTimeStepSec >= 0.0f and
	 represents how many seconds have elapsed since the last update
	 (usually a small number such as 1/30 seconds (at 30 fps) etc.).
	POST: The physics engine simulates all physical interactions.
	 This method updates the entities on the server.  After it
	 finishes any changes in a body's velocity, torque, position, etc.
	 will already be updated on the server, so there is no need to
	 loop through all the ents on the server and update their
	 physical information.  Note that the server's entity's thrust
	 and torque are set to zero, this is because if an entity wants
	 to continue accelerating it needs to keep setting the forces
	 (that is in order to speed up forces need to be applied over time,
	 remember that forces imply acceleration, but if the object only
	 accelerates a little then it will not have a big velocity, and if
	 there is a significant amount of friction, then the veloctiy will
	 be reduced to zero very quickly).
	 
	 Future Notes:
	 Additionally if a body's veloctiy becomes extremely low, that
	 body will be put to sleep and will not be effected by calls
	 to UpdateBody.  If another bodies collides with a sleeping body
	 it will then become awake.  Note that AI controlled bodies should
	 be set to never fall asleep, only dumb objects.
*/
void CLPhysNewton::Simulate(lg_float fTimeStepSec)
{
	//::NewtonBodySetAp(m_pNewtonWorld, UpdateFromSrv);
	static lg_float fCumuSecs=0.0f;
	fCumuSecs+=fTimeStepSec;
	if(fCumuSecs<=0.003f)
		return;
	m_fTimeStep=fCumuSecs;	
	NewtonUpdate(m_pNewtonWorld, fCumuSecs);
	fCumuSecs=0;
}

void CLPhysNewton::SetBodyFlag(lg_void* pBody, lg_dword nFlagID, lg_dword nValue)
{

}

void CLPhysNewton::SetBodyVector(lg_void* pBody, lg_dword nVecID, ML_VEC3* pVec)
{

}

void CLPhysNewton::SetBodyFloat(lg_void* pBody, lg_dword nFloatID, lg_float fValue)
{

}

/* SetBodyPosition
	PRE: pBody must point to a body created by the phys engine.
	POST: The new position is set, and the server's entity's
	 m_v3Face and m_v3Pos is updated as well (so it does not 
	 need to be updated on the server side).
	 
	NOTES: This should rarely be called, it may need to be called
	 in the case of teleports of some kind, or in the case that
	 precise rotation needs to be specifed (as in the case of a
	 first person shooter, if we just had the mouse do a certain
	 amount of torque rotation might not be as smooth as directly
	 updating the rotation, but in that case the translation part
	 of the matrix should not be changed, only the rotation part).
*/
void CLPhysNewton::SetBodyPosition(lg_void* pBody, ML_MAT* pMat)
{
	NewtonBody* pPhysBody=(NewtonBody*)pBody;
	LEntitySrv* pEntSrv=(LEntitySrv*)::NewtonBodyGetUserData(pPhysBody);
	NewtonBodySetMatrix(pPhysBody, (dFloat*)pMat);
	pEntSrv->m_matPos=*pMat;
	
	//Calculate the face vectors.
	ML_Vec3TransformNormalArray(
		pEntSrv->m_v3Face, sizeof(ML_VEC3), 
		s_v3StdFace, sizeof(ML_VEC3), 
		pMat, 3);
}
	
lg_void* CLPhysNewton::GetBodySaveInfo(lg_void* pBody, lg_dword* pSize)
{
	return LG_NULL;
}

lg_void* CLPhysNewton::LoadBodySaveInfo(lg_void* pData, lg_dword nSize)
{
	return LG_NULL;
}

void CLPhysNewton::UpdateFromSrv(const NewtonBody* const body, dFloat timestep, int threadIndex)
{
	static lg_dword nPhysFlags;
	LEntitySrv* pEnt=(LEntitySrv*)NewtonBodyGetUserData(body);
	nPhysFlags=pEnt->m_pAI->PhysFlags();
	if(LG_CheckFlag(nPhysFlags, AI_PHYS_DIRECT_LINEAR))
	{
		
	}
	else
	{
		//ML_Vec3Scale(&pEnt->m_v3Thrust, &pEnt->m_v3Thrust, 1.0f/s_pPhys->m_fTimeStep);
		NewtonBodyAddForce(body, (dFloat*)&pEnt->m_v3Thrust);
		
		ML_Vec3Scale(&pEnt->m_v3Impulse, &pEnt->m_v3Impulse, 1.0f/s_pPhys->m_fTimeStep);
		NewtonBodyAddForce(body, (dFloat*)&pEnt->m_v3Impulse);
	}
	
	if(LG_CheckFlag(nPhysFlags, AI_PHYS_DIRECT_ANGULAR))
	{
		/*
		NewtonBodySetOmega(body, (dFloat*)&s_v3Zero);
		ML_Vec3Scale(&pEnt->m_v3Torque, &pEnt->m_v3Torque, 60);
		NewtonBodyAddTorque(body, (dFloat*)&pEnt->m_v3Torque);
		ML_MAT matTemp, matTemp2;
		NewtonBodyGetMatrix(body, (dFloat*)&matTemp);
		ML_MatRotationAxis(&matTemp2, &pEnt->m_v3Torque, ML_Vec3Length(&pEnt->m_v3Torque));
		ML_MatMultiply(&matTemp, &matTemp, &matTemp2);
		NewtonBodySetMatrix(body, (dFloat*)&matTemp);
		*/
		#if 0
		static dFloat mass[4];
		//static dFloat omega[3];
		//NewtonBodyGetOmega(body, omega);
		NewtonBodyGetMassMatrix(body, &mass[0], &mass[1], &mass[2], &mass[3]);
		ML_Vec3Scale(&pEnt->m_v3Torque, &pEnt->m_v3Torque, mass[0]);
		//ML_Vec3Add(&pEnt->m_v3Torque, (ML_VEC3*)omega, &pEnt->m_v3Torque);
		NewtonBodySetOmega(body, (dFloat*)&pEnt->m_v3Torque);
		#elif 0
		ML_VEC3 v3Temp;
		NewtonBodyGetTorque(body, (dFloat*)&v3Temp);
		ML_Vec3Scale(&v3Temp, &v3Temp, -2.0f);
		NewtonBodyAddTorque(body, (dFloat*)&v3Temp);
		
		NewtonBodyAddTorque(body, (dFloat*)&pEnt->m_v3Torque);
		#elif 1
		ML_Vec3Scale(&pEnt->m_v3Torque, &pEnt->m_v3Torque, 1000.0f);
		NewtonBodyAddTorque(body, (dFloat*)&pEnt->m_v3Torque);
		#else
		#endif
	}
	else
	{
		NewtonBodyAddTorque(body, (dFloat*)&pEnt->m_v3Torque);
	}
	//Apply grav
	//ML_VEC3 v3Grav={0.0f, -9.8f*pEnt->m_fMass, 0.0f};
	//NewtonBodyAddForce(body, (dFloat*)&v3Grav);
	
	/*
	ML_VEC3 v3Grav;
	ML_Vec3Scale(&v3Grav, &s_pPhys->m_v3Grav, pEnt->m_fMass);
	NewtonBodyAddForce(body, (dFloat*)&v3Grav);
	*/
	pEnt->m_v3Torque=s_v3Zero;
	pEnt->m_v3Thrust=s_v3Zero;
	pEnt->m_v3Impulse=s_v3Zero;
	#if 0
	if(LG_CheckFlag(pEnt->m_nFlags1, LEntitySrv::ENT_INT))
	{
		NewtonBodySetTorque(body, (dFloat*)&s_v3Zero.x);
	}
	#endif
}

void CLPhysNewton::UpdateToSrv(const NewtonBody* const body, const dFloat* const matrix, int threadIndex)
{
	LEntitySrv* pEnt=(LEntitySrv*)NewtonBodyGetUserData(body);
	pEnt->m_matPos=(ML_MAT)*(ML_MAT*)matrix;
	NewtonBodyGetVelocity(body, (dFloat*)&pEnt->m_v3Vel);
	NewtonBodyGetTorque(body, (dFloat*)&pEnt->m_v3Torque);
	ML_Vec3TransformNormalArray(
		pEnt->m_v3Face,
		sizeof(ML_VEC3),
		s_v3StdFace,
		sizeof(ML_VEC3),
		&pEnt->m_matPos,
		3);
		
	NewtonBodyGetAABB(body, (dFloat*)&pEnt->m_aabbBody.v3Min, (dFloat*)&pEnt->m_aabbBody.v3Max);
	//Call the post physics AI routine.
	pEnt->m_pAI->PostPhys(pEnt);
}
