#if 0
#include <NxCooking.h>
#include <NxCapsuleController.h>
#include "lp_physx2.h"
#include "lg_err.h"
#include "lg_err_ex.h"
#include "lw_entity.h"
#include "lw_map.h"

#pragma comment(lib, "PhysXLoader.lib")
#pragma comment(lib, "NxCooking.lib")
#pragma comment(lib, "NxCharacter.lib")

void CLPhysPhysX::Init(lg_dword nMaxBodies)
{
	//Create the SDK:
	m_pSDK=NxCreatePhysicsSDK(NX_PHYSICS_SDK_VERSION, LG_NULL/*&m_Alloc*/, &m_Out);
	LG_ASSERT(m_pSDK, "Could not initialize NVIDIA PhysX.");
	
	//Controller manager creation:
	m_pCtrlrMgr=NxCreateControllerManager(&m_Alloc);
	LG_ASSERT(m_pCtrlrMgr, "Could not initialize NVIDIA PhysX Controller Manager.");
	
	//Initialize mesh cooking (For maps).
	//LG_ASSERT(NxInitCooking(), "Could not initalize NVIDIA PhysX Mesh Cooking.");
	m_pCooking=NxGetCookingLib(NX_PHYSICS_SDK_VERSION);
	LG_ASSERT(m_pCooking->NxInitCooking(&m_Alloc, &m_Out), "Could not initialize NVIDIA PhysX Cooking.");

	//Set some default parameters (we can use cvars for these if we want).
	m_pSDK->setParameter(NX_SKIN_WIDTH, 0.001f);
	/*
	m_pSDK->setParameter(NX_VISUALIZATION_SCALE, 1);
	m_pSDK->setParameter(NX_VISUALIZE_COLLISION_SHAPES, 1);
	m_pSDK->setParameter(NX_VISUALIZE_ACTOR_AXES, 1);
	*/
	
	#ifdef _DEBUG
	m_pSDK->getFoundationSDK().getRemoteDebugger()->connect("localhost", 5425);
	#endif _DEBUG
	
	//Now we create the scene
	NxSceneDesc sd;
	NxSceneLimits lim;
	lim.maxNbActors=nMaxBodies;
	lim.maxNbBodies=nMaxBodies;
	sd.setToDefault();
	
	sd.gravity.set(0.0f, 0.0f, 0.0f);
	sd.flags |= NX_SF_ENABLE_ACTIVETRANSFORMS; //We want to use active transforms.
	sd.limits=&lim;
	
	//sd.braodPhase=NX_BROADPHASE_COHERENT;
	//sd.collisionDetection=NxTrue;
	m_pScene=m_pSDK->createScene(sd);
	
	LG_ASSERT(m_pScene, "Could not initialize NVIDIA PhysX scence.")
	
	//Setup the default material:
	NxMaterial* pDM = m_pScene->getMaterialFromIndex(0);
	pDM->setRestitution(0.0f);
	pDM->setStaticFriction(0.7f);
	pDM->setDynamicFriction(0.5f);
	
	NxMaterialDesc mtrDesc;
	mtrDesc.restitution=0.0f;
	mtrDesc.dynamicFriction=1.0f;
	mtrDesc.staticFriction=0.0f;
	m_pMtrInt=m_pScene->createMaterial(mtrDesc);
	
	mtrDesc.restitution=0.0f;
	mtrDesc.dynamicFriction=0.5f;
	mtrDesc.staticFriction=0.7f;
	m_pMtrMap=m_pScene->createMaterial(mtrDesc);
	m_pActrMap=LG_NULL;
	
	m_nATs=0;
	m_pATs=LG_NULL;
}

