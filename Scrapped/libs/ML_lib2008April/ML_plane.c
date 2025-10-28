#include "ML_lib.h"

ml_float ML_FUNC ML_PlaneDotCoord(const ML_PLANE* pPlane, const ML_VEC3* pV)
{
	return ML_Vec3Dot((ML_VEC3*)pPlane, pV)-pPlane->d;
}

ML_PLANE* ML_FUNC ML_PlaneScale(ML_PLANE* pOut, const ML_PLANE* pPlane, ml_float s)
{
	pOut->a=pPlane->a;
	pOut->b=pPlane->b;
	pOut->c=pPlane->c;
	pOut->d=pPlane->d*s;
	return pOut;
}