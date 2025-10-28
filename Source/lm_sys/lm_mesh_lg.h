/* CLMeshLG - The legacy game implementation of a Legacy Mesh.
	Copryight (c) 2008 Beem Software
*/
#ifndef __LM_MESH_LG_H__
#define __LM_MESH_LG_H__

#include "lm_mesh_anim.h"
#include "lm_skin.h"

#include <d3d9.h>
#include "lg_tmgr.h"

class CLMeshLG: public CLMeshAnim
{
friend class CLMeshNode;
friend class CLSkelLG;
friend class CLMeshTree;
//Constants and flags:
private:
	//m_nFlags flags:
	static const lg_dword LM_FLAG_D3D_READY=0x00010000;
	static const lg_dword LM_FLAG_D3D_VALID=0x00020000;
	
	//The vertex format:
	static const lg_dword LM_VERTEX_FORMAT=(D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX1);
	
public:
	//MakeD3DReady flags:
	static const lg_dword D3DR_FLAG_LOAD_DEF_TEX=0x00000001;
	
//Member variables:
private:
	IDirect3DVertexBuffer9* m_pVB;
	IDirect3DIndexBuffer9*  m_pIB;
	tm_tex*                 m_pTex;
	
	LMPath                  m_szMeshFilePath;
	
//Methods:
public:
	CLMeshLG();
	~CLMeshLG();
	//Necessary overrides:
	virtual lg_bool Load(LMPath szFile);
	virtual lg_bool Save(LMPath szFile);
	virtual lg_void Unload();
	
	//New functionality for the class:
	lg_void MakeD3DReady(lg_dword flags);
	lg_void Validate();
	lg_void Invalidate();
	
	lg_bool IsD3DReady();
	
	lg_void Render();
	lg_void Render(const CLSkin* pSkin);
	
protected:
	//Necessary overrides:
	virtual MeshVertex* LockTransfVB();
	virtual void UnlockTransfVB();
	
//Static (global) support methods:
private:
	//The read function:
	static lg_uint __cdecl ReadFn(lg_void* file, lg_void* buffer, lg_uint size);
		
//Static variables used by all instances:
private:
	static IDirect3DDevice9* s_pDevice;
public:
	static lg_void LM_Init(IDirect3DDevice9* pDevice);
	static lg_void LM_Shutdown();
	static lg_void LM_SetRenderStates();
	static lg_void LM_SetSkyboxRenderStates();
};


#endif __LM_MESH_LG_H__