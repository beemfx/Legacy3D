#pragma comment(lib, "NxCharacter.lib")
#pragma comment(lib, "NxExtensions.lib")
#pragma comment(lib, "NxCooking.lib")
#pragma comment(lib, "PhysXLoader.lib")

#include "lg_err.h"
#include "lg_err_ex.h"
#include "lp_physx.h"

class UserStream;


NxPhysicsSDK* CElementPhysX::s_pNxPhysics=LG_NULL;
NxScene*      CElementPhysX::s_pNxScene=LG_NULL;
CLPhysXOutputStream CElementPhysX::s_Output;

NxActor* CElementPhysX::s_pWorldActor=LG_NULL;

void CElementPhysX::Initialize()
{
	s_pNxPhysics=NxCreatePhysicsSDK(NX_PHYSICS_SDK_VERSION, 0, &s_Output);
	if(!s_pNxPhysics)
		throw ERROR_CODE(LG_ERR_DEFAULT, "Could not initialize PhysX engine.");
	
	s_pNxPhysics->setParameter(NX_SKIN_WIDTH, 0.01f);
	
	s_pNxPhysics->setParameter(NX_VISUALIZATION_SCALE, 1);
	s_pNxPhysics->setParameter(NX_VISUALIZE_COLLISION_SHAPES, 1);
	s_pNxPhysics->setParameter(NX_VISUALIZE_ACTOR_AXES, 1);
	
	
	NxSceneDesc SceneDesc;
	//memset(&SceneDesc, 0, sizeof(NxSceneDesc));	
	SceneDesc.gravity.set(0.0f, -9.8f, 0.0f);
	//SceneDesc.broadPhase = NX_BROADPHASE_COHERENT;
	//SceneDesc.collisionDetection = LG_TRUE;
	s_pNxScene=s_pNxPhysics->createScene(SceneDesc);
	if(!s_pNxScene)
		throw ERROR_CODE(LG_ERR_DEFAULT, "Could not initialize PhysX scene.");
		
	// Create the default material
	NxMaterial* defaultMaterial = s_pNxScene->getMaterialFromIndex(0);
	defaultMaterial->setRestitution(0.0f);
	defaultMaterial->setStaticFriction(0.5f);
	defaultMaterial->setDynamicFriction(0.5f);
}

void CElementPhysX::Shutdown()
{
	
	if(s_pNxScene)
		s_pNxPhysics->releaseScene(*s_pNxScene);
	
	if(s_pNxPhysics)
	{
		s_pNxPhysics->release();
		s_pNxPhysics=LG_NULL;
	}
}

void CElementPhysX::SetupWorld(CLWorldMap* pMap)
{
	if(s_pWorldActor)
		s_pNxScene->releaseActor(*s_pWorldActor);
	
	s_pWorldActor=LG_NULL;
	
	#if 0
	//Test code...
	NxPlaneShapeDesc planeDesc;
	NxActorDesc actorDesc;
 	actorDesc.shapes.pushBack(&planeDesc);
	s_pWorldActor=s_pNxScene->createActor(actorDesc);
	#elif 0
	 gLevelMesh.convertLevel();

    // Build physical model
    NxTriangleMeshDesc levelDesc;
    levelDesc.numVertices = g_Level.m_numOfVerts;
    levelDesc.numTriangles = gLevelMesh.nbTriangles;
    levelDesc.pointStrideBytes = sizeof(tBSPVertex);
    levelDesc.triangleStrideBytes = 3*sizeof(NxU32);
    levelDesc.points = (const NxPoint*)&(g_Level.m_pVerts[0].vPosition);
    levelDesc.triangles = gLevelMesh.triangles;
    levelDesc.flags = 0;

    NxTriangleMeshShapeDesc levelShapeDesc;
    NxInitCooking();
    if (0)
    {
        // Cooking from file
        bool status = NxCookTriangleMesh(levelDesc, UserStream("c:\\tmp.bin", false));
        levelShapeDesc.meshData = gPhysicsSDK->createTriangleMesh(UserStream("c:\\tmp.bin", true));
    }
    else
    {
        // Cooking from memory
        MemoryWriteBuffer buf;
        bool status = NxCookTriangleMesh(levelDesc, buf);
        levelShapeDesc.meshData = gPhysicsSDK->createTriangleMesh(MemoryReadBuffer(buf.data));
    }

    NxActorDesc actorDesc;
    actorDesc.shapes.pushBack(&levelShapeDesc);
    NxActor* actor = gScene->createActor(actorDesc);
    actor->userData = (void*)1;

     return actor;
	#elif 1
	lg_dword* pIndexes=new lg_dword[pMap->m_nVertexCount];
	for(lg_dword i=0; i<pMap->m_nVertexCount; i++)
	{
		pIndexes[i]=i;
	}
	NxTriangleMeshDesc desc;
	desc.numTriangles=pMap->m_nVertexCount/3;
	desc.numVertices=pMap->m_nVertexCount;
	desc.pointStrideBytes=sizeof(LW_VERTEX);
	desc.triangleStrideBytes=sizeof(lg_dword)*3;
	desc.points=pMap->m_pVertexes;
	desc.triangles=pIndexes;
	NxActorDesc actorDesc;
	NxTriangleMeshShapeDesc triShape;
	
	NxInitCooking();
	CLNxStream buf("/dbase/cook.bin", LG_FALSE);
	lg_bool status = NxCookTriangleMesh(desc, buf);
	buf.Close();
	buf.Open("/dbase/cook.bin", LG_TRUE);
	triShape.meshData=s_pNxPhysics->createTriangleMesh(buf);
	actorDesc.shapes.pushBack(&triShape);
	s_pWorldActor=s_pNxScene->createActor(actorDesc);
	delete[]pIndexes;
	#endif
}

void CElementPhysX::Simulate(lg_float fSeconds)
{
	s_pNxScene->simulate((NxReal)fSeconds);
	s_pNxScene->flushStream();
	s_pNxScene->fetchResults(NX_RIGID_BODY_FINISHED, LG_TRUE);
}

void CLPhysXOutputStream::reportError(NxErrorCode code, const char* message, const char* file, int line)
{
	Err_Printf("PhysX ERROR: %d: %s (%s.%d)", code, message, file, line);
}
NxAssertResponse CLPhysXOutputStream::reportAssertViolation(const char* message, const char* file, int line)
{
	Err_Printf("PhysX ASSERT VIOLATION: %s (%s.%d)", message, file, line);
	return NX_AR_BREAKPOINT;
}
void CLPhysXOutputStream::print(const char* message)
{
	Err_Printf("PhysX: %s", message);
}


