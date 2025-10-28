#include "lw_client.h"
#include "lg_err.h"
#include "lx_sys.h"
#include "lg_func.h"
#include "lc_sys2.h"
#include "ld_sys.h"

CLWorldClnt::CLWorldClnt():
	m_Map(),
	m_nConnectType(CLNT_CONNECT_DISC),
	m_pEntList(LG_NULL),
	m_bRunning(LG_FALSE)
{
	
}

CLWorldClnt::~CLWorldClnt()
{
	//Technically we don't want to shut down here, but
	//just in case.
	//Shutdown();
}

void CLWorldClnt::Init(CLInput* pInput)
{
	Err_Printf("===Initializing Legacy World Client===");
	Err_IncTab();
	
	m_pInput=pInput;

	Err_DecTab();
	Err_Printf("======================================");
}

void CLWorldClnt::Shutdown()
{
	Err_Printf("===Destroying Legacy World Client===");
	Disconnect();
	Err_Printf("====================================");
}

lg_void CLWorldClnt::Broadcast()
{

}

lg_void CLWorldClnt::Receive()
{
	switch(m_nConnectType)
	{
	case CLNT_CONNECT_LOCAL:
	{
		//Interpret commands for the client:
		CLListStack* pCmds=&m_pSrvLocal->m_Clients[m_pSrvLocal->m_nClntRefs[this->m_nIDOnSrv]].m_CmdList;
		while(!pCmds->IsEmpty())
		{
			CLWCmdItem* pCmd=(CLWCmdItem*)pCmds->Pop();
			ProcessSrvCmd(pCmd);
			m_pSrvLocal->m_UnusedCmds.Push(pCmd);
		}
		break;
	}
	case CLNT_CONNECT_TCP:
		break;
	case CLNT_CONNECT_UDP:
		break;
	default:
	case CLNT_CONNECT_DISC:
		break;
	}
}

lg_void CLWorldClnt::ProcessSrvCmd(const CLWCmdItem* pCmd)
{
	switch(pCmd->Command)
	{
	case STCC_ENT_DESTROY:
		STCC_ENT_DESTROY_INFO* pInfo=(STCC_ENT_DESTROY_INFO*)pCmd->Params;
		RemoveEnt(pInfo->nEntID);
		break;
	}
}

lg_void CLWorldClnt::RemoveEnt(const lg_dword nUEID)
{
	DeleteRasterInfo(m_pEntList[nUEID].m_pRasterInfo);
	m_pEntList[nUEID].m_pRasterInfo=LG_NULL;
	m_EntsAll.Remove(&m_pEntList[nUEID]);
}

lg_void CLWorldClnt::Update()
{
	if(m_nConnectType==CLNT_CONNECT_DISC)
		return;
	
	Receive();
		
	for(LEntityClnt* pEnt = (LEntityClnt*)m_EntsAll.m_pFirst; pEnt; pEnt=(LEntityClnt*)pEnt->m_pNext)
	{
		pEnt->m_matPos=m_pSrvLocal->m_pEntList[pEnt->m_nUEID].m_matPos;
		pEnt->m_aabbBody=m_pSrvLocal->m_pEntList[pEnt->m_nUEID].m_aabbBody;
		pEnt->m_nAnimFlags1=m_pSrvLocal->m_pEntList[pEnt->m_nUEID].m_nAnimFlags1;
		pEnt->m_nAnimFlags2=m_pSrvLocal->m_pEntList[pEnt->m_nUEID].m_nAnimFlags2;
		pEnt->m_v3Vel=m_pSrvLocal->m_pEntList[pEnt->m_nUEID].m_v3Vel;
		memcpy(pEnt->m_v3Look, m_pSrvLocal->m_pEntList[pEnt->m_nUEID].m_v3Look, sizeof(ML_VEC3)*3);
		
		LEntRasterInfo* pRInfo=(LEntRasterInfo*)pEnt->m_pRasterInfo;
		pRInfo->m_MeshTree.SetAnimation(pEnt->m_nAnimFlags1);
		pRInfo->m_MeshTree.SetAnimation(pEnt->m_nAnimFlags2);
	}
	
	#if 1
	LEntityClnt* pEnt=&m_pEntList[m_nPCEnt];
	Err_MsgPrintf(
		"Ent %d: Pos: (%.3f, %.3f, %.3f) Vel: (%.3f, %.3f, %.3f)", 
		pEnt->m_nUEID, 
		pEnt->m_matPos._41,
		pEnt->m_matPos._42,
		pEnt->m_matPos._43,
		pEnt->m_v3Vel.x,
		pEnt->m_v3Vel.y,
		pEnt->m_v3Vel.z);
	#endif
	
	//Send information back to the server for the temp entity: This is only for testing,
	//in the future it will be sent in the appropriate method.
	LEntitySrv* pPCEnt=&m_pSrvLocal->m_pEntList[m_nPCEnt];
	pPCEnt->m_nCmdsActive=m_pInput->m_nCmdsActive[0];
	pPCEnt->m_nCmdsPressed=m_pInput->m_nCmdsPressed[0];
	pPCEnt->m_fAxis[0]=m_pInput->m_fAxis[0];
	pPCEnt->m_fAxis[1]=m_pInput->m_fAxis[1];
}

