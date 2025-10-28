/* CLMeshTree - The mesh tree class.  In the game an objects
	appearance may contain one or more meshes which are combined
	together in a tree, the hiearchy described in an XML file.
	
	Each mesh is required to have a base mesh (and only one base
	mesh) with all other meshes attached to the base mesh.  There
	are limits on how many meshes a Mesh Tree can contain.  Also
	Meshes can be changed on the fly (as in the case of switching
	weapons, etc.).
	
	This class is exclusively for the Legacy game, and not any
	other utilities or plugins.
	
	Note that when multiple instances of a mesh or skel are used
	they will refer to the same data (which was aquired from
	their respective resource managers), but each in game object
	should have it's own instance of a CLMeshTree class (that way
	they can be animated independantly).
	
	Copyright (c) 2008 Beem Software
*/
#ifndef __MESH_TREE_H__
#define __MESH_TREE_H__

#include "lm_mesh_lg.h"
#include "lm_skel_lg.h"

class CLMeshTree: public CLMBase
{
//Constants & flags:
public:
	//Maximum number of nodes that can make up
	//a mesh tree:
	static const lg_dword MT_MAX_MESH_NODES=10;
	//Maximum number of skeletons to be used 
	//by a tree (applies to all nodes).
	static const lg_dword MT_MAX_SKELS=10; 
private:	
	/* The LM_FLAG_RASTER_READY indicates that the mesh
		is ready to be rendered (that is since Update was
		called, all the transformations have been carried
		out, if this flag isn't set and we want to render,
		then it is necessary to call DoTransform for each
		of the vertexes.
	*/
	static const lg_dword LM_FLAG_RASTER_READY=0x00010000;
	
//Internally used structures:
private:
	class MeshNode
	{
	friend class CLMeshTree;
	private:
		CLMeshLG* m_pMesh;    //The mesh assiated with the node.
		CLSkin*   m_pSkin;    //The skin associated with this node's mesh.
		CLSkelLG* m_pDefSkel; //The default skel associated with the mesh.
	
		//Data that changes as the animation changes:
		
		//Current animation:
		CLSkelLG* m_pSkel;         //Skel for the current animation.
		lg_dword  m_nAnim;         //Animation within skel for the current animation.
		lg_float  m_fAnimProgress; //The time of the current animation (0.0f to 100.0f).
		lg_float  m_fAnimSpeed;    //Number of seconds to cycle through the animation.
		
		lg_float  m_fElapsed;      //Amount of time elapsed since an update (in seconds).
		//lg_long   m_nTime;         //Time store for animation purposes.
		//lg_dword  m_nLastUpdate;   //Time that the animation was last updated.
		
		//Transitioning animation:
		CLSkelLG* m_pPrevSkel;         //The previous skel animation.
		lg_dword  m_nPrevAnim;         //The previous anim within the prev skel.
		lg_float  m_fPrevAnimProgress; //Previous progress.
		lg_float  m_fPostTransSpeed;   //The speed of the animation after transitions occur.
		
		//Que for transitiong animations:
		//If we switch animations while transitioning,
		//we have to wait for the transition to complete
		//before transitioning again, so we store the
		//animation we are waiting to transition to,
		//in the following variable.
		CLSkelLG* m_pNextSkel;
		lg_dword  m_nNextAnim;
		
		//Tree structures:
		lg_dword  m_nNumChildren;
		MeshNode* m_pChildren[MT_MAX_MESH_NODES]; //Children of this node.
		lg_dword  m_nParentJoint; //The reference to the joint of wich this node is attached.	
		
	public:
		MeshNode();
		~MeshNode();
		
		/* PRE: N/A.
			POST: Set's up the node with the default information specified.
			Set's animation information to default.
		*/
		lg_void Load(CLMeshLG* pMesh, CLSkin* pSkin, CLSkelLG* pSkel);
		/* PRE: N/A
			POST: Clears all information from the node.
		*/
		lg_void Clear();
		
		/* PRE: N/A
			POST: Renders the node, and all children nodes
			in the heierarchy.
		*/
		lg_void Render(ml_mat* pMatWorldTrans);
		
