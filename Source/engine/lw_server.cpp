#include "lw_server.h"
#include "lg_err.h"
#include "lg_cvars.h"
#include "lp_legacy.h"
#include "lp_newton2.h"
#include "lp_physx2.h"
#include "lg_mmgr.h"
#include "lg_func.h"
#include "lw_ai.h"

#include "lx_sys.h"

CLWorldSrv::CLWorldSrv():
	m_pEntList(LG_NULL),
	m_pPhys(LG_NULL),
	m_nFlags(0)
{

}

CLWorldSrv::~CLWorldSrv()
{
	//Technically we don't want to shut down the
	//server here, but just in case.
	Shutdown();
}

lg_void CLWorldSrv::LoadEntTemplate(lg_cstr szTemplateScript)
{
	Err_Printf("Loading entity template:");
	Err_IncTab();
	
	UnloadEntTemplate();
	
	lg_dword nCount=0;
	lx_ent_template* pTemplate=LX_LoadEntTemplate(szTemplateScript, &nCount);
	
	if(pTemplate)
	{
		Err_Printf("%d entity types:", nCount);
		
		for(lg_dword i=0; i<nCount; i++)
		{
			Err_Printf("Type %d: \"%s\".", pTemplate[i].nID, pTemplate[i].szScript);
		}
		
		Err_Printf("Creating templates...");
		m_nNumTemplateEnts=nCount;
		m_pTemplateEnt=new EntTemplate[m_nNumTemplateEnts];
		LG_ASSERT(m_pTemplateEnt, "Out of memory.");
		
		for(lg_dword i=0; i<m_nNumTemplateEnts; i++)
		{
			SetupEntityFromScript(&m_pTemplateEnt[i].Ent, &m_pTemplateEnt[i].BdyInfo, pTemplate[i].szScript);
		}
		
		LX_DestroyEntTemplate(pTemplate);
		Err_Printf("Done.");
	}
	
	Err_DecTab();
}

lg_void CLWorldSrv::UnloadEntTemplate()
{
	LG_SafeDeleteArray(m_pTemplateEnt);
	m_nNumTemplateEnts=0;
}

lg_void CLWorldSrv::Broadcast()
{
	//Everything in the m_AllClients should be broadcast to
	//all the connected clients:
	for(lg_dword i=0; i<m_nNumClnts; i++)
	{
		ClntData* pClnt = &m_Clients[m_nClntRefs[i]];
		
		switch(pClnt->nType)
		{
		case CLNT_CONNECT_LOCAL:
		{
			//We don't need to do anything with the commands
			//that are specifically for the client, as they 
			//will be processed by the client, we do however
			//need to copy in the commands that are for all
			//clients.
			for(CLWCmdItem* pCmd=(CLWCmdItem*)m_AllClients.m_CmdList.m_pFirst; pCmd; pCmd=(CLWCmdItem*)pCmd->m_pNext)
			{
				CLWCmdItem* pDestCmd=(CLWCmdItem*)m_UnusedCmds.Pop();
				if(!pDestCmd)
				{
					Err_Printf("No commands available for local broadcast (%s.%d).", __FILE__, __LINE__);
					break;
				}
				pDestCmd->Command=pCmd->Command;
				pDestCmd->Size=pCmd->Size;
				memcpy(pDestCmd->Params, pCmd->Params, pCmd->Size);
				pClnt->m_CmdList.Push(pDestCmd);
			}
			break;
		}
		case CLNT_CONNECT_TCP:
		case CLNT_CONNECT_UDP:
		default:
			break;
		}
	}
	//Now clear the list for all commands.
	while(!m_AllClients.m_CmdList.IsEmpty())
	{
		m_UnusedCmds.Push(m_AllClients.m_CmdList.Pop());
	}
}
lg_void CLWorldSrv::Receive()
{
}

