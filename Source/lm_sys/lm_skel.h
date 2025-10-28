#ifndef __LM_SKEL_H__
#define __LM_SKEL_H__

#include "lm_base.h"

class CLSkel2: public CLMBase
{
friend class CLMeshAnim;
//Constants & Flags:
public:
	static const lg_dword LM_SKEL_ID=0x4C4B534C;
	static const lg_dword LM_SKEL_VER=105;
//Privately used structures:
public:
	
	
	struct SkelJoint{
		LMName szName;    //32
		LMName szParent;  //32
		lg_dword nParent; // 4
		lg_float fPos[3]; //12
		lg_float fRot[3]; //12
		//Relevant size:    92
		//The following info is not currently
		//stored in the file format:
		ML_MAT matLocal;
		ML_MAT matFinal;
		lg_dword nJointRef;
		static const lg_dword SKEL_JOINT_SIZE_IN_FILE=92;
	};
	
	struct SkelJointPos{
		lg_float fPos[3];
		lg_float fRot[3];
	};
	
	struct SkelAnim{
		LMName szName;          //Animation name.
		lg_dword nFirstFrame;   //First frame of anim.
		lg_dword nNumFrames;    //Number of frams in the anim.
		lg_dword nLoopingFrames; //Number of frames to loop (unused, may get dropped).
		lg_float fRate;         //Recommended rate of animation.
		lg_dword nFlags;        //flags for the animation.
		
		static const lg_dword ANIM_FLAG_DEFAULT=0x00000000;
		static const lg_dword ANIM_FLAG_LOOPBACK=0x00000001; //Means loop forward then backward.
	};
	
	
	//The CLFrame class, stores information about the frames.
	class SkelFrame
	{
	public:
		lg_dword m_nNumJoints;
		SkelJointPos* LocalPos;
		ML_AABB aabbBox;
		ML_MAT* Local;
		ML_MAT* Final;
		
	public:
		SkelFrame();
		SkelFrame(lg_dword nNumJoints);
		~SkelFrame();
		lg_bool Initialize(lg_dword nNumJoints);
		lg_dword GetNumJoints();
		const ML_MAT* GetFinalMat(lg_dword nJoint);
		const ML_MAT* GetLocalMat(lg_dword nJoint);
		
		lg_bool SetFinalMat(lg_dword nJoint, ML_MAT* pM);
		lg_bool SetLocalMat(lg_dword nJoint, ML_MAT* pM);
		lg_bool SetLocalMat(lg_dword nJoint, float* position, float* rotation);
	};
	
public:
	friend class CLMAnim;
public:
	CLSkel2();
	~CLSkel2();
	
	virtual lg_bool Load(LMPath szFile)=0;
	virtual lg_bool Save(LMPath szFile)=0;
	
	virtual lg_void Unload();

	lg_bool  CalcExData();
	lg_dword GetNumFrames();
	lg_dword GetNumJoints();
	lg_dword GetNumKeyFrames();
	lg_dword GetNumAnims();
	lg_dword GetParentBoneRef(lg_dword nBone);
	ML_MAT* GenerateJointTransform(ML_MAT* pOut, lg_dword nJoint, lg_dword nFrame1, lg_dword nFrame2, float t);
	ML_MAT* GenerateJointTransform(ML_MAT* pOut, lg_dword nJoint, lg_dword nFrame1, lg_dword nFrame2, float t, CLSkel2* pSkel2);
	ML_AABB* GenerateAABB(ML_AABB* pOut, lg_dword nFrame1, lg_dword nFrame2, lg_float t);
	const ML_MAT* GetBaseTransform(lg_dword nJoint);
	
	lg_dword GetFrameFromTime(lg_dword nAnim, lg_float fTime, lg_float* pFrameTime, lg_dword* pFrame2);
	
	const SkelAnim* GetAnim(lg_dword n);
	
	lg_bool IsLoaded();
	
	const ml_aabb* GetMinsAndMaxes(ml_aabb* pBounds);
protected:
	lg_bool AllocateMemory();
	void DeallocateMemory();
	
	lg_bool Serialize(
		lg_void* file, 
		ReadWriteFn read_or_write, 
		RW_MODE mode);
protected:
	lg_dword m_nID;
	lg_dword m_nVersion;
	LMName   m_szSkelName;
	lg_dword m_nNumJoints;
	lg_dword m_nNumKeyFrames;
	lg_dword m_nNumAnims;
	
	SkelJoint* m_pBaseSkeleton;
	SkelFrame* m_pFrames;
	SkelAnim*  m_pAnims;
public:
	static ML_MAT* EulerToMatrix(ML_MAT* pOut, float* pEuler);
};

#endif __LM_SKEL_H__