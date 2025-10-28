#ifndef __LG_MTR_MGR_H__
#define __LG_MTR_MGR_H__

#include "lg_types.h"
#include "lg_hash_mgr.h"
#include "lg_tmgr.h"
#include "lg_fxmgr.h"
#include "lg_list_stack.h"

typedef hm_item lg_material;

struct MtrItem: public CLListStack::LSItem
{
public:
	tm_tex Texture;
	fxm_fx Effect;
};

LG_CACHE_ALIGN class CLMtrMgr: public CLHashMgr<MtrItem>
{
private:
	//We are using a list stack to create and destroy
	//materials, in the manner as for the CLMeshMgr and CLSkelMgr.
	MtrItem* m_pMtrMem;
	CLListStack m_stkMtr;
	
	MtrItem m_DefMtr;
public:
	/* PRE: Specify the maximum number of materials in nMaxMtr
		POST: Ready for use.
	*/
	CLMtrMgr(lg_dword nMaxMtr);
	/* PRE: N/A
		POST: Manager is destroyed.
	*/
	~CLMtrMgr();
private:
	//Internally used methods.
	virtual MtrItem* DoLoad(lg_path szFilename, lg_dword nFlags);
	virtual void DoDestroy(MtrItem* pItem);
public:
	/* PRE: mtr should be a material, and pVB a valid vertex buffer.
		Should be within the Device methdos of BeginScene and EndScene.
		POST: Currently just renders whatever is passed into the
		method.  In the future it will probably stick the vertices
		into some kind of buffer.
	*/
	void RenderVB(
		lg_material mtr, 
		IDirect3DDevice9* pDevice, 
		IDirect3DVertexBuffer9* pVB);
		
	/* PRE: PRE: mtr should be loaded.
		POST: Obtains the interfaces as specified.
	*/
	void GetInterfaces(lg_material mtr, tm_tex* pTex, ID3DXEffect** pFX);
	
//Static stuff:
private:
	static CLMtrMgr* s_pMtrMgr;
public:
	static lg_material MTR_Load(lg_path szFilename, lg_dword nFlags);
	static void MTR_RenderVB(lg_material mtr, IDirect3DDevice9* pDevice, IDirect3DVertexBuffer9* pVB);
	static void MTR_GetInterfaces(lg_material mtr, tm_tex* pTex, ID3DXEffect** pFX);
};

#endif __LG_MTR_MGR_H__