lg_bool CLWorldSrv::AcceptLocalConnect(lg_dword* pPCEnt, lg_dword* pID, lg_cstr szName)
{
	lg_bool bRes=LG_FALSE;
	Err_Printf("Obtained local connect request from: \"%s\" @ localhost", szName);
	Err_IncTab();
	
	if(m_nNumClnts>=LSRV_MAX_CLIENTS)
	{
		Err_Printf("Cannot establish connect %d/%d clients connected.", m_nNumClnts, LSRV_MAX_CLIENTS);
		bRes=LG_FALSE;
	}
	else
	{
		lg_dword nSlot=0xFF;
		//Find the first avaialbe slot for the client:
		for(lg_dword i=0; nSlot==0xFF && i<LSRV_MAX_CLIENTS; i++)
		{
			lg_bool bFound=LF_FALSE;
			for(lg_dword j=0; j<m_nNumClnts; j++)
			{
				if(i==m_nClntRefs[j])
					bFound=LG_TRUE;
			}
			
			if(!bFound)
				nSlot=i;
		}
		
		if(nSlot==0xFF)
		{
			Err_Printf("Cannot establish connection, no open data structure found.");
			bRes=LG_FALSE;
		}
		else
		{
			m_nClntRefs[m_nNumClnts++]=nSlot;
			
			Err_Printf("Connecting %s to structure %d.", szName, nSlot);
			m_Clients[nSlot].nClntID=nSlot;
			m_Clients[nSlot].nType=CLNT_CONNECT_LOCAL;
			LG_strncpy(m_Clients[nSlot].szName, szName, LG_MAX_STRING);
			ml_mat matTemp;
			ML_MatTranslation(&matTemp, 0.0f, 0.0f, 0.0f);
			m_Clients[nSlot].nClntEnt=AddEntity("/dbase/objects/jack_basic.xml", &matTemp);
			*pPCEnt=m_Clients[nSlot].nClntEnt;
			*pID=m_Clients[nSlot].nClntID;
			bRes=LG_TRUE;
		}
	}
	
	
	Err_DecTab();
	
	return bRes;
}

lg_void CLWorldSrv::Init()
{
	//If we initialize the server and it was already
	//running we shutdown first.
	if(IsRunning())
		Shutdown();
		
	Err_Printf("===Initializing Legacy World Server===");
	Err_IncTab();
	
	m_pTemplateEnt=LG_NULL;
	m_nNumTemplateEnts=0;
	
	m_szName=CV_Get(CVAR_srv_Name)->szValue;
	Err_Printf("SERVER \"%s\" @ localhost", m_szName);
	InitEnts();
	Err_Printf("Initializing timer...");
	m_Timer.Initialize();
	Err_Printf("Initialzing phsycis engine...");
	switch((PHYS_ENGINE)CV_Get(CVAR_srv_PhysEngine)->nValue)
	{
	default:
	case PHYS_ENGINE_LEGACY:
		Err_Printf("Using Legacy physics engine.");
		m_pPhys=(CLPhys*)new CLPhysLegacy();
		break;
#if L3D_WITH_NEWTONGD_PHYSICS
	case PHYS_ENGINE_NEWTON:
		Err_Printf("Using Newton Game Dynamics physics engine.");
		m_pPhys=(CLPhys*)new CLPhysNewton();
		break;
#endif
#if 0
	case PHYS_ENGINE_PHYSX:
		Err_Printf("Using NVIDIA PHYSX physics engine.");
		m_pPhys=(CLPhys*)new CLPhysPhysX();
		break;
#endif
	} 
	LG_ASSERT(m_pPhys, LG_TEXT("Physics engine was not created."));
	m_pPhys->Init(m_nMaxEnts);
	
	//Initialize the ai:
	m_AIMgr.LoadAI(CV_Get(CVAR_lg_GameLib)->szValue);
	
	m_nEntKillCount=0;
	
	m_Timer.Update();
	//m_bRunning=LG_TRUE;
	m_nFlags=LSRV_FLAG_RUNNING;
	m_nNumClnts=0;
	
	//Initialize the command sturctuers:
	m_pCmdList=new CLWCmdItem[LSRV_MAX_CMDS];
	LG_ASSERT(m_pCmdList, "Out of memory.");
	m_UnusedCmds.Init(m_pCmdList, LSRV_MAX_CMDS, sizeof(CLWCmdItem));
	
	
	
	//A Test:
	LoadEntTemplate("/dbase/scripts/basic_ent_template.xml");
	
	
	Err_DecTab();
	Err_Printf("======================================");
}