		/* PRE: pChild should be a valid Node.
			Should only be called by CLMeshTree::Load.
			POST: The child is added to heirarchy.
		*/ 
		lg_void AddChild(MeshNode* pChild, lg_str szJoint);
	};
	
//Private data:
private:
	//The nodes associated with the mesh,
	//note that m_MeshNodes[0] is always
	//the root node.
	MeshNode m_MeshNodes[MT_MAX_MESH_NODES];
	lg_dword m_nNumNodes;
	//The skeletons that can be used by the
	//mesh nodes (these are used by all nodes,
	//not just the root node).
	CLSkelLG* m_pSkels[MT_MAX_SKELS];
	lg_dword  m_nNumSkels;
	
	//Base transform, this transform is always applied:
	ml_mat m_matBaseTrans;
	
//User methods:
public:
	CLMeshTree();
	~CLMeshTree();
	
	/* PRE: N/A
		POST: Loads the tree as described in the XML files,
		returns LG_TRUE if it succeeded.
	*/
	lg_bool Load(lg_path szFile);
	/* PRE: pSrcTree must be a loaded mesh node.
		POST: Creates a mesh tree that is a duplicate
		of the source tree (it has the same meshes, skeletons,
		and skins, this is much faster than Load because it
		doesn't have to access the disk, or allocate memory.
		
		Note that some settings are set to default, and not 
		the info from the source tree.
		
		Returns LG_TRUE if it succeeded.
	*/
	lg_bool Duplicate(CLMeshTree* pSrcTree);
	
	/* PRE: N/A
		POST: If a mesh tree was loaded, it will be unlaoded.
	*/
	lg_void Unload();
	
	/* PRE: Mesh tree should be loaded.
		POST: Rasterizes the mesh based on the matrix passed.
	*/
	lg_void Render(ml_mat* pMatWorldTrans);
	
	/* PRE: Mesh tree should be loaded.  fDeltaTime>0
		fDeltaTimeSec represents the number of seconds that
		have passed since the last call to Update.
		POST: Generates the transform matricies for
		all of the meshes, based on how much time has
		passed.  Does not carry out the transform, because
		if we don't need to render the tree there is no point
		in carrying out the transform as that would be a
		waste of time.  Technically we don't need to call
		Update unless we are actually rendereing, but 
		fDeltaTimeSec should be the amount of time elapsed since
		the last call to Update.
	*/
	lg_void Update(lg_float fDeltaTimeSec);
	
	/* PRE: The mesh tree should be loaded.
		nNode: specifies which node we want to set the animation for, only
			this node will be effected 0 based.
		nSkel: specifies which skel we want to use for the anim, 0 based.
		nAnim: specifies the animation within the skeleton that we want
			to use, 0 based.
		fSpeed: specifies the number of seconds that it takes to complete
			one cycle of the animation.
		fTransitionTime: specifies the number of seconds that it takes to
			transition from the current animation to the animation specified.
			0.0f will make the transition immediate.
			
		POST: Subsequent calls to Update will use the new information to
		update the node specified.
		
		NOTES: Note that if SetAnimation is called on a MeshTree that
		is transitioning, then the transition isn't going to look
		verry smooth, because we don't store an entire buffer of
		transitions.  It is best to just try to wait til a transition
		is complete til changing an animation.  It might be a good
		idea to create another variable to set the next animation,
		that way if we are in a transition we can go to the next
		animation afterwards, or go right into it if we aren't.
	*/
	lg_void SetAnimation(lg_dword nNode, lg_dword nSkel, lg_dword nAnim, lg_float fSpeed, lg_float fTransitionTime);
	
	/* PRE: nFlags must have been prepared using the LWAI_SetAnim macro.
		POST: See above, and see lw_ai_sdk.h
	*/
	lg_void SetAnimation(lg_dword nFlags);
	/* PRE: nChild, should be a reference to on the the base nod's children.
		Should be called after Update.  Also note that if the base node doesn't
		have children, we'll run into problems.
		POST: pOut is filled in with the joint transform for the specified
		node.
	*/
	lg_void GetAttachTransform(ml_mat* pOut, lg_dword nChild);
private:
	/* PRE: N/A
		POST: N/A
		NOTES: Note used, but has to be overriden.
	*/
	lg_bool Serialize(lg_void* file, ReadWriteFn read_or_write, RW_MODE mode);
};

#endif __MESH_TREE_H__