lg_void CLWorldClnt::SetCamera(ML_MAT* pMat)
{
	static ml_vec3 v3Eye, v3At, v3Up, v3Temp;
	static ml_mat matTemp;
	
	if(m_nConnectType==CLNT_CONNECT_DISC)
		return;
	
	LEntityClnt* pEnt=&m_pEntList[m_nPCEnt];
	
	LEntRasterInfo* pRInfo=(LEntRasterInfo*)pEnt->m_pRasterInfo;
	pRInfo->m_MeshTree.GetAttachTransform(&matTemp, 1);
	
	ML_MatMultiply(&matTemp, &matTemp, &pEnt->m_matPos);
	
	v3Eye=*(ml_vec3*)&matTemp._41;
	
	ML_Vec3Add(&v3At, &pEnt->m_v3Look[0], &v3Eye);
	v3Up=*(ml_vec3*)&pEnt->m_v3Look[1];
	
	#define FIRST_PERSON 0
	#if !FIRST_PERSON
	ML_Vec3Subtract(&v3Temp, &v3At, &v3Eye);
	ML_Vec3Scale(&v3Temp, &v3Temp, 3.0f);
	ML_Vec3Subtract(&v3Eye, &v3At, &v3Temp);
	#endif !FIRST_PERSON
	
	ML_MatLookAtLH(
		pMat,
		&v3Eye,
		&v3At,
		&v3Up);
	#if 0
	//ML_MAT matTemp;
	ML_VEC3 v3Look[4];
	//Copy the position:
	memcpy(&v3Look[0], &pEnt->m_matPos._41, sizeof(v3Look[0]));
	v3Look[0].y+=1.0f;
	//Calculate the Look At (Just Translate it):
	ML_Vec3Add(&v3Look[1], &pEnt->m_v3Look[0], &v3Look[0]);
	//Copy in the up vector:
	memcpy(&v3Look[2], &pEnt->m_v3Look[1], sizeof(v3Look[2]));
	//If we want the camera away from the object we scale a bit:
	ML_Vec3Subtract(&v3Look[3], &v3Look[1], &v3Look[0]);
	ML_Vec3Scale(&v3Look[3], &v3Look[3], 5.0f);
	ML_Vec3Subtract(&v3Look[0], &v3Look[1], &v3Look[3]);
	
	ML_MatLookAtLH(
		pMat,
		&v3Look[0], //Eye
		&v3Look[1], //At
		&v3Look[2]); //Up
	#endif
	#if 0	
	v3Look[0].x=3.0f;
	v3Look[0].y=1.0f;
	v3Look[0].z=-1.0f;
	
	v3Look[1].x=0.0f;
	v3Look[1].y=1.0f;
	v3Look[1].z=-1.0f;
	
	v3Look[2].x=0.0f;
	v3Look[2].y=1.0f;
	v3Look[2].z=0.0f;
	
	ML_MatLookAtLH(
		pMat,
		&v3Look[0], //Eye
		&v3Look[1], //At
		&v3Look[2]); //Up
	#endif
	
}

