#include "ML_lib.h"

__inline void ML_AABBMinMax(ml_float* fMin, ml_float* fMax, const ML_AABB*pAABB, const ML_PLANE* pPlane)
{
	// Inspect the normal and compute the minimum and maximum
	// D values. fMin is the D value of the "frontmost" corner point
	if(pPlane->a > 0.0f)
	{
		*fMin=pPlane->a*pAABB->v3Min.x;
		*fMax=pPlane->a*pAABB->v3Max.x;
	}
	else
	{
		*fMin=pPlane->a*pAABB->v3Max.x;
		*fMax=pPlane->a*pAABB->v3Min.x;
	}
	
	if(pPlane->b > 0.0f)
	{
		*fMin+=pPlane->b*pAABB->v3Min.y;
		*fMax+=pPlane->b*pAABB->v3Max.y;
	}
	else
	{
		*fMin+=pPlane->b*pAABB->v3Max.y;
		*fMax+=pPlane->b*pAABB->v3Min.y;
	}
	
	if(pPlane->c > 0.0f)
	{
		*fMin+=pPlane->c*pAABB->v3Min.z;
		*fMax+=pPlane->c*pAABB->v3Max.z;
	}
	else
	{
		*fMin+=pPlane->c*pAABB->v3Max.z;
		*fMax+=pPlane->c*pAABB->v3Min.z;
	}
}

ml_bool ML_FUNC ML_AABBIntersectBlock(ML_AABB* pAABB, ML_PLANE* pPlanes, ml_dword nPlaneCount)
{
	//This function is only for testing because it isn't very fast, and it isn't
	//acurate for blocks smaller than the aabb.
	ml_dword i;
	ml_bool bIntersect;
	for(i=0, bIntersect=ML_TRUE; i<nPlaneCount; i++)
	{
		bIntersect=bIntersect&&ML_AABBIntersectPlane(pAABB, &pPlanes[i]);
	}
	return bIntersect;
}

ml_dword ML_FUNC ML_AABBIntersectPlane(const ML_AABB* pAABB, const ML_PLANE* pPlane)
{
	ml_float fMin, fMax;
	ml_bool bPosFound, bNegFound;
	ml_dword i;
	
	ML_AABBMinMax(&fMin, &fMax, pAABB, pPlane);
	fMin-=pPlane->d;
	fMax-=pPlane->d;
	
	if(fMax<=0.0f)
	{
		return 0x00000001;
	}
	
	//We have to do the full check...
	bPosFound=ML_FALSE;
	bNegFound=ML_FALSE;
	for(i=0; i<8; i++)
	{
		ML_VEC3 vT;
	
		if(ML_PlaneDotCoord(pPlane, ML_AABBCorner(&vT, pAABB, i))>0.0f)
			bPosFound=ML_TRUE;
		else
			bNegFound=ML_TRUE;
			
		if(bPosFound && bNegFound)
		{
			//An actuall intersection occured
			return 0x00000002;
		}
	}
	
	//The aabb was on the negative side of the plane
	if(bNegFound && !bPosFound)
		return 0x00000001;
	
	//The aabb was on the positive side of the plane	
	return 0x00000000;
}

ml_float ML_FUNC ML_AABBIntersectPlaneVel(const ML_AABB* pAABB, const ML_PLANE* pPlane, const ML_VEC3* pVel)
{
	const float kNoIntersection=1e30f;
	ml_float fTime;
	ml_float fMin, fMax;
	ml_float fDot=ML_Vec3Dot(pVel, (ML_VEC3*)pPlane);
	
	//We're moving away from the plane, so there is no intersection.
	
	if(fDot>=0.0f)
	{
		return 0.0f;//kNoIntersection;
	}
	
	ML_AABBMinMax(&fMin, &fMax, pAABB, pPlane);
	//We're already on the negative side of the plane
	if(fMax<=pPlane->d)
		return 0.0f;//kNoIntersection;
		
	fTime=(pPlane->d-fMin)/fDot;
	
	if(fTime<=0.0f)
	{
		//We were already penetrating
		return 0.0f;
	}
	
	//If fTime is > 1 then the plane was never hit.
	return fTime;
}

