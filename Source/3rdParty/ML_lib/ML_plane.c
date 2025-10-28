#include "ML_lib.h"

ml_float ML_FUNC ML_PlaneDotCoord(const ml_plane* pPlane, const ml_vec3* pV)
{
	return ML_Vec3Dot((ML_VEC3*)pPlane, pV)-pPlane->d;
}

ml_plane* ML_FUNC ML_PlaneScale(ml_plane* pOut, const ml_plane* pPlane, ml_float s)
{
	pOut->a=pPlane->a;
	pOut->b=pPlane->b;
	pOut->c=pPlane->c;
	pOut->d=pPlane->d*s;
	return pOut;
}