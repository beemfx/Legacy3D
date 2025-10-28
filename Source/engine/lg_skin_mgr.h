#ifndef __LG_SKIN_MGR_H__
#define __LG_SKIN_MGR_H__

#include "lg_types.h"
#include "lg_hash_mgr.h"
#include "lm_skin.h"
#include "lg_list_stack.h"

typedef hm_item sm_item;

struct SkinItem: public CLListStack::LSItem
{
public:
	CLSkin Skin;
};

LG_CACHE_ALIGN class CLSkinMgr: public CLHashMgr<SkinItem>
{
private:
	//We are using a list stack to create and destroy
	//materials, in the manner as for the CLMeshMgr and CLSkelMgr.
	SkinItem*   m_pSkinMem;
	CLListStack m_stkSkin;
	
	SkinItem m_DefSkin;
public:
	/* PRE: Specify the maximum number of materials in nMaxSkin
		POST: Ready for use.
	*/
	CLSkinMgr(lg_dword nMaxSkin);
	/* PRE: N/A
		POST: Manager is destroyed.
	*/
	~CLSkinMgr();
private:
	//Internally used methods.
	virtual SkinItem* DoLoad(lg_path szFilename, lg_dword nFlags);
	virtual void DoDestroy(SkinItem* pItem);
public:	
	/* PRE: skin should be loaded.
		POST: Obtains the interfaces as specified.
	*/
	CLSkin* GetInterface(sm_item skin);
	
//Static stuff:
private:
	static CLSkinMgr* s_pSkinMgr;
public:
	static CLSkin* SM_Load(lg_path szFilename, lg_dword nFlags);
};

#endif __LG_SKIN_MGR_H__