ML_PLANE* ML_FUNC ML_AABBToPlanes(ML_PLANE* pOut, ML_AABB* pAABB)
{
	pOut[0].a=0.0f;
	pOut[0].b=0.0f;
	pOut[0].c=-1.0f;
	pOut[0].d=ML_Vec3Dot(&pAABB->v3Min, (ML_VEC3*)&pOut[0]);
	
	pOut[1].a=1.0f;
	pOut[1].b=0.0f;
	pOut[1].c=0.0f;
	pOut[1].d=ML_Vec3Dot(&pAABB->v3Max, (ML_VEC3*)&pOut[1]);
	
	pOut[2].a=0.0f;
	pOut[2].b=0.0f;
	pOut[2].c=1.0f;
	pOut[2].d=ML_Vec3Dot(&pAABB->v3Max, (ML_VEC3*)&pOut[2]);
	
	pOut[3].a=-1.0f;
	pOut[3].b=0.0f;
	pOut[3].c=0.0f;
	pOut[3].d=ML_Vec3Dot(&pAABB->v3Min, (ML_VEC3*)&pOut[3]);
	
	pOut[4].a=0.0f;
	pOut[4].b=1.0f;
	pOut[4].c=0.0f;
	pOut[4].d=ML_Vec3Dot(&pAABB->v3Max, (ML_VEC3*)&pOut[4]);
	
	pOut[5].a=0.0f;
	pOut[5].b=-1.0f;
	pOut[5].c=0.0f;
	pOut[5].d=ML_Vec3Dot(&pAABB->v3Min, (ML_VEC3*)&pOut[5]);
	
	return pOut;
}

ml_dword ML_FUNC ML_AABBIntersectPlaneVelType(
	const ML_AABB* pAABB, 
	const ML_PLANE* pPlane, 
	const ML_VEC3* pVel, 
	ml_float* pTime)
{
	ml_float fMin, fMax;
	ml_float fTime;
	ml_float fDot=ML_Vec3Dot(pVel, (ML_VEC3*)pPlane);
	ml_dword nStartPos;
	ml_bool bToward;
	
	ML_AABBMinMax(&fMin, &fMax, pAABB, pPlane);
	
	//Find out where the box started at...
	if(fMax<=pPlane->d)
		nStartPos=ML_INTERSECT_ONNEG;		
	else if(fMin>=pPlane->d)
		nStartPos=ML_INTERSECT_ONPOS;
	else
		nStartPos=ML_INTERSECT_CUR;
	
	//If we we're already intersecting we won't
	//worry about complex calculations...
	if(nStartPos==ML_INTERSECT_CUR)
		return nStartPos;
	
	//If we weren't moving at all (relative to the planes axis),
	//the start position is the final position.
	if(fDot==0.0f)
		return nStartPos;
	
	//Determine if we are moving toward the positive or negative side of
	//the plane
	bToward=fDot>0.0f?ML_FALSE:ML_TRUE;
	
	if(nStartPos==ML_INTERSECT_ONPOS)
	{
		//If moving away from the plane and started on
		//the positive side, then we are still on the positive side.
		if(!bToward)
			return nStartPos;
		
		//If we are moving towrd and started on the positive side
		//then we will hit the plane at some point with the min
		//vertex.
		fTime=(pPlane->d-fMin)/fDot;
		if(fTime>1.0f)
			return nStartPos;
		
		if(pTime)*pTime=fTime;
		return ML_INTERSECT_HITPOS;
	}
	
	if(nStartPos==ML_INTERSECT_ONNEG)
	{
		//If moving toward the plane and started on the negative side,
		//then we are still on the negative side
		if(bToward)
			return nStartPos;
			
		//If we are moving away from the plane and we started
		//on the negative side then we will hit the plane
		//at some point with the max vertex.
		fTime=(pPlane->d-fMax)/fDot;
		if(fTime>1.0f)
			return nStartPos;
		
		if(pTime)*pTime=fTime;	
		return ML_INTERSECT_HITNEG;
		
	}
	//Otherwise we were already inside the plane
	//and therefore we either stayed inside, left out
	//the pos side, or left out the neg side.
	//This should have been caught above, but just
	//in case we somehow got here...
	return ML_INTERSECT_CUR;	
}


/*
ML_AABB* ML_AABBTransform(ML_AABB* pOut, ML_AABB* pAABB, ML_MAT* pM)
{
	*pOut=*pAABB;
	if(pM->_11>0.0f)
	{
		pOut->v3Min.x+=pM->_11*pOut->v3Min.x;
		pOut->v3Max.x+=pM->_11*pOut->v3Max.x;
	}
	return pOut;
}
*/

