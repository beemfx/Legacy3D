#include "lp_sys.h"

CLPhysics::CLPhysics():
	m_pFirstBody(LG_NULL),
	m_pMap(LG_NULL)
{

}

CLPhysics::~CLPhysics()
{
	RemoveAllBodies();
	m_pMap=LG_NULL;
}

CLPhysBody* CLPhysics::AddBody(ML_VEC3* v3Pos, ML_AABB* aabb, lg_float fMass)
{
	CLPhysBody* pNewBody=new CLPhysBody;
	if(!pNewBody)
		return LG_NULL;
		
	pNewBody->m_v3Pos=*v3Pos;
	pNewBody->m_fMass=fMass;
	memset(&pNewBody->m_v3PhysVel, 0, sizeof(ML_VEC3));
	pNewBody->m_aabbBase=*aabb;
	pNewBody->CalculateAABBs();
	pNewBody->m_pNext=m_pFirstBody;
	m_pFirstBody=pNewBody;
	return m_pFirstBody;
}

void CLPhysics::SetWorld(CLWorldMap* pMap)
{
	m_pMap=pMap;
}

void CLPhysics::RemoveAllBodies()
{
	CLPhysBody* pTemp=m_pFirstBody;
	while(pTemp)
	{
		CLPhysBody* pNext=pTemp->m_pNext;
		delete pTemp;
		pTemp=pNext;
	}
	m_pFirstBody=LG_NULL;
}

void CLPhysics::RemoveBody(CLPhysBody* pBody)
{
	CLPhysBody* pTemp;
	if(m_pFirstBody==pBody)
	{
		pTemp=m_pFirstBody->m_pNext;
		delete m_pFirstBody;
		m_pFirstBody=pTemp;
		return;
	}
	pTemp=m_pFirstBody;
	while(pTemp)
	{
		if(pTemp==pBody)
		{
			pTemp=pTemp->m_pNext;
			delete pBody;
			return;
		}
		pTemp=pTemp->m_pNext;
	}
	return;
}

void CLPhysics::Processes(lg_dword nTimeStep)
{
	lg_float fTime=nTimeStep/1000.0f;
	for(CLPhysBody* pBody=m_pFirstBody; pBody; pBody=pBody->m_pNext)
	{
		pBody->Process(fTime, m_pMap);
	}
	
	for(CLPhysBody* pEnt=m_pFirstBody; pEnt; pEnt=pEnt->m_pNext)
	{
		for(CLPhysBody* pEnt2=pEnt->m_pNext; pEnt2; pEnt2=pEnt2->m_pNext)
		{
			//Err_Printf("Detecting 0x%08X against 0x%08X", pEnt, pEnt2);
			pEnt->Collision(pEnt2, fTime);
		}
	}
	
	for(CLPhysBody* pBody=m_pFirstBody; pBody; pBody=pBody->m_pNext)
	{
		pBody->Update();
	}
}

////////////
//Body Code
////////////

ML_VEC3 CLPhysBody::s_v3WishVel;
lg_dword CLPhysBody::s_nCollisionCount;
lg_bool CLPhysBody::s_bWasInBlock;


