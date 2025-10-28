#include "lg_malloc.h"
#include "lg_sys.h"
#include "lw_sys.h"
#include "lg_err.h"
#include "lg_err_ex.h"
#include "lvf_sys.h"

#include "le_test.h"
#include "ld_sys.h"

CLPhysics CLWorld::s_LegacyPhysics;

CLWorld::CLWorld():
	m_pEntities(LG_NULL),
	m_Camera()
{
	Initialize();
}

CLWorld::~CLWorld()
{
	m_WorldMap.Unload();
	//L_safe_release(m_pDevice);
	s_LegacyPhysics.RemoveAllBodies();
	RemoveAllEntities();
	
	Err_Printf("Destroying Newton Game Dynamics physics engine...");
	CElementNewton::Shutdown();
	CElementPhysX::Shutdown();
}

void CLWorld::RemoveEntity(CLEntity* pEnt)
{
	CLEntity* pTempEnt;
	if(pEnt==m_pEntities)
	{
		//Special case.
		pTempEnt=m_pEntities;
		m_pEntities=(CLBase3DEntity*)m_pEntities->GetNext();
		delete pTempEnt;
		return;
	}
	
	pTempEnt=pEnt->GetPrev();
	pTempEnt->SetNext(pEnt->GetNext());
	pEnt->SetPrev(pTempEnt);
	delete pEnt;
}

void CLWorld::RemoveAllEntities()
{
	for(CLBase3DEntity* pEnt=m_pEntities; pEnt; )
	{
		CLBase3DEntity* pTemp=(CLBase3DEntity*)pEnt->GetNext();
		//In the future we won't use delete, there will be a
		//set maximum entities and we will simply remove 
		//entities from the linked list.
		delete pEnt;
		pEnt=pTemp;
	}
}

void CLWorld::Initialize()
{
	if(!s_pDevice)
		throw CLError(LG_ERR_INVALID_PARAM, __FILE__, __LINE__, "CLWorld Initializion: No device specified.");
	
	//Create the newton world...
	Err_Printf("Initializing Newton Game Dynamics physics engine...");
	CElementNewton::Initialize();
	CLEntity::s_pWorldMap=&m_WorldMap;
	
	Err_Printf("Initializing PhysX physics engine...");
	CElementPhysX::Initialize();
	
	
	LoadLevel("dbase/scripts/levels/Cedar Hills.xml");
}

lg_bool CLWorld::LoadMap(lf_path szFilename)
{
	lg_bool bLoaded=m_WorldMap.LoadFromWS(szFilename, s_pDevice);
	//lg_bool bLoaded=m_WorldMap.Load(szFilename, s_pDevice);
	if(bLoaded)
	{
		Err_Printf("Setting up Newton Game Dynamics world geometry...");
		CElementNewton::SetupWorld(&m_WorldMap);
		Err_Printf("Setting up PhysX world geometry...");
		CElementPhysX::SetupWorld(&m_WorldMap);
		
		s_LegacyPhysics.SetWorld(&m_WorldMap);
	}
	else
	{
		CElementNewton::SetupWorld(LG_NULL);
		CElementPhysX::SetupWorld(LG_NULL);
	}
	return bLoaded;
}

