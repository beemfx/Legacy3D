/* ML_matC - The matrix functions written in C code, for assembly
	code see ML_mat.nasm. */
//#include <xmmintrin.h>
#include "ML_lib.h"
#include "ML_mat.h"

ml_mat*  ML_FUNC ML_MatTranslation(ml_mat* pOut, ml_float x, ml_float y, ml_float z)
{
	pOut->_11=1.0f; pOut->_12=0.0f; pOut->_13=0.0f; pOut->_14=0.0f;
	pOut->_21=0.0f; pOut->_22=1.0f; pOut->_23=0.0f; pOut->_24=0.0f;
	pOut->_31=0.0f; pOut->_32=0.0f; pOut->_33=1.0f; pOut->_34=0.0f;
	pOut->_41=x; pOut->_42=y; pOut->_43=z; pOut->_44=1.0f;
	return pOut;
}

ml_mat* ML_FUNC ML_MatRotationAxis(ml_mat* pOut, const ML_VEC3* pAxis, float fAngle)
{
	//From Fletcher Dunn & Ian Parberry
	ml_float s, c;
	ml_float a, ax, ay, az;
	//We need a unit vector for the rotation, we'll assume it isn't a 0 vector:
	ML_VEC3 v3Axis;
	
	if(!fAngle)
		return ML_MatIdentity(pOut);
	
	ML_Vec3Normalize(&v3Axis, pAxis);
	
	//Get sin an cosin of rotation angle
	s=ML_sincosf(fAngle, &c);
	
	//Compute 1 - cos(theta) and some common subexpressions
	a = 1.0f - c;
	ax = a * v3Axis.x;
	ay = a * v3Axis.y;
	az = a * v3Axis.z;
	
	//Set the matrix elements.  There is still a little more
	//opportunity for optimization due to the many common
	//subexpressions.  We'll let the compiler handle that...
	
	pOut->_11 = ax*v3Axis.x + c;
	pOut->_12 = ax*v3Axis.y + v3Axis.z*s;
	pOut->_13 = ax*v3Axis.z - v3Axis.y*s;
	pOut->_14 = 0;
	
	pOut->_21 = ay*v3Axis.x - v3Axis.z*s;
	pOut->_22 = ay*v3Axis.y + c;
	pOut->_23 = ay*v3Axis.z + v3Axis.x*s;
	pOut->_24 = 0;
	
	pOut->_31 = az*v3Axis.x + v3Axis.y*s;
	pOut->_32 = az*v3Axis.y - v3Axis.x*s;
	pOut->_33 = az*v3Axis.z + c;
	pOut->_34 = 0;
	
	pOut->_41 = pOut->_42 = pOut->_43 = 0;
	pOut->_44 = 1;
	
	return pOut;
}

ml_mat* ML_FUNC ML_MatLookAtLH(ml_mat* pOut, const ML_VEC3* pEye, const ML_VEC3* pAt, const ML_VEC3* pUp)
{
	/*
	zaxis = normal(At - Eye)
	xaxis = normal(cross(Up, zaxis))
	yaxis = cross(zaxis, xaxis)
	    
	 xaxis.x           yaxis.x           zaxis.x          0
	 xaxis.y           yaxis.y           zaxis.y          0
	 xaxis.z           yaxis.z           zaxis.z          0
	-dot(xaxis, eye)  -dot(yaxis, eye)  -dot(zaxis, eye)  l
	*/
	
	ML_VEC3 zaxis, xaxis, yaxis;
	
	ML_Vec3Normalize(&zaxis, ML_Vec3Subtract(&zaxis, pAt, pEye));
	ML_Vec3Normalize(&xaxis, ML_Vec3Cross(&xaxis, pUp, &zaxis));
	ML_Vec3Cross(&yaxis, &zaxis, &xaxis);
	
	pOut->_11=xaxis.x; pOut->_12=yaxis.x; pOut->_13=zaxis.x; pOut->_14=0.0f;
	pOut->_21=xaxis.y; pOut->_22=yaxis.y; pOut->_23=zaxis.y; pOut->_24=0.0f;
	pOut->_31=xaxis.z; pOut->_32=yaxis.z; pOut->_33=zaxis.z; pOut->_34=0.0f;
	pOut->_41=-ML_Vec3Dot(&xaxis, pEye); pOut->_42=-ML_Vec3Dot(&yaxis, pEye); pOut->_43=-ML_Vec3Dot(&zaxis, pEye); pOut->_44=1.0f;
	
	return pOut;
}