void CLPhysPhysX::Shutdown()
{
	if(m_pActrMap)
	{
		m_pScene->releaseActor(*m_pActrMap);
		m_pActrMap=LG_NULL;
	}
	//Releaset the materials:
	m_pScene->releaseMaterial(*m_pMtrInt);
	m_pMtrInt=LG_NULL;
	m_pScene->releaseMaterial(*m_pMtrMap);
	m_pMtrMap=LG_NULL;
	//Release the scene
	m_pSDK->releaseScene(*m_pScene);
	m_pScene=LG_NULL;
	
	//Release controller manager:
	NxReleaseControllerManager(m_pCtrlrMgr);
	
	//Release cooking:
	m_pCooking->NxCloseCooking();
	m_pCooking=LG_NULL;
	
	//Finally we destroy the SDK.
	m_pSDK->release();
	m_pSDK=LG_NULL;
}

lg_void* CLPhysPhysX::AddIntelligentBody(lg_void* pEnt, lp_body_info* pInfo)
{
	NxController* pCtrlr;
	NxCapsuleControllerDesc crd;
	crd.setToDefault();
	crd.radius=(pInfo->m_aabbBody.v3Max.x-pInfo->m_aabbBody.v3Min.x)*0.5f;
	crd.height=(pInfo->m_aabbBody.v3Max.y-pInfo->m_aabbBody.v3Min.y)-2.0f*crd.radius;
	//CtrlrDesc.materialIndex=m_pMtrInt->getMaterialIndex();
	crd.position.set(pInfo->m_matPos._41, pInfo->m_matPos._42, pInfo->m_matPos._43);
	crd.upDirection=NX_Y;
	crd.slopeLimit=0;
	crd.stepOffset=0.5f;
	#if 0
	NxCapsuleControllerDesc desc;
			NxVec3 tmp			= startPos[i];
			desc.position.x		= tmp.x;
			desc.position.y		= tmp.y;
			desc.position.z		= tmp.z;
			desc.radius			= gInitialRadius * scale;
			desc.height			= gInitialHeight * scale;
			desc.upDirection	= NX_Y;
			//		desc.slopeLimit		= cosf(NxMath::degToRad(45.0f));
			desc.slopeLimit		= 0;
			desc.skinWidth		= SKINWIDTH;
			desc.stepOffset		= 0.5;
			desc.stepOffset		= gInitialRadius * 0.5 * scale;
			//	desc.stepOffset	= 0.01f;
			//		desc.stepOffset		= 0;	// Fixes some issues
			//		desc.stepOffset		= 10;
			desc.callback		= &gControllerHitReport;
			gManager->createController(&scene, desc);
	#endif
	/*
	//Now that we have the shape, let's set the offset of it.
	ML_MAT matDims;
	ML_MatIdentity(&matDims);
	//When setting the offset of the shape, it is necessary to realize
	//that the legacy defintion of a shape places the object standing
	//on top of the XZ plane, therefore we need only add the offset of
	//the x and z dimensions, however for the y direction we need to move
	//the shape up one half of it's height, then add the offset.
	matDims._41=pInfo->m_fShapeOffset[0];
	matDims._42=(pInfo->m_aabbBody.v3Max.y-pInfo->m_aabbBody.v3Min.y)*0.5f+pInfo->m_fShapeOffset[1];
	matDims._43=pInfo->m_fShapeOffset[2];
	CtrlrDesc.localPose.setColumnMajor44((NxReal*)&matDims);
	*/
	
	pCtrlr=m_pCtrlrMgr->createController(m_pScene, crd);
	NxActor* pActor = pCtrlr->getActor();
	pActor->userData=pEnt;
	return pCtrlr;
	/*
	NxActor* pActor = (NxActor*)AddDynamicBody(pEnt, pInfo);
	
	return pActor;
	*/
}

