#ifndef __LM_MESH_D3D_BASIC_H__
#define __LM_MESH_D3D_BASIC_H__

//This is an extremely basic D3D mesh for testing
//purposes, it is designed to be compatible with
//the Legacy Engine and not LMEdit.

#include <d3d9.h>
#include "lm_mesh_anim.h"
#include "lg_tmgr.h"

class CLMeshD3DBasic: public CLMeshAnim
{
private:
	tm_tex* m_pTex;
	MeshVertex* m_pTransfVB;
public:
	virtual lg_bool Load(LMPath szFile);
	virtual lg_bool Save(LMPath szFile);
	virtual lg_void Unload();

protected:
	virtual MeshVertex* LockTransfVB();
	virtual lg_void UnlockTransfVB();
	
public:
	CLMeshD3DBasic(): CLMeshAnim(), m_pTex(LG_NULL), m_pTransfVB(LG_NULL){};
	lg_void Render();
	
	static IDirect3DDevice9* s_pDevice;
	
private:
	//The read function:
	static lg_uint __cdecl ReadFn(lg_void* file, lg_void* buffer, lg_uint size);
};

#endif __LM_MESH_D3D_BASIC_H__