#include "lm_skel.h"
#include "ML_lib.h"
#include "lg_func.h"


//Alright here is the skeleton class.
//Further this needs to support some kind of
//mechanism for pointing out different animations
//based on a range of animations.

//Also need to change the way the model gets rendered.
//Either CLegacyMesh needs to have a RenderWithSkel funciton.
//Or CLSkel2 needs to have a RenderModel functions.


CLSkel2::CLSkel2():
CLMBase(),
m_nID(0),
m_nVersion(0),
m_nNumJoints(0),
m_nNumKeyFrames(0),
m_pBaseSkeleton(LG_NULL),
m_pFrames(LG_NULL),
m_nNumAnims(0),
m_pAnims(LG_NULL)
{
	m_szSkelName[0]=0;
}

CLSkel2::~CLSkel2()
{
	Unload();
}

lg_dword CLSkel2::GetNumAnims()
{
	return m_nNumAnims;
}

lg_dword CLSkel2::GetNumJoints()
{
	return m_nNumJoints;
}
lg_dword CLSkel2::GetNumKeyFrames()
{
	return m_nNumKeyFrames;
}
lg_bool CLSkel2::IsLoaded()
{
	return LG_CheckFlag(m_nFlags, LM_FLAG_LOADED);
}
lg_dword CLSkel2::GetParentBoneRef(lg_dword nBone)
{
	return m_pBaseSkeleton[nBone].nParent;
}

lg_void CLSkel2::Unload()
{
	DeallocateMemory();

	m_nID=0;
	m_nVersion=0;
	m_nNumJoints=0;
	m_nNumKeyFrames=0;
	m_nFlags=0;
	m_nNumAnims=0;
}
#if 0
//The read and write LF2 functions are used for the legacy mesh
//and legacy skel serialize methods when the LF2 file system is in
//use.
lg_uint CLSkel2::ReadLF2(void* buffer, lg_uint size, lg_uint count, void* stream)
{
	return LF_Read((LF_FILE3)stream, buffer, size*count);
}

lg_bool CLSkel2::Load(lf_path szFilename)
{
	Unload();
	
	LF_FILE3 fIn=LF_Open(szFilename, LF_ACCESS_READ, LF_OPEN_EXISTING);
	
	if(!fIn)
		return LG_FALSE;
	
	lg_bool bResult=Serialize(fIn, ReadLF2, LG_TRUE);
		
	if(!bResult)
		Unload();
	
	m_bLoaded=bResult;
	//We will now get all the calculations for the extra data (the local
	//and final matrices for the base skeleton and keyframes).
	//this->CalcExData();
	if(m_bLoaded)
	{
		CalcExData();
	}
	LF_Close(fIn);
	return m_bLoaded;
}
#endif
#if 0
lg_bool CLSkel2::Load(void* file, LM_RW_FN read)
{	
	//Unload in case a skeleton is already loaded.
	Unload();
	
	lg_bool bResult=Serialize(
		file,
		read,
		LG_TRUE);
		
	if(!bResult)
		Unload();
		
	//pcb->close(file);
	
	m_bLoaded=bResult;
	//We will now get all the calculations for the extra data (the local
	//and final matrices for the base skeleton and keyframes).
	//this->CalcExData();
	if(m_bLoaded)
	{
		CalcExData();
	}
	return m_bLoaded;
}

lg_bool CLSkel2::Save(void* file, LM_RW_FN write)
{
	if(!m_bLoaded)
		return LG_FALSE;
		
	if(!m_pBaseSkeleton || !m_pFrames)
		return LG_FALSE;
	
	if(!file)
		return LG_FALSE;
		
	return Serialize(
		file,
		write,
		LG_FALSE);
}
#endif

