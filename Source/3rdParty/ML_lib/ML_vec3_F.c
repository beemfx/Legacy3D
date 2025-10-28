#include "ML_vec3.h"
#include "ML_lib.h"

/** ML_VEC3 FPU FUNCTIONS  ***
***                        ***
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
******************************/

ml_vec3* ML_FUNC ML_Vec3Add_F(ml_vec3* pOut, const ml_vec3* pV1, const ml_vec3* pV2)
{
	pOut->x=pV1->x+pV2->x;
	pOut->y=pV1->y+pV2->y;
	pOut->z=pV1->z+pV2->z;
	return pOut;
}

ml_vec3* ML_FUNC ML_Vec3Subtract_F(ml_vec3* pOut, const ml_vec3* pV1, const ml_vec3* pV2)
{
	pOut->x=pV1->x-pV2->x;
	pOut->y=pV1->y-pV2->y;
	pOut->z=pV1->z-pV2->z;
	return pOut;
}

ml_vec3* ML_FUNC ML_Vec3Cross_F(ml_vec3* pOut, const ml_vec3* pV1, const ml_vec3* pV2)
{
	ml_vec3 v3Res;
	v3Res.x=pV1->y*pV2->z - pV1->z*pV2->y;
	v3Res.y=pV1->z*pV2->x - pV1->x*pV2->z;
	v3Res.z=pV1->x*pV2->y - pV1->y*pV2->x;
	*pOut=v3Res;
	return pOut;
}

ml_float ML_FUNC ML_Vec3Dot_F(const ml_vec3* pV1, const ml_vec3* pV2)
{
	return pV1->x*pV2->x + pV1->y*pV2->y + pV1->z*pV2->z;
}

ml_float ML_FUNC ML_Vec3Length_F(const ml_vec3* pV)
{
	return ML_sqrtf(pV->x*pV->x + pV->y*pV->y + pV->z*pV->z);
}

ml_float ML_FUNC ML_Vec3LengthSq_F(const ml_vec3* pV)
{
	return pV->x*pV->x + pV->y*pV->y + pV->z*pV->z;
}

ml_vec3* ML_FUNC ML_Vec3Normalize_F(ml_vec3* pOut, const ml_vec3* pV)
{
	ml_float fLen=1.0f/ML_Vec3Length(pV);
	
	pOut->x=pV->x*fLen;
	pOut->y=pV->y*fLen;
	pOut->z=pV->z*fLen;
	return pOut;
}


ml_vec3* ML_FUNC ML_Vec3Scale_F(ml_vec3* pOut, const ml_vec3* pV, ml_float s)
{
	pOut->x=pV->x*s;
	pOut->y=pV->y*s;
	pOut->z=pV->z*s;
	return pOut;
}


ml_float ML_FUNC ML_Vec3Distance_F(const ml_vec3* pV1, const ml_vec3* pV2)
{
	ml_vec3 v3T;
	v3T.x=pV1->x - pV2->x;
	v3T.y=pV1->y - pV2->y;
	v3T.z=pV1->z - pV2->z;
	return ML_Vec3Length(&v3T);
}

ml_float ML_FUNC ML_Vec3DistanceSq_F(const ml_vec3* pV1, const ml_vec3* pV2)
{
	ml_vec3 v3T;
	v3T.x=pV1->x - pV2->x;
	v3T.y=pV1->y - pV2->y;
	v3T.z=pV1->z - pV2->z;
	return ML_Vec3LengthSq(&v3T);
}


ml_vec4* ML_FUNC ML_Vec3Transform_F(ml_vec4* pOut, const ml_vec3* pV, const ml_mat* pM)
{
	//Just do pM * (pV, 1.0)
	//We know that pOut can't be pV, so we don't need to worry about
	//making room on the stack, but we will because one might cast
	//pV from a ml_vec4, also this function is rarely used anyway.
	
	ml_vec4 v4Res;
	
	v4Res.x = pM->_11*pV->x + pM->_21*pV->y + pM->_31*pV->z + pM->_41*1.0f;
	v4Res.y = pM->_12*pV->x + pM->_22*pV->y + pM->_32*pV->z + pM->_42*1.0f;
	v4Res.z = pM->_13*pV->x + pM->_23*pV->y + pM->_33*pV->z + pM->_43*1.0f;
	v4Res.w = pM->_14*pV->x + pM->_24*pV->y + pM->_34*pV->z + pM->_44*1.0f;
	
	*pOut=v4Res;
	return pOut;
}


ml_vec3* ML_FUNC ML_Vec3TransformCoord_F(ml_vec3* pOut, const ml_vec3* pV, const ml_mat* pM)
{
	ml_vec3  v3Res;
	ml_float w;
	
	//We'll calculat w first, so that we can divide right away (we'll use 1.0f/w)
	w = 1.0f/(pM->_14*pV->x + pM->_24*pV->y + pM->_34*pV->z + pM->_44*1.0f);
	
	v3Res.x = (pM->_11*pV->x + pM->_21*pV->y + pM->_31*pV->z + pM->_41*1.0f)*w;
	v3Res.y = (pM->_12*pV->x + pM->_22*pV->y + pM->_32*pV->z + pM->_42*1.0f)*w;
	v3Res.z = (pM->_13*pV->x + pM->_23*pV->y + pM->_33*pV->z + pM->_43*1.0f)*w;
	*pOut=v3Res;
	return pOut;
}


ml_vec3* ML_FUNC ML_Vec3TransformNormal_F(ml_vec3* pOut, const ml_vec3* pV, const ml_mat* pM)
{
	//Transform normal is only concerned with the rotation portion of the matrix:
	ml_vec3 v3Res;
	
	v3Res.x = (pM->_11*pV->x + pM->_21*pV->y + pM->_31*pV->z);
	v3Res.y = (pM->_12*pV->x + pM->_22*pV->y + pM->_32*pV->z);
	v3Res.z = (pM->_13*pV->x + pM->_23*pV->y + pM->_33*pV->z);
	*pOut=v3Res;
	return pOut;
}



ml_vec4* ML_FUNC ML_Vec3TransformArray(ml_vec4* pOut, ml_uint nOutStride, const ml_vec3* pV, ml_uint nInStride, const ML_MAT* pM, ml_uint nNum)
{
	ml_uint i;
	for(i=0; i<nNum; i++)
		ML_Vec3Transform((ml_vec4*)((ml_byte*)pOut+i*nOutStride), (ml_vec3*)((ml_byte*)pV+i*nInStride), pM);
		
	return pOut;
}

ml_vec3* ML_FUNC ML_Vec3TransformCoordArray(ml_vec3* pOut, ml_uint nOutStride, const ml_vec3* pV, ml_uint nInStride, const ML_MAT* pM, ml_uint nNum)
{
	ml_uint i;
	for(i=0; i<nNum; i++)
		ML_Vec3TransformCoord((ml_vec3*)((ml_byte*)pOut+i*nOutStride), (ml_vec3*)((ml_byte*)pV+i*nInStride), pM);
		
	return pOut;
}

ml_vec3* ML_FUNC ML_Vec3TransformNormalArray(ml_vec3* pOut, ml_uint nOutStride, const ml_vec3* pV, ml_uint nInStride, const ML_MAT* pM, ml_uint nNum)
{
	ml_uint i;
	for(i=0; i<nNum; i++)
		ML_Vec3TransformNormal((ml_vec3*)((ml_byte*)pOut+i*nOutStride), (ml_vec3*)((ml_byte*)pV+i*nInStride), pM);
		
	return pOut;
}