void CLPhysBody::Collision(CLPhysBody* pEnt, float fTime)
{	
	if(this==pEnt)
		return;
		
	//if(L_CHECK_FLAG(m_nFlags, LENTITY_NOENTITYCOLLIDE) || L_CHECK_FLAG(pEnt->m_nFlags, LENTITY_NOENTITYCOLLIDE))
	//	return;
		
	if(!ML_AABBIntersect(&m_aabbCat, &pEnt->m_aabbCat, LG_NULL))
		return;
		
	//m_v3ScaleVel.x=m_v3ScaleVel.y=m_v3ScaleVel.z=0.0f;
	//pEnt->m_v3ScaleVel.x=pEnt->m_v3ScaleVel.y=pEnt->m_v3ScaleVel.z=0.0f;
	//return;
	
	ML_VEC3 v3Vel;
	ML_Vec3Subtract(&v3Vel, &m_v3ScaleVel, &pEnt->m_v3ScaleVel);
	
	lg_float fColTime=ML_AABBIntersectMoving(
								&m_aabbCurrent, 
								&pEnt->m_aabbCurrent, 
								&v3Vel);
								
	if(fColTime>=1e30f)
		return;
	
	//There was a collision, calulate the new velocities.
	
	//ML_VEC3 v3Temp;
	//AddExtForce(&pEnt->m_v3PhysVel);//ML_Vec3Scale(&v3Temp, &pEnt->m_v3Vel, 1.0f));//0.5f));
	//pEnt->AddExtForce(&m_v3PhysVel);//ML_Vec3Scale(&v3Temp, &m_v3Vel, 1.0f));//0.5f));
	#if 0
	#elif 1
	{
		ML_VEC3 v3Ref;
		ML_Vec3Subtract(&v3Ref, &m_v3PhysVel, &pEnt->m_v3PhysVel);
		lg_float fM1=(this->m_fMass-pEnt->m_fMass)/(m_fMass+pEnt->m_fMass);
		lg_float fM2=(this->m_fMass*2)/(this->m_fMass+pEnt->m_fMass);
		
		m_v3PhysVel.x=fM1*v3Ref.x;
		m_v3PhysVel.y=fM1*v3Ref.y;
		m_v3PhysVel.z=fM1*v3Ref.z;
		
		ML_VEC3 v3Temp;
		
		v3Temp.x=fM2*v3Ref.x;
		v3Temp.y=fM2*v3Ref.y;
		v3Temp.z=fM2*v3Ref.z;
		
		ML_Vec3Add(&m_v3PhysVel, &pEnt->m_v3PhysVel, &m_v3PhysVel);
		ML_Vec3Add(&pEnt->m_v3PhysVel, &pEnt->m_v3PhysVel, &v3Temp);
		
		m_v3ScaleVel.x=m_v3ScaleVel.y=m_v3ScaleVel.z=0.0f;
		pEnt->m_v3ScaleVel.x=pEnt->m_v3ScaleVel.y=pEnt->m_v3ScaleVel.z=0.0f;
	}
	#elif 0
	if(0)//fColTime>0.2f)
	{
		ML_Vec3Scale(&m_v3Vel, &m_v3Vel, 0.8f*fColTime);
		ML_Vec3Scale(&pEnt->m_v3Vel, &pEnt->m_v3Vel, 0.8f*fColTime);
	}
	else
	{
		m_v3Vel.x=m_v3Vel.y=m_v3Vel.z=0.0f;
		pEnt->m_v3Vel.x=pEnt->m_v3Vel.y=pEnt->m_v3Vel.z=0.0f;
	}
	#elif 0
	m_v3ScaleVel.x=m_v3ScaleVel.y=m_v3ScaleVel.z=0.0f;
	pEnt->m_v3ScaleVel.x=pEnt->m_v3ScaleVel.y=pEnt->m_v3ScaleVel.z=0.0f;
	#endif
	return;
}

void CLPhysBody::Update()
{
	ML_Vec3Add(&m_v3Pos, &m_v3Pos, &m_v3ScaleVel);
	
	//Update the current AABB (really this is only for rendering)
	//purposes
	CalculateAABBs();
}

void CLPhysBody::GetPosition(ML_VEC3* v3Pos)
{
	*v3Pos=m_v3Pos;
}

void CLPhysBody::SetVelocity(ML_VEC3* v3Vel)
{
	m_v3PhysVel=*v3Vel;
}

void CLPhysBody::GetVelocity(ML_VEC3* v3Vel)
{
	*v3Vel=m_v3PhysVel;
}

void CLPhysBody::GetAABB(ML_AABB* aabb)
{
	*aabb=m_aabbCurrent;
}

void CLPhysBody::Process(lg_float fTime, CLWorldMap* pMap)
{
	ML_Vec3Scale(&m_v3ScaleVel, &m_v3PhysVel, fTime);
	CalculateAABBs();
	if(!pMap)
		return;
		
	m_nNumRegions=pMap->CheckRegions(&m_aabbCat, m_nRegions, 8);
	WorldClip(pMap, LG_TRUE);
	ML_Vec3Scale(&m_v3PhysVel, &m_v3ScaleVel, 1.0f/fTime);
}

void CLPhysBody::WorldClip(CLWorldMap* pMap, lg_bool bSlide)
{
	//For now this is just a test collision system.
	if(!pMap->m_bLoaded)
		return;
	
	//Start out with the proposed velocity as
	//the velocity determined by the AI.
	s_v3WishVel=m_v3ScaleVel;
	s_nCollisionCount=0;
	s_bWasInBlock=LG_FALSE;
	
	//Should really only check the regions that the entities
	//catenated aabb are within, this will be done in the future
	for(lg_dword i=0; i<m_nNumRegions; i++)
	{
		RegionClip(&pMap->m_pRegions[m_nRegions[i]], bSlide);
	}
	//If there was 1 or less collisions and
	//we were never in a block, then chances are
	//the clip worked fine.
	if(s_nCollisionCount<1 && !s_bWasInBlock)
	{
		m_v3ScaleVel=s_v3WishVel;
		return;
	}
	
	//If more than one collision occured we'll recheck
	//for any collisions... This time we won't slide
	//we'll just stop the movement.
	s_nCollisionCount=0;
	s_bWasInBlock=LG_FALSE;
	for(lg_dword i=0; i<m_nNumRegions; i++)
	{
		RegionClip(&pMap->m_pRegions[m_nRegions[i]], LG_FALSE);
	}
	//if(s_nCollisionCount)
	//{
	//	s_v3WishVel.x=s_v3WishVel.y=s_v3WishVel.z=0.0f;
	//}
	m_v3ScaleVel=s_v3WishVel;
	
	return;
}

