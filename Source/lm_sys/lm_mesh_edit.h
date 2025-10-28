#ifndef __LM_EDIT_H__
#define __LM_EDIT_H__

#include "lm_base.h"
#include "lm_mesh_anim.h"

#include <d3d9.h>
//CLMeshEditD3D the editable and renderable
//version of CLMesh...


class CLMeshEdit: public CLMeshAnim
{
friend class CLSkelEdit;
private:
	IDirect3DTexture9** m_ppTex;
	MeshVertex* m_pTransfVB;
	
	static const lg_dword LM_FLAG_RENDER_TEX=0x00020000;
public:
	virtual lg_bool Load(LMPath szFile);
	virtual lg_bool Save(LMPath szFile);
	virtual lg_void Unload();

protected:
	virtual MeshVertex* LockTransfVB();
	virtual lg_void UnlockTransfVB();
	
	static lg_str GetDirFromPath(lg_str szDir, lg_cstr szFullPath);
	
public:
	CLMeshEdit(): CLMeshAnim(), m_ppTex(LG_NULL), m_pTransfVB(LG_NULL){};
	lg_void Render();
	lg_void SetRenderTexture(lg_bool bRenderTexture);
	const ml_aabb* GetBoundingBox();
	
	static IDirect3DDevice9* s_pDevice;
	
private:
	//The read function:
	static lg_uint __cdecl ReadFn(lg_void* file, lg_void* buffer, lg_uint size);
	static lg_uint __cdecl WriteFn(lg_void* file, lg_void* buffer, lg_uint size);
};

#endif __LM_EDIT_H__