lg_bool CLSkel2::Serialize(
	lg_void* file, 
	ReadWriteFn read_or_write, 
	RW_MODE mode)
{
	lg_dword i=0, j=0;
	
	//Read or Write the file header.
	read_or_write(file, &m_nID, 4);
	read_or_write(file, &m_nVersion, 4);
	read_or_write(file, m_szSkelName, 32);
	read_or_write(file, &m_nNumJoints, 4);
	read_or_write(file, &m_nNumKeyFrames, 4);
	read_or_write(file, &m_nNumAnims, 4);
	
	if((m_nID != LM_SKEL_ID) || (m_nVersion != LM_SKEL_VER))
		return LG_FALSE;
		
	//Allocate memory if we are loading.
	if(mode==RW_READ)
		if(!AllocateMemory())return LG_FALSE;
		
	//Read or write the base skeleton.
	for(i=0; i<m_nNumJoints; i++)
	{
		read_or_write(file, &m_pBaseSkeleton[i], SkelJoint::SKEL_JOINT_SIZE_IN_FILE);
	}
	
	//Read or write the key frames.
	for(i=0; i<m_nNumKeyFrames; i++)
	{
		for(j=0; j<m_nNumJoints; j++)
		{
			read_or_write(file, &m_pFrames[i].LocalPos[j], sizeof(SkelJointPos));
		}
		//Write the bounding box.
		read_or_write(file, &m_pFrames[i].aabbBox, sizeof(ML_AABB));
	}
	//Read or write the animation information.
	for(i=0; i<m_nNumAnims; i++)
	{
		read_or_write(file, &m_pAnims[i], sizeof(SkelAnim));
	}
	return LG_TRUE;
}

const ml_aabb* CLSkel2::GetMinsAndMaxes(ml_aabb* pBounds)
{
	//This only gets the mins and the maxes for the default frame, it
	//does not get mins and maxes for keyframes.
	ml_aabb bounds;
	lg_dword i=0;
	bounds.v3Min.x=m_pBaseSkeleton[0].matFinal._41;
	bounds.v3Max.x=m_pBaseSkeleton[0].matFinal._41;
	bounds.v3Min.y=m_pBaseSkeleton[0].matFinal._42;
	bounds.v3Max.y=m_pBaseSkeleton[0].matFinal._42;
	bounds.v3Min.z=m_pBaseSkeleton[0].matFinal._43;
	bounds.v3Max.z=m_pBaseSkeleton[0].matFinal._43;
	
	for(i=1; i<m_nNumJoints; i++)
	{
		bounds.v3Min.x=LG_Min(bounds.v3Min.x, m_pBaseSkeleton[i].matFinal._41);
		bounds.v3Max.x=LG_Max(bounds.v3Max.x, m_pBaseSkeleton[i].matFinal._41);
		bounds.v3Min.y=LG_Min(bounds.v3Min.y, m_pBaseSkeleton[i].matFinal._42);
		bounds.v3Max.y=LG_Max(bounds.v3Max.y, m_pBaseSkeleton[i].matFinal._42);
		bounds.v3Min.z=LG_Min(bounds.v3Min.z, m_pBaseSkeleton[i].matFinal._43);
		bounds.v3Max.z=LG_Max(bounds.v3Max.z, m_pBaseSkeleton[i].matFinal._43);
	}
	
	*pBounds=bounds;
	return pBounds;
}

const CLSkel2::SkelAnim* CLSkel2::GetAnim(lg_dword n)
{
	n=LG_Clamp(n, 0, m_nNumAnims-1);	
	return &m_pAnims[n];
}