lg_void* CLPhysPhysX::AddDynamicBody(lg_void* pEnt, lp_body_info* pInfo)
{
	NxActorDesc ad;
	NxBodyDesc bd;
	NxCapsuleShapeDesc csd;
	NxBoxShapeDesc bsd;
	NxShapeDesc* pShape=LG_NULL;
	
	//Create the shape:
	switch(pInfo->m_nShape)
	{
	default:
	case PHYS_SHAPE_BOX:
		bsd.setToDefault();
		//The dimensions are radii, so we multiply the dimensions by half.
		bsd.dimensions.set(
			(pInfo->m_aabbBody.v3Max.x-pInfo->m_aabbBody.v3Min.x)*0.5f,
			(pInfo->m_aabbBody.v3Max.y-pInfo->m_aabbBody.v3Min.y)*0.5f,
			(pInfo->m_aabbBody.v3Max.z-pInfo->m_aabbBody.v3Min.z)*0.5f);
		pShape=&bsd;
		break;
	case PHYS_SHAPE_CAPSULE:
		csd.setToDefault();
		csd.radius=(pInfo->m_aabbBody.v3Max.x-pInfo->m_aabbBody.v3Min.x)*0.5f;
		csd.height=(pInfo->m_aabbBody.v3Max.y-pInfo->m_aabbBody.v3Min.y)-2.0f*csd.radius;
		
		
		pShape=&csd;
		break;
	/*
	case PHYS_SHAPE_CYLINDER:
		break;
	case PHYS_SHAPE_SPHERE:
		break;
	*/
	}
	
	if(LG_CheckFlag(pInfo->m_nAIPhysFlags, AI_PHYS_INT))
	{
		pShape->materialIndex=m_pMtrInt->getMaterialIndex();
	}
	
	//Now that we have the shape, let's set the offset of it.
	ML_MAT matDims;
	ML_MatIdentity(&matDims);
	//When setting the offset of the shape, it is necessary to realize
	//that the legacy defintion of a shape places the object standing
	//on top of the XZ plane, therefore we need only add the offset of
	//the x and z dimensions, however for the y direction we need to move
	//the shape up one half of it's height, then add the offset.
	matDims._41=pInfo->m_fShapeOffset[0];
	matDims._42=(pInfo->m_aabbBody.v3Max.y-pInfo->m_aabbBody.v3Min.y)*0.5f+pInfo->m_fShapeOffset[1];
	matDims._43=pInfo->m_fShapeOffset[2];
	pShape->localPose.setColumnMajor44((NxReal*)&matDims);
	
	//Setup the body description for the actor:
	bd.setToDefault();
	bd.linearDamping=0.5f;
	bd.angularDamping=2.0f;
	
	if(LG_CheckFlag(pInfo->m_nAIPhysFlags, AI_PHYS_INT))
	{
		LG_SetFlag(bd.flags, NX_BF_FROZEN_ROT_X);
		LG_SetFlag(bd.flags, NX_BF_FROZEN_ROT_Z);
		
		LG_SetFlag(bd.flags, NX_BF_FROZEN_ROT_Y);
	}
	
	//Setup 
	//if(LG_CheckFlag(pInfo->m_nAIPhysFlags, AI_PHYS_INT))
	//	LG_SetFlag(bd.flags, NX_BF_KINEMATIC);
	/*
	ML_MatIdentity(&matDims);
	//Not really sure if this center of mass stuff is correct at this time
	matDims._41=pInfo->m_fMassCenter[0];
	matDims._42=(pInfo->m_aabbBody.v3Max.y-pInfo->m_aabbBody.v3Min.y)*0.5f+pInfo->m_fMassCenter[1];
	matDims._43=pInfo->m_fMassCenter[2];
	bd.massLocalPose.setColumnMajor44((NxReal*)&matDims);
	*/
	bd.mass=pInfo->m_fMass;
	//Setup the actor description:
	ad.setToDefault();
	ad.shapes.pushBack(pShape);
	ad.body=&bd;
	ad.globalPose.setColumnMajor44((NxReal*)&pInfo->m_matPos);
	//Create the body, attach the ent to it and return the body:
	NxActor* pActor = m_pScene->createActor(ad);
	pActor->userData=pEnt;
	return pActor;
}

