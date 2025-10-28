/* lg_tmgr.h - The texture manager.
	Copryight (c) 2008 Blaine Myers
*/
#ifndef __LG_TMGR_H__
#define __LG_TMGR_H__

#include <d3d9.h>
#include "lg_types.h"
#include "lg_hash_mgr.h"

//All textures are represented by tm_tex, in this way they
//are not specifically Direct3D based.  Rather they are a
//reference.  Only the Texture manager itself sets the texture.
//Should probably do something similar for vertex and index
//buffers.
typedef hm_item tm_tex;

class CLTMgr: public CLHashMgr<IDirect3DTexture9>{
public:
	//Flags:
	const static lg_dword TM_FORCENOMIP=0x00000010;
	
private:
	//Internal data, strictly initialized in the constructor.
	D3DPOOL m_nPool;
	IDirect3DDevice9* m_pDevice; //The device.
public:
	/* Constructor
		PRE: Direct3D must be initialized.
		POST: The instance is created.
	*/
	CLTMgr(IDirect3DDevice9* pDevice, lg_dword nMaxTex);
	/* PRE: N/A.
		POST: Destroys everything, unloads all textures, and
		deallocates all memory associated.  Note that after the manager is
		destroyed, any textures that have been obtained are still usable
		as only the reference count is decreased.  Any textures obtained
		must still be released.
	*/
	~CLTMgr();
	
	/* PRE: See above, this is for lightmaps.
		POST: See above, but note that textures loaded from memory are not
		stored in the texture manager, and cannot be reobtained.  This
		method should only be used to load lightmaps, or other textures
		that there is certainly only going to be one copy of.
	*/
	tm_tex LoadTextureMem(lg_void* pData, lg_dword nSize, lg_dword nFlags);
	
	/* PRE: N/A
		POST: Sets the specified texture for rendering in the
		specified stage.
	*/
	void SetTexture(tm_tex texture, lg_dword nStage);
	
	/* PRE: Called if device was lost.
		POST: Prepares the texture manager for a reset.
	*/
	void Invalidate();
	/* PRE: Device must be reset.
		POST: Validates all textures.
	*/
	void Validate();
private:
	virtual IDirect3DTexture9* DoLoad(lg_path szFilename, lg_dword nFlags);
	virtual void DoDestroy(IDirect3DTexture9* pItem);
private:
	//The static texture manager is so textures can be loaded on demand
	//without having acces to teh established texture manager within
	//the game class.
	static CLTMgr* s_pTMgr;
public:
	/* PRE: The CLTMgr must be initialized.
		POST: If the specified texture is available it is loaded,
		see the public member LoadTexture for more details.
	*/
	static tm_tex TM_LoadTex(lg_path szFilename, lg_dword nFlags);
	/* PRE: The CLTMgr must be initialized.
		POST: If the specified texture is available it is loaded,
		see the public member LoadTextureMem for more details.
	*/
	static tm_tex TM_LoadTexMem(lg_void* pData, lg_dword nSize, lg_dword nFlags);
	
	/* PRE: texture must be a texture obtained with TM_LoadTex*,
		or 0 for no texture.
		POST: The texture set will be used for rendering.
	*/
	static void TM_SetTexture(tm_tex texture, lg_dword nStage);
};

#endif __LG_TMGR_H__