ml_mat* ML_FUNC ML_MatPerspectiveFovLH(ml_mat* pOut, float fovy, float Aspect, float zn, float zf)
{
	float yScale=1.0f/ML_tanf(fovy/2.0f);
	float xScale=yScale/Aspect;
	
	pOut->_11=xScale; pOut->_12=pOut->_13=pOut->_14=0.0f;
	pOut->_21=0.0f; pOut->_22=yScale; pOut->_23=0.0f; pOut->_24=0.0f;
	pOut->_31=0.0f; pOut->_32=0.0f; pOut->_33=zf/(zf-zn); pOut->_34=1.0f;
	pOut->_41=0.0f; pOut->_42=0.0f; pOut->_43=zn*zf/(zn-zf); pOut->_44=0.0f;
	return pOut;
}

ml_mat* ML_FUNC ML_MatRotationYawPitchRoll(ml_mat* pOut, float Yaw, float Pitch, float Roll)
{
	ml_float cosx, sinx, cosy, siny, cosz, sinz;
	sinx=ML_sincosf(Pitch, &cosx);
	siny=ML_sincosf(Yaw, &cosy);
	sinz=ML_sincosf(Roll, &cosz);
	
	pOut->_11=cosz*cosy+sinz*sinx*siny; pOut->_12=sinz*cosx; pOut->_13=cosz*-siny+sinz*sinx*cosy; pOut->_14=0.0f;
	pOut->_21=-sinz*cosy+cosz*sinx*siny; pOut->_22=cosz*cosx; pOut->_23=sinz*siny+cosz*sinx*cosy; pOut->_24=0.0f;
	pOut->_31=cosx*siny; pOut->_32=-sinx; pOut->_33=cosx*cosy; pOut->_34=0.0f;
	pOut->_41=pOut->_42=pOut->_43=0.0f; pOut->_44=1.0f;
	return pOut;
}

ml_mat* ML_FUNC ML_MatScaling(ml_mat* pOut, float sx, float sy, float sz)
{
	pOut->_11=sx; pOut->_22=sy; pOut->_33=sz; pOut->_44=1.0f;
	pOut->_12=pOut->_13=pOut->_14=0.0f;
	pOut->_21=pOut->_23=pOut->_24=0.0f;
	pOut->_31=pOut->_32=pOut->_34=0.0f;
	pOut->_41=pOut->_42=pOut->_43=0.0f;
	return pOut;
}

float ML_FUNC ML_MatDeterminant(ml_mat* pM)
{
	return pM->_11*(pM->_22*pM->_33-pM->_23*pM->_32) +
	       pM->_12*(pM->_23*pM->_31-pM->_21*pM->_33) +
	       pM->_13*(pM->_21*pM->_32-pM->_22*pM->_31);
}