lg_void CLWorldSrv::Shutdown()
{
	//We only need to shutdown the server if it was running.
	if(IsRunning())
	{
		Err_Printf("===Destroying Legacy World Server===");
		DestroyEnts();
		m_pPhys->Shutdown();
		LG_SafeDelete(m_pPhys);
		m_nFlags=0;
		m_Map.Unload();
		LG_SafeDeleteArray(m_pCmdList);
		UnloadEntTemplate();
		Err_Printf("====================================");
	}
}

lg_bool CLWorldSrv::SetupEntityFromScript(lg_srv_ent* pEnt, lp_body_info* pBdyInfo, lg_cstr szScript)
{
		
	lx_object* pObj=LX_LoadObject(szScript);
	if(!pObj)
		return LG_FALSE;
	
	LG_strncpy(pEnt->m_szObjScript, szScript, LG_MAX_PATH);
	pEnt->m_fMass=pObj->fMass;
	ML_MatIdentity(&pEnt->m_matPos);
	pEnt->m_v3Thrust.x=0.0f;
	pEnt->m_v3Thrust.y=0.0f;
	pEnt->m_v3Thrust.z=0.0f;
	pEnt->m_v3Impulse.x=0.0f;
	pEnt->m_v3Impulse.y=0.0f;
	pEnt->m_v3Impulse.z=0.0f;
	pEnt->m_v3Torque.x=0.0f;
	pEnt->m_v3Torque.y=0.0f;
	pEnt->m_v3Torque.z=0.0f;
	pEnt->m_v3AngVel.x=0.0f;
	pEnt->m_v3AngVel.y=0.0f;
	pEnt->m_v3AngVel.z=0.0f;
	pEnt->m_v3Vel.x=0.0f;
	pEnt->m_v3Vel.y=0.0f;
	pEnt->m_v3Vel.z=0.0f;
	pEnt->m_v3ExtFrc.x=0.0f;
	pEnt->m_v3ExtFrc.x=0.0f;
	pEnt->m_v3ExtFrc.x=0.0f;
	pEnt->m_nFlags1=0;
	pEnt->m_nFlags2=0;
	pEnt->m_nMode1=0;
	pEnt->m_nMode2=0;
	pEnt->m_nNumRegions=0;
	pEnt->m_nRegions[0]=0;
	pEnt->m_nAnimFlags1=0;
	pEnt->m_nAnimFlags2=0;
	pEnt->m_nCmdsActive=0;
	pEnt->m_nCmdsPressed=0;
	pEnt->m_fAxis[0]=0.0f;
	pEnt->m_fAxis[1]=0.0f;
	pEnt->m_nFuncs=0;
	//Setup AI:
	pEnt->m_pAI=m_AIMgr.GetAI(pObj->szAiFunction);
	
	//Phsyics flags:
	switch(pObj->nShape)
	{
	default:
	case PHYS_SHAPE_BOX:
		pEnt->m_nPhysFlags=pEnt->EF_PHYS_BOX;
		break;
	case PHYS_SHAPE_CAPSULE:
		pEnt->m_nPhysFlags=pEnt->EF_PHYS_CAPSULE;
		break;
	case PHYS_SHAPE_SPHERE:
		pEnt->m_nPhysFlags=pEnt->EF_PHYS_SPHERE;
		break;
	case PHYS_SHAPE_CYLINDER:
		pEnt->m_nPhysFlags=pEnt->EF_PHYS_CYLINDER;
		break;
	}
	
	//Calculate the body size
	//The base bounding volume is set about the center of
	//the XZ plane and the y lenght starts at zero 
	//(so by default an object at 0, 0, 0 would be standing
	//directly on and in the center of the XZ plane), the
	//offset of the shape is then added to this.
	pEnt->m_aabbBody.v3Min.x=-pObj->fRadius;
	pEnt->m_aabbBody.v3Min.y=0;
	pEnt->m_aabbBody.v3Min.z=-pObj->fRadius;
	pEnt->m_aabbBody.v3Max.x=pObj->fRadius;
	pEnt->m_aabbBody.v3Max.y=pObj->fHeight;
	pEnt->m_aabbBody.v3Max.z=pObj->fRadius;
	ML_Vec3Add(&pEnt->m_aabbBody.v3Min, &pEnt->m_aabbBody.v3Min, (ML_VEC3*)pObj->fShapeOffset);
	ML_Vec3Add(&pEnt->m_aabbBody.v3Max, &pEnt->m_aabbBody.v3Max, (ML_VEC3*)pObj->fShapeOffset);
	
	//Based off the flag gotten from the AI we'll assign
	//the intelligence flag.
	if(LG_CheckFlag(pEnt->m_pAI->PhysFlags(), AI_PHYS_INT))
	{
		pEnt->m_nFlags1|=LEntitySrv::EF_1_INT;
	}
	else
	{
		pEnt->m_nFlags1|=LEntitySrv::EF_1_INERT;
	}
	
	pBdyInfo->m_fMass=pEnt->m_fMass;
	pBdyInfo->m_aabbBody=pEnt->m_aabbBody;
	pBdyInfo->m_matPos=pEnt->m_matPos;
	pBdyInfo->m_nAIPhysFlags=0;
	pBdyInfo->m_nShape=(PHYS_SHAPE)pObj->nShape;
	memcpy(pBdyInfo->m_fShapeOffset, pObj->fShapeOffset, sizeof(lg_float)*3);
	memcpy(pBdyInfo->m_fMassCenter, pObj->fMassCenter, sizeof(lg_float)*3);
	pBdyInfo->m_nAIPhysFlags=pEnt->m_pAI->PhysFlags();
	
	LX_DestroyObject(pObj);
	
	return LG_TRUE;
}

