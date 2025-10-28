#include <string.h>
#include "lm_mesh_anim.h"
#include "lg_func.h"

CLMeshAnim::CLMeshAnim()
	: CLMesh2()
	, m_pAnimMats(LG_NULL)
	, m_pAttachMats(LG_NULL)
	, m_pBoneRef(LG_NULL)
{
	
}
CLMeshAnim::~CLMeshAnim()
{
	DeleteAnimData();
}

lg_bool CLMeshAnim::InitAnimData()
{
	//Make sure the data is deleted...
	DeleteAnimData();
	
	//Prepare the joints for rendering.
	m_pAnimMats=new ML_MAT[m_nNumBones*2];
	memset( m_pAnimMats , 0 , sizeof(*m_pAnimMats)*m_nNumBones*2 );
	m_pAttachMats=&m_pAnimMats[m_nNumBones];
	//Set up the bone references
	m_pBoneRef=new lg_dword[m_nNumBones];
	memset( m_pBoneRef , 0 , sizeof(*m_pBoneRef)*m_nNumBones );
	
	//We we couldn't allocate memory
	//the make sure no memory is allocated an return.
	if(!m_pAnimMats || !m_pBoneRef)
	{
		DeleteAnimData();
		return LG_FALSE;
	}
	
	for(lg_dword i=0; i<m_nNumBones; i++)
		m_pBoneRef[i]=i;
		
	return LG_TRUE;
}

lg_void CLMeshAnim::DeleteAnimData()
{
	if(m_pAnimMats){delete[]m_pAnimMats;}
	if(m_pBoneRef){delete[]m_pBoneRef;}
	
	m_pAnimMats=LG_NULL;
	m_pAttachMats=LG_NULL;
	m_pBoneRef=LG_NULL;
}

lg_void CLMeshAnim::SetupDefFrame()
{
	for(lg_dword i=0; i<m_nNumBones*2; i++)
		ML_MatIdentity(&m_pAnimMats[i]);
}


/**********************************************************************
*** Technically the prepare frame functions should
*** get a compatible skeleton, not all skeleton's are compatible
*** with every model so in the future there will be a method to
*** check compatibility, but this should not be called every frame
*** as it would only use unecessary processor power, instead
*** it is up to the the programmer to make sure a skeleton is
*** compatible before using it for rendering, as the function stands
*** right now the PrepareFrame functions simply try to prepare a
*** frame based off joint references
***********************************************************************/
lg_void CLMeshAnim::SetupFrame(lg_dword dwFrame1, lg_dword dwFrame2, lg_float t, CLSkel2* pSkel)
{
	if(!pSkel || !pSkel->IsLoaded())
	{
		SetupDefFrame();
		return;
	}
	
	ML_MAT* pJointTrans=(ML_MAT*)m_pAnimMats;
	//Prepare each joints final transformation matrix.
	for(lg_dword i=0; i<m_nNumBones; i++)
	{
		//To calculate the interpolated transform matrix
		//we need only call MCSkel's GenerateJointTransform method.
		pSkel->GenerateJointTransform(&pJointTrans[i], m_pBoneRef[i], dwFrame1, dwFrame2, t);
		//We don't necessarily need to setup attachment matrices for every bone (only for the
		//ones that actually get objects attached to them), but for now we set them up for
		//all bone anyway.
		ML_MatMultiply(&m_pAttachMats[i], pSkel->GetBaseTransform(m_pBoneRef[i]), &pJointTrans[i]);
	}
	
	return;
}