/* PRE: IDirect3DDevice9::BeginScene() must be called.
	POST: All elements of the scene are rasterized.  This
		includes the map and visible entities.
*/
lg_void CLWorldClnt::Render()
{
	if(m_nConnectType==CLNT_CONNECT_DISC)
		return;
		
	/*
	#define TESTLIGHT
	#ifdef TESTLIGHT
	D3DLIGHT9 Light={
		D3DLIGHT_DIRECTIONAL, //Type
		{1.0f, 1.0f, 1.0f, 0.0f},//Diffuse
		{0.0f, 0.0f, 0.0f, 0.0f},//Specular
		{0.75f, 0.5f, 0.25f, 0.0f},//Ambient
		{0.0f, 0.0f, 0.0f},//Position
		{-1.0f, -1.0f, 0.0f},//Direction
		0.0f,//Range
		0.0f,//Falloff
		0.0f,//Attenuation0
		0.0f,//Attenuation1
		0.0f,//Attenuation2
		0.0f,//Theta
		0.0f};//Phi
	
	s_pDevice->SetLight(0, &Light);
	s_pDevice->LightEnable(0, TRUE);
	D3DMATERIAL9 mtrl;
	memset( &mtrl, 0, sizeof(mtrl) );
	mtrl.Diffuse.r = mtrl.Ambient.r = 1.0f;
	mtrl.Diffuse.g = mtrl.Ambient.g = 1.0f;
	mtrl.Diffuse.b = mtrl.Ambient.b = 1.0f;
	mtrl.Diffuse.a = mtrl.Ambient.a = 1.0f;
	s_pDevice->SetMaterial(&mtrl);
	#else
	s_pDevice->SetRenderState(D3DRS_LIGHTING, LG_FALSE);
	#endif
*/
		
	D3DXMATRIX matT;
	SetCamera((ML_MAT*)&matT);
	
	ML_MatIdentity((ML_MAT*)&matT);
	ML_MatPerspectiveFovLH((ML_MAT*)&matT, D3DX_PI*0.25f, 4.0f/3.0f, 0.1f, 100.0f);
	//matT._11=2.0f/1.0f;
	//matT._22=2.0f/1.0f;
	s_pDevice->SetTransform(D3DTS_PROJECTION, &matT);
	
	/*
	ML_VEC3 v3Pos={0.0f, 0.0f, 0.0f};
	ML_VEC3 v3LookAt;
	ML_Vec3Subtract(&v3LookAt, &m_v3LookAt, &m_v3Pos);
	ML_MatLookAtLH(
		&m_matView,
		&v3Pos,
		&v3LookAt,
		&m_v3Up);
	*/
	
	matT._41=matT._42=matT._43=0;
	s_pDevice->SetTransform(D3DTS_VIEW, &matT);
	
	CLMeshLG::LM_SetSkyboxRenderStates();
	m_Skybox.Render();
	
	//s_pDevice->SetTransform(D3DTS_VIEW, (D3DMATRIX*)&m_matView);
	SetCamera((ML_MAT*)&matT);
	s_pDevice->SetTransform(D3DTS_VIEW, &matT);
	
	/*
	D3DVIEWPORT9 vp;
	D3DVIEWPORT9 vpOld;
	s_pDevice->GetViewport(&vpOld);
	vp.X=0;
	vp.Y=300;
	vp.Width=400;
	vp.Height=300;
	vp.MinZ=0.0f;
	vp.MaxZ=1.0f;
	s_pDevice->SetViewport(&vp);
	*/
	m_Map.Render();
	/*
	vp.X=400;
	vp.Y=300;
	vp.Width=400;
	vp.Height=300;
	vp.MinZ=0.0f;
	vp.MaxZ=1.0f;
	s_pDevice->SetViewport(&vp);
	*/
	
	CLMeshLG::LM_SetRenderStates();
	
	//Technically we should only render visible entities.
	for(LEntityClnt* pEnt = (LEntityClnt*)m_EntsAll.m_pFirst; pEnt; pEnt=(LEntityClnt*)pEnt->m_pNext)
	{
		LEntRasterInfo* pRInfo=(LEntRasterInfo*)pEnt->m_pRasterInfo;
		D3DXMATRIX matTemp=*(D3DXMATRIX*)&m_pSrvLocal->m_pEntList[pEnt->m_nUEID].m_matPos;
		
		pRInfo->m_MeshTree.Update(m_pSrvLocal->m_Timer.GetFElapsed());
		pRInfo->m_MeshTree.Render((ML_MAT*)&matTemp);
		
		#if 0 //_DEBUG
		LD_DrawAABB(s_pDevice, &pEnt->m_aabbBody, 0xFF00FF00);
		#endif _DEBUG
	}
	
	#if 0 // _DEBUG
	lg_dword nDebugFlags=0;
	//LG_SetFlag(nDebugFlags, m_Map.MAPDEBUG_HULLTRI);
	LG_SetFlag(nDebugFlags, m_Map.MAPDEBUG_HULLAABB);
	LG_SetFlag(nDebugFlags, m_Map.MAPDEBUG_WORLDAABB);
	m_Map.RenderAABBs(nDebugFlags);
	#endif _DEBUG
	
	//s_pDevice->SetViewport(&vpOld);
}