void CLPhysBody::RegionClip(LW_REGION_EX* pRegion, lg_bool bSlide)
{
	lg_dword nRes;
	
	for(lg_dword i=0; i<pRegion->nGeoBlockCount; i++)
	{
		if(!ML_AABBIntersect(&m_aabbCat, &pRegion->pGeoBlocks[i].aabbBlock, LG_NULL))
			continue;
			
		nRes=BlockClip(&pRegion->pGeoBlocks[i], bSlide);
		
		if(nRes>0)
		{
			//If a collision occured we need to
			//recalculate the AABB for the entity...
			ML_Vec3Add(&m_aabbNext.v3Min, &m_aabbCurrent.v3Min, &s_v3WishVel);
			ML_Vec3Add(&m_aabbNext.v3Max, &m_aabbCurrent.v3Max, &s_v3WishVel);
			//and calculate the catenated AABB....
			ML_AABBCatenate(&m_aabbCat, &m_aabbCurrent, &m_aabbNext);
		} 	
	}
}

lg_dword CLPhysBody::BlockClip(LW_GEO_BLOCK_EX* pBlock, lg_bool bSlide)
{
	lg_bool bClipped=LG_FALSE;
	
	lg_float fTime;
	ML_PLANE plnIntr;
	
	lg_dword nRes=BlockCollision(pBlock, &s_v3WishVel, &fTime, &plnIntr);
	if(nRes==1)
	{
		if(bSlide)
		{
			#define MOVE_SCALE 0.9f
			ML_VEC3 v3Temp;
			ML_Vec3Scale(&v3Temp, &s_v3WishVel, fTime*MOVE_SCALE);
			ML_Vec3Subtract(&s_v3WishVel, &s_v3WishVel, &v3Temp);
			ClipVel(&s_v3WishVel, &s_v3WishVel, &plnIntr, 1.0001f);
			ML_Vec3Add(&s_v3WishVel, &s_v3WishVel, &v3Temp);
		}
		else
		{
			s_v3WishVel.x=s_v3WishVel.y=s_v3WishVel.z=0.0f;
			//ML_Vec3Scale(&s_v3WishVel, &s_v3WishVel, fTime*MOVE_SCALE);
		}
		
		s_nCollisionCount++;
	}
	else if(nRes==2)
	{
		s_nCollisionCount++;
		s_bWasInBlock=LG_TRUE;
	}	
	
	return nRes;
}

lg_dword CLPhysBody::BlockCollision(
	LW_GEO_BLOCK_EX* pBlock, 
	const ML_VEC3* pVel, 
	lg_float* pColTime, 
	ML_PLANE* pHitPlane,
	lg_bool bAABBCheck)
{
	lg_float fColTime=1e30f;
	lg_dword nIntrPlane=0xFFFFFFFF;
	LW_GEO_BLOCK_EX* pChkBlock=pBlock;
	LW_GEO_BLOCK_EX blkAABB;
	LW_GEO_FACE     plns[6];
	
	if(bAABBCheck)
	{
		blkAABB.pFaces=plns;
		ML_AABBToPlanes((ML_PLANE*)blkAABB.pFaces, &pBlock->aabbBlock);
		blkAABB.nFaceCount=6;
		pChkBlock=&blkAABB;
	}
	
	for(lg_dword i=0; i<pChkBlock->nFaceCount; i++)
	{
		lg_float fTime;
		lg_dword nType;
		
		nType=ML_AABBIntersectPlaneVelType(
			&m_aabbCurrent,
			&pChkBlock->pFaces[i],
			pVel,
			&fTime);
		
		switch(nType)
		{
		case ML_INTERSECT_ONPOS:
			//If we made it here we weren't actually colliding.
			return 0;
		default:
			continue;
		case ML_INTERSECT_HITPOS:
			if(fColTime>fTime)
			{
				fColTime=fTime;
				nIntrPlane=i;
			}
		}	
	}
	
	if(nIntrPlane<pChkBlock->nFaceCount)
	{
		if(pColTime)
			*pColTime=fColTime;
		if(pHitPlane)
			*pHitPlane=pChkBlock->pFaces[nIntrPlane];
		
		return 1;
	}
	
	//If we made it here we need to check against the planes,
	//because we probably hit a corner.
	if(!bAABBCheck)
		return BlockCollision(
			pBlock,
			pVel,
			pColTime,
			pHitPlane,
			LG_TRUE);
			
	//If we made it here we we're probably already inside the block.
	return 2;
}