lg_void* CLPhysPhysX::AddBody(lg_void* pEnt, lp_body_info* pInfo)
{
	/*
	if(LG_CheckFlag(pInfo->m_nAIPhysFlags, AI_PHYS_INT))
		return AddIntelligentBody(pEnt, pInfo);
	else
	*/
	return AddDynamicBody(pEnt, pInfo);
}
void CLPhysPhysX::RemoveBody(lg_void* pBody)
{
	NxActor* pActor=(NxActor*)pBody;
	m_pScene->releaseActor(*pActor);
	#if 0
	m_pScene->simulate(0.0f);
	m_pScene->flushStream();
	m_pScene->fetchResults(NX_RIGID_BODY_FINISHED, LG_TRUE);
	#endif
}
void CLPhysPhysX::SetupWorld(lg_void* pWorldMap)
{
	CLMap* pMap=(CLMap*)pWorldMap;
	if(m_pActrMap)
	{
		m_pScene->releaseActor(*m_pActrMap);
		m_pActrMap=LG_NULL;
	}
	
	//If null was passed as the parameter it means we
	//no longer want the world.
	if(!pMap)
		return;
	
	CLNxMemStream stream(1024);
	
	#if 1
	NxTriangleMeshDesc desc;
	desc.numTriangles=pMap->m_nGeoTriCount;//pMap->m_nVertexCount/3;
	desc.numVertices=pMap->m_nGeoTriCount*3;//pMap->m_nVertexCount;
	desc.pointStrideBytes=sizeof(ML_VEC3);
	desc.points=pMap->m_pGeoTris;//pMap->m_pVertexes;
	
	NxActorDesc actorDesc;
	NxTriangleMeshShapeDesc triShape;
	
	stream.Reset();
	lg_bool status = m_pCooking->NxCookTriangleMesh(desc, stream);
	
	stream.Reset();
	triShape.meshData=m_pSDK->createTriangleMesh(stream);
	actorDesc.shapes.pushBack(&triShape);
	triShape.materialIndex=m_pMtrMap->getMaterialIndex();
	m_pActrMap=m_pScene->createActor(actorDesc);
	#else
	NxActorDesc ad;
	NxConvexMeshDesc cmd;
	
	ad.setToDefault();
	
	for(lg_dword i=0; i<pMap->m_nGeoBlockCount; i++)
	{
		cmd.setToDefault();
		cmd.numVertices=pMap->m_pGeoBlocks[i].nVertexCount;
		cmd.pointStrideBytes=sizeof(LW_GEO_VERTEX);
		cmd.points=pMap->m_pGeoBlocks[i].pVerts;
		cmd.flags=NX_CF_COMPUTE_CONVEX;
		
		stream.Reset();
		lg_bool bRes=m_pCooking->NxCookConvexMesh(cmd, stream);
		stream.Reset();
		if(bRes)
		{
			NxConvexShapeDesc csd;
			csd.meshData=m_pSDK->createConvexMesh(stream);
			if(csd.meshData)
			{
				ad.shapes.push_back(&csd);
			}
		}
	}
	
	m_pActrMap=m_pScene->createActor(ad);
	#endif
}
void CLPhysPhysX::SetGravity(ML_VEC3* pGrav)
{
	m_pScene->setGravity((NxVec3&)*pGrav);
}

void CLPhysPhysX::Simulate(lg_float fTimeStepSec)
{
	
	
	//Do the updates, really I don't want to loop through every actor
	//but I don't know if there is a way to loop only through bodies that
	//are awake.
	static NxU32 nActCount;
	static NxActor** ppActors;
	
	nActCount=m_pScene->getNbActors();
	ppActors=m_pScene->getActors();
	for(NxU32 i=0; i<nActCount; ++i)
	{
		if(!ppActors[i]->isSleeping())
		{
			//We need to update the actors based on info from the server.
			UpdateFromSrv(ppActors[i]);
		}
	}
	
	
	// Get the Active Transforms from the scene
	m_nATs=0;
	m_pATs = m_pScene->getActiveTransforms(m_nATs);
	
	m_pScene->simulate((NxReal)fTimeStepSec);
	
	//We only need to update objects if they are moving:
	if(m_nATs && m_pATs)
	{
		for(NxU32 i=0; i<m_nATs; ++i)
		{
			UpdateToSrv(m_pATs[i].actor, (lg_srv_ent*)m_pATs[i].userData, &m_pATs[i].actor2World);	
		}	
	}
	
	m_pScene->flushStream();
	m_pScene->fetchResults(NX_RIGID_BODY_FINISHED, LG_TRUE);
	
}
	