/* PRE: pSrv is the local server.
	POST: The client is connected to the server,
		and ready for updating.  If a connection
		could not be esablished then an error message
		is printed.
*/
lg_void CLWorldClnt::ConnectLocal(CLWorldSrv* pSrv)
{
	Disconnect();
	
	if(!pSrv->IsRunning())
	{
		Err_Printf("LEGACY CLIENT: Can't connect to the specified server.");
		return;
	}
	
	Err_Printf("Connecting to \"%s\" ...", pSrv->m_szName);
	Err_Printf("Initializing Entity List...");
	m_nMaxEnts=pSrv->m_nMaxEnts;
	Err_Printf("Allocating memory for %u entities...", m_nMaxEnts);
	m_pEntList=new LEntityClnt[m_nMaxEnts];
	if(!m_pEntList)
		return;
	Err_Printf("%u bytes allocated (%uMB).", m_nMaxEnts*sizeof(LEntityClnt), m_nMaxEnts*sizeof(LEntityClnt)/1048576);
	//Assign UEIDs
	for(lg_dword i=0; i<m_nMaxEnts; i++)
	{
		m_pEntList[i].m_pRasterInfo=LG_NULL;
		m_pEntList[i].m_nUEID=i;
	}
	
	//Establish the connection, and obtain the pc entitiy.
	if(!pSrv->AcceptLocalConnect(&m_nPCEnt, &m_nIDOnSrv, "Default Client"))
		Disconnect();
	
	//Go ahead and create entities for the ones that exist.
	//Note that in the server intelligent entities are separated
	//from inert ones, but in the client it doesn't matter
	//so all entities are stored in the same list.  (In the
	//future, however, they may be dived into lists based
	//on their region, fut faster rasterization.
	m_EntsAll.Clear();
	//We need to initially duplicate all inert entities...
	for(LEntitySrv* pEnt=(LEntitySrv*)pSrv->m_EntsInert.m_pFirst; pEnt; pEnt=(LEntitySrv*)pEnt->m_pNext)
	{
		LEntityClnt* pEntCli = &m_pEntList[pEnt->m_nUEID];
		UpdateEntSrvToCli(pEntCli, pEnt);
		m_EntsAll.Push(pEntCli);
	}
	
	//And all intelligent entities...
	for(LEntitySrv* pEnt=(LEntitySrv*)pSrv->m_EntsInt.m_pFirst; pEnt; pEnt=(LEntitySrv*)pEnt->m_pNext)
	{
		LEntityClnt* pEntCli = &m_pEntList[pEnt->m_nUEID];
		UpdateEntSrvToCli(pEntCli, pEnt);
		m_EntsAll.Push(pEntCli);
	}
	
		
	m_pSrvLocal=pSrv;
	m_Map.Init(s_pDevice, &pSrv->m_Map);
	m_Skybox.Load(pSrv->m_szSkyboxFile, pSrv->m_szSkyboxSkinFile);
	//m_Map.Validate();
	m_nConnectType=CLNT_CONNECT_LOCAL;
		
}

