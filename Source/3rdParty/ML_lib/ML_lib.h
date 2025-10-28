#ifndef __ML_LIB2_H__
#define __ML_LIB2_H__

#include "ML_types.h"

#ifdef __cplusplus
extern "C" {
#endif __cplusplus

#ifdef ML_LIB_MAIN
#define ML_DECLARE_FUNC(out, fn) out (ML_FUNC * fn)
#else !ML_LIB_MAIN
#define ML_DECLARE_FUNC(out, fn) extern out (ML_FUNC * fn)
extern const ml_mat ML_matIdentity;
extern const ml_vec3 ML_v3Zero;
#endif ML_LIB_MAIN

typedef enum _ML_INSTR{
	ML_INSTR_BEST=0,
	ML_INSTR_F,
	ML_INSTR_MMX,
	ML_INSTR_SSE,
	ML_INSTR_SSE2,
	ML_INSTR_SSE3,
	ML_INSTR_AMDMMX,
	ML_INSTR_3DNOW,
	ML_INSTR_3DNOW2
}ML_INSTR;

/* PRE: N/A
	POST: Sets the instruction support, based on nInstr
	If the Instruction set was not supported  or
	if ML_ISNTR_BEST was passed the instruction will
	will default to the best instruction set available.
	Returns false if the library could not be initalized.
	NOTE: Must be called before any ML_* functions are used.
*/
ml_bool ML_FUNC ML_Init(ML_INSTR nInstr);
/* PRE: N/A
	POST: Returns true if the specified instruction
	set is supported.
*/
ml_bool  ML_FUNC ML_InstrSupport(ML_INSTR nInstr);
/* PRE: N/A
	POST: Returns the best available instruction set
	for the processor.
*/
ML_INSTR ML_FUNC ML_GetBestSupport();


/************************
*** ml_vec3 functions ***
************************/
ML_DECLARE_FUNC(ml_vec3*, ML_Vec3Add)(ml_vec3* pOut, const ml_vec3* pV1, const ml_vec3* pV2);
ML_DECLARE_FUNC(ml_vec3*, ML_Vec3Subtract)(ml_vec3* pOut, const ml_vec3* pV1, const ml_vec3* pV2);
ML_DECLARE_FUNC(ml_vec3*, ML_Vec3Cross)(ml_vec3* pOut, const ml_vec3* pV1, const ml_vec3* pV2);
ML_DECLARE_FUNC(ml_float, ML_Vec3Dot)(const ml_vec3* pV1, const ml_vec3* pV2);
ML_DECLARE_FUNC(ml_float, ML_Vec3Length)(const ml_vec3* pV);
ML_DECLARE_FUNC(ml_float, ML_Vec3LengthSq)(const ml_vec3* pV);
ML_DECLARE_FUNC(ml_vec3*, ML_Vec3Normalize)(ml_vec3* pOut, const ml_vec3* pV);
ML_DECLARE_FUNC(ml_vec3*, ML_Vec3Scale)(ml_vec3* pOut, const ml_vec3* pV, ml_float s);
ML_DECLARE_FUNC(ml_float, ML_Vec3Distance)(const ml_vec3* pV1, const ml_vec3* pV2);
ML_DECLARE_FUNC(ml_float, ML_Vec3DistanceSq)(const ml_vec3* pV1, const ml_vec3* pV2);
ML_DECLARE_FUNC(ml_vec4*, ML_Vec3Transform)(ml_vec4* pOut, const ml_vec3* pV, const ml_mat* pM);
ML_DECLARE_FUNC(ml_vec3*, ML_Vec3TransformCoord)(ml_vec3* pOut, const ml_vec3* pV, const ml_mat* pM);
ML_DECLARE_FUNC(ml_vec3*, ML_Vec3TransformNormal)(ml_vec3* pOut, const ml_vec3* pV, const ml_mat* pM);

/***********************
*** ml_mat functions *** 
***********************/
ml_mat*  ML_FUNC ML_MatIdentity(ml_mat* pOut);
ML_DECLARE_FUNC(ml_mat*, ML_MatMultiply)(ml_mat* pOut, const ml_mat* pM1, const ml_mat* pM2);
ml_mat*  ML_FUNC ML_MatRotationX(ml_mat* pOut, ml_float fAngle);
ml_mat*  ML_FUNC ML_MatRotationY(ml_mat* pOut, ml_float fAngle);
ml_mat*  ML_FUNC ML_MatRotationZ(ml_mat* pOut, ml_float fAngle);
ml_mat*  ML_FUNC ML_MatRotationAxis(ml_mat* pOut, const ml_vec3* pAxis, ml_float fAngle);
ml_mat*  ML_FUNC ML_MatRotationYawPitchRoll(ml_mat* pOut, ml_float Yaw, ml_float Pitch, ml_float Roll);
ml_mat*  ML_FUNC ML_MatScaling(ml_mat* pOut, ml_float sx, ml_float sy, ml_float sz);
ml_mat*  ML_FUNC ML_MatPerspectiveFovLH(ml_mat* pOut, ml_float fovy, ml_float Aspect, ml_float zn, ml_float zf);
ml_mat*  ML_FUNC ML_MatLookAtLH(ml_mat* pOut, const ml_vec3* pEye, const ml_vec3* pAt, const ml_vec3* pUp);
ml_mat*  ML_FUNC ML_MatRotationQuat(ml_mat* pOut, const ML_QUAT* pQ);
ml_mat*  ML_FUNC ML_MatSlerp(ml_mat* pOut, ml_mat* pM1, ml_mat* pM2, ml_float t);
ml_float ML_FUNC ML_MatDeterminant(ml_mat* pM);
ML_DECLARE_FUNC(ml_mat*, ML_MatInverse)(ml_mat* pOut, ml_float* pDet, const ml_mat* pM);
ml_mat*  ML_FUNC ML_MatTranslation(ml_mat* pOut, ml_float x, ml_float y, ml_float z);


/************************
*** ml_quat functions ***
************************/
ml_quat* ML_FUNC ML_QuatRotationMat(ml_quat* pOut, const ml_mat* pM);
ml_quat* ML_FUNC ML_QuatSlerp(ml_quat* pOut, const ml_quat* pQ1, const ml_quat* pQ2, ml_float t);

/*************************
*** ml_plane functions ***
*************************/

ml_float  ML_FUNC ML_PlaneDotCoord(const ML_PLANE* pPlane, const ml_vec3* pV);
ml_plane* ML_FUNC ML_PlaneScale(ml_plane* pOut, const ml_plane* pPlane, ml_float s);	

/************************
*** ml_aabb functions ***
************************/
typedef enum _ML_AABB_CORNER{
	ML_AABB_BLF=0x00, /* Botton Left Front */
	ML_AABB_BRF=0x01, /* Bottom Right Front */
	ML_AABB_TLF=0x02, /* Top Left Front */
	ML_AABB_TRF=0x03, /* Top Right Front */
	ML_AABB_BLB=0x04, /* Bottom Left Back */
	ML_AABB_BRB=0x05, /* Bottom Right Back */
	ML_AABB_TLB=0x06, /* Top Left Back */
	ML_AABB_TRB=0x07  /* Top Right Back */
}ML_AABB_CORNER;

ml_vec3* ML_FUNC ML_AABBCorner(ml_vec3* pV, const ml_aabb* pAABB, const ML_AABB_CORNER ref);
ml_aabb* ML_FUNC ML_AABBTransform(ml_aabb* pOut, const ml_aabb* pAABB, const ml_mat* pM);
ml_aabb* ML_FUNC ML_AABBFromVec3s(ml_aabb* pOut, const ml_vec3* pVList, const ml_dword nNumVecs);
ml_bool  ML_FUNC ML_AABBIntersect(const ml_aabb* pBox1, const ml_aabb* pBox2, ml_aabb* pIntersect);
ml_float ML_FUNC ML_AABBIntersectMoving(const ml_aabb* pBoxMove, const ml_aabb* pBoxStat, const ml_vec3* pVel);
ml_aabb* ML_FUNC ML_AABBCatenate(ml_aabb* pOut, const ml_aabb* pBox1, const ml_aabb* pBox2);
ml_aabb* ML_FUNC ML_AABBAddPoint(ml_aabb* pOut, const ml_aabb* pBox, const ml_vec3* pVec);


#define ML_INTERSECT_ONPOS  0x00000000
#define ML_INTERSECT_ONNEG  0x00000001
#define ML_INTERSECT_CUR    0x00000002
#define ML_INTERSECT_HITPOS 0x00000003
#define ML_INTERSECT_HITNEG 0x00000004

ml_bool   ML_FUNC ML_AABBIntersectBlock(ml_aabb* pAABB, ML_PLANE* pPlanes, ml_dword nPlaneCount);
ml_dword  ML_FUNC ML_AABBIntersectPlane(const ml_aabb* pAABB, const ML_PLANE* pPlane);
ml_float  ML_FUNC ML_AABBIntersectPlaneVel(const ml_aabb* pAABB, const ML_PLANE* pPlane, const ml_vec3* pVel);
ml_dword  ML_FUNC ML_AABBIntersectPlaneVelType(const ml_aabb* pAABB, const ml_plane* pPlane, const ml_vec3* pVel, ml_float* pTime);
ml_plane* ML_FUNC ML_AABBToPlanes(ml_plane* pOut, ml_aabb* pAABB);



ml_vec4* ML_FUNC ML_Vec3TransformArray(
	ml_vec4* pOut, 
	ml_uint nOutStride, 
	const ml_vec3* pV, 
	ml_uint nInStride, 
	const ml_mat* pM, 
	ml_uint nNum);
	
ml_vec3* ML_FUNC ML_Vec3TransformCoordArray(
	ml_vec3* pOut, 
	ml_uint nOutStride, 
	const ml_vec3* pV, 
	ml_uint nInStride, 
	const ml_mat* pM, 
	ml_uint nNum);

ml_vec3* ML_FUNC ML_Vec3TransformNormalArray(
	ml_vec3* pOut, 
	ml_uint nOutStride, 
	const ml_vec3* pV, 
	ml_uint nInStride, 
	const ml_mat* pM, 
	ml_uint nNum);



/*****************************
*** Miscelanious functions ***
*****************************/
ml_dword ML_FUNC ML_NextPow2(const ml_dword n); // Find the next highest power of 2.
ml_dword ML_FUNC ML_Pow2(const ml_byte n);      // Find (2^n).

ml_float ML_FUNC ML_sqrtf(const ml_float f);
ml_float ML_FUNC ML_cosf(const ml_float f);
ml_float ML_FUNC ML_sinf(const ml_float f);
ml_float ML_FUNC ML_tanf(const ml_float f);
ml_float ML_FUNC ML_acosf(const ml_float f);
ml_float ML_FUNC ML_asinf(const ml_float f);
ml_float ML_FUNC ML_sincosf(const ml_float f, ml_float *cos);

ml_long  ML_FUNC ML_absl(ml_long l);
ml_float ML_FUNC ML_absf(ml_float f);

#ifdef __cplusplus
}
#endif __cplusplus

#endif __ML_LIB2_H__