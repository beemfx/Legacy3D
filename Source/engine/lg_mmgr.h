/* lg_mmgr.h - The Legacy Game Mesh Manager is responsible for
	loading meshes, allowing objects to get meshes to use, cleaning
	up the meshes, etc, meshes should always be loaded using the
	game's instance of this class, though meshes themselves can be
	used however an object sees fit to use it.
	
	CLMMgr uses a hash table meshes and skeletons.
	The manager also has validate and invalidate calls to validate
	and invalidate the Direct3D components, for this reason the
	objects that use the meshes don't need to worry about validatation
	at all, if because of the mesh manager a mesh is not valid
	if an in game object tries to render it, nothing will get rendered,
	but the game will not crash.
	
	Copyright (c) 2008 Blaine Myers
*/

/*
Usage: Only one instance of CLMMgr should be declared,
and that should be in the actual game class.  From there
any other classes that need meshes may use the static
member functions to load them, but all management of
meshes (validating, invalidating, unloading) should be
taken care of in the game class.  Creating a CLMMgr
is all done in the constructor, so the desired values
must be specified at that time.  Direct3D should be 
initialized first if we need rendering (in the case of
a dedicated server we won't need rendering, so null
can be passed).  We also need to know the max number
of meshes that we need, as well as skeletons.  These
values could be stored in cvars for instance.

See the declaration of the class for further instructions.

Notes: The meshes and skeletons are stored in a hash
table.  The hash table is set up so that if a duplicate
hash is detected, it will go to the next available slot.
The manager also keeps track of how many objects share
the same hash, and so we are able to speed up the
search as to whether or not a mesh is even in the manager.
For this reason if we load the number of meshes that are
available it is possible that the object might be stored
quite far away from the actual hash location (in fact it
might wrap all the way around the the furthest location
available), so it is probably a good idea to have more
meshes available than are actually needed (skeletons
too).

Also the manager is designed for only one instance to be
created, if a second instance is created an exception
will be thrown.
*/
#ifndef __LG_MMGR_H__
#define __LG_MMGR_H__
#include "lg_types.h"
#include "lm_mesh_lg.h"
#include "lm_skel_lg.h"
#include "lg_hash_mgr.h"
#include "lg_list_stack.h"

LG_CACHE_ALIGN struct MeshItem: public CLListStack::LSItem
{
	CLMeshLG Mesh;
};

LG_CACHE_ALIGN class CLMeshMgr: public CLHashMgr<MeshItem>
{
private:	
	MeshItem* m_pMeshMem;
	CLListStack m_stkMeshes;
public:
	CLMeshMgr(IDirect3DDevice9* pDevice, lg_dword nMaxMeshes);
	~CLMeshMgr();
private:
	virtual MeshItem* DoLoad(lg_path szFilename, lg_dword nFlags);
	virtual void DoDestroy(MeshItem* pItem);
public:
	CLMeshLG* GetInterface(hm_item mesh);
	void Validate();
	void Invalidate();
};

LG_CACHE_ALIGN struct SkelItem: public CLListStack::LSItem
{
	CLSkelLG Skel;
};

LG_CACHE_ALIGN class CLSkelMgr: public CLHashMgr<SkelItem>
{
private:	
	SkelItem* m_pSkelMem;
	CLListStack m_stkSkels;
public:
	CLSkelMgr(lg_dword nMaxSkels);
	~CLSkelMgr();
private:
	virtual SkelItem* DoLoad(lg_path szFilename, lg_dword nFlags);
	virtual void DoDestroy(SkelItem* pItem);
public:
	CLSkelLG* GetInterface(hm_item skel);
};

class CLMMgr
{
//friend class CLGlobalMethods;
public:
	//Flags for loading:
	const static lg_dword MM_RETAIN=0x00000001;
	const static lg_dword MM_NOD3D= 0x00000002;
	