lg_void CLWorldSrv::SetupEntFromTemplate(lg_srv_ent* pEnt, lp_body_info* pBdyInfo, const lg_dword nTemplate)
{	
	lg_srv_ent* pEntSrc = &m_pTemplateEnt[nTemplate].Ent;
	lp_body_info* pBdyInfoSrc=&m_pTemplateEnt[nTemplate].BdyInfo;
	
	LG_strncpy(pEnt->m_szObjScript, pEntSrc->m_szObjScript, LG_MAX_PATH);
	pEnt->m_fMass=pEntSrc->m_fMass;
	pEnt->m_matPos=pEntSrc->m_matPos;
	pEnt->m_v3Thrust=pEntSrc->m_v3Thrust;
	pEnt->m_v3Impulse=pEntSrc->m_v3Impulse;
	pEnt->m_v3Torque=pEntSrc->m_v3Torque;
	pEnt->m_v3AngVel=pEntSrc->m_v3AngVel;
	pEnt->m_v3Vel=pEntSrc->m_v3Vel;
	pEnt->m_v3ExtFrc=pEntSrc->m_v3ExtFrc;
	pEnt->m_nFlags1=pEntSrc->m_nFlags1;
	pEnt->m_nFlags2=pEntSrc->m_nFlags2;
	pEnt->m_nMode1=pEntSrc->m_nMode1;
	pEnt->m_nMode2=pEntSrc->m_nMode2;
	pEnt->m_nNumRegions=0;
	pEnt->m_nRegions[0]=0;
	pEnt->m_nAnimFlags1=pEntSrc->m_nAnimFlags1;
	pEnt->m_nAnimFlags2=pEntSrc->m_nAnimFlags1;
	pEnt->m_nCmdsActive=0;
	pEnt->m_nCmdsPressed=0;
	pEnt->m_fAxis[0]=0.0f;
	pEnt->m_fAxis[1]=0.0f;
	pEnt->m_nFuncs=pEntSrc->m_nFuncs;
	//Setup AI:
	pEnt->m_pAI=pEntSrc->m_pAI;
	pEnt->m_nPhysFlags=pEntSrc->m_nPhysFlags;
	pEnt->m_aabbBody=pEntSrc->m_aabbBody;
	
	pBdyInfo->m_fMass=pBdyInfoSrc->m_fMass;
	pBdyInfo->m_aabbBody=pBdyInfoSrc->m_aabbBody;
	pBdyInfo->m_matPos=pEnt->m_matPos;
	pBdyInfo->m_nAIPhysFlags=pBdyInfoSrc->m_nAIPhysFlags;
	pBdyInfo->m_nShape=pBdyInfoSrc->m_nShape;
	memcpy(pBdyInfo->m_fShapeOffset, pBdyInfoSrc->m_fShapeOffset, sizeof(lg_float)*3);
	memcpy(pBdyInfo->m_fMassCenter, pBdyInfoSrc->m_fMassCenter, sizeof(lg_float)*3);
	pBdyInfo->m_nAIPhysFlags=pEnt->m_pAI->PhysFlags();
}

