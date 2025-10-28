/* ML_quat.c - Code for quaternion computations.
	Copyrgiht (C) 2006, Blaine Myers.
	
	Note the quaternion stuff is the only stuff that isn't written in
	assembly (mostly because it is so complicated and it will probably
	take me a while to write the assembly).
*/
#include "ML_lib.h"


ml_quat* ML_FUNC ML_QuatSlerp(ml_quat* pOut, const ml_quat* pQ1, const ml_quat* pQ2, float t)
{
	float sine, cosine;
	float beta = 1.0f - t;
	float quat[4];

	quat[0] = pQ2->x;
	quat[1] = pQ2->y;
	quat[2] = pQ2->z;
	quat[3] = pQ2->w;

	// Smelling of SIMD...
	cosine = (pQ1->x*pQ2->x) + (pQ1->y*pQ2->y) + (pQ1->z*pQ2->z) + (pQ1->w*pQ2->w);

	// If negative cosine, invert to stay in 0 to Pi range
	if(cosine < 0.0f)
	{
 		quat[0] = -quat[0];
		quat[1] = -quat[1];
		quat[2] = -quat[2];
		quat[3] = -quat[3];
		cosine = -cosine;
	}

	// if cosine of angle between them is not 0.0
	if(1.0f - cosine > 0.0001f)
	{
		// get the angle between them
		cosine = ML_acosf(cosine);

		// Find the reciprocal of the sin(angle)
		sine = 1.0f / ML_sinf(cosine);
		// recalculate weights
		beta = ML_sinf(cosine * beta) * sine;
		t = ML_sinf(cosine * t) * sine;
	}
	// TODO: This smells of SIMD parallelism...
	pOut->x = (beta * pQ1->x) + (t * quat[0]);
	pOut->y = (beta * pQ1->y) + (t * quat[1]);
	pOut->z = (beta * pQ1->z) + (t * quat[2]);
	pOut->w = (beta * pQ1->w) + (t * quat[3]);

	return pOut;
}

ml_quat* ML_FUNC ML_QuatRotationMat(ml_quat* pOut, const ml_mat* pM)
{
	float fTrace=pM->_11+pM->_22+pM->_33+1.0f;
	
	if(fTrace > 0.00000001f)
	{
		pOut->w=ML_sqrtf(fTrace)*0.5f;
		fTrace=0.25f/pOut->w;
		pOut->x=(pM->_23-pM->_32)*fTrace;
		pOut->y=(pM->_31-pM->_13)*fTrace;
		pOut->z=(pM->_12-pM->_21)*fTrace;
		return pOut;
	}

	if( (pM->_11>pM->_22) && (pM->_11>pM->_33))
	{
		fTrace=pM->_11-pM->_22-pM->_33+1.0f;
		pOut->x=ML_sqrtf(fTrace)*0.5f;
		fTrace=0.25f/pOut->x;
		pOut->w=(pM->_23-pM->_32)*fTrace;
		pOut->y=(pM->_12+pM->_21)*fTrace;
		pOut->z=(pM->_31+pM->_13)*fTrace;
		return pOut;
	}
	else if(pM->_22>pM->_33)
	{
		//MessageBox(0, "Y", 0, 0);
		fTrace=pM->_22-pM->_11-pM->_33+1.0f;
		pOut->y=ML_sqrtf(fTrace)*0.5f;
		fTrace=0.25f/pOut->y;
		pOut->w=(pM->_31-pM->_13)*fTrace;
		pOut->x=(pM->_12+pM->_21)*fTrace;
		pOut->z=(pM->_23+pM->_32)*fTrace;
		return pOut;
	}
	else
	{
		//MessageBox(0, "Z", 0, 0);
		fTrace=pM->_33-pM->_11-pM->_22+1.0f;
		pOut->z=ML_sqrtf(fTrace)*0.5f;
		fTrace=0.25f/pOut->z;
		pOut->w=(pM->_12-pM->_21)*fTrace;
		pOut->x=(pM->_31+pM->_13)*fTrace;
		pOut->y=(pM->_23+pM->_32)*fTrace;
		return pOut;
	}
}