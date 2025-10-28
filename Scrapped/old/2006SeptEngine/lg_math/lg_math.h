/* lm_math.h - Header for the mathmetical functions for
	Legacy 3D game engine.

	Copyright (c) 2006, Blaine Myers.
*/
#ifndef __LM_MATH_H__
#define __LM_MATH_H__

#include "common.h"

#ifdef __cplusplus
extern "C"{
#endif __cplusplus

/*************************************
	Structures for the math functions. 
*************************************/
typedef struct _L_matrix{
	float _11, _12, _13, _14;
	float _21, _22, _23, _24;
	float _31, _32, _33, _34;
	float _41, _42, _43, _44;
}L_matrix, L_mat;

typedef struct _L_vector2{
	float x;
	float y;
}L_vector2, L_vec2;

typedef struct _L_vector3{
	float x;
	float y;
	float z;
}L_vector3, L_vec3;

typedef struct _L_vector4{
	float x;
	float y;
	float z;
	float w;
}L_vector4, L_vec4;

/***********************
	L_vector2 functions.
***********************/

/***********************
	L_vector3 functions.
***********************/
L_vector3* __cdecl L_vec3add(L_vector3* pOut, const L_vector3* pV1, const L_vector3* pV2);
L_vector3* __cdecl L_vec3cross(L_vector3* pOut, const L_vector3* pV1, const L_vector3* pV2);
float __cdecl L_vec3dot(const L_vector3* pV1, const L_vector3* pV2);
float __cdecl L_vec3length(const L_vector3* pV);
#define L_vec3magnitude L_vec3length
L_vector3* __cdecl L_vec3normalize(L_vector3* pOut, const L_vector3* pV);
L_vector3* __cdecl L_vec3scale(L_vector3* pOut, const L_vector3* pV, float s);
L_vector3* __cdecl L_vec3subtract(L_vector3* pOut, const L_vector3* pV1, const L_vector3* pV2);
//L_vector4* __cdecl L_vec3transform(L_vector4* pOut, const L_vector3* pV, const L_matrix* pM);
//L_vector3* __cdecl L_vec3transformcoord(L_vector3* pOut, const L_vector3* pV, const L_matrix* pM);
//L_vector3* __cdecl L_vec3transformnormal(L_vector3* pOut, const L_vector3* pV, const L_matrix* pM);
float __cdecl L_vec3distance(const L_vector3* PV1, const L_vector3* pV2);
float __cdecl L_vec3distance_C(const L_vector3* pV1, const L_vector3* pV2);
/***********************
	L_vector4 functions.
***********************/
	

/******************** 
	Matrix functions. 
********************/
L_matrix* __cdecl L_matident(L_matrix* pOut); /* Create an identity matrix. */
L_matrix* __cdecl L_matmultiply(L_matrix* pOut, const L_matrix* pM1, const L_matrix* pM2); /* Multiply (pM1 X pM2). */
L_matrix* __cdecl L_matmultiply_C(L_matrix* pOut, const L_matrix* pM1, const L_matrix* pM2); /* Same as above. */
/****************** 
	Misc functions. 
******************/
L_dword __cdecl L_nextpow2(const L_dword n); /* Find the next highest power of 2. */
L_dword __cdecl L_nextpow2_C(const L_dword n);   /* C Version. */
L_dword __cdecl L_pow2(const L_byte n);          /* Find (2^n). */

#ifdef __cplusplus
}
#endif __cplusplus
#endif /*__LM_MATH_H__*/