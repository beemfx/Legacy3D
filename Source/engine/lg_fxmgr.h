/* lg_fxmgr.h - The effect manager.
	Copryight (c) 2008 Blaine Myers
*/
#ifndef __LG_FXMGR_H__
#define __LG_FXMGR_H__

#include <d3d9.h>
#include <d3dx9.h>
#include "lg_types.h"
#include "lg_hash_mgr.h"

//All effects are represented by fxm_fx, in this way they
//are not specifically Direct3D based.  Rather they are a
//reference.
typedef hm_item fxm_fx;

class CLFxMgr: public CLHashMgr<ID3DXEffect>{
public:
	//Flags:	
private:
	//Internal data, strictly initialized in the constructor.
	D3DPOOL m_nPool;
	IDirect3DDevice9* m_pDevice; //The device.
public:
	/* Constructor
		PRE: Direct3D must be initialized.
		POST: The instance is created.
	*/
	CLFxMgr(IDirect3DDevice9* pDevice, lg_dword nMaxFx);
	/* PRE: N/A.
		POST: Destroys everything, unloads all effects, and
		deallocates all memory associated.  Note that after the manager is
		destroyed, any effects that have been obtained are still usable
		as only the reference count is decreased.  Any effects obtained
		must still be released.
	*/
	~CLFxMgr();
	
	/* PRE: N/A
		POST: Sets the specified effect for rendering in the
		specified stage.
	*/
	void SetEffect(fxm_fx effect);
	
	/* PRE: N/A
		POST: Returns the interface of the effect,
		this will not be used in the future.
	*/
	ID3DXEffect* GetInterface(fxm_fx effect);
	
	/* PRE: Called if device was lost.
		POST: Prepares the effect manager for a reset.
	*/
	void Invalidate();
	
	/* PRE: Device must be reset.
		POST: Validates all effects.
	*/
	void Validate();
private:
	//Internal methods:
	/* PRE: Called only from the load method.
		POST: Basically a check to see if a effect
		is already loaded in the manager.  If it is, it is
		returned and Loadeffect ends quickly.  If it isn't
		The manager knows it must load the effect.
	*/
	fxm_fx GetEffect(lg_path szFilename, lg_dword* pHash);
	
	/* PRE: Called only from the load method.
		POST: This actually loads a direct3d effect and returns
		it, it uses all internal flags to decide how the effect
		is to be loaded.
	*/
	virtual ID3DXEffect* DoLoad(lg_path szFilename, lg_dword nFlags);
	virtual void DoDestroy(ID3DXEffect* pItem);
private:
	//The static effect manager is so effects can be loaded on demand
	//without having acces to teh established effect manager within
	//the game class.
	static CLFxMgr* s_pFxMgr;
public:
	/* PRE: The CLTMgr must be initialized.
		POST: If the specified effect is available it is loaded,
		see the public member Loadeffect for more details.
	*/
	static fxm_fx FXM_LoadFx(lg_path szFilename, lg_dword nFlags);
	
	/* PRE: effect must be a effect obtained with FXM_LoadFx*,
		or 0 for no effect.
		POST: The effect set will be used for rendering.
	*/
	static void FXM_SetEffect(fxm_fx effect);
	
	/* PRE: Temp only.
		POST: Returns interface specified, or teh default one.
	*/
	static ID3DXEffect* FXM_GetInterface(fxm_fx effect);
};


#endif __LG_FXMGR_H__