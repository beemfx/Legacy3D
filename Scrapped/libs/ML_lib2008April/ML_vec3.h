#ifndef __ML_VEC3_H__
#define __ML_VEC3_H__

#include "ML_lib.h"

#ifdef __cplusplus
extern "C"{
#endif __cplusplus

/*************************************
	ML_VEC3 function implimentations:
*************************************/
ML_VEC3* ML_FUNC ML_Vec3Add_F(ML_VEC3* pOut, const ML_VEC3* pV1, const ML_VEC3* pV2);
ML_VEC3* ML_FUNC ML_Vec3Add_SSE(ML_VEC3* pOut, const ML_VEC3* pV1, const ML_VEC3* pV2);
ML_VEC3* ML_FUNC ML_Vec3Add_3DNOW(ML_VEC3* pOut, const ML_VEC3* pV1, const ML_VEC3* pV2);

ML_VEC3* ML_FUNC ML_Vec3Cross_F(ML_VEC3* pOut, const ML_VEC3* pV1, const ML_VEC3* pV2);
ML_VEC3* ML_FUNC ML_Vec3Cross_SSE(ML_VEC3* pOut, const ML_VEC3* pV1, const ML_VEC3* pV2);
ML_VEC3* ML_FUNC ML_Vec3Cross_3DNOW(ML_VEC3* pOut, const ML_VEC3* pV1, const ML_VEC3* pV2);

float ML_FUNC ML_Vec3Dot_F(const ML_VEC3* pV1, const ML_VEC3* pV2);
float ML_FUNC ML_Vec3Dot_SSE3(const ML_VEC3* pV1, const ML_VEC3* pV2);
float ML_FUNC ML_Vec3Dot_SSE(const ML_VEC3* pV1, const ML_VEC3* pV2);
float ML_FUNC ML_Vec3Dot_3DNOW(const ML_VEC3* pV1, const ML_VEC3* pV2);

float ML_FUNC ML_Vec3Length_F(const ML_VEC3* pV);
float ML_FUNC ML_Vec3Length_SSE3(const ML_VEC3* pV);
float ML_FUNC ML_Vec3Length_SSE(const ML_VEC3* pV);
float ML_FUNC ML_Vec3Length_3DNOW(const ML_VEC3* pV);

float ML_FUNC ML_Vec3LengthSq_F(const ML_VEC3* pV);
float ML_FUNC ML_Vec3LengthSq_SSE3(const ML_VEC3* pV);
float ML_FUNC ML_Vec3LengthSq_SSE(const ML_VEC3* pV);
float ML_FUNC ML_Vec3LengthSq_3DNOW(const ML_VEC3* pV);

ML_VEC3* ML_FUNC ML_Vec3Normalize_F(ML_VEC3* pOut, const ML_VEC3* pV);
ML_VEC3* ML_FUNC ML_Vec3Normalize_SSE3(ML_VEC3* pOut, const ML_VEC3* pV);
ML_VEC3* ML_FUNC ML_Vec3Normalize_SSE(ML_VEC3* pOut, const ML_VEC3* pV);
ML_VEC3* ML_FUNC ML_Vec3Normalize_3DNOW(ML_VEC3* pOut, const ML_VEC3* pV);

ML_VEC3* ML_FUNC ML_Vec3Scale_F(ML_VEC3* pOut, const ML_VEC3* pV, float s);
ML_VEC3* ML_FUNC ML_Vec3Scale_SSE(ML_VEC3* pOut, const ML_VEC3* pV, float s);
ML_VEC3* ML_FUNC ML_Vec3Scale_3DNOW(ML_VEC3* pOut, const ML_VEC3* pV, float s);

ML_VEC3* ML_FUNC ML_Vec3Subtract_F(ML_VEC3* pOut, const ML_VEC3* pV1, const ML_VEC3* pV2);
ML_VEC3* ML_FUNC ML_Vec3Subtract_SSE(ML_VEC3* pOut, const ML_VEC3* pV1, const ML_VEC3* pV2);
ML_VEC3* ML_FUNC ML_Vec3Subtract_3DNOW(ML_VEC3* pOut, const ML_VEC3* pV1, const ML_VEC3* pV2);

float ML_FUNC ML_Vec3Distance_F(const ML_VEC3* pV1, const ML_VEC3* pV2);
float ML_FUNC ML_Vec3Distance_SSE3(const ML_VEC3* pV1, const ML_VEC3* pV2);
float ML_FUNC ML_Vec3Distance_SSE(const ML_VEC3* pV1, const ML_VEC3* pV2);
float ML_FUNC ML_Vec3Distance_3DNOW(const ML_VEC3* pV1, const ML_VEC3* pV2);

ML_VEC4* ML_FUNC ML_Vec3Transform_F(ML_VEC4* pOut, const ML_VEC3* pV, const ML_MAT* pM);
ML_VEC4* ML_FUNC ML_Vec3Transform_SSE(ML_VEC4* pOut, const ML_VEC3* pV, const ML_MAT* pM);
ML_VEC4* ML_FUNC ML_Vec3Transform_3DNOW(ML_VEC4* pOut, const ML_VEC3* pV, const ML_MAT* pM);

ML_VEC3* ML_FUNC ML_Vec3TransformCoord_F(ML_VEC3* pOut, const ML_VEC3* pV, const ML_MAT* pM);
ML_VEC3* ML_FUNC ML_Vec3TransformCoord_SSE(ML_VEC3* pOut, const ML_VEC3* pV, const ML_MAT* pM);
ML_VEC3* ML_FUNC ML_Vec3TransformCoord_3DNOW(ML_VEC3* pOut, const ML_VEC3* pV, const ML_MAT* pM);

ML_VEC3* ML_FUNC ML_Vec3TransformNormal_F(ML_VEC3* pOut, const ML_VEC3* pV, const ML_MAT* pM);
ML_VEC3* ML_FUNC ML_Vec3TransformNormal_SSE(ML_VEC3* pOut, const ML_VEC3* pV, const ML_MAT* pM);
ML_VEC3* ML_FUNC ML_Vec3TransformNormal_3DNOW(ML_VEC3* pOut, const ML_VEC3* pV, const ML_MAT* pM);


#ifdef __cplusplus
} /* extern "C" */
#endif __cplusplus

#endif __ML_VEC3_H__