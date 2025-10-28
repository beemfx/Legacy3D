#if 0
#ifndef __LM_SYS_H__
#define __LM_SYS_H__

#include <common.h>
#include "ML_lib.h"
#ifdef LM_USE_LF2
#include "lf_sys2.h"
#endif LM_USE_LF2


#define LMESH_ID      (*(lg_dword*)"LMSH")
#define LMESH_VERSION (122) //Final version will be 200.


#define LMESH_MAX_NAME (32)
#define LMESH_MAX_PATH (260)

//The Structures:
#pragma pack(1)

typedef char LMESH_NAME[LMESH_MAX_NAME];
typedef char LMESH_PATH[LMESH_MAX_PATH];


typedef struct _LSKEL_JOINT{
	LMESH_NAME szName;
	LMESH_NAME szParentBone;
	lg_dword nParentBone;
	float position[3];
	float rotation[3];
}LSKEL_JOINT;

typedef struct _LSKEL_JOINTEX: public LSKEL_JOINT{
	ML_MAT Local;
	ML_MAT Final;
	lg_dword    nJointRef;
}LSKEL_JOINTEX;

typedef struct _LSKEL_JOINTPOS{
	float position[3];
	float rotation[3];
}LSKEL_JOINTPOS;

typedef struct _LSKEL_ANIM{
	LMESH_NAME m_szName;
	lg_dword m_nFirstFrame;     //First frame for the animation.
	lg_dword m_nNumFrames;      //Number of frames in the animation.
	lg_dword m_nLoopingFrames;  //Number of frames to loop in the animation.
	float   m_fRate;           //Recommendation for rate at which animation should be processed.
	lg_dword m_nFlags;
}LSKEL_ANIM;

#define LSKEL_ANIM_DEFAULT  0x00000000
#define LSKEL_ANIM_LOOPBACK 0x00000001 //After the animation completes it's sequence of frames it then does them in reverse.


#pragma pack()


#define LSKEL_ID      (*(lg_dword*)"LSKL")
#define LSKEL_VERSION 105

//The Format:
class CLMesh;
class CLSkel;
class CLFrame;

//The read/write function is pretty similar to fread and fwrite
//the first paramter is the out buffer, the second and third
//are the size and count, and the fourth is the file stream.
//The return value is either the number of bytes written
//or read.
typedef lg_uint (__cdecl * LM_RW_FN)(void*,lg_uint,lg_uint,void*);

class CLSkel
{
public:
	friend class CLMesh;
	friend class CLMeshD3D;
	friend class CLMeshAnim;
public:
	CLSkel();
	~CLSkel();

	lg_bool Load(void* file, LM_RW_FN read);
	lg_bool Save(void* file, LM_RW_FN write);
	#ifdef LM_USE_LF2
	lg_bool Load(lf_path szFilename);
	#endif LM_USE_LF2
	lg_bool CalcExData();
	virtual lg_bool Unload();
	lg_dword GetNumFrames();
	lg_dword GetNumJoints();
	lg_dword GetNumKeyFrames();
	lg_dword GetNumAnims();
	lg_dword GetParentBoneRef(lg_dword nBone);
	ML_MAT* GenerateJointTransform(ML_MAT* pOut, lg_dword nJoint, lg_dword nFrame1, lg_dword nFrame2, float t);
	ML_MAT* GenerateJointTransform(ML_MAT* pOut, lg_dword nJoint, lg_dword nFrame1, lg_dword nFrame2, float t, CLSkel* pSkel2);
	ML_AABB* GenerateAABB(ML_AABB* pOut, lg_dword nFrame1, lg_dword nFrame2, float t);
	const ML_MAT* GetBaseTransform(lg_dword nJoint);
	
	lg_dword GetFrameFromTime(lg_dword nAnim, float fTime, float* pFrameTime, lg_dword* pFrame2);
	
	const LSKEL_ANIM* GetAnim(lg_dword n);
	
	lg_bool IsLoaded();
	
	lg_bool GetMinsAndMaxes(
		float* fXMin,
		float* fXMax,
		float* fYMin,
		float* fYMax,
		float* fZMin,
		float* fZMax);
protected:
	lg_bool AllocateMemory();
	void DeallocateMemory();
	
	lg_bool Serialize(
		void* file, 
		LM_RW_FN read_or_write, 
		lg_bool bLoading);
protected:
	lg_dword m_nID;
	lg_dword m_nVersion;
	LMESH_NAME m_szSkelName;
	lg_dword m_nNumJoints;
	lg_dword m_nNumKeyFrames;
	lg_dword m_nNumAnims;
	
	LSKEL_JOINTEX*  m_pBaseSkeleton;
	CLFrame* m_pFrames;
	LSKEL_ANIM* m_pAnims;

	lg_bool m_bLoaded;
public:
	static ML_MAT* EulerToMatrix(ML_MAT* pOut, float* pEuler);
	static lg_uint ReadLF2(void* buffer, lg_uint size, lg_uint count, void* stream);
};



//The CLFrame class, stores information about the frames.
class CLFrame
{
friend class CLSkel;
public:
	lg_dword m_nNumJoints;
	LSKEL_JOINTPOS* LocalPos;
	ML_AABB aabbBox;
	ML_MAT* Local;
	ML_MAT* Final;
	
public:
	CLFrame();
	CLFrame(lg_dword nNumJoints);
	~CLFrame();
	lg_bool Initialize(lg_dword nNumJoints);
	lg_dword GetNumJoints();
	const ML_MAT* GetFinalMat(lg_dword nJoint);
	const ML_MAT* GetLocalMat(lg_dword nJoint);
	
	lg_bool SetFinalMat(lg_dword nJoint, ML_MAT* pM);
	lg_bool SetLocalMat(lg_dword nJoint, ML_MAT* pM);
	lg_bool SetLocalMat(lg_dword nJoint, float* position, float* rotation);
};
#endif

#endif //__SM_SYS_H__