ML_AABB* ML_FUNC ML_AABBAddPoint(ML_AABB* pOut, const ML_AABB* pBox, const ML_VEC3* pVec)
{
	ML_VEC3 v3[3];
	v3[0]=pBox->v3Min;
	v3[1]=pBox->v3Max;
	v3[2]=*pVec;
	return ML_AABBFromVec3s(pOut, v3, 3);
}

ML_AABB* ML_FUNC ML_AABBCatenate(ML_AABB* pOut, const ML_AABB* pBox1, const ML_AABB* pBox2)
{
	ML_VEC3 v3[4];
	v3[0]=pBox1->v3Min;
	v3[1]=pBox1->v3Max;
	v3[2]=pBox2->v3Min;
	v3[3]=pBox2->v3Max;
	return ML_AABBFromVec3s(pOut, v3, 4);
}

ml_bool ML_FUNC ML_AABBIntersect(const ML_AABB* pBox1, const ML_AABB* pBox2, ML_AABB* pIntersect)
{
	if(pBox1->v3Min.x > pBox2->v3Max.x)return ML_FALSE;
	if(pBox1->v3Max.x < pBox2->v3Min.x)return ML_FALSE;
	if(pBox1->v3Min.y > pBox2->v3Max.y)return ML_FALSE;
	if(pBox1->v3Max.y < pBox2->v3Min.y)return ML_FALSE;
	if(pBox1->v3Min.z > pBox2->v3Max.z)return ML_FALSE;
	if(pBox1->v3Max.z < pBox2->v3Min.z)return ML_FALSE;
	
	if(pIntersect)
	{
		pIntersect->v3Min.x=MLG_Max(pBox1->v3Min.x, pBox2->v3Min.x);
		pIntersect->v3Max.x=MLG_Min(pBox1->v3Max.x, pBox2->v3Max.x);
		pIntersect->v3Min.y=MLG_Max(pBox1->v3Min.y, pBox2->v3Min.y);
		pIntersect->v3Max.y=MLG_Min(pBox1->v3Max.y, pBox2->v3Max.y);
		pIntersect->v3Min.z=MLG_Max(pBox1->v3Min.z, pBox2->v3Min.z);
		pIntersect->v3Max.z=MLG_Min(pBox1->v3Max.z, pBox2->v3Max.z);
	}
	return ML_TRUE;
}
ml_bool ML_FUNC ML_AABBIntersectVel(ML_VEC3* pOut, const ML_AABB* pBox1Start, const ML_AABB* pBox1End, const ML_AABB* pBox2Static)
{
	return ML_FALSE;
	/*
	ML_AABB aabbI1, aabbI2;
	ml_bool bResult=ML_AABBIntersect(pBox1Start, pBox2Static, &aabbI1);
	bResult==bResult || ML_AABBIntersect(pBox1End, pBox2Static, &aabbI2);
	if(!bResult)
		return ML_FALSE;
		
	ML_VEC3 v3Max, v3Min, v3Temp;
	ML_Vec3Subtract(&v3Max, &pBox1End->v3Max, &pBox1Start->v3Max);
	ML_Vec3Subtract(&v3Min, &pBox1End->v3Min, &pBox1Start->v3Min);
	*/
}
float ML_FUNC ML_AABBIntersectMoving(const ML_AABB* pBoxMove, const ML_AABB* pBoxStat, const ML_VEC3* pVel)
{
	// We'll return this huge number if no intersection
	#define swap(a, b) {float f=b; b=a; a=f;}
	const float kNoIntersection = 1e30f;

	// Initialize interval to contain all the time under consideration

	float	tEnter = 0.0f;
	float	tLeave = 1.0f;

	//
	// Compute interval of overlap on each dimension, and intersect
	// this interval with the interval accumulated so far.  As soon as
	// an empty interval is detected, return a negative result
	// (no intersection.)  In each case, we have to be careful for
	// an infinite of empty interval on each dimension
	//

	// Check x-axis

	if (pVel->x == 0.0f) 
	{
		// Empty or infinite inverval on x

		if (
			(pBoxStat->v3Min.x >= pBoxMove->v3Max.x) ||
			(pBoxStat->v3Max.x <= pBoxMove->v3Min.x)
		) {

			// Empty time interval, so no intersection

			return kNoIntersection;
		}

		// Inifinite time interval - no update necessary

	} else {

		// Divide once

		float	oneOverD = 1.0f / pVel->x;

		// Compute time value when they begin and end overlapping

		float	xEnter = (pBoxStat->v3Min.x - pBoxMove->v3Max.x) * oneOverD;
		float	xLeave = (pBoxStat->v3Max.x - pBoxMove->v3Min.x) * oneOverD;

		// Check for interval out of order

		if (xEnter > xLeave) 
		{
			swap(xEnter, xLeave);
		}

		// Update interval

		if (xEnter > tEnter) tEnter = xEnter;
		if (xLeave < tLeave) tLeave = xLeave;

		// Check if this resulted in empty interval

		if (tEnter > tLeave) {
			return kNoIntersection;
		}
	}
	
	// Check y-axis

	if (pVel->y == 0.0f) {

		// Empty or infinite inverval on y

		if (
			(pBoxStat->v3Min.y >= pBoxMove->v3Max.y) ||
			(pBoxStat->v3Max.y <= pBoxMove->v3Min.y)
		) {

			// Empty time interval, so no intersection

			return kNoIntersection;
		}

		// Inifinite time interval - no update necessary

	} else {

		// Divide once

		float	oneOverD = 1.0f / pVel->y;

		// Compute time value when they begin and end overlapping

		float	yEnter = (pBoxStat->v3Min.y - pBoxMove->v3Max.y) * oneOverD;
		float	yLeave = (pBoxStat->v3Max.y - pBoxMove->v3Min.y) * oneOverD;

		// Check for interval out of order

		if (yEnter > yLeave) {
			swap(yEnter, yLeave);
		}

		// Update interval

		if (yEnter > tEnter) tEnter = yEnter;
		if (yLeave < tLeave) tLeave = yLeave;

		// Check if this resulted in empty interval

		if (tEnter > tLeave) {
			return kNoIntersection;
		}
	}
	
	// Check z-axis

	if (pVel->z == 0.0f) {

		// Empty or infinite inverval on z

		if (
			(pBoxStat->v3Min.z >= pBoxMove->v3Max.z) ||
			(pBoxStat->v3Max.z <= pBoxMove->v3Min.z)
		) {

			// Empty time interval, so no intersection

			return kNoIntersection;
		}

		// Inifinite time interval - no update necessary

	} else {

		// Divide once

		float	oneOverD = 1.0f / pVel->z;

		// Compute time value when they begin and end overlapping

		float	zEnter = (pBoxStat->v3Min.z - pBoxMove->v3Max.z) * oneOverD;
		float	zLeave = (pBoxStat->v3Max.z - pBoxMove->v3Min.z) * oneOverD;

		// Check for interval out of order

		if (zEnter > zLeave) {
			swap(zEnter, zLeave);
		}

		// Update interval

		if (zEnter > tEnter) tEnter = zEnter;
		if (zLeave < tLeave) tLeave = zLeave;

		// Check if this resulted in empty interval

		if (tEnter > tLeave) {
			return kNoIntersection;
		}
	}

	// OK, we have an intersection.  Return the parametric point in time
	// where the intersection occurs

	return tEnter;
}
/*
float	intersectMovingAABB(
	const AABB3 &stationaryBox,
	const AABB3 &movingBox,
	const Vector3 &d
) {

	// We'll return this huge number if no intersection

	const float kNoIntersection = 1e30f;

	// Initialize interval to contain all the time under consideration

	float	tEnter = 0.0f;
	float	tLeave = 1.0f;

	//
	// Compute interval of overlap on each dimension, and intersect
	// this interval with the interval accumulated so far.  As soon as
	// an empty interval is detected, return a negative result
	// (no intersection.)  In each case, we have to be careful for
	// an infinite of empty interval on each dimension
	//

	// Check x-axis

	if (d.x == 0.0f) {

		// Empty or infinite inverval on x

		if (
			(stationaryBox.min.x >= movingBox.max.x) ||
			(stationaryBox.max.x <= movingBox.min.x)
		) {

			// Empty time interval, so no intersection

			return kNoIntersection;
		}

		// Inifinite time interval - no update necessary

	} else {

		// Divide once

		float	oneOverD = 1.0f / d.x;

		// Compute time value when they begin and end overlapping

		float	xEnter = (stationaryBox.min.x - movingBox.max.x) * oneOverD;
		float	xLeave = (stationaryBox.max.x - movingBox.min.x) * oneOverD;

		// Check for interval out of order

		if (xEnter > xLeave) {
			swap(xEnter, xLeave);
		}

		// Update interval

		if (xEnter > tEnter) tEnter = xEnter;
		if (xLeave < tLeave) tLeave = xLeave;

		// Check if this resulted in empty interval

		if (tEnter > tLeave) {
			return kNoIntersection;
		}
	}
	
	// Check y-axis

	if (d.y == 0.0f) {

		// Empty or infinite inverval on y

		if (
			(stationaryBox.min.y >= movingBox.max.y) ||
			(stationaryBox.max.y <= movingBox.min.y)
		) {

			// Empty time interval, so no intersection

			return kNoIntersection;
		}

		// Inifinite time interval - no update necessary

	} else {

		// Divide once

		float	oneOverD = 1.0f / d.y;

		// Compute time value when they begin and end overlapping

		float	yEnter = (stationaryBox.min.y - movingBox.max.y) * oneOverD;
		float	yLeave = (stationaryBox.max.y - movingBox.min.y) * oneOverD;

		// Check for interval out of order

		if (yEnter > yLeave) {
			swap(yEnter, yLeave);
		}

		// Update interval

		if (yEnter > tEnter) tEnter = yEnter;
		if (yLeave < tLeave) tLeave = yLeave;

		// Check if this resulted in empty interval

		if (tEnter > tLeave) {
			return kNoIntersection;
		}
	}
	
	// Check z-axis

	if (d.z == 0.0f) {

		// Empty or infinite inverval on z

		if (
			(stationaryBox.min.z >= movingBox.max.z) ||
			(stationaryBox.max.z <= movingBox.min.z)
		) {

			// Empty time interval, so no intersection

			return kNoIntersection;
		}

		// Inifinite time interval - no update necessary

	} else {

		// Divide once

		float	oneOverD = 1.0f / d.z;

		// Compute time value when they begin and end overlapping

		float	zEnter = (stationaryBox.min.z - movingBox.max.z) * oneOverD;
		float	zLeave = (stationaryBox.max.z - movingBox.min.z) * oneOverD;

		// Check for interval out of order

		if (zEnter > zLeave) {
			swap(zEnter, zLeave);
		}

		// Update interval

		if (zEnter > tEnter) tEnter = zEnter;
		if (zLeave < tLeave) tLeave = zLeave;

		// Check if this resulted in empty interval

		if (tEnter > tLeave) {
			return kNoIntersection;
		}
	}

	// OK, we have an intersection.  Return the parametric point in time
	// where the intersection occurs

	return tEnter;
}
*/