lg_dword CLSkel2::GetFrameFromTime(lg_dword nAnim, float fTime, float* pFrameTime, lg_dword* pFrame2)
{
	#define PREP_MAX_RANGE 100.0f
	if(!m_nNumAnims)
		return 0;
	
	nAnim=LG_Clamp(nAnim, 0, m_nNumAnims-1);
	fTime=LG_Clamp(fTime, 0.0f, PREP_MAX_RANGE);
	
	float fFrame;
	lg_dword nFrame1, nFrame2;
	
	if(LG_CheckFlag(m_pAnims[nAnim].nFlags, SkelAnim::ANIM_FLAG_LOOPBACK))
	{
		if(fTime>=50.0f)
			fTime=100.0f-fTime;
		
		fFrame=m_pAnims[nAnim].nFirstFrame + 
			((float)m_pAnims[nAnim].nNumFrames-1-0.000001f) * 
			fTime/(PREP_MAX_RANGE*0.5f);
	}
	else
	{
		fFrame=m_pAnims[nAnim].nFirstFrame + 
			((float)m_pAnims[nAnim].nNumFrames-0.000001f) * 
			fTime/(PREP_MAX_RANGE);
	}
	
	nFrame1=(lg_dword)fFrame;
	nFrame2=nFrame1>=(m_pAnims[nAnim].nFirstFrame+m_pAnims[nAnim].nNumFrames-1)?m_pAnims[nAnim].nFirstFrame:nFrame1+1;
	
	if(pFrameTime)
		*pFrameTime=fFrame-nFrame1;
	if(pFrame2)
		*pFrame2=nFrame2;
		
	return nFrame1;
}

ML_AABB* CLSkel2::GenerateAABB(ML_AABB* pOut, lg_dword nFrame1, lg_dword nFrame2, float t)
{
	if(!pOut)
		return pOut;
		
	nFrame1=LG_Clamp(nFrame1, 1, m_nNumKeyFrames);
	nFrame2=LG_Clamp(nFrame2, 1, m_nNumKeyFrames);
	t=LG_Clamp(t, 0.0f, 1.0f);
	//We should probably have an AABB for frame 0 (the default frame).
	ML_AABB *a, *b;
	a=&m_pFrames[nFrame1-1].aabbBox;
	b=&m_pFrames[nFrame2-1].aabbBox;
	pOut->v3Min.x=a->v3Min.x+(b->v3Min.x-a->v3Min.x)*t;
	pOut->v3Min.y=a->v3Min.y+(b->v3Min.y-a->v3Min.y)*t;
	pOut->v3Min.z=a->v3Min.z+(b->v3Min.z-a->v3Min.z)*t;
	
	pOut->v3Max.x=a->v3Max.x+(b->v3Max.x-a->v3Max.x)*t;
	pOut->v3Max.y=a->v3Max.y+(b->v3Max.y-a->v3Max.y)*t;
	pOut->v3Max.z=a->v3Max.z+(b->v3Max.z-a->v3Max.z)*t;
	return pOut;
}
ML_MAT* CLSkel2::GenerateJointTransform(ML_MAT* pOut, lg_dword nJoint, lg_dword nFrame1, lg_dword nFrame2, float t)
{
	if(!pOut)
		return LG_NULL;
	nJoint=LG_Clamp(nJoint, 0, m_nNumJoints-1);
	nFrame1=LG_Clamp(nFrame1, 0, m_nNumKeyFrames);
	nFrame2=LG_Clamp(nFrame2, 0, m_nNumKeyFrames);
	t=LG_Clamp(t, 0.0f, 1.0f);
	
	
	const ML_MAT *pM1, *pM2;
	ML_MAT MI;
	ML_MatIdentity(&MI);
	
	pM1=nFrame1>0?m_pFrames[nFrame1-1].GetFinalMat(nJoint):&MI;
	pM2=nFrame2>0?m_pFrames[nFrame2-1].GetFinalMat(nJoint):&MI;
	
	return ML_MatSlerp(
		pOut,
		(ML_MAT*)pM1,
		(ML_MAT*)pM2,
		t);
}