/* PRE: Server inititialzed.
	POST: Adds a new entity to the world server (if room for
		more entities is available.  Information about the entity
		is based upon scripts.  Scripts must be properly formed
		or a unpredictable functional entity may be created.
		In the future there will also be an AddClonedEntity, that
		will create an entity based off another entity, in that way
		scripts files will not need to be loaded.
		
	NOTE: This function isn't even close to complection as it
		doesn't really use all the script information yet, and
		the physics engine is not completed, which is required
		for full functionality.
*/
lg_dword CLWorldSrv::AddEntity(lf_path szObjScript, ML_MAT* matPos)
{
	//No point in adding an ent if the server isn't running...
	if(!IsRunning())
		return -1;

	LEntitySrv* pNew = (LEntitySrv*)m_EntsBlank.Pop();
	if(!pNew)
	{
		return -1;
	}
	lp_body_info bdyinfo;
	if(SetupEntityFromScript(pNew, &bdyinfo, szObjScript))
	{
		pNew->m_matPos=*matPos;
		bdyinfo.m_matPos=*matPos;
		pNew->m_pPhysBody=m_pPhys->AddBody(pNew, &bdyinfo);
		
		//LX_DestroyObject(pObj);
		//Based off the flag gotten from the AI we'll insert
		//the entity into either the intelligent list or the
		//inert list.
		if(LG_CheckFlag(pNew->m_pAI->PhysFlags(), AI_PHYS_INT))
		{
			m_EntsInt.Push(pNew);
		}
		else
		{
			m_EntsInert.Push(pNew);
		}
		
		pNew->m_pAI->Init(pNew);
		return pNew->m_nUEID;
	}
	else
	{
		return -1;
	}
}

lg_dword CLWorldSrv::AddEntity(lg_dword nTemplate, ML_MAT* matPos)
{
	if(!IsRunning() || nTemplate>=m_nNumTemplateEnts)
		return -1;
		
	lg_srv_ent* pNew=(lg_srv_ent*)m_EntsBlank.Pop();
	if(!pNew)
		return -1;
		
	lp_body_info BdyInfo;
	
	SetupEntFromTemplate(pNew, &BdyInfo, nTemplate);

	pNew->m_matPos=*matPos;
	BdyInfo.m_matPos=*matPos;
	pNew->m_pPhysBody=m_pPhys->AddBody(pNew, &BdyInfo);
	
	//Based off the flag gotten from the AI we'll insert
	//the entity into either the intelligent list or the
	//inert list.
	if(LG_CheckFlag(pNew->m_pAI->PhysFlags(), AI_PHYS_INT))
	{
		m_EntsInt.Push(pNew);
	}
	else
	{
		m_EntsInert.Push(pNew);
	}
	
	pNew->m_pAI->Init(pNew);
	return pNew->m_nUEID;
}

lg_void CLWorldSrv::RemoveEnt(const lg_dword nID)
{
	LEntitySrv* pEnt=&m_pEntList[nID];
	
	//Also need code so that client will know what entity has been
	//removed.
	
	if(LG_CheckFlag(pEnt->m_nFlags1, LEntitySrv::EF_1_INT))
	{
		m_EntsInt.Remove(pEnt);
	}
	else
	{
		m_EntsInert.Remove(pEnt);
	}
	
	m_pPhys->RemoveBody(pEnt->m_pPhysBody);
		
	pEnt->m_nFlags1=LEntitySrv::EF_1_BLANK;
	m_EntsBlank.Push(pEnt);
	//Entity removal needs to be sent to all clients:
	CLWCmdItem* pCmd=(CLWCmdItem*)m_UnusedCmds.Pop();
	if(!pCmd)
	{
		Err_Printf("Could not send command, command limit too small. (%s.%d)", __FILE__, __LINE__);
	}
	else
	{
		STCC_ENT_DESTROY_INFO* pInfo=(STCC_ENT_DESTROY_INFO*)pCmd->Params;
		pCmd->Command=STCC_ENT_DESTROY;
		pCmd->Size=sizeof(STCC_ENT_DESTROY_INFO);
		pInfo->nEntID=nID;
		//Push the command:
		m_AllClients.m_CmdList.Push(pCmd);
	}
}