void CLPhysBody::ClipVel(ML_VEC3* pOut, ML_VEC3* pIn, ML_PLANE* pPlane, lg_float overbounce)
{
	float backoff=ML_Vec3Dot(&m_v3ScaleVel, (ML_VEC3*)pPlane);
	
	#if 1
	if(backoff<0.0f)
	{
		backoff*=overbounce;
	}
	else
	{
		backoff/=overbounce;
	}
	#endif
	
	float change=pPlane->a*backoff;
	pOut->x=pIn->x-change;
	change=pPlane->b*backoff;
	pOut->y=pIn->y-change;
	change=pPlane->c*backoff;
	pOut->z=pIn->z-change;
}

#if 0
OVERBOUCE 1.001f
void PM_ClipVelocity( vec3_t in, vec3_t normal, vec3_t out, float overbounce ) {
	float	backoff;
	float	change;
	int		i;
	
	backoff = DotProduct (in, normal);
	
	if ( backoff < 0 ) {
		backoff *= overbounce;
	} else {
		backoff /= overbounce;
	}

	for ( i=0 ; i<3 ; i++ ) {
		change = normal[i]*backoff;
		out[i] = in[i] - change;
	}
}
#endif

void CLPhysBody::CalculateAABBs()
{
	//Calculate the current AABB (add the position to the base AABB).
	ML_Vec3Add(&m_aabbCurrent.v3Min, &m_aabbBase.v3Min, &m_v3Pos);
	ML_Vec3Add(&m_aabbCurrent.v3Max, &m_aabbBase.v3Max, &m_v3Pos);

	
	//Calculate the potential AABB (add the velocity to the current AABB).
	ML_Vec3Add(&m_aabbNext.v3Min, &m_aabbCurrent.v3Min, &m_v3ScaleVel);
	ML_Vec3Add(&m_aabbNext.v3Max, &m_aabbCurrent.v3Max, &m_v3ScaleVel);
	
	//Calculate the concatenated AABB.
	ML_AABBCatenate(&m_aabbCat, &m_aabbCurrent, &m_aabbNext);
}

#if 0
TRACE_RESULT CLPhysBody::GroundTrace(LW_REGION_EX* pRegion, ML_PLANE* pGround)
{
	lg_float fTrace=(m_v3PhysVel.y+s_fGrav*s_Timer.GetFElapsed())*s_Timer.GetFElapsed();
	//if(fTrace>0.0f)
	//	return LG_FALSE;
		
	ML_VEC3 v3={0.0f, fTrace, 0.0f};
	ML_AABB aabb=m_aabbCurrent;
	if(fTrace>0.0f)
		aabb.v3Max.y+=fTrace;
	else
		aabb.v3Min.y+=fTrace;
		
	lg_float fTime=1e30f;
	ML_PLANE plnOut;
	lg_bool bHit=LG_FALSE;
	//Should only check against regions that the object is actually
	//in (region check is not implemented yet).
	for(lg_dword i=0; i<pRegion->nGeoBlockCount; i++)
	{
		if(!ML_AABBIntersect(&aabb, &pRegion->pGeoBlocks[i].aabbBlock, LG_NULL))
			continue;
		
		ML_PLANE plnHit;
		lg_float fColTime;	
		lg_dword nRes=BlockCollision(
			&pRegion->pGeoBlocks[i],
			&v3,
			&fColTime,
			&plnHit);
		
		if(nRes==1)
		{	
			if(!bHit)
			{
				bHit=LG_TRUE;
				fTime=fColTime;
				plnOut=plnHit;
			}
			else if(fColTime<fTime)
			{
				fTime=fColTime;	
				plnOut=plnHit;
			}
		}
	}

	if(!bHit)
		return TRACE_INAIR;
			
	if(pGround)
		*pGround=plnOut;
		
	if(fTrace>0.0f)
		return TRACE_HITCEILING;

	return TRACE_ONGROUND;
}
#endif

