#include "lg_mgr.h"
#include "lg_err.h"
#include "lg_cvars.h"

CLMgrs::CLMgrs()
: m_pMMgr(LG_NULL)
, m_pFxMgr(LG_NULL)
, m_pTexMgr(LG_NULL)
, m_pMtrMgr(LG_NULL)
, m_pSkinMgr(LG_NULL)
{

}

CLMgrs::~CLMgrs()
{

}

lg_void CLMgrs::InitMgrs(IDirect3DDevice9* pDevice)
{
	//We'll initalize all managers here, note that they have
	//to be initialized in a certain order (i.e. the mesh &
	//skeleton manager use the texture, fx, and material managers
	//so it must be initialized last.)
	Err_Printf("=== Initializing Resource Managers ===");
	Err_IncTab();
	
	//Should get maximum textures from a cvar.
	Err_Printf("Texture manager...");
	m_pTexMgr=new CLTMgr(pDevice, CV_Get(CVAR_lg_MaxTex)->nValue);
	Err_Printf("FX manager...");
	m_pFxMgr=new CLFxMgr(pDevice, CV_Get(CVAR_lg_MaxFx)->nValue);
	Err_Printf("Material manager...");
	m_pMtrMgr=new CLMtrMgr(CV_Get(CVAR_lg_MaxMtr)->nValue);
	Err_Printf("Skin manager...");
	m_pSkinMgr=new CLSkinMgr(CV_Get(CVAR_lg_MaxSkin)->nValue);
	Err_Printf("Mesh & skeleton manager...");
	m_pMMgr=new CLMMgr(pDevice, CV_Get(CVAR_lg_MaxMeshes)->nValue, CV_Get(CVAR_lg_MaxSkels)->nValue);
	
	Err_DecTab();
	Err_Printf("======================================");
}

lg_void CLMgrs::ShutdownMgrs()
{
	Err_Printf("=== Destroying Resource Managers ===");
	Err_IncTab();
	
	//We'll juse destroy these in the opposite order
	//that we initialized them.
	Err_Printf("Mesh & skeleton manager...");
	LG_SafeDelete(m_pMMgr);
	Err_Printf("Skin manager...");
	LG_SafeDelete(m_pSkinMgr);
	Err_Printf("Material manager...");
	LG_SafeDelete(m_pMtrMgr);
	Err_Printf("FX manager...");
	LG_SafeDelete(m_pFxMgr);
	Err_Printf("Texture manager...");
	LG_SafeDelete(m_pTexMgr);
	
	Err_DecTab();
	Err_Printf("====================================");
}

lg_void CLMgrs::ValidateMgrs()
{
	m_pTexMgr->Validate();
	m_pFxMgr->Validate();
	m_pMMgr->ValidateMeshes();
}

lg_void CLMgrs::InvalidateMgrs()
{
	m_pTexMgr->Validate();
	m_pFxMgr->Validate();
	m_pMMgr->InvalidateMeshes();
}

