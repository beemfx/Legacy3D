#ifndef __ml_mat_H__
#define __ml_mat_H__

#include "ML_lib.h"

#ifdef __cplusplus
extern "C"{
#endif __cplusplus

//We don't declar ML_MatIdentity here because there is only one implimentation.
ml_mat* ML_FUNC ML_MatMultiply(ml_mat* pOut, const ml_mat* pM1, const ml_mat* pM2);
ml_mat* ML_FUNC ML_MatInverse(ml_mat* pOut, float* pDet, const ml_mat* pM);
//ml_mat* ML_FUNC ML_MatInverse_SSE(ml_mat* pOut, float* pDet, ml_mat* pM);

#ifdef __cplusplus
}
#endif __cplusplus

#endif __ml_mat_H__