lg_void CLWorldSrv::Pause()
{
	LG_SetFlag(m_nFlags, LSRV_FLAG_PAUSED);
}
lg_void CLWorldSrv::Resume()
{
	LG_UnsetFlag(m_nFlags, LSRV_FLAG_PAUSED);
	m_Timer.Update();
	m_Timer.Update();
	m_pPhys->Simulate(m_Timer.GetFElapsed());
}
lg_void CLWorldSrv::TogglePause()
{
	if(LG_CheckFlag(m_nFlags, LSRV_FLAG_PAUSED))
		Resume();
	else
		Pause();
}

lg_void CLWorldSrv::ProcessEntFuncs(LEntitySrv* pEnt)
{
	if(!pEnt->m_nFuncs)
		return;
		
	
	if(LG_CheckFlag(pEnt->m_nFuncs, AI_FUNC_DELETE))
	{
		//When removing an entity, we add it to the kill list.
		m_nEntKillList[m_nEntKillCount++]=pEnt->m_nUEID;	
	}
	
}

/* PRE: World server should be initialized.
	POST: The world is processed by the server, that means all
	AI (including client updates) is processed, physics are
	processed.
*/
lg_void CLWorldSrv::ProcessServer()
{
	//The server doesn't get processed if it isn't running.
	if(!IsRunning() || LG_CheckFlag(m_nFlags, LSRV_FLAG_PAUSED))
		return;
		
	//First thing to do is update the time.
	m_Timer.Update();
	#if 0
	//Process the AI.
	AI_DATA aidata;
	aidata.nTime=m_Timer.GetTime();
	aidata.fTimeStepSec=m_Timer.GetFElapsed();
	aidata.pPhysWorld=m_pPhys;
	#endif
	GAME_AI_GLOBALS AiGlobals={m_Timer.GetFElapsed()};
	m_AIMgr.SetGlobals(&AiGlobals);
	//In the future we will only call the ai function for entities
	//that are within range of an area of interest, for now it
	//is called for all intelligent entities.
	//Also note that the aidata getting passed right now
	//is strictly preliminary and a more robust version will
	//exist as this project progresses.
	
	for(LEntitySrv* pEnt=(LEntitySrv*)m_EntsInt.m_pFirst; pEnt; pEnt=(LEntitySrv*)pEnt->m_pNext)
	{
		//Run the AI routine for all entities, not that an AI routine will
		//always exist, even if it is the default one (in which nothing occurs).
		//We only call the PrePhys method here, the PostPhys method is called from
		//the physics engine, and it is only called on entities that are being
		//processed by the physics engine, so PostPhys is not necessarily always
		//called.
		LG_UnsetFlag(pEnt->m_nAnimFlags1, LWAI_ANIM_CHANGE);
		LG_UnsetFlag(pEnt->m_nAnimFlags2, LWAI_ANIM_CHANGE);
		pEnt->m_nFuncs=0;
		pEnt->m_pAI->PrePhys(pEnt);
		ProcessEntFuncs(pEnt);
		#if 0
		if(pEnt->m_nFuncs)
		{
			if(LG_CheckFlag(pEnt->m_nFuncs, AI_FUNC_DELETE))
			{
				//The procedure for removing an entity isn't too complex,
				//but it isn't as simple as calling RemoveEnt on the entities
				//id, because we are currently looping through all intelligent
				//entities, so we have to save where we were in the list,
				//so we don't end up skipping any entities, or ending up
				//with a null reference.
				lg_dword nID=pEnt->m_nUEID;
				pEnt=(LEntitySrv*)pEnt->m_pPrev;
				
				RemoveEnt(nID);
				
				//If the reference we saved was null, that means that the
				//entity we removed was at the beginning of the list, and
				//we need to reset the current ent to the beginning of the
				//list.
				if(pEnt==LG_NULL)
				{
					pEnt=(LEntitySrv*)m_EntsInt.m_pFirst;
				}
				
			}
		}
		#endif

	}
	
	//Process the physics engine.
	//Note that the physics engine stores information
	//about the server bodies, so we never have to 
	//explicitley update the physics bodies from the
	//server.  For this reason an entities ai needs to update
	//it's m_v3Thrust, and m_v3Torque members to represent
	//the amount of force applied.  This need to be set every
	//frame as the physics engine resets them to zero after
	//Simulate is called.  Note that bodies that have "fallen
	//asleep" will not be affect by changes to these values,
	//bodies can be awakened when a moving bodie bumps into
	//them.
	m_pPhys->Simulate(33.0f);//m_Timer.GetFElapsed());
	
	//Process the kill list, removing all entities in it.
	while(m_nEntKillCount)
	{
		RemoveEnt(m_nEntKillList[--m_nEntKillCount]);
	}
	
	Broadcast();
}