/* C Code Using Cramer's Rule Provided by Intel (tm) */
ml_mat* ML_FUNC ML_MatInverse(ml_mat* pOut, ml_float* pDet, const ml_mat* pM)
{
	float tmp[12]; /* temp array for pairs */
	float src[16]; /* array of transpose source matrix */
	float det; /* determinant */
	int i, j;
	float* mat=(float*)pM;
	float* dst=(float*)pOut;
	/* transpose matrix */
	for (i = 0; i < 4; i++) 
	{
		src[i] = mat[i*4];
		src[i + 4] = mat[i*4 + 1];
		src[i + 8] = mat[i*4 + 2];
		src[i + 12] = mat[i*4 + 3];
	}
	/* calculate pairs for first 8 elements (cofactors) */
	tmp[0] = src[10] * src[15];
	tmp[1] = src[11] * src[14];
	tmp[2] = src[9] * src[15];
	tmp[3] = src[11] * src[13];
	tmp[4] = src[9] * src[14];
	tmp[5] = src[10] * src[13];
	tmp[6] = src[8] * src[15];
	tmp[7] = src[11] * src[12];
	tmp[8] = src[8] * src[14];
	tmp[9] = src[10] * src[12];
	tmp[10] = src[8] * src[13];
	tmp[11] = src[9] * src[12];
	/* calculate first 8 elements (cofactors) */
	dst[0] = tmp[0]*src[5] + tmp[3]*src[6] + tmp[4]*src[7];
	dst[0] -= tmp[1]*src[5] + tmp[2]*src[6] + tmp[5]*src[7];
	dst[1] = tmp[1]*src[4] + tmp[6]*src[6] + tmp[9]*src[7];
	dst[1] -= tmp[0]*src[4] + tmp[7]*src[6] + tmp[8]*src[7];
	dst[2] = tmp[2]*src[4] + tmp[7]*src[5] + tmp[10]*src[7];
	dst[2] -= tmp[3]*src[4] + tmp[6]*src[5] + tmp[11]*src[7];
	dst[3] = tmp[5]*src[4] + tmp[8]*src[5] + tmp[11]*src[6];
	dst[3] -= tmp[4]*src[4] + tmp[9]*src[5] + tmp[10]*src[6];
	dst[4] = tmp[1]*src[1] + tmp[2]*src[2] + tmp[5]*src[3];
	dst[4] -= tmp[0]*src[1] + tmp[3]*src[2] + tmp[4]*src[3];
	dst[5] = tmp[0]*src[0] + tmp[7]*src[2] + tmp[8]*src[3];
	dst[5] -= tmp[1]*src[0] + tmp[6]*src[2] + tmp[9]*src[3];
	dst[6] = tmp[3]*src[0] + tmp[6]*src[1] + tmp[11]*src[3];
	dst[6] -= tmp[2]*src[0] + tmp[7]*src[1] + tmp[10]*src[3];
	dst[7] = tmp[4]*src[0] + tmp[9]*src[1] + tmp[10]*src[2];
	dst[7] -= tmp[5]*src[0] + tmp[8]*src[1] + tmp[11]*src[2];
	/* calculate pairs for second 8 elements (cofactors) */
	tmp[0] = src[2]*src[7];
	tmp[1] = src[3]*src[6];
	tmp[2] = src[1]*src[7];
	tmp[3] = src[3]*src[5];
	tmp[4] = src[1]*src[6];
	tmp[5] = src[2]*src[5];
	
	tmp[6] = src[0]*src[7];
	tmp[7] = src[3]*src[4];
	tmp[8] = src[0]*src[6];
	tmp[9] = src[2]*src[4];
	tmp[10] = src[0]*src[5];
	tmp[11] = src[1]*src[4];
	/* calculate second 8 elements (cofactors) */
	dst[8] = tmp[0]*src[13] + tmp[3]*src[14] + tmp[4]*src[15];
	dst[8] -= tmp[1]*src[13] + tmp[2]*src[14] + tmp[5]*src[15];
	dst[9] = tmp[1]*src[12] + tmp[6]*src[14] + tmp[9]*src[15];
	dst[9] -= tmp[0]*src[12] + tmp[7]*src[14] + tmp[8]*src[15];
	dst[10] = tmp[2]*src[12] + tmp[7]*src[13] + tmp[10]*src[15];
	dst[10]-= tmp[3]*src[12] + tmp[6]*src[13] + tmp[11]*src[15];
	dst[11] = tmp[5]*src[12] + tmp[8]*src[13] + tmp[11]*src[14];
	dst[11]-= tmp[4]*src[12] + tmp[9]*src[13] + tmp[10]*src[14];
	dst[12] = tmp[2]*src[10] + tmp[5]*src[11] + tmp[1]*src[9];
	dst[12]-= tmp[4]*src[11] + tmp[0]*src[9] + tmp[3]*src[10];
	dst[13] = tmp[8]*src[11] + tmp[0]*src[8] + tmp[7]*src[10];
	dst[13]-= tmp[6]*src[10] + tmp[9]*src[11] + tmp[1]*src[8];
	dst[14] = tmp[6]*src[9] + tmp[11]*src[11] + tmp[3]*src[8];
	dst[14]-= tmp[10]*src[11] + tmp[2]*src[8] + tmp[7]*src[9];
	dst[15] = tmp[10]*src[10] + tmp[4]*src[8] + tmp[9]*src[9];
	dst[15]-= tmp[8]*src[9] + tmp[11]*src[10] + tmp[5]*src[8];
	/* calculate determinant */
	if(pDet)
		det=*pDet;
	else
		det=src[0]*dst[0]+src[1]*dst[1]+src[2]*dst[2]+src[3]*dst[3];
	/* calculate matrix inverse */
	if(det==0.0f)
		return (ML_MAT*)dst;
	det = 1.0f/det;
	for (j = 0; j < 16; j++)
		dst[j] *= det;
	return (ML_MAT*)dst;
}