//This second version of GenerateJointTransform uses a second skeleton to generate
//the transform.  Note that both skeletons should be compatible with each other and
//have the same base skeleton, or the transformation may not be pretty.
ML_MAT* CLSkel2::GenerateJointTransform(ML_MAT* pOut, lg_dword nJoint, lg_dword nFrame1, lg_dword nFrame2, float t, CLSkel2* pSkel2)
{
	if(!pOut)
		return LG_NULL;
	nJoint=LG_Clamp(nJoint, 0, m_nNumJoints-1);
	nFrame1=LG_Clamp(nFrame1, 0, m_nNumKeyFrames);
	nFrame2=LG_Clamp(nFrame2, 0, pSkel2->m_nNumKeyFrames);
	t=LG_Clamp(t, 0.0f, 1.0f);
	
	
	const ML_MAT *pM1, *pM2;
	ML_MAT MI;
	ML_MatIdentity(&MI);
	
	pM1=nFrame1>0?m_pFrames[nFrame1-1].GetFinalMat(nJoint):&MI;
	pM2=nFrame2>0?pSkel2->m_pFrames[nFrame2-1].GetFinalMat(nJoint):&MI;
	
	return ML_MatSlerp(
		pOut,
		(ML_MAT*)pM1,
		(ML_MAT*)pM2,
		t);
}

const ML_MAT* CLSkel2::GetBaseTransform(lg_dword nJoint)
{
	nJoint=LG_Clamp(nJoint, 0, m_nNumJoints-1);
	
	return &m_pBaseSkeleton[nJoint].matFinal;
}

lg_bool CLSkel2::AllocateMemory()
{
	lg_dword i=0, j=0;
	//Deallocate memory first just in case,
	//note that because the poitners are set to null in
	//the constructor it is okay to call this before they
	//are initialized.
	DeallocateMemory();
	//Allocate memory for the frames and base skeleton.
	//m_pKeyFrames=new LMESH_KEYFRAME[m_nNumKeyFrames];
	
	m_pFrames=new SkelFrame[m_nNumKeyFrames];
	if(!m_pFrames)
		return LG_FALSE;
		
	for(i=0; i<m_nNumKeyFrames;i++)
	{
		if(!m_pFrames[i].Initialize(m_nNumJoints))
		{
			LG_SafeDeleteArray(m_pFrames);
			return LG_FALSE;
		}
	}

	m_pBaseSkeleton=new SkelJoint[m_nNumJoints];
	if(!m_pBaseSkeleton)
	{
		LG_SafeDeleteArray(m_pFrames);
		return LG_FALSE;
	}
	
	m_pAnims=new SkelAnim[m_nNumAnims];
	if(!m_pAnims)
	{
		LG_SafeDeleteArray(m_pFrames);
		LG_SafeDeleteArray(m_pBaseSkeleton);
		return LG_FALSE;
	}
	return LG_TRUE;
}

void CLSkel2::DeallocateMemory()
{
	LG_SafeDeleteArray(m_pBaseSkeleton);
	LG_SafeDeleteArray(m_pFrames);
	LG_SafeDeleteArray(m_pAnims);
}


lg_dword CLSkel2::GetNumFrames()
{
	return m_nNumKeyFrames;
}


