#include "ML_lib.h"

ML_VEC4* ML_FUNC ML_Vec3TransformArray(ML_VEC4* pOut, ml_uint nOutStride, const ML_VEC3* pV, ml_uint nInStride, const ML_MAT* pM, ml_uint nNum)
{
	ml_uint i;
	for(i=0; i<nNum; i++)
		ML_Vec3Transform((ML_VEC4*)((ml_byte*)pOut+i*nOutStride), (ML_VEC3*)((ml_byte*)pV+i*nInStride), pM);
		
	return pOut;
}

ML_VEC3* ML_FUNC ML_Vec3TransformCoordArray(ML_VEC3* pOut, ml_uint nOutStride, const ML_VEC3* pV, ml_uint nInStride, const ML_MAT* pM, ml_uint nNum)
{
	ml_uint i;
	for(i=0; i<nNum; i++)
		ML_Vec3TransformCoord((ML_VEC3*)((ml_byte*)pOut+i*nOutStride), (ML_VEC3*)((ml_byte*)pV+i*nInStride), pM);
		
	return pOut;
}

ML_VEC3* ML_FUNC ML_Vec3TransformNormalArray(ML_VEC3* pOut, ml_uint nOutStride, const ML_VEC3* pV, ml_uint nInStride, const ML_MAT* pM, ml_uint nNum)
{
	ml_uint i;
	for(i=0; i<nNum; i++)
		ML_Vec3TransformNormal((ML_VEC3*)((ml_byte*)pOut+i*nOutStride), (ML_VEC3*)((ml_byte*)pV+i*nInStride), pM);
		
	return pOut;
}