void CLPhysPhysX::SetBodyFlag(lg_void* pBody, lg_dword nFlagID, lg_dword nValue){}
void CLPhysPhysX::SetBodyVector(lg_void* pBody, lg_dword nVecID, ML_VEC3* pVec){}
void CLPhysPhysX::SetBodyFloat(lg_void* pBody, lg_dword nFloatID, lg_float fValue){}
void CLPhysPhysX::SetBodyPosition(lg_void* pBody, ML_MAT* pMat)
{
}
	
lg_void* CLPhysPhysX::GetBodySaveInfo(lg_void* pBody, lg_dword* pSize)
{
	return LG_NULL;
}
lg_void* CLPhysPhysX::LoadBodySaveInfo(lg_void* pData, lg_dword nSize)
{
	return LG_NULL;
}

void CLPhysPhysX::UpdateFromSrv(NxActor* pSrc)
{
	static NxMat34 mat34;
	static NxMat33 mat33;
	static NxVec3 v3T;
	static ML_MAT matT;
	
	lg_srv_ent* pEnt=(lg_srv_ent*)pSrc->userData;
	
	if(LG_CheckFlag(pEnt->m_pAI->PhysFlags(), AI_PHYS_DIRECT_LINEAR))
	{
		/*
		mat34.M.id();
		mat34.t.set((NxVec3&)pEnt->m_v3Thrust);
		mat34*=pSrc->getGlobalPosition();
		*/
		v3T=pSrc->getGlobalPosition();
		v3T.add(v3T, (NxVec3&)pEnt->m_v3Thrust);
		pSrc->setGlobalPosition(v3T);
	}
	else
	{
		pSrc->addForce(*(NxVec3*)&pEnt->m_v3Thrust, NX_FORCE);
		pSrc->addForce(*(NxVec3*)&pEnt->m_v3Impulse, NX_IMPULSE);
	}
	
	if(LG_CheckFlag(pEnt->m_pAI->PhysFlags(), AI_PHYS_DIRECT_ANGULAR))
	{
		
		ML_MatRotationAxis(&matT, &pEnt->m_v3Torque, ML_Vec3Length(&pEnt->m_v3Torque));
		mat34.setColumnMajor44((NxF32*)&matT);
		mat33=mat34.M;
		mat33*=pSrc->getGlobalOrientation();
		pSrc->setGlobalOrientation(mat33);
		
		//pSrc->addTorque(*(NxVec3*)&pEnt->m_v3Torque, NX_VELOCITY_CHANGE);
	}
	else
	{
		pSrc->addTorque(*(NxVec3*)&pEnt->m_v3Torque);
	}
	
	pEnt->m_v3Thrust=s_v3Zero;
	pEnt->m_v3Torque=s_v3Zero;
	pEnt->m_v3Impulse=s_v3Zero;
}


