/* ML_lib.h - Header LM_lib math library.

	Copyright (c) 2006, Blaine Myers.
	
	With code by:
	Patrick Baggett, 2005 (baggett.patrick@gmail.com)
	Under LGPL License
*/

#ifndef __ML_LIB_H__
#define __ML_LIB_H__

#include <d3dx9.h>

#ifdef __cplusplus
extern "C"{
#endif __cplusplus

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

/* A few definitions. */
#define ML_TRUE  (1)
#define ML_FALSE (0)

#define ML_FUNC __cdecl


/****************************************
	Some constants for the math library.
****************************************/

#define ML_PI ((float)3.141592654f)

/*************************************
	Structures for the math functions. 
*************************************/
typedef D3DXMATRIX ML_MAT;
typedef D3DXQUATERNION ML_QUAT;
typedef D3DXVECTOR2 ML_VEC2;
typedef D3DXVECTOR3 ML_VEC3;
typedef D3DXVECTOR4 ML_VEC4;

/***********************
	ML_vector2 functions.
***********************/

/***********************
	ML_VEC3 functions.
***********************/
#define ML_Vec3Add(pOut, pV1, pV2) (ML_VEC3*)D3DXVec3Add((D3DXVECTOR3*)pOut, (D3DXVECTOR3*)pV1, (D3DXVECTOR3*)pV2)
#define ML_Vec3Dot(pV1, pV2) D3DXVec3Dot((D3DXVECTOR3*)pV1, (D3DXVECTOR3*)pV2)
#define ML_Vec3Length(pV) D3DXVec3Length((D3DXVECTOR3*)pV)
#define ML_Vec3Magnitude ML_Vec3Length
#define ML_Vec3Normalize(pOut, pV) D3DXVec3Normalize((D3DXVECTOR3*)pOut, (D3DXVECTOR3*)pV)
#define ML_Vec3Scale(pOut, pV, s) D3DXVec3Scale((D3DXVECTOR3*)pOut, (D3DXVECTOR3*)pV, s)
#define ML_Vec3Subtract(ML_VEC3* pOut, const ML_VEC3* pV1, const ML_VEC3* pV2)
#define ML_Vec3Transform(pOut, pV, pM) D3DXVec3Transform(pOut, pV, pM)
#define ML_Vec3TransformCoord(pOut, pV, pM) D3DXVec3TransformCoord(pOut, pV, pM)
#define ML_Vec3TransformNormal(pOut, pV, pM) D3DXVec3TransformNormal(pOut, pV, pM)
#define ML_Vec3Distance(pV1, pV2) D3DXVec3Distance(pV1, pV2)

/***********************
	ML_vector4 functions.
***********************/
	

/******************** 
	Matrix functions. 
********************/

#define ML_MatIdentity D3DXMatrixIdentity
#define ML_MatMultiply D3DXMatrixMultiply
#define ML_MatRotationX D3DXMatrixRotationX
#define ML_MatRotationY D3DXMatrixRotationY
#define ML_MatRotationZ D3DXMatrixRotationZ
#define ML_MatPerspectiveFovLH D3DXMatrixPerspectiveFovLH
#define ML_MatRotationQuat D3DXMatrixRotationQuat
ML_MAT* ML_MatSlerp(ML_MAT* pOut, ML_MAT* pM1, ML_MAT* pM2, float t);
#define ML_MatDeterminant D3DXMatrixDeterminant
#define ML_MatInverse D3DXMatrixInverse

/*************************
	Quaternion functions.
*************************/
#define ML_QuatRotationMat D3DXQuaternionRotationMatrix
#define ML_QuatSlerp D3DXQauternionSlerp
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


#ifdef __cplusplus
}
#endif __cplusplus
#endif /*__ML_LIB_H__*/