	//Flags for unloading:
	const static lg_dword MM_FORCE_UNLOAD=0x00000002;
	
private:
	CLMeshMgr m_MeshMgr;
	CLSkelMgr m_SkelMgr;
public:
	/* Constructor
		PRE: If we want to render a valide Direct3D Device must be passed.
		Also we need to declare, upon construction, the maximum number of
		meshes and skeletons that we can load.  Additionally since Meshes
		use Direct3D resources such as textures, things like the Texture
		Manager must be initialized before the initialization of this
		manager.
		POST: The isntance is created.
	*/
	CLMMgr(IDirect3DDevice9* pDevice, lg_dword nMaxMesh, lg_dword nMaxSkel);
	/* PRE: N/A.
		POST: Destroys everything, unloads all meshes and skels, and
		deallocates all memory associated.  Note that after the manager is
		destroyed, any attempt to use a CLMeshLG or CLSkelLG obtained from
		it will cause memory access violations.
	*/
	~CLMMgr();
	/* PRE: Use flags to specify how the mesh should be loaded,
		CLMMgr::MM_RETAIN means that upon a call to UnloadMeshes
		the mesh will survive (this is useful for meshes that will
		always be used such as weapons, main characters, etc.).
		CLMMgr::MM_NOD3D means that the mesh will be loaded, but it
		will not be renderable.  This is useful for physics engine
		loading, not that if later on the same mesh is loaded
		(or in other words retrieved) without the MM_NOD3D flag,
		it will be made renderable, at that time.  So when the
		server is initialized calling MM_NOD3D, the meshes will
		not be renderable, but when the client calls the method
		without the flag, the mesh will then be renderable.
		POST: If szFilename is a valid mesh file, the mesh will
		be returned, if not LG_NULL is returned.  Note that if the
		mesh was previously loaded, the mesh will only be obtained
		a second copy will not be loaded.
	*/
	CLMeshLG* LoadMesh(lg_path szFilename, lg_dword nFlags);
	/* PRE: Same as for meshes, but works on skeletons, and the
		MM_NOD3D flag would have no effect as skeletons are not
		renderable in the actual game anyway.
		POST: Same as for meshes.
	*/
	CLSkelLG* LoadSkel(lg_path szFilename, lg_dword nFlags);
	
	/* PRE: N/A
		POST: Unloades all meshes, except for those loaded with the
		MM_RETIAN flag, unless CLMMgr::MM_FORCE_UNLOAD is specifed,
		in which case all meshes are unloaded.
		NOTES: Most likely to be called in between map loads.
	*/
	void UnloadMeshes(lg_dword nFlags);
	/* PRE: N/A
		POST: Same as for meshes.
		NOTES: Same as for meshes.
	*/
	void UnloadSkels(lg_dword nFlags);
	
	/* PRE: InvalidateMeshes should have been called, otherwise
		we are simply wasting processor power.
		POST: Validates the Direct3D components of all meshes.
	*/
	void ValidateMeshes();
	/* PRE: The Direct3D device needs to be reset.
		POST: Invalidates the Direct3D components of all
		meshes.  Useful for resetting the video device.
	*/
	void InvalidateMeshes();
	
	//Debug Functions:
	
	/* PRE: N/A
		POST: Prints information about all the meshes loaded,
		output is sent to the console.  The output is for
		debugging only, so some of the information may appear
		to be gibberish.
	*/
	void PrintMeshes();
	/* PRE: N/A
		POST: Same as above, but for skels.
	*/
	void PrintSkels();
private:
	//The static mesh manager is so that meshes can be loaded
	//on demand without having access to the extablished mesh
	//manager within the game class.
	static CLMMgr* s_pMMgr; //Only one instance of this is created during the constructor.
public:

	/* PRE: The CLMMgr must be initialized.
		POST: If the specified mesh is available it is loaded,
		see the public member LoadMesh for more details.
	*/
	static CLMeshLG* MM_LoadMesh(lg_path szFilename, lg_dword nFlags);
	/* PRE: The CLMMgr must be initialized.
		POST: If the specifed skel is available it is loaded,
		see the public member LoadSkel for more details.
	*/
	static CLSkelLG* MM_LoadSkel(lg_path szFilename, lg_dword nFlags);
};


#endif __LG_MMGR_H__