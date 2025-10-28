/* lg_meshmgr.h - The Legacy Game Mesh Manager is responsible for
	loading meshes, allowing objects to get meshes to use, cleaning
	up the meshes, etc, meshes should always be loaded using the
	game's instance of this class, though meshes themselves can be
	used however an object sees fit to use it.
	
	CLMeshMgr uses linked lists to store both meshes and skeletons.
	The manager also has validate and invalidate calls to validate
	and invalidate the Direct3D components, for this reason the
	objects that use the meshes don't need to worry about validatation
	at all, if because of the mesh manager a mesh is not valid
	if an in game object tries to render it, nothing will get rendered,
	but the game will not crash.
	
	Copyright (c) 2007 Blaine Myers
*/
#ifndef __LG_MESHMGR_H__
#define __LG_MESHMGR_H__
#include "lf_sys2.h"
#include "lm_d3d.h"
#include "lg_stack.h"

//When a mesh or skel is loaded into the mesh manager the 
//LMMGR_LOAD_RETAIN flag may be set, if this flag is set
//when UnloadMeshes is called any meshes that have this
//flag set will survive the call to UnloadMeshes
//unles UnloadMeshes has teh LMMGR_UNLOAD_FORCE_ALL flag
//passed to it, in which case all meshes or skels will
//be unloaded despite the RETAIN flag.  The idea behind
//retaining a mesh is in case a level is divided into
//multiple part where certain meshes will always be
//rendered (monster meshes for example) but some meshes are
//not rendered (grass for example would only be rendered in
//outdoor scenes, not indoors).  Or if certain meshes
//are used through the entire game (weapons, HUD stuff, etc)
//and therefore have no reason to be unloaded.
#define LMMGR_LOAD_RETAIN 0x00000001 

#define LMMGR_UNLOAD_FORCE_ALL 0x00000001 


//NOTES: In mesh (and skel) loading, unloading, and getting 
//the name is used.  Because every time a mesh (or skel) is
//loaded or loaded the order of the items in the list is subject
//to change extensively.  On an unload for example the order of
//the list is completely reveresed (except for the removed mesh
//which is no longer in the list at all).  For that reason a
//reference number cannot be used to get or delete a particular
//mesh.  Also note that a mesh (or skel) that has been gotten
//with the GetMesh (or GetSkel) method is not independent of
//the mesh manager, and if that mesh is deleted in the mesh manager
//then the gotten pointer is no longer going to point at a valid
//CLMeshD3D (unless a new mesh happens to be allocated in exactly
//the same spot, and so anywhere that gets a mesh should be aware
//of when that mesh is destroyed so that it no longer uses it.
//Also note that before any meshes are loaded InitializeD3D should
//be called with a reference to the games Direct3D device passed
//to it, if this method is not called first then when LoadMesh
//is called the meshes will never have their CreateD3DComponents
//method called, and will not be renderable (unless a function that
//calls GetMesh calls the CreateD3DComponents method on the mesh
//that it has gotten.
class CLMeshMgr
{
private:
	//LMESH_LINK and LSKEL_LINK are the link structures
	//for the linked lists of CLMeshD3Ds and CLSkels
	//within the Mesh Manager.
	typedef struct _LMESH_LINK
	{
		CLMeshD3D* pMesh;
		LMESH_NAME szName;
		lg_dword nFlags;
		lg_dword nHashCode;
		struct _LMESH_LINK* pNext;
	}LMESH_LINK;

	typedef struct _LSKEL_LINK
	{
		CLSkel* pSkel;
		LMESH_NAME szName;
		lg_dword nFlags;
		lg_dword nHashCode;
		struct _LSKEL_LINK* pNext;
	}LSKEL_LINK;
private:
	lg_dword m_nNumMeshes;
	LMESH_LINK* m_pFirstMesh;
	
	lg_dword m_nNumSkels;
	LSKEL_LINK* m_pFirstSkel;
	
public:
	CLMeshMgr(IDirect3DDevice9* pDevice, lg_dword nMaxMesh, lg_dword nMaxSkel);
	~CLMeshMgr();
	
	CLMeshD3D* LoadMesh(lf_path szFilename, lg_dword nFlags);
	CLMeshD3D* GetMesh(lg_char* szName);
	
	CLSkel* LoadSkel(lf_path szFilename, lg_dword nFlags);
	CLSkel* GetSkel(lg_char* szName);
	
	lg_bool UnloadMesh(lg_char* szName);
	lg_bool UnloadMeshes(lg_dword nFlags);
	
	lg_bool UnloadSkel(lg_char* szName);
	lg_bool UnloadSkels(lg_dword nFlags);
	
	lg_bool UnloadAll(lg_dword nFlags);
	
	lg_bool ValidateMeshes();
	void InvalidateMeshes();
	
//The static mesh manager is so that meshes can be loaded
//on demand without having access to the extablished mesh
//manager within the game class.
private:
	static CLMeshMgr* s_pMeshMgr;
public:
	static CLMeshD3D* GlobalLoadMesh(lf_path szFilename, lg_dword Flags);
	static CLSkel* GlobalLoadSkel(lf_path szFilename, lg_dword Flags);
};


//#define LG_LoadMesh CLMeshMgr::GlobalLoadMesh
//#define LG_LoadSkel CLMeshMgr::GlobalLoadSkel

#endif __LG_MESHMGR_H__