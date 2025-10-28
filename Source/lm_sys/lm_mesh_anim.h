#ifndef __LM_MESH_ANIM_H__
#define __LM_MESH_ANIM_H__

#include "lm_mesh.h"
#include "lm_skel.h"
//#include "lm_sys.h"

class CLMeshAnim: public CLMesh2
{
//Internally used data:
protected:
	//Some structures to hold the stuff that is used every
	//frame but is only temporary.
	ML_MAT*   m_pAnimMats;
	ML_MAT*   m_pAttachMats;
	
	//The joint reference coordination array.
	lg_dword* m_pBoneRef;
public:
	CLMeshAnim();
	~CLMeshAnim();
	
	/* PRE: pSkel should be loaded.
		POST: The mesh will be compatible with the
		skeleton, and should properly be animated
		with the skeleton (assuming the skeleton was
		designed for the mesh, note taht when more than
		one skeleton is used with a mesh, then those
		skeletons should be compatible with each other
		or the animation will be haywire.
	*/
	lg_void SetCompatibleWith(CLSkel2* pSkel);
	
	/* PRE: N/A
		POST: Sets up the default frame for the mesh (base pose).
		DoTransform must be called to carry out the transform.
	*/
	lg_void SetupDefFrame();
	
	/* PRE: pSkel should be loaded properly and compatbile with the mesh.
		POST: Sets up the transformation frames.  DoTransform should still
		be called.
	*/
	lg_void SetupFrame(lg_dword dwFrame1, lg_dword dwFrame2, lg_float t, CLSkel2* pSkel);
	
	/* PRE: See above.  Chooses a particular animation form the skel
		and then loops through it approapriately.
		POST: See above, must call DoTransform afterwards.
	*/
	lg_void CLMeshAnim::SetupFrame(lg_dword nAnim, lg_float fTime, CLSkel2* pSkel);
	
	/* PRE: Called after a Setup method, this method then
		can transition to another skeleton's animation.
		POST: Should be setup, but DoTransform must be
		called to carry out the transform.
	*/
	lg_void TransitionTo(lg_dword nAnim, lg_float fTime, CLSkel2* pSkel);
	
	/* PRE: Should have called one of the Setup* methods
		POST: Transforms all the vertices according to the
		animation data that was Setup*.
		NOTE: This transforms whatever data was acquired by
		the LockTransfVB() method (weither it is a 
		IDirect3DVertexBuffer data or whatever.
	*/
	lg_void DoTransform();
	
	/* The following methods return the transform
		for a particular joint, and the attach transform.
		The attach transform is the transformation matrix
		that an object being attached to that particular
		joint needs to be transformed by, so to attach
		a head to a body, you'd get the transform for
		the neck and use that transform to position the
		head.
	*/
	lg_dword GetJointRef(lg_cstr szJoint);
	const ML_MAT* GetJointTransform(lg_dword nRef);
	const ML_MAT* GetJointAttachTransform(lg_dword nRef);
	
protected:
	/* PRE: Should be called somewhere within the
		overriden Load method, once the mesh header has
		been read.
		POST: Allocates memory and sets up structurs
		associated with the animation.  Returns false if
		it could not initialize the data.
	*/
	lg_bool InitAnimData();
	/* PRE: Should be called during the Unload overriden
		method.
		POST: Deletes all data created with InitAnimData.
	*/
	lg_void DeleteAnimData();
	
	
//Overrides:
protected:
	/* PRE: Called during the SetupMesh methods.
		POST: Must return a destination vertex buffer
		with consisting of a list of vertexes that
		will contain the transformed data.  Should
		return LG_NULL if it failed for some reason.
	*/
	virtual MeshVertex* LockTransfVB()=0;
	/* PRE: Called during the SetupMesh methods.
		POST: Should lock or do whatever is necessary
		to finalize the transformed vertexes.
	*/
	virtual void UnlockTransfVB()=0;
	
public:
	//This class doesn't override the Load or Save methods:
	//virtual lg_bool Load(LMPath szFile)=0;
	//virtual lg_bool Save(LMPath szFile)=0;
};

#endif __LM_MESH_ANIM_H__