ML_AABB* ML_AABBFromVec3s(ML_AABB* pOut, const ML_VEC3* pVList, const ml_dword nNumVecs)
{
	ml_dword i;
	if(!nNumVecs)
		return pOut;
		
	/* Set the AABB equal to the first vector. */
	pOut->v3Max.x=pVList[0].x;
	pOut->v3Max.y=pVList[0].y;
	pOut->v3Max.z=pVList[0].z;
	
	pOut->v3Min.x=pVList[0].x;
	pOut->v3Min.y=pVList[0].y;
	pOut->v3Min.z=pVList[0].z;
	
	for(i=1; i<nNumVecs; i++)
	{
		pOut->v3Min.x=MLG_Min(pVList[i].x, pOut->v3Min.x);
		pOut->v3Min.y=MLG_Min(pVList[i].y, pOut->v3Min.y);
		pOut->v3Min.z=MLG_Min(pVList[i].z, pOut->v3Min.z);
		
		pOut->v3Max.x=MLG_Max(pVList[i].x, pOut->v3Max.x);
		pOut->v3Max.y=MLG_Max(pVList[i].y, pOut->v3Max.y);
		pOut->v3Max.z=MLG_Max(pVList[i].z, pOut->v3Max.z);
	}
	
	return pOut;
}


