#ifndef __ml_vec3_H__
#define __ml_vec3_H__

#include "ML_types.h"

#ifdef __cplusplus
extern "C"{
#endif __cplusplus

/*************************************
	ml_vec3 function implimentations:
*************************************/
/**** AVAILABLE FUNCTIONS ****
*** ML_Vec3Add             ***
*** ML_Vec3Subtract        ***
*** ML_Vec3Cross           ***
*** ML_Vec3Dot             ***
*** ML_Vec3Length          ***
*** ML_Vec3LengthSq        ***
*** ML_Vec3Normalize       ***
*** ML_Vec3Scale           ***
*** ML_Vec3Distance        ***
*** ML_Vec3DistanceSq      ***
*** ML_Vec3Transform       ***
*** ML_Vec3TransformCoord  ***
*** ML_Vec3TransformNormal ***
*****************************/
ml_vec3* ML_FUNC ML_Vec3Add_F(ml_vec3* pOut, const ml_vec3* pV1, const ml_vec3* pV2);
ml_vec3* ML_FUNC ML_Vec3Add_SSE(ml_vec3* pOut, const ml_vec3* pV1, const ml_vec3* pV2);
ml_vec3* ML_FUNC ML_Vec3Add_3DNOW(ml_vec3* pOut, const ml_vec3* pV1, const ml_vec3* pV2);

ml_vec3* ML_FUNC ML_Vec3Cross_F(ml_vec3* pOut, const ml_vec3* pV1, const ml_vec3* pV2);
ml_vec3* ML_FUNC ML_Vec3Cross_SSE(ml_vec3* pOut, const ml_vec3* pV1, const ml_vec3* pV2);
ml_vec3* ML_FUNC ML_Vec3Cross_3DNOW(ml_vec3* pOut, const ml_vec3* pV1, const ml_vec3* pV2);

ml_float ML_FUNC ML_Vec3Dot_F(const ml_vec3* pV1, const ml_vec3* pV2);
ml_float ML_FUNC ML_Vec3Dot_SSE3(const ml_vec3* pV1, const ml_vec3* pV2);
ml_float ML_FUNC ML_Vec3Dot_SSE(const ml_vec3* pV1, const ml_vec3* pV2);
ml_float ML_FUNC ML_Vec3Dot_3DNOW(const ml_vec3* pV1, const ml_vec3* pV2);

ml_float ML_FUNC ML_Vec3Length_F(const ml_vec3* pV);
ml_float ML_FUNC ML_Vec3Length_SSE3(const ml_vec3* pV);
ml_float ML_FUNC ML_Vec3Length_SSE(const ml_vec3* pV);
ml_float ML_FUNC ML_Vec3Length_3DNOW(const ml_vec3* pV);

ml_float ML_FUNC ML_Vec3LengthSq_F(const ml_vec3* pV);
ml_float ML_FUNC ML_Vec3LengthSq_SSE3(const ml_vec3* pV);
ml_float ML_FUNC ML_Vec3LengthSq_SSE(const ml_vec3* pV);
ml_float ML_FUNC ML_Vec3LengthSq_3DNOW(const ml_vec3* pV);

ml_vec3* ML_FUNC ML_Vec3Normalize_F(ml_vec3* pOut, const ml_vec3* pV);
ml_vec3* ML_FUNC ML_Vec3Normalize_SSE3(ml_vec3* pOut, const ml_vec3* pV);
ml_vec3* ML_FUNC ML_Vec3Normalize_SSE(ml_vec3* pOut, const ml_vec3* pV);
ml_vec3* ML_FUNC ML_Vec3Normalize_3DNOW(ml_vec3* pOut, const ml_vec3* pV);

ml_vec3* ML_FUNC ML_Vec3Scale_F(ml_vec3* pOut, const ml_vec3* pV, ml_float s);
ml_vec3* ML_FUNC ML_Vec3Scale_SSE(ml_vec3* pOut, const ml_vec3* pV, ml_float s);
ml_vec3* ML_FUNC ML_Vec3Scale_3DNOW(ml_vec3* pOut, const ml_vec3* pV, ml_float s);

ml_vec3* ML_FUNC ML_Vec3Subtract_F(ml_vec3* pOut, const ml_vec3* pV1, const ml_vec3* pV2);
ml_vec3* ML_FUNC ML_Vec3Subtract_SSE(ml_vec3* pOut, const ml_vec3* pV1, const ml_vec3* pV2);
ml_vec3* ML_FUNC ML_Vec3Subtract_3DNOW(ml_vec3* pOut, const ml_vec3* pV1, const ml_vec3* pV2);

ml_float ML_FUNC ML_Vec3Distance_F(const ml_vec3* pV1, const ml_vec3* pV2);
ml_float ML_FUNC ML_Vec3Distance_SSE3(const ml_vec3* pV1, const ml_vec3* pV2);
ml_float ML_FUNC ML_Vec3Distance_SSE(const ml_vec3* pV1, const ml_vec3* pV2);
ml_float ML_FUNC ML_Vec3Distance_3DNOW(const ml_vec3* pV1, const ml_vec3* pV2);

ml_float ML_FUNC ML_Vec3DistanceSq_F(const ml_vec3* pV1, const ml_vec3* pV2);
ml_float ML_FUNC ML_Vec3DistanceSq_SSE3(const ml_vec3* pV1, const ml_vec3* pV2);
ml_float ML_FUNC ML_Vec3DistanceSq_SSE(const ml_vec3* pV1, const ml_vec3* pV2);
ml_float ML_FUNC ML_Vec3DistanceSq_3DNOW(const ml_vec3* pV1, const ml_vec3* pV2);

ml_vec4* ML_FUNC ML_Vec3Transform_F(ml_vec4* pOut, const ml_vec3* pV, const ml_mat* pM);
ml_vec4* ML_FUNC ML_Vec3Transform_SSE(ml_vec4* pOut, const ml_vec3* pV, const ml_mat* pM);
ml_vec4* ML_FUNC ML_Vec3Transform_3DNOW(ml_vec4* pOut, const ml_vec3* pV, const ml_mat* pM);

ml_vec3* ML_FUNC ML_Vec3TransformCoord_F(ml_vec3* pOut, const ml_vec3* pV, const ml_mat* pM);
ml_vec3* ML_FUNC ML_Vec3TransformCoord_SSE(ml_vec3* pOut, const ml_vec3* pV, const ml_mat* pM);
ml_vec3* ML_FUNC ML_Vec3TransformCoord_3DNOW(ml_vec3* pOut, const ml_vec3* pV, const ml_mat* pM);

ml_vec3* ML_FUNC ML_Vec3TransformNormal_F(ml_vec3* pOut, const ml_vec3* pV, const ml_mat* pM);
ml_vec3* ML_FUNC ML_Vec3TransformNormal_SSE(ml_vec3* pOut, const ml_vec3* pV, const ml_mat* pM);
ml_vec3* ML_FUNC ML_Vec3TransformNormal_3DNOW(ml_vec3* pOut, const ml_vec3* pV, const ml_mat* pM);


#ifdef __cplusplus
} /* extern "C" */
#endif __cplusplus

#endif __ml_vec3_H__