#ifndef __LW_SERVER_H__
#define __LW_SERVER_H__

#include "lw_entity.h"
#include "lw_ai.h"
#include "lt_sys.h"
#include "lw_map.h"
#include "lv_sys.h"
#include "lp_sys2.h"
#include "lw_net.h"

class CLWorldSrv{
friend class CLWorldClnt;
//Internal structures:
private:
	//Data stored about the clients.
	struct ClntData{
		lg_string szName;
		lg_dword nClntID;
		CLNT_CONNECT_TYPE nType;
		lg_dword  nClntEnt; //The entity of most interest to the client.
		
		CLListStack m_CmdList;
	};
	
	struct EntTemplate
	{
		lg_srv_ent Ent;
		lp_body_info BdyInfo;
	};
//Constants:
private:
	static const lg_dword LSRV_MAX_CLIENTS=8;
	static const lg_dword LSRV_MAX_CMDS=1024;
//Flags:
private:
	//Server is running:
	static const lg_dword LSRV_FLAG_RUNNING=0x00000001;
	//Server is running, but it is paused:
	static const lg_dword LSRV_FLAG_PAUSED=0x00000002;
private:
	//static const lg_dword MAX_ENTS=10000; //10,000 is about 14MB.
	lg_str m_szName; //The name of the server.
	
	//Clients connected to the server:
	lg_dword m_nNumClnts;
	lg_dword m_nClntRefs[LSRV_MAX_CLIENTS];
	ClntData m_Clients[LSRV_MAX_CLIENTS];
	ClntData m_AllClients;
	
	//Network commands:
	CLWCmdItem* m_pCmdList;
	CLListStack m_UnusedCmds;
	
	lg_path m_szSkyboxFile; //The path of the skybox mesh (to be sent to clients)
	lg_path m_szSkyboxSkinFile; //The path of the skin for the skybox (to be sent to clients)
	
	//Entity management variables.
	lg_dword    m_nMaxEnts;   //The maximum number of entities available.
	LEntitySrv* m_pEntList;   //The list of all entities (used or unused).
	LEntList    m_EntsBlank;  //The list of unused entities.
	LEntList    m_EntsInt;    //The list of intelligent entities.
	LEntList    m_EntsInert;  //The list of inert (unintelligent) entities.
	
	//The physics engine.
	CLPhys* m_pPhys;
	
	CLTimer  m_Timer; //The timer.
	CLMap    m_Map;      //The map.
	
	//The AI manager:
	CLWAIMgr m_AIMgr;
	
	lg_dword m_nFlags; //See above for what the flags mean.
	
	static const lg_dword m_nEntKillListSize=10;
	lg_dword m_nEntKillList[m_nEntKillListSize];
	lg_dword m_nEntKillCount;
private:
	lg_bool InitEnts();
	lg_void DestroyEnts();
	
	//Entity templates:
	lg_dword    m_nNumTemplateEnts;
	EntTemplate* m_pTemplateEnt;
	
	lg_void LoadEntTemplate(lg_cstr szTemplateScript);
	lg_void UnloadEntTemplate();
	
	lg_bool SetupEntityFromScript(lg_srv_ent* pEnt, lp_body_info* pBdyInfo, lg_cstr szScript);
	lg_void SetupEntFromTemplate(lg_srv_ent* pEnt, lp_body_info* pBdyInfo, const lg_dword nTemplate);
protected:
	lg_void AddClient(const ClntData* pData);
	lg_bool AcceptLocalConnect(lg_dword* pPCEnt, lg_dword* pID, lg_cstr szName);
	
	lg_void Broadcast();
	lg_void Receive();
public:
	CLWorldSrv();
	~CLWorldSrv();
	
	lg_void Init();
	lg_void Shutdown();
	lg_bool IsRunning(){return LG_CheckFlag(m_nFlags, LSRV_FLAG_RUNNING);}
	
	lg_void ProcessServer();
	__inline lg_void ProcessEntFuncs(LEntitySrv* pEnt);
	//lg_void CreateTestEnts();
	lg_dword AddEntity(lf_path szObjScript, ML_MAT* matPos);
	lg_dword AddEntity(lg_dword nTemplate, ml_mat* matPos);
	lg_void RemoveEnt(const lg_dword nID);
	lg_void LoadMap(lg_str szMapFile);
	lg_void LoadLevel(lg_str szLevelScriptFile);
	
	lg_void ClearEnts();
	
	//Debugging functions:
	lg_void PrintEntInfo();
	
	/* PRE: N/A
		POST: If the server was runing it get's paused.
	*/
	lg_void Pause();
	/* PRE: N/A
		POST: If the server was paused, it will resume.
	*/
	lg_void Resume();
	
	/* PRE: N/A
		POST: Togles the pause status.
	*/
	lg_void TogglePause();

};

#endif  __LW_SERVER_H__