ML_VEC3* ML_AABBCorner(ML_VEC3* pV, const ML_AABB* pAABB, const ML_AABB_CORNER ref)
{
	pV->x=(ref&0x01)?pAABB->v3Max.x:pAABB->v3Min.x;
	pV->y=(ref&0x02)?pAABB->v3Max.y:pAABB->v3Min.y;
	pV->z=(ref&0x04)?pAABB->v3Max.z:pAABB->v3Min.z;
	return pV;
}

ML_AABB* ML_AABBTransform(ML_AABB* pOut, const ML_AABB* pAABB, const ML_MAT* pM)
{
	ML_AABB aabbTmp;
	
	// Start with the translation portion
	aabbTmp.v3Min.x=pM->_41;
	aabbTmp.v3Min.y=pM->_42;
	aabbTmp.v3Min.z=pM->_43;
	aabbTmp.v3Max=aabbTmp.v3Min;

	// Examine each of the 9 matrix elements
	// and compute the new AABB
	if (pM->_11 > 0.0f) {
		aabbTmp.v3Min.x += pM->_11 * pAABB->v3Min.x; aabbTmp.v3Max.x += pM->_11 * pAABB->v3Max.x;
	} else {
		aabbTmp.v3Min.x += pM->_11 * pAABB->v3Max.x; aabbTmp.v3Max.x += pM->_11 * pAABB->v3Min.x;
	}

	if (pM->_12 > 0.0f) {
		aabbTmp.v3Min.y += pM->_12 * pAABB->v3Min.x; aabbTmp.v3Max.y += pM->_12 * pAABB->v3Max.x;
	} else {
		aabbTmp.v3Min.y += pM->_12 * pAABB->v3Max.x; aabbTmp.v3Max.y += pM->_12 * pAABB->v3Min.x;
	}

	if (pM->_13 > 0.0f) {
		aabbTmp.v3Min.z += pM->_13 * pAABB->v3Min.x; aabbTmp.v3Max.z += pM->_13 * pAABB->v3Max.x;
	} else {
		aabbTmp.v3Min.z += pM->_13 * pAABB->v3Max.x; aabbTmp.v3Max.z += pM->_13 * pAABB->v3Min.x;
	}

	if (pM->_21 > 0.0f) {
		aabbTmp.v3Min.x += pM->_21 * pAABB->v3Min.y; aabbTmp.v3Max.x += pM->_21 * pAABB->v3Max.y;
	} else {
		aabbTmp.v3Min.x += pM->_21 * pAABB->v3Max.y; aabbTmp.v3Max.x += pM->_21 * pAABB->v3Min.y;
	}

	if (pM->_22 > 0.0f) {
		aabbTmp.v3Min.y += pM->_22 * pAABB->v3Min.y; aabbTmp.v3Max.y += pM->_22 * pAABB->v3Max.y;
	} else {
		aabbTmp.v3Min.y += pM->_22 * pAABB->v3Max.y; aabbTmp.v3Max.y += pM->_22 * pAABB->v3Min.y;
	}

	if (pM->_23 > 0.0f) {
		aabbTmp.v3Min.z += pM->_23 * pAABB->v3Min.y; aabbTmp.v3Max.z += pM->_23 * pAABB->v3Max.y;
	} else {
		aabbTmp.v3Min.z += pM->_23 * pAABB->v3Max.y; aabbTmp.v3Max.z += pM->_23 * pAABB->v3Min.y;
	}

	if (pM->_31 > 0.0f) {
		aabbTmp.v3Min.x += pM->_31 * pAABB->v3Min.z; aabbTmp.v3Max.x += pM->_31 * pAABB->v3Max.z;
	} else {
		aabbTmp.v3Min.x += pM->_31 * pAABB->v3Max.z; aabbTmp.v3Max.x += pM->_31 * pAABB->v3Min.z;
	}

	if (pM->_32 > 0.0f) {
		aabbTmp.v3Min.y += pM->_32 * pAABB->v3Min.z; aabbTmp.v3Max.y += pM->_32 * pAABB->v3Max.z;
	} else {
		aabbTmp.v3Min.y += pM->_32 * pAABB->v3Max.z; aabbTmp.v3Max.y += pM->_32 * pAABB->v3Min.z;
	}

	if (pM->_33 > 0.0f) {
		aabbTmp.v3Min.z += pM->_33 * pAABB->v3Min.z; aabbTmp.v3Max.z += pM->_33 * pAABB->v3Max.z;
	} else {
		aabbTmp.v3Min.z += pM->_33 * pAABB->v3Max.z; aabbTmp.v3Max.z += pM->_33 * pAABB->v3Min.z;
	}
	
	*pOut=aabbTmp;
	return pOut;
}

ML_AABB* ML_AABBTransformAlt(ML_AABB* pOut, const ML_AABB* pAABB, const ML_MAT* pM)
{
	ml_dword i;
	ML_VEC3 v3Corners[8];
	//The idea behind this is to get all the corners, transform the corners and then
	//make a new AABB based on the transformed corners.  More math is required for
	//this.
	for(i=0; i<8; i++)
	{
		ML_AABBCorner(&v3Corners[i], pAABB, (ML_AABB_CORNER)i);
		ML_Vec3TransformCoord(&v3Corners[i], &v3Corners[i], pM);
	}
	
	return ML_AABBFromVec3s(pOut, v3Corners, 8);
}