lg_void CLWorldClnt::Disconnect()
{
	Err_Printf("Disconnecting...");
	m_Skybox.Unload();
	m_Map.UnInit();
	m_nConnectType=CLNT_CONNECT_DISC;
	ClearEntities();
	LG_SafeDeleteArray(m_pEntList);
}

lg_void CLWorldClnt::ClearEntities()
{
	for(LEntityClnt* pEnt=(LEntityClnt*)m_EntsAll.m_pFirst; pEnt; pEnt=(LEntityClnt*)pEnt->m_pNext)
	{
		DeleteRasterInfo(pEnt->m_pRasterInfo);
		pEnt->m_pRasterInfo=LG_NULL;
	}
	m_EntsAll.Clear();
}

lg_void CLWorldClnt::Validate()
{
	m_Map.Validate();
}

lg_void CLWorldClnt::Invalidate()
{
	m_Map.Invalidate();
}

/* UpdateEntSrvToCli
	PRE: pDest and pSrc should have the same UEID.
	POST: All client relative data is copied from pSrc
		to pDest.  This method is primarily for local
		connections only, and not network.
*/
lg_void CLWorldClnt::UpdateEntSrvToCli(LEntityClnt* pDest, const LEntitySrv* pSrc)
{		
	//pDest->m_nUEID=       pSrc->m_nUEID;
	pDest->m_matPos=      pSrc->m_matPos;
	pDest->m_nFlags1=     pSrc->m_nFlags1;
	pDest->m_nFlags2=     pSrc->m_nFlags2;
	pDest->m_nMode1=      pSrc->m_nMode1;
	pDest->m_nMode2=      pSrc->m_nMode2;
	pDest->m_nNumRegions= pSrc->m_nNumRegions;
	memcpy(pDest->m_nRegions, pSrc->m_nRegions, sizeof(pDest->m_nRegions));
	pDest->m_v3AngVel=    pSrc->m_v3AngVel;
	pDest->m_v3Thrust=    pSrc->m_v3Thrust;
	pDest->m_v3Vel=       pSrc->m_v3Vel;
	pDest->m_aabbBody=    pSrc->m_aabbBody;
	pDest->m_nAnimFlags1= pSrc->m_nAnimFlags1;
	pDest->m_nAnimFlags2= pSrc->m_nAnimFlags2;
	memcpy(pDest->m_v3Look, pSrc->m_v3Look, sizeof(ML_VEC3)*3);
	
	//Create the raster info, hopefully not done here in the future
	//as new is called several times for this.
	pDest->m_pRasterInfo=CreateRasterInfo(pSrc->m_szObjScript);
	//DeleteRasterInfo(pDest->m_pRasterInfo);
}

lg_void* CLWorldClnt::CreateRasterInfo(const lg_path szObjScript)
{
	lx_object* pObj=LX_LoadObject(szObjScript);
	if(!pObj)
		return LG_NULL;
	
	LEntRasterInfo* pRasterInfo = new LEntRasterInfo;
	//pRasterInfo->m_fScale=pObj->fScale;
	pRasterInfo->m_MeshTree.Load(pObj->szModelFile);
	pRasterInfo->m_MeshTree.SetAnimation(0, 0, 0, 2.0f, 0.0f);
	LX_DestroyObject(pObj);
	
	return pRasterInfo;
}
lg_void  CLWorldClnt::DeleteRasterInfo(lg_void* pInfo)
{
	if(!pInfo)
		return;
		
	LEntRasterInfo* pRasterInfo=(LEntRasterInfo*)pInfo;
	//LM_DeleteMeshNodes(pRasterInfo->m_pMeshNodes);
	//LG_SafeDeleteArray(pRasterInfo->m_pSkels);
	pRasterInfo->m_MeshTree.Unload();
	LG_SafeDelete(pRasterInfo);
}

