/* ML_lib.h - Header LM_lib math library.

	Copyright (c) 2006, Blaine Myers.
	
	With code by:
	Patrick Baggett, 2005 (baggett.patrick@gmail.com)
	Under LGPL License
*/

#ifndef __ML_LIB_H__
#define __ML_LIB_H__

#ifdef __cplusplus
extern "C"{
#endif __cplusplus

#ifdef ML_LIB_BYTEPACK
#include <pshpack1.h>
#endif ML_LIB_BYTEPACK

/* Some types that we will use.*/
typedef unsigned char  ml_byte;
typedef signed char    ml_sbyte;
typedef unsigned short ml_word;
typedef signed short   ml_short;
typedef unsigned long  ml_dword;
typedef signed long    ml_long;
typedef int            ml_bool;
typedef unsigned int   ml_uint;
typedef signed int     ml_int;
typedef void           ml_void;
typedef float          ml_float;

/* A few definitions. */
#define ML_TRUE  (1)
#define ML_FALSE (0)

#define MLG_Max(v1, v2) ((v1)>(v2))?(v1):(v2)
#define MLG_Min(v1, v2) ((v1)<(v2))?(v1):(v2)
#define MLG_Clamp(v1, min, max) ( (v1)>(max)?(max):(v1)<(min)?(min):(v1) )

#define ML_FUNC __cdecl



/****************************************
	Some constants for the math library.
****************************************/

#define ML_PI     ((float)3.141592654f)
#define ML_2PI    ((float)3.141592654f*2.0f)
#define ML_HALFPI ((float)3.141592654f*0.5f)


/*************************************
	Structures for the math functions. 
*************************************/
typedef struct _ML_MATRIX{
	float _11, _12, _13, _14;
	float _21, _22, _23, _24;
	float _31, _32, _33, _34;
	float _41, _42, _43, _44;
}ML_MATRIX, ML_MAT;

typedef struct _ML_QUATERNION{
	float x, y, z, w;
}ML_QUATERNION, ML_QUAT;

typedef struct _ML_VECTOR2{
	float x, y;
}ML_VECTOR2, ML_VEC2;

typedef struct _ML_VECTOR3{
	float x, y, z;
}ML_VECTOR3, ML_VEC3;

typedef struct _ML_VECTOR4{
	float x, y, z, w;
}LML_VECTOR4, ML_VEC4;

typedef struct _ML_PLANE{
	float a, b, c, d;
}ML_PLANE;

typedef struct _ML_AABB{
	ML_VEC3 v3Min;
	ML_VEC3 v3Max;
}ML_AABB;

/***********************
	ML_vector2 functions.
***********************/

/***********************
	ML_VEC3 functions.
***********************/
ML_VEC3* ML_FUNC ML_Vec3Add(ML_VEC3* pOut, const ML_VEC3* pV1, const ML_VEC3* pV2);
ML_VEC3* ML_FUNC ML_Vec3Cross(ML_VEC3* pOut, const ML_VEC3* pV1, const ML_VEC3* pV2);
float ML_FUNC ML_Vec3Dot(const ML_VEC3* pV1, const ML_VEC3* pV2);
float ML_FUNC ML_Vec3Length(const ML_VEC3* pV);
float ML_FUNC ML_Vec3LengthSq(const ML_VEC3* pV);
#define ML_Vec3Magnitude ML_Vec3Magnitude
ML_VEC3* ML_FUNC ML_Vec3Normalize(ML_VEC3* pOut, const ML_VEC3* pV);
ML_VEC3* ML_FUNC ML_Vec3Scale(ML_VEC3* pOut, const ML_VEC3* pV, float s);
ML_VEC3* ML_FUNC ML_Vec3Subtract(ML_VEC3* pOut, const ML_VEC3* pV1, const ML_VEC3* pV2);
ML_VEC4* ML_FUNC ML_Vec3Transform(ML_VEC4* pOut, const ML_VEC3* pV, const ML_MAT* pM);
ML_VEC4* ML_FUNC ML_Vec3TransformArray(
	ML_VEC4* pOut, 
	ml_uint nOutStride, 
	const ML_VEC3* pV, 
	ml_uint nInStride, 
	const ML_MAT* pM, 
	ml_uint nNum);
ML_VEC3* ML_FUNC ML_Vec3TransformCoord(ML_VEC3* pOut, const ML_VEC3* pV, const ML_MAT* pM);
ML_VEC3* ML_FUNC ML_Vec3TransformCoordArray(
	ML_VEC3* pOut, 
	ml_uint nOutStride, 
	const ML_VEC3* pV, 
	ml_uint nInStride, 
	const ML_MAT* pM, 
	ml_uint nNum);
ML_VEC3* ML_FUNC ML_Vec3TransformNormal(ML_VEC3* pOut, const ML_VEC3* pV, const ML_MAT* pM);
ML_VEC3* ML_FUNC ML_Vec3TransformNormalArray(
	ML_VEC3* pOut, 
	ml_uint nOutStride, 
	const ML_VEC3* pV, 
	ml_uint nInStride, 
	const ML_MAT* pM, 
	ml_uint nNum);
float ML_FUNC ML_Vec3Distance(const ML_VEC3* pV1, const ML_VEC3* pV2);

/***********************
	ML_vector4 functions.
***********************/

/**********************
	ML_PLANE functions
**********************/

ml_float ML_FUNC ML_PlaneDotCoord(const ML_PLANE* pPlane, const ML_VEC3* pV);
ML_PLANE* ML_FUNC ML_PlaneScale(ML_PLANE* pOut, const ML_PLANE* pPlane, ml_float s);	

/******************** 
	Matrix functions. 
********************/

ML_MAT* ML_FUNC ML_MatIdentity(ML_MAT* pOut);
ML_MAT* ML_FUNC ML_MatMultiply(ML_MAT* pOut, const ML_MAT* pM1, const ML_MAT* pM2);
ML_MAT* ML_FUNC ML_MatRotationX(ML_MAT* pOut, float fAngle);
ML_MAT* ML_FUNC ML_MatRotationY(ML_MAT* pOut, float fAngle);
ML_MAT* ML_FUNC ML_MatRotationZ(ML_MAT* pOut, float fAngle);
ML_MAT* ML_FUNC ML_MatRotationAxis(ML_MAT* pOut, const ML_VEC3* pAxis, float fAngle);
ML_MAT* ML_FUNC ML_MatRotationYawPitchRoll(ML_MAT* pOut, float Yaw, float Pitch, float Roll);
ML_MAT* ML_FUNC ML_MatScaling(ML_MAT* pOut, float sx, float sy, float sz);
ML_MAT* ML_FUNC ML_MatPerspectiveFovLH(ML_MAT* pOut, float fovy, float Aspect, float zn, float zf);
ML_MAT* ML_FUNC ML_MatLookAtLH(ML_MAT* pOut, const ML_VEC3* pEye, const ML_VEC3* pAt, const ML_VEC3* pUp);
ML_MAT* ML_FUNC ML_MatRotationQuat(ML_MAT* pOut, const ML_QUAT* pQ);
ML_MAT* ML_FUNC ML_MatSlerp(ML_MAT* pOut, ML_MAT* pM1, ML_MAT* pM2, float t);
float ML_FUNC ML_MatDeterminant(ML_MAT* pM);
ML_MAT* ML_FUNC ML_MatInverse(ML_MAT* pOut, float* pDet, ML_MAT* pM);
ML_MAT* ML_FUNC ML_MatInverse_SSE(ML_MAT* pOut, float* pDet, ML_MAT* pM);

/*************************
	Quaternion functions.
*************************/
ML_QUAT* ML_FUNC ML_QuatRotationMat(ML_QUAT* pOut, const ML_MAT* pM);
ML_QUAT* ML_FUNC ML_QuatSlerp(ML_QUAT* pOut, const ML_QUAT* pQ1, const ML_QUAT* pQ2, float t);

/*******************
	AABB functions.
*******************/
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

ML_VEC3* ML_FUNC ML_AABBCorner(ML_VEC3* pV, const ML_AABB* pAABB, const ML_AABB_CORNER ref);
ML_AABB* ML_FUNC ML_AABBTransform(ML_AABB* pOut, const ML_AABB* pAABB, const ML_MAT* pM);
ML_AABB* ML_FUNC ML_AABBFromVec3s(ML_AABB* pOut, const ML_VEC3* pVList, const ml_dword nNumVecs);
ml_bool ML_FUNC ML_AABBIntersect(const ML_AABB* pBox1, const ML_AABB* pBox2, ML_AABB* pIntersect);
float ML_FUNC ML_AABBIntersectMoving(const ML_AABB* pBoxMove, const ML_AABB* pBoxStat, const ML_VEC3* pVel);
ML_AABB* ML_FUNC ML_AABBCatenate(ML_AABB* pOut, const ML_AABB* pBox1, const ML_AABB* pBox2);
ML_AABB* ML_FUNC ML_AABBAddPoint(ML_AABB* pOut, const ML_AABB* pBox, const ML_VEC3* pVec);


#define ML_INTERSECT_ONPOS  0x00000000
#define ML_INTERSECT_ONNEG  0x00000001
#define ML_INTERSECT_CUR    0x00000002
#define ML_INTERSECT_HITPOS 0x00000003
#define ML_INTERSECT_HITNEG 0x00000004

ml_bool ML_FUNC ML_AABBIntersectBlock(ML_AABB* pAABB, ML_PLANE* pPlanes, ml_dword nPlaneCount);
ml_dword ML_FUNC ML_AABBIntersectPlane(const ML_AABB* pAABB, const ML_PLANE* pPlane);
ml_float ML_FUNC ML_AABBIntersectPlaneVel(const ML_AABB* pAABB, const ML_PLANE* pPlane, const ML_VEC3* pVel);
ml_dword ML_FUNC ML_AABBIntersectPlaneVelType(const ML_AABB* pAABB, const ML_PLANE* pPlane, const ML_VEC3* pVel, ml_float* pTime);
ML_PLANE* ML_FUNC ML_AABBToPlanes(ML_PLANE* pOut, ML_AABB* pAABB);
/********************************
  Instruction Set Determination
********************************/


/****************** 
	Misc functions. 
******************/

ml_dword ML_FUNC ML_NextPow2(const ml_dword n); // Find the next highest power of 2.
ml_dword ML_FUNC ML_Pow2(const ml_byte n);      // Find (2^n).

float ML_FUNC ML_sqrtf(const float f);
float ML_FUNC ML_cosf(const float f);
float ML_FUNC ML_sinf(const float f);
float ML_FUNC ML_tanf(const float f);
float ML_FUNC ML_acosf(const float f);
float ML_FUNC ML_asinf(const float f);
float ML_FUNC ML_sincosf(const float f, float *cos);

ml_long ML_FUNC ML_absl(ml_long l);
ml_float ML_FUNC ML_absf(ml_float f);


/******************************
	Instruction set functions.
******************************/

typedef enum _ML_INSTR{
	ML_INSTR_F=     0x00000000,
	ML_INSTR_MMX=   0x00000001,
	ML_INSTR_SSE=   0x00000002,
	ML_INSTR_SSE2=  0x00000003,
	ML_INSTR_SSE3=  0x00000004,
	ML_INSTR_AMDMMX=0x00000005,
	ML_INSTR_3DNOW= 0x00000006,
	ML_INSTR_3DNOW2=0x00000007
}ML_INSTR;

ml_bool ML_SetSIMDSupport(ML_INSTR nInstr);
ML_INSTR ML_FUNC ML_SetBestSIMDSupport();

#ifdef ML_LIB_BYTEPACK
#include <poppack.h>
#endif ML_LIB_BYTEPACK

#ifdef __cplusplus
}
#endif __cplusplus
#endif /*__ML_LIB_H__*/