#ifndef __LW_CLIENT_H__
#define __LW_CLIENT_H__

#include "li_sys.h"
#include "lw_server.h"
#include "lw_map.h"
#include "lv_sys.h"
#include "lw_skybox.h"
#include "lg_fxmgr.h"
#include <d3dx9.h>

//For a test:
#include "lm_mesh_lg.h"
#include "lm_mesh_tree.h"

typedef struct _LEntRasterInfo{
	//lg_dword    m_nMeshCount;
	//lg_dword    m_nSkelCount;
	CLMeshTree m_MeshTree;
	//CLSkelLG**    m_pSkels;
	//lg_float    m_fScale;
}LEntRasterInfo;

class CLWorldClnt: public CElementD3D, public CElementInput{
private:
	CLNT_CONNECT_TYPE m_nConnectType;

	CLMapD3D    m_Map;
	CLSkybox3   m_Skybox;
	CLWorldSrv* m_pSrvLocal;
	
	//The entities.
	lg_dword     m_nMaxEnts; //Maximum number of ents availabe (gotten from server).
	LEntityClnt* m_pEntList; //Master list of entities.
	LEntList     m_EntsAll;  //List of any entities that are used.
	
	lg_dword     m_nPCEnt;  //Ent controlled by the player.
	lg_dword     m_nIDOnSrv; //ID of this client, as stored on the server.
	
	CLInput*     m_pInput;
	
	lg_bool      m_bRunning;
public:
	CLWorldClnt();
	~CLWorldClnt();
	
	void Init(CLInput* pInput);
	void Shutdown();
	
	lg_void ConnectLocal(CLWorldSrv* pSrv);
	lg_void ConnectTCP();
	lg_void Disconnect();
	
	lg_void Update();
	
	lg_void SetCamera(ML_MAT* pMat);
	lg_void Render();
	
	lg_void Validate();
	lg_void Invalidate();
	
private:
	lg_void ClearEntities();
	lg_void AddEnt(const lg_dword nUEID, const LEntityClnt* pEnt, const lg_path szObjScipt);
	__inline lg_void RemoveEnt(const lg_dword nUEID);
	
//Networking:
private:
	lg_void Broadcast();
	lg_void Receive();
	
	lg_void ProcessSrvCmd(const CLWCmdItem* pCmd);
private:
	__inline static lg_void UpdateEntSrvToCli(LEntityClnt* pDest, const LEntitySrv* pSrc);
	__inline static lg_void* CreateRasterInfo(const lg_path szObjScript);
	__inline static lg_void  DeleteRasterInfo(lg_void* pRasterInfo);
};


#endif  __LW_CLIENT_H__