void CLPhysPhysX::UpdateToSrv(NxActor* pSrc, lg_srv_ent* pEnt, NxMat34* pPos)
{			
	//We first check to make sure the ent isn't blank,
	//because if we are using active transforms, and the entity
	//was deleted, the pointer to the entitiy will still
	//exist, but it won't actually exist (and pSrc would not
	//be a valid actor).
	if(LG_CheckFlag(pEnt->m_nFlags1, pEnt->EF_1_BLANK))
		return;
		
	//Err_Printf("Calling: %s", pEnt->m_szObjScript);
	pPos->getColumnMajor44((NxF32*)&pEnt->m_matPos);
	
	pSrc->getShapes()[0]->getWorldBounds((NxBounds3&)pEnt->m_aabbBody);
	static NxVec3 v3;
	v3 = pSrc->getLinearVelocity();
	pEnt->m_v3Vel=*(ML_VEC3*)&v3;
	v3 = pSrc->getAngularVelocity();
	pEnt->m_v3AngVel=*(ML_VEC3*)&v3;
	
	ML_Vec3TransformNormalArray(
		pEnt->m_v3Face,
		sizeof(ML_VEC3),
		s_v3StdFace,
		sizeof(ML_VEC3),
		&pEnt->m_matPos,
		3);
	
	//Call the post physics AI routine.
	pEnt->m_pAI->PostPhys(pEnt);
}

/******************
*** CXOut class ***
******************/

void CLPhysPhysX::CXOut::reportError(NxErrorCode code, const char* message, const char* file, int line)
{
	Err_Printf("PhysX: ERROR: %d: %s (%s.%d)", code, message, file, line);
}
NxAssertResponse CLPhysPhysX::CXOut::reportAssertViolation(const char* message, const char* file, int line)
{
	Err_Printf("PhysX: ASSERT VIOLATION: %s (%s.%d)", message, file, line);
	return NX_AR_BREAKPOINT;
}
void CLPhysPhysX::CXOut::print(const char* message)
{
	Err_Printf("PhysX: %s", message);
}

/********************************
*** The file stream for PhysX ***
********************************/

CLPhysPhysX::CLNxStream::CLNxStream(const lg_char filename[], lg_bool bLoad):
m_File(LG_NULL)
{
	Open(filename, bLoad);
}

CLPhysPhysX::CLNxStream::~CLNxStream()
{
	Close();
}
	
NxU8 CLPhysPhysX::CLNxStream::readByte()const
{
	NxU8 b;
	LF_Read(m_File, &b, 1);
	return b;
}
NxU16 CLPhysPhysX::CLNxStream::readWord()const
{
	NxU16 b;
	LF_Read(m_File, &b, 2);
	return b;
}
NxU32 CLPhysPhysX::CLNxStream::readDword()const
{
	NxU32 b;
	LF_Read(m_File, &b, 4);
	return b;
}
NxReal CLPhysPhysX::CLNxStream::readFloat()const
{
	NxReal b;
	LF_Read(m_File, &b, 4);
	return b;
}
NxF64 CLPhysPhysX::CLNxStream::readDouble()const
{
	NxF64 b;
	LF_Read(m_File, &b, 8);
	return b;
}
void CLPhysPhysX::CLNxStream::readBuffer(void* buffer, NxU32 size)const
{
	LF_Read(m_File, buffer, size);
}
	
NxStream& CLPhysPhysX::CLNxStream::storeByte(NxU8 n)
{
	LF_Write(m_File, &n, 1);
	return *this;
}
NxStream& CLPhysPhysX::CLNxStream::storeWord(NxU16 w)
{
	LF_Write(m_File, &w, 2);
	return *this;
}
NxStream& CLPhysPhysX::CLNxStream::storeDword(NxU32 d)
{
	LF_Write(m_File, &d, 4);
	return *this;
}
NxStream& CLPhysPhysX::CLNxStream::storeFloat(NxReal f)
{
	LF_Write(m_File, &f, 4);
	return *this;
}
NxStream& CLPhysPhysX::CLNxStream::storeDouble(NxF64 f)
{
	LF_Write(m_File, &f, 8);
	return *this;
}
NxStream& CLPhysPhysX::CLNxStream::storeBuffer(const void* buffer, NxU32 size)
{
	LF_Write(m_File, (lf_void*)buffer, size);
	return *this;
}
	
