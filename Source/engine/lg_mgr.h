#ifndef __LG_MGR_H__
#define __LG_MGR_H__

#include "lg_mmgr.h"      //Mesh & Skel manager.
#include "lg_fxmgr.h"     //FX Manager.
#include "lg_tmgr.h"      //Texture manager.
#include "lg_mtr_mgr.h"   //Material manager.
#include "lg_skin_mgr.h"  //Skin manager.

class CLMgrs
{
protected:
	CLMMgr*    m_pMMgr;   //Mesh & skeleton manager.
	CLFxMgr*   m_pFxMgr;  //FX Manger.
	CLTMgr*    m_pTexMgr; //Texture manager.
	CLMtrMgr*  m_pMtrMgr; //Material manager.
	CLSkinMgr* m_pSkinMgr;//Skin Manager.
protected:
	CLMgrs();
	~CLMgrs();
	
	lg_void InitMgrs(IDirect3DDevice9* pDevice);
	lg_void ShutdownMgrs();
	
	lg_void ValidateMgrs();
	lg_void InvalidateMgrs();
};

#endif  __LG_MGR_H__