/* PRE: Server Running, valid level script specified.
	POST: The level is set up, not for loading savegames,
	or transitioning between hubs.
*/
lg_void CLWorldSrv::LoadLevel(lg_str szLevelScriptFile)
{
	//Can't load a level if the server isn't running.
	if(!IsRunning())
		return;
	
	lx_level* pLevel=LX_LoadLevel(szLevelScriptFile);
	if(!pLevel)
		return;
	#if 1
	Err_Printf("SERVER->LoadLevel:");
	Err_Printf("   Map: \"%s\"", pLevel->szMapFile);
	Err_Printf("   SkyBox: \"%s\"", pLevel->szSkyBoxFile);
	Err_Printf("   SkyBox Skin: \"%s\"", pLevel->szSkyBoxSkin);
	Err_Printf("   Music: \"%s\"", pLevel->szMusicFile);
	Err_Printf("   Actors: %d", pLevel->nActorCount);
	for(lg_dword i=0; i< pLevel->nActorCount; i++)
	{
		Err_Printf("      Script: \"%s\"", pLevel->pActors[i].szObjFile);
		Err_Printf("      Position: (%f, %f, %f)", pLevel->pActors[i].fPosition[0], pLevel->pActors[i].fPosition[1], pLevel->pActors[i].fPosition[2]);
	}
	Err_Printf("   Objects: %d", pLevel->nObjCount);
	for(lg_dword i=0; i< pLevel->nObjCount; i++)
	{
		Err_Printf("      Script: \"%s\"", pLevel->pObjs[i].szObjFile);
		Err_Printf("      Position: (%f, %f, %f)", pLevel->pObjs[i].fPosition[0], pLevel->pObjs[i].fPosition[1], pLevel->pObjs[i].fPosition[2]);
	}
	#endif
	
	ClearEnts();
	LoadMap(pLevel->szMapFile);
	ml_vec3 v3Grav={0.0f, -9.8f, 0.0f};
	m_pPhys->SetGravity(&v3Grav);
	
	//Add entities...
	ML_MAT matPos;
	ML_MatIdentity(&matPos);
	
	for(lg_dword i=0; i<pLevel->nObjCount; i++)
	{
		memcpy(&matPos._41, pLevel->pObjs[i].fPosition, sizeof(lg_float)*3);
		AddEntity(pLevel->pObjs[i].szObjFile, &matPos);
	}
	
	for(lg_dword i=0; i<pLevel->nActorCount; i++)
	{
		memcpy(&matPos._41, pLevel->pActors[i].fPosition, sizeof(lg_float)*3);
		AddEntity(pLevel->pActors[i].szObjFile, &matPos);
	}
	
	LG_strncpy(m_szSkyboxFile, pLevel->szSkyBoxFile, LG_MAX_PATH);
	LG_strncpy(m_szSkyboxSkinFile, pLevel->szSkyBoxSkin, LG_MAX_PATH);
	
	
	LX_DestroyLevel(pLevel);	
	
	//Since we loaded a new level we'll call Update on the time
	//just to reset the elapsed time to 0 (we've got to call it twice
	//for that, we'll also process the physics engine).
	m_Timer.Update();
	m_Timer.Update();
	m_pPhys->Simulate(0.0f);
}

/* PRE: N/A
	POST: The new map is loaded into the server, or no map
	if the map does not exist, so no matter what the previous
	map will no longer be loaded.
*/
lg_void CLWorldSrv::LoadMap(lg_str szMapFile)
{
	//We don't load a map if the server isn't running.
	if(!IsRunning())
		return;
		
	Err_Printf("Loading \"%s\"...", szMapFile);
	m_Map.Load(szMapFile);
	m_pPhys->SetupWorld(&m_Map);
	//Should post an update that the map has changed for the
	//clients.
}