lg_bool CLSkel2::CalcExData()
{
	lg_dword i=0, j=0;

	if(!LG_CheckFlag(m_nFlags, LM_FLAG_LOADED))
		return LG_FALSE;

	//Firstly we must convert Euler angles for the base skeleton and
	//keyframes to matrices.
	//Read the base skeleton.
	for(i=0; i<m_nNumJoints; i++)
	{

		//Now create the rotation matrixes (in the final format the rotation
		//matrices will probably be stored instead of the euler angles.
		EulerToMatrix((ML_MAT*)&m_pBaseSkeleton[i].matLocal, (float*)&m_pBaseSkeleton[i].fRot);
		//Now stick the translation into the matrix.
		m_pBaseSkeleton[i].matLocal._41=m_pBaseSkeleton[i].fPos[0];
		m_pBaseSkeleton[i].matLocal._42=m_pBaseSkeleton[i].fPos[1];
		m_pBaseSkeleton[i].matLocal._43=m_pBaseSkeleton[i].fPos[2];

		m_pBaseSkeleton[i].nJointRef=i;
	}

	//Read the key frames.
	
	for(i=0; i<m_nNumKeyFrames; i++)
	{
		for(j=0; j<m_nNumJoints; j++)
		{
			EulerToMatrix(&m_pFrames[i].Local[j], (float*)&m_pFrames[i].LocalPos[j].fRot);
			m_pFrames[i].Local[j]._41=m_pFrames[i].LocalPos[j].fPos[0];
			m_pFrames[i].Local[j]._42=m_pFrames[i].LocalPos[j].fPos[1];
			m_pFrames[i].Local[j]._43=m_pFrames[i].LocalPos[j].fPos[2];
		}
	}

	//Calculate the final matrices for the base skeleton.
	//This is simply a matter of multiplying each joint by
	//all of it's parent's matrices.
	for(i=0; i<m_nNumJoints; i++)
	{
		SkelJoint* pTemp=&m_pBaseSkeleton[i];
		m_pBaseSkeleton[i].matFinal=pTemp->matLocal;
		while(pTemp->nParent)
		{
			pTemp=&m_pBaseSkeleton[pTemp->nParent-1];
			ML_MatMultiply((ML_MAT*)&m_pBaseSkeleton[i].matFinal, (ML_MAT*)&m_pBaseSkeleton[i].matFinal, (ML_MAT*)&m_pBaseSkeleton[pTemp->nJointRef].matLocal);
		}
	}

	//We calculate the final transformation matrix for each joint for each
	//frame now, that way we don't have to calculate them on the fly.  It
	//takes more memory this way, but the real time rendering is faster because
	//it isn't necessary to calculate each frames matrix every frame, it is only
	//necessary to interploate the joint matrixes for the given frame.
	
	//For each joint...
	for(i=0; i<m_nNumJoints; i++)
	{
		//For each frame for each joint...
		for(j=0; j<m_nNumKeyFrames; j++)
		{
			//1. Obtain the base joint for that joint.
			SkelJoint* pTemp=&m_pBaseSkeleton[i];
			//2. Start out by making the final translation for the frame the local frame joint
			//location multiplyed the by the local base joint.
			
			//ML_MatMultiply((ML_MAT*)&m_pKeyFrames[j].pJointPos[i].Final, (ML_MAT*)&m_pKeyFrames[j].pJointPos[i].Local, (ML_MAT*)&pTemp->Local);
			ML_MatMultiply((ML_MAT*)&m_pFrames[j].Final[i], &m_pFrames[j].Local[i], &pTemp->matLocal);
			//3. Then if the joint has a parent...
			while(pTemp->nParent)
			{
				//3 (cont'd). It is necessary to multiply the final frame matrix
				//by the same calculation in step 2 (frame pos for the frame * local pos for the frame).
				pTemp=&m_pBaseSkeleton[pTemp->nParent-1];
				ML_MAT MT;
				ML_MatMultiply(
					(ML_MAT*)&m_pFrames[j].Final[i],
					(ML_MAT*)&m_pFrames[j].Final[i],
					ML_MatMultiply(&MT, (ML_MAT*)&m_pFrames[j].Local[pTemp->nJointRef], (ML_MAT*)&pTemp->matLocal));
				
			}

			//4. Fianlly the final position needs to be multiplied by the
			//final base position's inverse so that the transformation is
			//relative to the joint's location and not to 0,0,0.
			ML_MAT MI;
			ML_MatInverse(&MI, LG_NULL, &m_pBaseSkeleton[i].matFinal);
			ML_MatMultiply((ML_MAT*)&m_pFrames[j].Final[i], &MI, (ML_MAT*)&m_pFrames[j].Final[i]);
		
		}
		
	}
	return LG_TRUE;
}