//TransformVerts is called to actually transform
//all the vertices in the mesh based upon the joint transforms.
lg_void CLMeshAnim::DoTransform()
{
	//Don't need to do the transform if the mesh isn't loaded.
	if(!LG_CheckFlag(m_nFlags, LM_FLAG_LOADED))
		return;
		
	//Lock the vertex buffer to prepare to write the data.
	MeshVertex* pVerts=LockTransfVB();
	//If we couldn't lock the buffer, there is nothing we can do.
	if(!pVerts)
		return;

	//Now that the transformations are all set up,
	//all we have to do is multiply each vertex by the 
	//transformation matrix for it's joint.  Note that
	//the texture coordinates have already been established
	//so they are not recopyed here.
	for(lg_dword i=0; i<m_nNumVerts; i++)
	{
		//Copy the vertex we want.
		//memcpy(&pVerts[i], &m_pVertices[i], sizeof(LMESH_VERTEX));
		
		//If the vertex's bone is -1 it means that it is
		//not attached to a joint so we don't need to transform it.
		if(m_pVertBoneList[i]!=-1)
		{
			//Multiply each vertex by it's joints transform.
			ML_Vec3TransformCoord((ML_VEC3*)&pVerts[i].x, (ML_VEC3*)&m_pVerts[i].x, &m_pAnimMats[m_pVertBoneList[i]]);	
			//Do the same for each vertex's normal, but we don't transorm by the
			//x,y,z.. so we call Vec3TransformNormal instead of Coord.
			ML_Vec3TransformNormal((ML_VEC3*)&pVerts[i].nx, (ML_VEC3*)&m_pVerts[i].nx, &m_pAnimMats[m_pVertBoneList[i]]);
		}
	}
	
	//We've written all the vertices so lets unlock the VB.
	UnlockTransfVB();
	return;
}


//TransitionTo should be called after a Setup* call, 
//it will generate transforms to transform the current frame to the new animation.
//DoTransform must be called again afterwards.
lg_void CLMeshAnim::TransitionTo(lg_dword nAnim, lg_float fTime, CLSkel2* pSkel)
{
	const CLSkel2::SkelAnim* pAnim=pSkel->GetAnim(nAnim);
	for(lg_dword i=0; i<m_nNumBones; i++)
	{
		ML_MAT matTemp;
		ML_MAT matTemp2;
		pSkel->GenerateJointTransform(&matTemp, m_pBoneRef[i], pAnim->nFirstFrame, pAnim->nFirstFrame, 0.0f);
		ML_MatMultiply(&matTemp2, pSkel->GetBaseTransform(m_pBoneRef[i]), &matTemp);
		ML_MatSlerp(&m_pAnimMats[i], &m_pAnimMats[i], &matTemp, fTime);
		ML_MatSlerp(&m_pAttachMats[i], &m_pAttachMats[i], &matTemp2, fTime);
	}
}

lg_dword CLMeshAnim::GetJointRef(lg_cstr szJoint)
{
	for(lg_dword i=0; i<m_nNumBones; i++)
	{
		if(_stricmp(m_pBoneNameList[i], szJoint)==0)
			return i;
	}
	return 0xFFFFFFFF;
}


const ML_MAT* CLMeshAnim::GetJointTransform(lg_dword nRef)
{
	if(nRef<m_nNumBones)
		return &m_pAnimMats[nRef];
	else
		return &ML_matIdentity;
}

const ML_MAT* CLMeshAnim::GetJointAttachTransform(lg_dword nRef)
{
	if(nRef<m_nNumBones)
		return &m_pAttachMats[nRef];//&m_pAnimMats[nRef+m_nNumBones];
	else
		return LG_NULL;
}


//This verison of prepare frames takes an animation and
//it wants a value from 0.0f to 100.0f.  0.0f would be
//the first frame of the animation, and 100.0f would also
//be the first frame of the animation.
lg_void CLMeshAnim::SetupFrame(lg_dword nAnim, lg_float fTime, CLSkel2* pSkel)
{
	lg_dword nFrame1, nFrame2;
	nFrame1=pSkel->GetFrameFromTime(nAnim, fTime, &fTime, &nFrame2);
	SetupFrame(nFrame1, nFrame2, fTime, pSkel);
}

lg_void CLMeshAnim::SetCompatibleWith(CLSkel2* pSkel)
{
	if(!pSkel || !pSkel->IsLoaded())
		return;
		
	for(lg_dword i=0; i<m_nNumBones; i++)
	{
		for(lg_dword j=0; j<pSkel->m_nNumJoints; j++)
		{
			if(_stricmp(m_pBoneNameList[i], pSkel->m_pBaseSkeleton[j].szName)==0)
			{
				m_pBoneRef[i]=j;
				j=pSkel->m_nNumJoints+1;
			}
		}
	}
}