lg_bool CLWorld::LoadLevel(lf_path szXMLFilename)
{
	//This is just temporary, a real xml file will be loaded later on.
	
	//Some test stuff...
	//m_SkyBox.Load("/dbase/meshes/skybox/skybox.lmsh");	
	m_SkyBox.Load("/dbase/meshes/skybox/SkyMATTER3/SkyMATTER3.lmsh");	
	
	//lg_char szMap[]="/dbase/maps/solids_test.3dw";
	//lg_char szMap[]="/dbase/maps/Cedar Hills.3dw";
	//lg_char szMap[]="/dbase/maps/tri_test.3dw";
	//lg_char szMap[]="/dbase/maps/thing.3dw";
	lg_char szMap[]="/dbase/maps/room_test.3dw";
	//lg_char szMap[]="/dbase/maps/block.3dw";
	//lg_char szMap[]="/dbase/maps/house.3dw";
	//lg_char szMap[]="/dbase/maps/simple_example.3dw";
	
	//m_WorldMap.LoadFromWS(szMap, s_pDevice);
	LoadMap(szMap);
	
	//Create some entities.
	ML_VEC3 v3Pos={2.0f, 0.2f, -2.0f};
	CLBase3DEntity* pNewEnt;
	
	pNewEnt=AddEntity(new CLJack());
	pNewEnt->Initialize(&v3Pos);
	
	ML_VEC3 offset={0.0f, 1.2f, 0.0f};
	//ML_VEC3 offset={0.0f, 1.5f*30.0f, 0.0f};
	//m_Camera.AttachToObject(&m_Player, CLCamera::CLCAMERA_ATTACH_FOLLOW, &offset, 2.0f);
	m_Camera.AttachToObject(
		pNewEnt, 
		CLCamera::CLCAMERA_ATTACH_FOLLOW, 
		//CLCamera::CLCAMERA_ATTACH_EYE, 
		&offset, 1.5f);//*30.0f);
		
	#if 1
	v3Pos.x=2.0f;
	v3Pos.z=-1.0f;
	v3Pos.y=0.2f;
	pNewEnt=AddEntity(new CLBlaineEnt());
	pNewEnt->Initialize(&v3Pos);
	
	v3Pos.x=1.0f;
	v3Pos.z=2.0f;
	v3Pos.y=1.0f;
	pNewEnt=AddEntity(new CLBarrelEnt());
	pNewEnt->Initialize(&v3Pos);
	#endif
	
	//A test
	#if 0
	CLVFont font;
	font.Load("/dbase/font/font2.xml");
	#endif
	
	//Test stuff...
	//lg_str szTestSound="/dbase/music/singerstrangetheme.ogg";
	//lg_str szTestSound="/dbase/music/LARGEWAVE01.wav";
	lg_str szTestSound="InTown.ogg";
	LC_SendCommandf("MUSIC_START \"%s\"", szTestSound);
	
	//UPdate the timer so the phsysics engine isn't way ahead.
	s_Timer.Update();
	return LG_TRUE;
}

lg_bool CLWorld::LoadSky(lf_path szFilename)
{
	m_SkyBox.Load(szFilename);
	return LG_TRUE;
}

void CLWorld::ProcessEntities()
{
	//First things first is to process all the entitie's AI (processing
	//AI doesn't move the object, but it generates a proposoed movement
	//vector stored int m_v3Vel.
	for(CLEntity* pEnt=m_pEntities; pEnt; pEnt=pEnt->GetNext())
	{
		pEnt->ProcessFrame();
	}
	
	//Process Newton Game Dynamics physics engine.
	CElementNewton::Simulate(s_Timer.GetFElapsed());
	//s_LegacyPhysics.Processes(s_Timer.GetElapsed());
	//Update PhysX scene...
	CElementPhysX::Simulate(s_Timer.GetFElapsed());
	
	this->s_LegacyPhysics.Processes(s_Timer.GetElapsed());
	//Update the camera...
	m_Camera.Update();
}

CLBase3DEntity* CLWorld::AddEntity(CLBase3DEntity* pEnt)
{
	if(!pEnt)
	{
		Err_Printf("CLWorld::AddEntity ERROR: No entity specified.");
		return LG_NULL;
	}
	pEnt->SetPrev(LG_NULL);
	pEnt->SetNext(m_pEntities);
	if(m_pEntities)
		m_pEntities->SetPrev(pEnt);
	m_pEntities=pEnt;
	return pEnt;
}