/* PRE: Usually called when loading a level.
	POST: Clears all entities, and removes them from the
	physics engine.
*/
lg_void CLWorldSrv::ClearEnts()
{
	//Empties out all the entities.
	LEntitySrv* pCurrent=LG_NULL;
	
	while(!m_EntsInt.IsEmpty())
	{
		pCurrent=(LEntitySrv*)m_EntsInt.Pop();
		m_pPhys->RemoveBody(pCurrent->m_pPhysBody);
		m_EntsBlank.Push(pCurrent);
	}
	
	while(!m_EntsInert.IsEmpty())
	{
		pCurrent=(LEntitySrv*)m_EntsInert.Pop();
		m_pPhys->RemoveBody(pCurrent->m_pPhysBody);
		m_EntsBlank.Push(pCurrent);
	}
}

/* PRE: N/A
	POST: Allocates memory for the maximum number of available enties in
	m_pEntList.  Stores all entities in the m_pEntsBlank list.  Returns
	false if there was not enough memory.  Should only be called
	when the world server is created.
*/
lg_bool CLWorldSrv::InitEnts()
{
	const lg_dword MAX_ENTS = ::CV_Get(CVAR_srv_MaxEntities)->nValue;
	m_nMaxEnts=MAX_ENTS;
	Err_Printf("Initializing entity list...");
	//Initialzie all the LEntLists (note that this does not allocate
	//memory, the memory has already been allocated).
	DestroyEnts();
	
	Err_Printf("Allocating memory for %u entities...", MAX_ENTS);
	//Allocate memory for the maximum number of available enties.
	m_pEntList = new LEntitySrv[MAX_ENTS];
	if(!m_pEntList)
		return LG_FALSE;
	
	Err_Printf("%u bytes allocated (%uMB).", MAX_ENTS*sizeof(LEntitySrv), MAX_ENTS*sizeof(LEntitySrv)/1048576);
		
	//If we got the memory that we needed, then we put all the entities
	//in m_pEntsBlank list (which is where unused entities are stored).
	//We also assign the ID to each entity (which is it's spot in the
	//base list, this ID will be used by all clients because memory
	//addresses will certainly be different from client to client).
	Err_Printf("Assigning Unique Entity IDs (UEIDs)...");
	for(lg_dword i=0; i<MAX_ENTS; i++)
	{
		//Once set m_nEntID should NEVER be changed.
		m_pEntList[i].m_nUEID = i;
		m_pEntList[i].m_nFlags1=LEntitySrv::EF_1_BLANK;
		m_EntsBlank.Push(&m_pEntList[i]);
	}
	m_EntsInt.Clear();
	m_EntsInert.Clear();
	Err_Printf("Entity list created.");
	return LG_TRUE;
}

/* PRE: Should only be called when completely finnished with world.
	POST: Deallocates memory that was used by entities, closes
	lists.  Should only be called upon the destruction of the
	world server.
*/
lg_void CLWorldSrv::DestroyEnts()
{
	//Close all the entities lists.
	m_EntsBlank.Clear();
	m_EntsInt.Clear();
	m_EntsInert.Clear();
	//Deallocate the entity's memory.
	LG_SafeDeleteArray(m_pEntList);
}

lg_void CLWorldSrv::PrintEntInfo()
{
	Err_Printf("===Legacy (%s) Entity Information===", m_szName);
	Err_Printf("Avaialbe entitis: %d", m_nMaxEnts);
	Err_Printf("Intelligent entities:");
	for(LEntitySrv* pEnt=(LEntitySrv*)m_EntsInt.m_pFirst; pEnt; pEnt=(LEntitySrv*)pEnt->m_pNext)
	{
		Err_Printf("%d: \"%s\" (%f, %f, %f)", 
			pEnt->m_nUEID, 
			pEnt->m_szObjScript, 
			pEnt->m_matPos._41, pEnt->m_matPos._42, pEnt->m_matPos._43);
	}
	Err_Printf("Inert entities:");
	for(LEntitySrv* pEnt=(LEntitySrv*)m_EntsInert.m_pFirst; pEnt; pEnt=(LEntitySrv*)pEnt->m_pNext)
	{
		Err_Printf("%d: \"%s\" (%f, %f, %f)", 
			pEnt->m_nUEID, 
			pEnt->m_szObjScript, 
			pEnt->m_matPos._41, pEnt->m_matPos._42, pEnt->m_matPos._43);
	}
	Err_Printf("========================================");
}

