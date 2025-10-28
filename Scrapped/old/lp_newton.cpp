#include "lp_newton.h"
#include "lg_err_ex.h"
#include "lg_malloc.h"
#include "lg_err.h"

NewtonWorld* CElementNewton::s_pNewtonWorld=LG_NULL;
NewtonBody* CElementNewton::s_pWorldBody=LG_NULL;

lg_int CElementNewton::MTR_DEFAULT=0;
lg_int CElementNewton::MTR_LEVEL=0;
lg_int CElementNewton::MTR_CHARACTER=0;
lg_int CElementNewton::MTR_OBJECT=0;

void CElementNewton::Initialize()
{
	s_pNewtonWorld=NewtonCreate(0, 0);//(NewtonAllocMemory)LG_Malloc, (NewtonFreeMemory)LG_Free);
	if(!s_pNewtonWorld)
		throw CLError(LG_ERR_DEFAULT, __FILE__, __LINE__, "Newton Initialization: Could not create physics model.");
	
	MTR_DEFAULT=NewtonMaterialGetDefaultGroupID(s_pNewtonWorld);
	NewtonMaterialSetDefaultSoftness(s_pNewtonWorld, MTR_DEFAULT, MTR_DEFAULT, 1.0f);
	NewtonMaterialSetDefaultElasticity(s_pNewtonWorld, MTR_DEFAULT, MTR_DEFAULT, 0.0f);
	NewtonMaterialSetDefaultCollidable(s_pNewtonWorld, MTR_DEFAULT, MTR_DEFAULT, 1);
	NewtonMaterialSetDefaultFriction(s_pNewtonWorld, MTR_DEFAULT, MTR_DEFAULT, 1.0f, 1.0f);
	//NewtonMaterialSetCollisionCallback (s_pNewtonWorld, MTR_DEFAULT, MTR_DEFAULT, &wood_wood, GenericContactBegin, GenericContactProcess, GenericContactEnd);  

	MTR_LEVEL=NewtonMaterialCreateGroupID(s_pNewtonWorld);
	MTR_CHARACTER=NewtonMaterialCreateGroupID(s_pNewtonWorld);
	MTR_OBJECT=NewtonMaterialCreateGroupID(s_pNewtonWorld);
	
	NewtonMaterialSetDefaultSoftness(s_pNewtonWorld, MTR_CHARACTER, MTR_LEVEL, 1.0f);
	NewtonMaterialSetDefaultElasticity(s_pNewtonWorld, MTR_CHARACTER, MTR_LEVEL, 0.0f);
	NewtonMaterialSetDefaultCollidable(s_pNewtonWorld, MTR_CHARACTER, MTR_LEVEL, 1);
	NewtonMaterialSetDefaultFriction(s_pNewtonWorld, MTR_CHARACTER, MTR_LEVEL, 0.0f, 0.0f);
	
	NewtonMaterialSetDefaultSoftness(s_pNewtonWorld, MTR_OBJECT, MTR_LEVEL, 1.0f);
	NewtonMaterialSetDefaultElasticity(s_pNewtonWorld, MTR_OBJECT, MTR_LEVEL, 0.0f);
	NewtonMaterialSetDefaultCollidable(s_pNewtonWorld, MTR_OBJECT, MTR_LEVEL, 1);
	NewtonMaterialSetDefaultFriction(s_pNewtonWorld, MTR_OBJECT, MTR_LEVEL, 1.0f, 0.5f);
	
	NewtonMaterialSetDefaultSoftness(s_pNewtonWorld, MTR_OBJECT, MTR_CHARACTER, 0.5f);
	NewtonMaterialSetDefaultElasticity(s_pNewtonWorld, MTR_OBJECT, MTR_CHARACTER, 0.0f);
	NewtonMaterialSetDefaultCollidable(s_pNewtonWorld, MTR_OBJECT, MTR_CHARACTER, 1);
	NewtonMaterialSetDefaultFriction(s_pNewtonWorld, MTR_OBJECT, MTR_CHARACTER, 1.0f, 0.5f);
	
	NewtonMaterialSetDefaultSoftness(s_pNewtonWorld, MTR_CHARACTER, MTR_CHARACTER, 0.5f);
	NewtonMaterialSetDefaultElasticity(s_pNewtonWorld, MTR_CHARACTER, MTR_CHARACTER, 0.0f);
	NewtonMaterialSetDefaultCollidable(s_pNewtonWorld, MTR_CHARACTER, MTR_CHARACTER, 1);
	NewtonMaterialSetDefaultFriction(s_pNewtonWorld, MTR_CHARACTER, MTR_CHARACTER, 1.0f, 0.5f);
	
	NewtonMaterialSetDefaultSoftness(s_pNewtonWorld, MTR_OBJECT, MTR_OBJECT, 0.5f);
	NewtonMaterialSetDefaultElasticity(s_pNewtonWorld, MTR_OBJECT, MTR_OBJECT, 0.0f);
	NewtonMaterialSetDefaultCollidable(s_pNewtonWorld, MTR_OBJECT, MTR_OBJECT, 1);
	NewtonMaterialSetDefaultFriction(s_pNewtonWorld, MTR_OBJECT, MTR_OBJECT, 1.0f, 0.5f);
}

void CElementNewton::Shutdown()
{
	SetupWorld(LG_NULL);
	NewtonMaterialDestroyAllGroupID(s_pNewtonWorld);
	NewtonDestroy(s_pNewtonWorld);
}

void CElementNewton::SetupWorld(CLWorldMap* pMap)
{
	if(s_pWorldBody)
	{
		NewtonDestroyBody(s_pNewtonWorld, s_pWorldBody);
		s_pWorldBody=LG_NULL;
	}
	if(!pMap)
		return;
		
	NewtonCollision* pCol=NewtonCreateTreeCollision(
		s_pNewtonWorld,
		LG_NULL);
		
	if(!pCol)
	{
		Err_Printf("   CLWorldMap::Load ERROR: Could not create Newton collisison.");
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
	s_pWorldBody=NewtonCreateBody(s_pNewtonWorld, pCol);
	NewtonReleaseCollision(s_pNewtonWorld, pCol);
	
	NewtonBodySetMaterialGroupID(s_pWorldBody, MTR_LEVEL);
	
	NewtonSetWorldSize(
		s_pNewtonWorld, 
		(dFloat*)&pMap->m_aabbMapBounds.v3Min, 
		(dFloat*)&pMap->m_aabbMapBounds.v3Max);
		
	return;
}

void CElementNewton::Simulate(lg_float fSeconds)
{
	static lg_float fCumuSecs=0.0f;
	fCumuSecs+=fSeconds;
	if(fCumuSecs<=0.003f)
		return;
		
	NewtonUpdate(s_pNewtonWorld, fCumuSecs);
	fCumuSecs=0;
}