void CLWorld::SetEntityLight(CLBase3DEntity* pEnt)
{
	if(!m_WorldMap.m_bLoaded)
		return;
	//Right now we'll just get the first light
	//from the first region that the entity is in.
	LW_LIGHT lt;
	memset(&lt, 0, sizeof(LW_LIGHT));
	if(pEnt->m_nNumRegions)
	{
		//if(m_WorldMap.m_pRegions[pEnt->m_nRegions[0]].nLightcount)
		lt=m_WorldMap.m_pRegions[pEnt->m_nRegions[0]].pLights[0];
	}
	
	D3DLIGHT9 Light;
	Light.Type=D3DLIGHT_DIRECTIONAL;
	Light.Diffuse.r=lt.Color.r;
	Light.Diffuse.g=lt.Color.g;
	Light.Diffuse.b=lt.Color.b;
	Light.Diffuse.a=lt.Color.a;
	Light.Specular.a=Light.Specular.r=Light.Specular.g=Light.Specular.b=0.0f;
	Light.Ambient.a=Light.Ambient.r=Light.Ambient.g=Light.Ambient.b=0.3f;
	ML_Vec3Subtract((ML_VEC3*)&Light.Direction, &pEnt->m_v3Pos, &lt.v3Pos);
	
	#if 0
	ML_AABB aabb;
	aabb.v3Min=lt.v3Pos;
	aabb.v3Max=aabb.v3Min;
	aabb.v3Max.x+=0.25f;
	aabb.v3Max.y+=0.25f;
	aabb.v3Max.z+=0.25f;
	LD_DrawAABB(s_pDevice, &aabb, 0xFF00FF00);
	#endif
	s_pDevice->SetLight(0, &Light);
	s_pDevice->LightEnable(0, TRUE);
}

void CLWorld::Render()
{
	m_Camera.RenderForSkybox();
	CLMeshD3D::SetSkyboxRenderStates();
	m_SkyBox.Render();
	m_Camera.Render();
	
	s_pDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
	s_pDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
	s_pDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
	m_WorldMap.Render();
	
	s_pDevice->SetRenderState(D3DRS_LIGHTING, TRUE);
	CLMeshD3D::SetRenderStates();
	for(CLBase3DEntity* pEnt=m_pEntities; pEnt; pEnt=(CLBase3DEntity*)pEnt->GetNext())
	{
		SetEntityLight(pEnt);
		pEnt->Render();
	}
	
	#if 0
	lg_bool bRenderBlockAABB=LG_TRUE;
	lg_bool bRenderRegionAABB=LG_TRUE;
	lg_bool bRenderEntAABB=LG_TRUE;
	//For debug purposes we can draw some bounding volumes....
	s_pDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
	//For debug draw all geo block bounding volumes
	for(lg_dword i=0; i<m_WorldMap.m_nGeoBlockCount && bRenderBlockAABB; i++)
	{
		LD_DrawAABB(s_pDevice, &m_WorldMap.m_pGeoBlocks[i].aabbBlock, 0xFF0000FF);
	}
	
	for(lg_dword i=0; i<m_WorldMap.m_nRegionCount && bRenderRegionAABB; i++)
	{
		LD_DrawAABB(s_pDevice, &m_WorldMap.m_pRegions[i].aabbAreaBounds, 0xFFFFFF00);
	}
	
	for(CLBase3DEntity* pEnt=m_pEntities; pEnt && bRenderEntAABB; pEnt=(CLBase3DEntity*)pEnt->GetNext())
	{
		LD_DrawAABB(s_pDevice, &pEnt->m_aabbCurrent, 0xFFFF0000);
	}
	s_pDevice->SetRenderState(D3DRS_LIGHTING, TRUE);
	#endif
}

lg_bool CLWorld::Validate()
{
	lg_bool bResult=LG_TRUE;
	bResult=bResult&&m_WorldMap.Validate();
	return bResult;
}
void CLWorld::Invalidate()
{
	m_WorldMap.Invalidate();
}