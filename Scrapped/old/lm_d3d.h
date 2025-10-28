#ifndef __LM_D3D_H__
#define __LM_D3D_H__

//#include <d3dx9.h>
#include <d3d9.h>
//#include "lm_sys.h"

#if 0
//Some temporary bone rendering structures
//and definitions.
typedef struct _JOINTVERTEX{
	float x, y, z;
	float psize;
	lg_dword color;
}JOINTVERTEX;
#define JOINTVERTEX_FVF (D3DFVF_XYZ|D3DFVF_PSIZE|D3DFVF_DIFFUSE)

typedef struct _BONEVERTEX{
	float x, y, z;
	lg_dword color;
}BONEVERTEX;
#define BONEVERTEX_FVF (D3DFVF_XYZ|D3DFVF_DIFFUSE)


class CLSkelD3D: public CLSkel
{
public:
	CLSkelD3D();
	~CLSkelD3D();
	lg_bool CreateD3DComponents(IDirect3DDevice9* lpDevice);
	lg_bool Validate();
	lg_bool Invalidate();
	lg_bool Render();
	lg_bool RenderAABB();
	lg_bool PrepareFrame(lg_dword nAnim, float fTime);
	lg_bool PrepareFrame(lg_dword nFrame1, lg_dword nFrame2, float fTime);
	virtual lg_bool Unload();
protected:
	IDirect3DDevice9* m_pDevice;
	lg_bool            m_bD3DValid;
	D3DCOLOR          m_nColor;
	//The skeleton rendering vertexes.
	JOINTVERTEX*      m_pSkel;
	BONEVERTEX        m_vAABB[16];
	ML_AABB           m_aabb; //The AABB to be transformed and rendered.
};
#endif

#ifdef LM_USE_LF2


#include "lm_mesh_lg.h"
#include "lm_skin.h"

class CLMeshNode
{
friend class CLMeshNode;
public:
	CLMeshLG*   m_pMesh;      //The mesh for this node
	CLSkelLG*   m_pDefSkel;   //The default skeleton for this mesh.
	CLSkin       m_Skin; //The skin for this node.
private:
	CLSkelLG*      m_pSkel;      //The current skel for this node
	lg_dword      m_nAnim;      //The animation to use in the current skel
	float        m_fTime;      //The time for the animation (0.0f to 100.0f).
	lg_long       m_nAnimSpeed; //How many milliseconds it takes to cycle through the animation
	lg_dword      m_nTime;      //The saved time for animation purposes
	lg_dword      m_nLastUpdate;//The last time the animation was updated
	//The following info is used for transition animations
	CLSkelLG*      m_pPrevSkel;  //The previous skeleton
	lg_dword      m_nPrevAnim;  //The previous animation within the previous skeleton
	float        m_fPrevTime;  //The previous time for the previous animation
	
	lg_dword      m_nTransTime; //How many milliseconds it should take to transition to the new animation
private:
	CLMeshNode*  m_pNext;        //The next node in the list.
	CLMeshNode*  m_pChildren;    //The children of this node
	CLMeshNode*  m_pParent;      //The parent of the current node (not used)
	lg_dword      m_nParentJoint; //The reference of the joint to which the child is attached.
public:
	CLMeshNode();
	~CLMeshNode();
	//Load a node.  Note that the nodes are only used for frame and animation purposes,
	//they don't allocate or deallocate any memory.  For this reason the load
	//method takes a preexisting mesh, the SetAnimation method takes a preexisting skeleton.
	//For this reason it is necessary that skeletons and meshes are not unloaded while
	//a mesh node still contains a reference to them.
	void Load(CLMeshLG* pMesh);
	void SetCompatibleWith(CLSkelLG* pSkel);
	void AddChild(CLMeshNode* pNode, lg_cstr szJoint);
	void Unload();
	//It is only necessary to call Render on the topmost parent node, all child nodes
	//will be render according to the necessary joint transform.
	void Render(const ML_MAT* matTrans);
	
	//UpdateTime should be called once a frame, this sets all the time parameters
	//necessary for setting up the animation.  Update time will update all child
	//nodes times as well so it is only necessary to call update time for the topmost
	//node in a set.
	void UpdateTime(lg_dword nTime);
	//SetAnimation changes the current animation to a new one, if the same animation
	//is specified the animation will not change.  It is only necessary to call this
	//method when changing to a different animation, however calling it every frame
	//with the same paramters will not change anything, but it will waste processor
	//time.
	void SetAnimation(CLSkelLG* pSkel, lg_dword nAnim, lg_long nSpeed, lg_dword nTransTime);
	
	const CLMeshNode* GetParent();
};



//We can only use the mesh loading functions if FS2 is being used,
//as fs2 is used for opening the xml file.
CLMeshNode* LM_LoadMeshNodes(lg_path szXMLScriptFile, lg_dword* nNumMeshes);
void LM_DeleteMeshNodes(CLMeshNode* pNodes);
#endif LM_USE_LF2

#endif __LM_D3D_H__