ML_MAT* CLSkel2::EulerToMatrix(ML_MAT* pOut, float* pEuler)
{
	ML_MAT MX, MY, MZ;
	ML_MatRotationX(&MX, -pEuler[0]);
	ML_MatRotationY(&MY, -pEuler[1]);
	ML_MatRotationZ(&MZ, -pEuler[2]);
		
	return ML_MatMultiply(
		pOut,
		&MX,
		ML_MatMultiply(&MY, &MY, &MZ));
}


///////////////////////////////
/// SkelFrame Member Methods  ///
///////////////////////////////

CLSkel2::SkelFrame::SkelFrame():
	Local(LG_NULL),
	Final(LG_NULL),
	m_nNumJoints(0),
	LocalPos(LG_NULL)
{
	
}

CLSkel2::SkelFrame::SkelFrame(lg_dword nNumJoints):
	Local(LG_NULL),
	Final(LG_NULL),
	m_nNumJoints(0),
	LocalPos(LG_NULL)
{
	Initialize(nNumJoints);
}

CLSkel2::SkelFrame::~SkelFrame()
{
	LG_SafeDeleteArray(Local);
	LG_SafeDeleteArray(Final);
	LG_SafeDeleteArray(LocalPos);
}

lg_bool CLSkel2::SkelFrame::Initialize(lg_dword nNumJoints)
{
	LG_SafeDeleteArray(Local);
	LG_SafeDeleteArray(Final);
	LG_SafeDeleteArray(LocalPos);
	
	m_nNumJoints=nNumJoints;
	Local=new ML_MAT[nNumJoints];
	Final=new ML_MAT[nNumJoints];
	LocalPos=new SkelJointPos[nNumJoints];
	
	if(!Local || !Final || !LocalPos)
		return LG_FALSE;
		
	float f[3]={0.0f, 0.0f, 0.0f};
	for(lg_dword i=0; i<nNumJoints; i++)
		SetLocalMat(i, f, f);
	return LG_TRUE;
}

lg_dword CLSkel2::SkelFrame::GetNumJoints()
{
	return m_nNumJoints;
}

const ML_MAT* CLSkel2::SkelFrame::GetFinalMat(lg_dword nJoint)
{
	nJoint=LG_Clamp(nJoint, 0, m_nNumJoints-1);
	return &Final[nJoint];
}

const ML_MAT* CLSkel2::SkelFrame::GetLocalMat(lg_dword nJoint)
{
	nJoint=LG_Clamp(nJoint, 0, m_nNumJoints-1);
	return &Local[nJoint];
}

lg_bool CLSkel2::SkelFrame::SetFinalMat(lg_dword nJoint, ML_MAT* pM)
{
	if(nJoint>=m_nNumJoints)
		return LG_FALSE;
		
	Final[nJoint]=*pM;
	return LG_TRUE;
}

lg_bool CLSkel2::SkelFrame::SetLocalMat(lg_dword nJoint, float* position, float* rotation)
{
	if(nJoint>=m_nNumJoints)
		return LG_FALSE;
		
	LocalPos[nJoint].fPos[0]=position[0];
	LocalPos[nJoint].fPos[1]=position[1];
	LocalPos[nJoint].fPos[2]=position[2];
	
	LocalPos[nJoint].fRot[0]=rotation[0];
	LocalPos[nJoint].fRot[1]=rotation[1];
	LocalPos[nJoint].fRot[2]=rotation[2];
	
	CLSkel2::EulerToMatrix(&Local[nJoint], rotation);
	
	Local[nJoint]._41=position[0];
	Local[nJoint]._42=position[1];
	Local[nJoint]._43=position[2];
	return LG_TRUE;
}

lg_bool CLSkel2::SkelFrame::SetLocalMat(lg_dword nJoint, ML_MAT* pM)
{
	if(nJoint>=m_nNumJoints)
		return LG_FALSE;
	
	Local[nJoint]=*pM;
	return LG_TRUE;
}

