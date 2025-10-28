#ifndef __ML_MAT_H__
#define __ML_MAT_H__

#include "ML_lib.h"

#ifdef __cplusplus
extern "C"{
#endif __cplusplus

//We don't declar ML_MatIdentity here because there is only one implimentation.
ML_MAT* ML_FUNC ML_MatMultiply_F(ML_MAT* pOut, const ML_MAT* pM1, const ML_MAT* pM2);
ML_MAT* ML_FUNC ML_MatMultiply_SSE(ML_MAT* pOut, const ML_MAT* pM1, const ML_MAT* pM2);
ML_MAT* ML_FUNC ML_MatMultiply_3DNOW(ML_MAT* pOut, const ML_MAT* pM1, const ML_MAT* pM2);

ML_MAT* ML_FUNC ML_MatInverse_F(ML_MAT* pOut, float* pDet, ML_MAT* pM);
//ML_MAT* ML_FUNC ML_MatInverse_SSE(ML_MAT* pOut, float* pDet, ML_MAT* pM);

#ifdef __cplusplus
}
#endif __cplusplus

#endif __ML_MAT_H__