void CLPhysPhysX::CLNxStream::Open(const lg_char filename[], lg_bool bLoad)
{
	Close();
	m_File=LF_Open(filename, LF_ACCESS_READ|LF_ACCESS_WRITE, bLoad?LF_OPEN_EXISTING:LF_CREATE_ALWAYS);
}
void CLPhysPhysX::CLNxStream::Close()
{
	if(m_File)
	{
		LF_Close(m_File);
		m_File=LG_NULL;
	}
}

/**********************************
*** The memory stream for PhysX ***
**********************************/

CLPhysPhysX::CLNxMemStream::CLNxMemStream(lg_dword nSize)
: m_File(0)
{
	Open(nSize);
}

CLPhysPhysX::CLNxMemStream::~CLNxMemStream()
{
	Close();
}

void CLPhysPhysX::CLNxMemStream::Open(lg_dword nSize)
{
	Close();
	m_File.Open(nSize);
}
void CLPhysPhysX::CLNxMemStream::Close()
{
	m_File.Close();
}

void CLPhysPhysX::CLNxMemStream::Reset()
{
	m_File.Seek(m_File.MEM_SEEK_BEGIN, 0);
}
	
NxU8 CLPhysPhysX::CLNxMemStream::readByte()const
{
		
	NxU8 b;
	readBuffer(&b, 1);
	return b;
}
NxU16 CLPhysPhysX::CLNxMemStream::readWord()const
{
	NxU16 b;
	readBuffer(&b, 2);
	return b;
}
NxU32 CLPhysPhysX::CLNxMemStream::readDword()const
{
	NxU32 b;
	readBuffer(&b, 4);
	return b;
}
NxReal CLPhysPhysX::CLNxMemStream::readFloat()const
{
	NxReal b;
	readBuffer(&b, 4);
	return b;
}
NxF64 CLPhysPhysX::CLNxMemStream::readDouble()const
{
	NxF64 b;
	readBuffer(&b, 8);
	return b;
}
void CLPhysPhysX::CLNxMemStream::readBuffer(void* buffer, NxU32 size)const
{
	//Because of the const qualifier (which is required by the NxStream
	//template, we need to derefernce, the memory file, so that we can
	//call a non const method on it, it's rediculously retarded, but
	//we don't have much choice.
	(*(CLMemFile*)&m_File).Read(buffer, size);
}
	
NxStream& CLPhysPhysX::CLNxMemStream::storeByte(NxU8 n)
{
	return storeBuffer(&n, 1);
}
NxStream& CLPhysPhysX::CLNxMemStream::storeWord(NxU16 w)
{
	return storeBuffer(&w, 2);
}
NxStream& CLPhysPhysX::CLNxMemStream::storeDword(NxU32 d)
{
	return storeBuffer(&d, 4);
}
NxStream& CLPhysPhysX::CLNxMemStream::storeFloat(NxReal f)
{
	return storeBuffer(&f, 4);
}
NxStream& CLPhysPhysX::CLNxMemStream::storeDouble(NxF64 f)
{
	return storeBuffer(&f, 8);
}
NxStream& CLPhysPhysX::CLNxMemStream::storeBuffer(const void* buffer, NxU32 size)
{
	//Should probably resize if we don't 
	//have a big enough file.
	if((m_File.Tell()+size)>m_File.Size())
	{
		m_File.Resize(m_File.Size()*2);
	}
	m_File.Write(buffer, size);
	return *this;
}
	
/******************************
*** The allocator for PhysX ***
******************************/
#include "lg_malloc.h"

void * CLPhysPhysX::CXAlloc::malloc(NxU32 size)
{
	return LG_Malloc(size);
}
void * CLPhysPhysX::CXAlloc::mallocDEBUG(NxU32 size,const char *fileName, int line)
{
	return this->malloc(size);
}
void * CLPhysPhysX::CXAlloc::realloc(void * memory, NxU32 size)
{
	this->free(memory);
	return LG_Malloc(size);
}
void CLPhysPhysX::CXAlloc::free(void * memory)
{
	LG_Free(memory);
}
	
#endif