/* C Code Using Cramer's Rule and SIMD Provided by Intel (tm) */
/*
ml_mat* ML_MatInverse_SSE(ml_mat* pOut, float* pDet, ml_mat* pM)
{
	
	__m128 minor0, minor1, minor2, minor3;
	__m128 row0, row1, row2, row3;
	__m128 det, tmp1;
	float* src=(float*)pM;
	float* dst=(float*)pOut;
	
	tmp1 = _mm_loadh_pi(_mm_loadl_pi(tmp1, (__m64*)(src)), (__m64*)(src+ 4));
	row1 = _mm_loadh_pi(_mm_loadl_pi(row1, (__m64*)(src+8)), (__m64*)(src+12));
	row0 = _mm_shuffle_ps(tmp1, row1, 0x88);
	row1 = _mm_shuffle_ps(row1, tmp1, 0xDD);
	tmp1 = _mm_loadh_pi(_mm_loadl_pi(tmp1, (__m64*)(src+ 2)), (__m64*)(src+ 6));
	row3 = _mm_loadh_pi(_mm_loadl_pi(row3, (__m64*)(src+10)), (__m64*)(src+14));
	row2 = _mm_shuffle_ps(tmp1, row3, 0x88);
	row3 = _mm_shuffle_ps(row3, tmp1, 0xDD);
	// -----------------------------------------------
	tmp1 = _mm_mul_ps(row2, row3);
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
	minor0 = _mm_mul_ps(row1, tmp1);
	minor1 = _mm_mul_ps(row0, tmp1);
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
	minor0 = _mm_sub_ps(_mm_mul_ps(row1, tmp1), minor0);
	minor1 = _mm_sub_ps(_mm_mul_ps(row0, tmp1), minor1);
	minor1 = _mm_shuffle_ps(minor1, minor1, 0x4E);
	// -----------------------------------------------
	tmp1 = _mm_mul_ps(row1, row2);
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
	minor0 = _mm_add_ps(_mm_mul_ps(row3, tmp1), minor0);
	minor3 = _mm_mul_ps(row0, tmp1);
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
	minor0 = _mm_sub_ps(minor0, _mm_mul_ps(row3, tmp1));
	minor3 = _mm_sub_ps(_mm_mul_ps(row0, tmp1), minor3);
	minor3 = _mm_shuffle_ps(minor3, minor3, 0x4E);
	// -----------------------------------------------
	tmp1 = _mm_mul_ps(_mm_shuffle_ps(row1, row1, 0x4E), row3);
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
	row2 = _mm_shuffle_ps(row2, row2, 0x4E);
	minor0 = _mm_add_ps(_mm_mul_ps(row2, tmp1), minor0);
	minor2 = _mm_mul_ps(row0, tmp1);
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
	minor0 = _mm_sub_ps(minor0, _mm_mul_ps(row2, tmp1));
	minor2 = _mm_sub_ps(_mm_mul_ps(row0, tmp1), minor2);
	minor2 = _mm_shuffle_ps(minor2, minor2, 0x4E);
	// -----------------------------------------------
	tmp1 = _mm_mul_ps(row0, row1);
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
	minor2 = _mm_add_ps(_mm_mul_ps(row3, tmp1), minor2);
	minor3 = _mm_sub_ps(_mm_mul_ps(row2, tmp1), minor3);
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
	minor2 = _mm_sub_ps(_mm_mul_ps(row3, tmp1), minor2);
	minor3 = _mm_sub_ps(minor3, _mm_mul_ps(row2, tmp1));
	// -----------------------------------------------
	tmp1 = _mm_mul_ps(row0, row3);
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
	minor1 = _mm_sub_ps(minor1, _mm_mul_ps(row2, tmp1));
	minor2 = _mm_add_ps(_mm_mul_ps(row1, tmp1), minor2);
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
	minor1 = _mm_add_ps(_mm_mul_ps(row2, tmp1), minor1);
	minor2 = _mm_sub_ps(minor2, _mm_mul_ps(row1, tmp1));
	// -----------------------------------------------
	tmp1 = _mm_mul_ps(row0, row2);
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0xB1);
	minor1 = _mm_add_ps(_mm_mul_ps(row3, tmp1), minor1);
	minor3 = _mm_sub_ps(minor3, _mm_mul_ps(row1, tmp1));
	tmp1 = _mm_shuffle_ps(tmp1, tmp1, 0x4E);
	minor1 = _mm_sub_ps(minor1, _mm_mul_ps(row3, tmp1));
	minor3 = _mm_add_ps(_mm_mul_ps(row1, tmp1), minor3);
	// -----------------------------------------------
	det = _mm_mul_ps(row0, minor0);
	det = _mm_add_ps(_mm_shuffle_ps(det, det, 0x4E), det);
	det = _mm_add_ss(_mm_shuffle_ps(det, det, 0xB1), det);
	tmp1 = _mm_rcp_ss(det);
	det = _mm_sub_ss(_mm_add_ss(tmp1, tmp1), _mm_mul_ss(det, _mm_mul_ss(tmp1, tmp1)));
	det = _mm_shuffle_ps(det, det, 0x00);
	minor0 = _mm_mul_ps(det, minor0);
	_mm_storel_pi((__m64*)(src), minor0);
	_mm_storeh_pi((__m64*)(src+2), minor0);
	minor1 = _mm_mul_ps(det, minor1);
	_mm_storel_pi((__m64*)(src+4), minor1);
	_mm_storeh_pi((__m64*)(src+6), minor1);
	minor2 = _mm_mul_ps(det, minor2);
	_mm_storel_pi((__m64*)(src+ 8), minor2);
	_mm_storeh_pi((__m64*)(src+10), minor2);
	minor3 = _mm_mul_ps(det, minor3);
	_mm_storel_pi((__m64*)(src+12), minor3);
	_mm_storeh_pi((__m64*)(src+14), minor3);

	return (ml_mat*)dst;
}
*/

ml_mat* ML_FUNC ML_MatSlerp(ml_mat* pOut, ml_mat* pM1, ml_mat* pM2, float t)
{
	ml_mat *pXOut=pOut, *pXM1=pM1, *pXM2=pM2;
	ML_QUAT Q1, Q2;
	float x=pXM1->_41+t*(pXM2->_41-pXM1->_41);
	float y=pXM1->_42+t*(pXM2->_42-pXM1->_42);
	float z=pXM1->_43+t*(pXM2->_43-pXM1->_43);
	ML_QuatRotationMat(&Q1, pM1);
	ML_QuatRotationMat(&Q2, pM2);
	ML_QuatSlerp(&Q1, &Q1, &Q2, t);
	ML_MatRotationQuat(pXOut, &Q1);
	pXOut->_41=x;
	pXOut->_42=y;
	pXOut->_43=z;
	return pOut;
}
