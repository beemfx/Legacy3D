/* lp_legacy.cpp - The Legacy Physics engine, for additional
	remarks see lp_legacy.h
	
	Copyright (c) 2007 Blaine Myers
*/
#include "lp_legacy.h"
#include "lg_err.h"
#include "lg_func.h"
#include "lw_entity.h"
#include "lw_map.h"


//#define ITERATIVE_METHOD

void CLPhysLegacy::CollisionCheck()
{
	//Loop through all bodies accordingly and adjust for collisions.
	//In the future should only check bodies that are in the same section
	//of the map, not just all subsequent bodies.
	for(CLListStack::LSItem* pBody1=m_AwakeBodies.m_pFirst; pBody1; pBody1=pBody1->m_pNext)
	{
		m_nClsns=0;
		//Check against other bodies:
		for(CLListStack::LSItem* pBody2=m_AwakeBodies.m_pFirst; pBody2; pBody2=pBody2->m_pNext)
		{
			BodyClsnCheck((CLPhysBody*)pBody1, (CLPhysBody*)pBody2);
		}
		
		for(CLListStack::LSItem* pBody2=m_AsleepBodies.m_pFirst; pBody2; pBody2=pBody2->m_pNext)
		{
			BodyClsnCheck((CLPhysBody*)pBody1, (CLPhysBody*)pBody2);
		}
		
		#ifndef ITERATIVE_METHOD
		//Check against the hulls that make up the world:
		for(lg_dword i=0; i<m_nHullCount; i++)
		{
			HullClsnCheck((CLPhysBody*)pBody1, &m_pGeoHulls[i]);
		}
		#endif ITERATIVE_METHOD
	}
	
	#ifdef ITERATIVE_METHOD
	for(CLListStack::LSItem* pBody1=m_AwakeBodies.m_pFirst; pBody1; pBody1=pBody1->m_pNext)
	{
		m_nClsns=0;
		
		//Check against the hulls that make up the world:
		for(lg_dword i=0; i<m_nHullCount; i++)
		{
			HullClsnCheck((CLPhysBody*)pBody1, &m_pGeoHulls[i]);
		}
		
		if(m_nClsns)
		{
			m_nClsns=0;
			//Check against the hulls that make up the world:
			for(lg_dword i=0; i<m_nHullCount; i++)
			{
				HullClsnCheck((CLPhysBody*)pBody1, &m_pGeoHulls[i]);
			}
		}
		
		if(m_nClsns)
		{
			CLPhysBody* pBdy=(CLPhysBody*)pBody1;
			pBdy->m_v3Vel=s_v3Zero;
			SetPostMoveAABB(pBdy);
		}
	}
	#endif ITERATIVE_METHOD
	
	//Should probably also check any bodies that were woken up.
	
}
void CLPhysLegacy::BodyClsnCheck(CLPhysBody* pBody1, CLPhysBody* pBody2)
{
	if(pBody1==pBody2)
		return;
	
	//Check to see if there is any chance that the bodies collided.
	if(!ML_AABBIntersect(&pBody1->m_aabbMove, &pBody2->m_aabbMove, LG_NULL))
		return;
	
	//We'll calculate by subtracting the velocity of body 1 from body 2 to 
	//get a frame of reference where body 2 is not moving.
	static ML_VEC3 v3ScaleVel;
	static ML_VEC3 v3RefVel;
	ML_Vec3Subtract(&v3RefVel, &pBody1->m_v3Vel, &pBody2->m_v3Vel);
	//Adjust the velocity to reflect the actual amount of movement.
	ML_Vec3Scale(&v3ScaleVel, &v3RefVel, m_fTimeStep);
	
	static lg_float fColTime;
	
	//Now we'll find out when the collision occured.
	fColTime=ML_AABBIntersectMoving(
		&pBody1->m_aabbPreMove,
		&pBody2->m_aabbPreMove,
		&v3ScaleVel);
		
	//If we had a large collision time, then a collsion never really occured.
	if(fColTime>=1e30f)
		return;
		
	//Err_MsgPrintf("Collision");
	
	lg_bool bElastic=LG_TRUE;
	
	if(bElastic)
	{
		//Elastic collision:
		#ifdef FORCE_METHOD
		//Since there was a collision we'll calculate the new velocities.
		lg_float fM1=(pBody1->m_fMass-pBody2->m_fMass)/(pBody1->m_fMass+pBody2->m_fMass);
		lg_float fM2=(pBody1->m_fMass*2)/(pBody1->m_fMass+pBody2->m_fMass);
		
		ML_Vec3Scale(&pBody1->m_v3Vel, &v3RefVel, fM1);
		ML_Vec3Scale(&v3RefVel, &v3RefVel, fM2);
		
		ML_Vec3Add(&pBody1->m_v3ClsnForces, &pBody2->m_v3Vel, &pBody1->m_v3Vel);
		ML_Vec3Add(&pBody2->m_v3ClsnForces, &pBody2->m_v3Vel, &v3RefVel);
		#else
		//Since there was a collision we'll calculate the new velocities.
		lg_float fM1=(pBody1->m_fMass-pBody2->m_fMass)/(pBody1->m_fMass+pBody2->m_fMass);
		lg_float fM2=(pBody1->m_fMass*2)/(pBody1->m_fMass+pBody2->m_fMass);
		
		ML_Vec3Scale(&pBody1->m_v3Vel, &v3RefVel, fM1);
		ML_Vec3Scale(&v3RefVel, &v3RefVel, fM2);
		
		ML_Vec3Add(&pBody1->m_v3Vel, &pBody2->m_v3Vel, &pBody1->m_v3Vel);
		ML_Vec3Add(&pBody2->m_v3Vel, &pBody2->m_v3Vel, &v3RefVel);
		
		//Should probably add some torque, but it should be based
		//on where the body's hit each other.
		//pBody1->m_v3AngVel.y+=1.0f;
		//pBody2->m_v3AngVel.y+=1.0f;
		#endif
	}
	else
	{
		//Inelastic Collison:
		
		//This is not an accurate check, but the idea
		//is that if the bodies were already colliding
		//we don't need to adjust the velocity.
		if(pBody1->m_nColBody=pBody2->m_nItemID)
		{
			//Just set the velocities equal
			pBody2->m_v3Vel=pBody1->m_v3Vel=v3RefVel;
		}
		else
		{
			//We'll use the laws of conservation of momentum:
			pBody1->m_nColBody=pBody2->m_nItemID;
			pBody2->m_nColBody=pBody1->m_nItemID;
			
			lg_float fM=pBody1->m_fMass/(pBody1->m_fMass+pBody2->m_fMass);
			pBody2->m_v3Vel.x=v3RefVel.x*fM;
			pBody2->m_v3Vel.y=v3RefVel.y*fM;
			pBody2->m_v3Vel.z=v3RefVel.z*fM;
			
			pBody1->m_v3Vel=pBody2->m_v3Vel;
		}
	}
	
	
	//We also need to calculate new AABBs
	SetPostMoveAABB(pBody1);
	SetPostMoveAABB(pBody2);	
}


void CLPhysLegacy::HullClsnCheck(CLPhysBody* pBody, CLPhysHull* pHull)
{
	//Do a quick check to see if the aabbs intersect.
	//Should probably start with a sphere check for additional
	//speed.
	if(!ML_AABBIntersect(&pBody->m_aabbMove, &pHull->m_aabbHull, LG_NULL))
		return;
		
	//We'll calculate by subtracting the velocity of body 1 from body 2 to 
	//get a frame of reference where body 2 is not moving.
	static ML_VEC3 v3ScaleVel;
	static ml_plane plnIsect;
	//Adjust the velocity to reflect the actual amount of movement.
	ML_Vec3Scale(&v3ScaleVel, &pBody->m_v3Vel, m_fTimeStep);
	
	if(AABBIntersectHull(&pBody->m_aabbPreMove,pHull,&v3ScaleVel,&plnIsect))
	{
		m_nClsns++;
		
		#ifdef FORCE_METHOD
		//Applying forces rather than directly
		//manipulating:
		ml_vec3 v3Temp;
		ML_Vec3Scale(
			&v3Temp, (ml_vec3*)&plnIsect.a, 
			ML_Vec3Dot((ml_vec3*)&plnIsect.a, &pBody->m_v3Vel));
		ML_Vec3Subtract(&pBody->m_v3ClsnForces, &pBody->m_v3Vel, &v3Temp);
		#elif 1
		//Inelastic collision:
		//From Lengyel (p. 125)
		//This is modified bounce code, rather than reflecting
		//this code flattens the movement onto the surface.
		ml_vec3 v3Temp;
		lg_float fDot;
		fDot=ML_Vec3Dot((ml_vec3*)&plnIsect.a, &pBody->m_v3Vel);
		
		ML_Vec3Scale(&v3Temp, (ml_vec3*)&plnIsect.a, fDot);
		ML_Vec3Subtract(&pBody->m_v3Vel, &pBody->m_v3Vel, &v3Temp);
		//We don't want the body to get progressively closer to the plane
		//it is intersecting, so we'll apply just a litte bit of elasticity.
		//This is especially important when the game is running at a high
		//frame rate, because on quick frames the object may not move very
		//far, whereas on other frames it will move a great distance.
		//We multiply this by the time step because ultimately the velocity
		//is going to get scaled by the time step so we want to make sure
		//that when the game run's at different framerates the amount of
		//overbounce isn't noticeable.
		ml_float fOverBounce=1.0f;
		ML_Vec3Scale(&v3Temp, (ml_vec3*)&plnIsect.a, m_fTimeStep*fOverBounce);
		ML_Vec3Add(&pBody->m_v3Vel, &v3Temp, &pBody->m_v3Vel);
		/*
		ML_Vec3Scale(
			&v3Temp, (ml_vec3*)&plnIsect.a, 
			2.0f*ML_Vec3Dot((ml_vec3*)&plnIsect.a, &pBody->m_v3Vel));
				
		ML_Vec3Subtract(&pBody->m_v3Vel, &v3Temp, &pBody->m_v3Vel);
		*/
		#elif 0
		//Just push in the oppisite direction:
		ML_Vec3Scale(&pBody->m_v3Vel, &pBody->m_v3Vel, -1.0f);
		#else
		//Elastic collision:
		//From Lengyel (p. 125)
		//This is just standard bounce, code, that would cause
		//a bounce at a reflection angle.
		ml_vec3 v3Temp;
		
		ML_Vec3Scale(&pBody->m_v3Vel, &pBody->m_v3Vel, -1.0f);
		ML_Vec3Scale(
			&v3Temp, (ml_vec3*)&plnIsect.a, 
			2.0f*ML_Vec3Dot((ml_vec3*)&plnIsect.a, &pBody->m_v3Vel));
				
		ML_Vec3Subtract(&pBody->m_v3Vel, &v3Temp, &pBody->m_v3Vel);
		#endif
			
		ML_Vec3Scale(&v3ScaleVel, &pBody->m_v3Vel, m_fTimeStep);
		SetPostMoveAABB(pBody);
	}
}


lg_dword CLPhysLegacy::AABBIntersectHull(
	const ml_aabb* pAABB, 
	const CLPhysHull* pHull, 
	const ml_vec3* pVel, 
	ml_plane* pIsctPlane)
{
	static lg_float fTime;
	static lg_dword nType;
	static lg_bool  bIsect;
	

	bIsect=LG_FALSE;
	for(lg_dword i=0; i<pHull->m_nNumFaces; i++)
	{	
		nType=ML_AABBIntersectPlaneVelType(
			pAABB,
			&pHull->m_pFaces[i],
			pVel,
			&fTime);
		
		if(nType==ML_INTERSECT_ONPOS)
		{
			//If we ever find that we're on the positive side
			//of a plane, then we know for certain that there
			//wasn't a collision with this hull, so we can
			//end the function immediately.
			return 0;
		}
		else if(nType==ML_INTERSECT_HITPOS)
		{
			//If we intersected a plane, then we set the
			//intersection flag to true, and save the copy
			//of the plane that we intersected.  Since hulls
			//are convex we can only hit one plane on the hull
			//we still need to continue to check the planes
			//because even if we hit a plane it is possible 
			//that we are still on the positive side of 
			//another one.
			bIsect=LG_TRUE;
			*pIsctPlane=pHull->m_pFaces[i];
		}
	}
	
	//If we had an intersection we're done.
	if(bIsect)return 1;

	//If not, and we got here, then we probably hit a corner, and
	//we need to check against the bounding aabb.
	static ml_plane bounds[6];
	ML_AABBToPlanes(bounds, (ml_aabb*)&pHull->m_aabbHull);
	
	//The following is pretty much exactly the same as above,
	//but we're writing it again, because I don't want to use
	//recursion to check this.
	bIsect=LG_FALSE;
	for(lg_dword i=0; i<6; i++)
	{
		nType=ML_AABBIntersectPlaneVelType(
			pAABB,
			&bounds[i],
			pVel,
			&fTime);
		
		if(nType==ML_INTERSECT_ONPOS)
		{
			return 0;
		}
		else if(nType==ML_INTERSECT_HITPOS)
		{
			bIsect=LG_TRUE;
			*pIsctPlane=bounds[i];
		}
	}
	
	if(bIsect)return 1;
	
	//Technically we should never make it here, but it is a possibility.
	//And if we are here, then it means we were probably already inside
	//of the hull.
	return 0;
}
void CLPhysLegacy::SetPreMoveAABB(CLPhysBody* pBody)
{
	#if 1
	//The aabb is the base aabb adjusted by the objects
	//position, and center of gravity:
	ML_Vec3Add(
		&pBody->m_aabbPreMove.v3Min, 
		&pBody->m_aabbBase.v3Min, 
		(ML_VEC3*)&pBody->m_matPos._41);
	ML_Vec3Add(
		&pBody->m_aabbPreMove.v3Max, 
		&pBody->m_aabbBase.v3Max, 
		(ML_VEC3*)&pBody->m_matPos._41);
	#elif 0
	ML_AABBTransform(&pBody->m_aabbPreMove, &pBody->m_aabbBase, &pBody->m_matPos);
	#endif
		
}
void CLPhysLegacy::SetPostMoveAABB(CLPhysBody* pBody)
{
	#if 1
	static ML_VEC3 v3Temp;
	ML_Vec3Scale(&v3Temp, &pBody->m_v3Vel, m_fTimeStep);
	ML_Vec3Add(&v3Temp, &v3Temp, (ML_VEC3*)&pBody->m_matPos._41);
	
	//Now save the new volume of the body:
	ML_Vec3Add(
		&pBody->m_aabbPostMove.v3Min, 
		&pBody->m_aabbBase.v3Min, 
		(ML_VEC3*)&v3Temp);
		
	ML_Vec3Add(
		&pBody->m_aabbPostMove.v3Max, 
		&pBody->m_aabbBase.v3Max, 
		(ML_VEC3*)&v3Temp);
		
	//We'll also created a catenated AABB which
	//represents the volume occupied by the move.
	ML_AABBCatenate(&pBody->m_aabbMove, &pBody->m_aabbPreMove, &pBody->m_aabbPostMove);
	#elif 0
	ml_mat matTemp, matTemp2;
	ml_vec3 v3Temp;
	ML_Vec3Scale(&v3Temp, &pBody->m_v3Vel, m_fTimeStep);
	ML_MatTranslation(&matTemp, v3Temp.x, v3Temp.y, v3Temp.z);
	ML_MatRotationAxis(&matTemp2, &pBody->m_v3AngVel, m_fTimeStep*ML_Vec3Length(&v3Temp));
	ML_MatMultiply(&matTemp, &matTemp2, &matTemp);
	ML_MatMultiply(&matTemp, &matTemp, &pBody->m_matPos);
	ML_AABBTransform(&pBody->m_aabbPostMove, &pBody->m_aabbBase, &matTemp);
	ML_AABBCatenate(&pBody->m_aabbMove, &pBody->m_aabbPreMove, &pBody->m_aabbPostMove);
	#endif
}

void CLPhysLegacy::ApplyWorldForces(CLPhysBody* pBody)
{
	static ml_vec3 v3Temp;
	//Linear Drag:
	//We'll use (1/4)(surface area)(v^2) for drag.
	//We'll use m_fDrag as the surface area.
	lg_float fDrag=/*(0.25f)**/pBody->m_fSpeed*pBody->m_fSpeed*m_fDrag*m_fTimeStep;
	ML_Vec3Scale(&v3Temp, &pBody->m_v3Vel, fDrag);
	ML_Vec3Subtract(&pBody->m_v3Vel, &pBody->m_v3Vel, &v3Temp);
	
	//Angular drag:
	fDrag=pBody->m_fAngSpeed*pBody->m_fAngSpeed*m_fDrag*m_fTimeStep;
	ML_Vec3Scale(&v3Temp, &pBody->m_v3AngVel, fDrag);
	ML_Vec3Subtract(&pBody->m_v3AngVel, &pBody->m_v3AngVel, &v3Temp);
	
	//Add in the gravity force:
	//Note that technically the gravity force would
	//be m_fTimeStep*grav*mass*oneOverMass, but we
	//can just cancel out the mass.
	ML_Vec3Scale(&v3Temp, &m_v3Grav, m_fTimeStep);
	ML_Vec3Add(&pBody->m_v3Vel, &pBody->m_v3Vel, &v3Temp);
	
	#ifdef FORCE_METHOD
	//There may also have been forces carried over 
	//from the collision detection, they get applied
	//directly:
	ML_Vec3Add(&pBody->m_v3Vel, &pBody->m_v3Vel, &pBody->m_v3ClsnForces);
	pBody->m_v3ClsnForces=s_v3Zero;
	#endif
}

void CLPhysLegacy::ProcessAwakeBodies()
{
	static ML_VEC3 v3Temp;
	static ML_MAT matTemp, matTemp2;
	static lg_float fOneOverMass;
	
	//Only need to simulate for awake bodies.
	for(CLListStack::LSItem*pNode=m_AwakeBodies.m_pFirst; pNode; pNode=pNode->m_pNext)
	{
		//We'll loop through each body in the list stack of awake
		//bodies and process them.
		
		CLPhysBody* pBody=(CLPhysBody*)pNode;
		
		//Technically we only need to update the body if
		//it changed, and only awake bodies will change (or asleep
		//bodies that experienced a collision).
		
		//We need to update the body from the server,
		//this will get us the any server influenced physics
		//(such as thrust from an entity that is walking).
		UpdateFromSrv(pBody, (LEntitySrv*)pBody->m_pEnt);
		
		//Since we use 1/Mass quite frequenty we'll calcuate it
		fOneOverMass=1.0f/pBody->m_fMass;
		
		//Before moving the body, we'll save the volume of the body:
		//You'd think we could just set PreMove AABB to the PostMove
		//AABB, but this won't work cause those AABBs aren't set when
		//a body is created or if a body teleports, so we calculate them
		//on the spot.
		SetPreMoveAABB(pBody);
		
		//Apply forces exerted by the world (gravity, drag)
		ApplyWorldForces(pBody);
		
		//Apply forces exerted by the game engine (self propelled force,
		//i.e. if the object was walking, or got hit by a bullet and needs
		//to move or something).
		if(LG_CheckFlag(pBody->m_nFlags, AI_PHYS_DIRECT_LINEAR))
		{
			//If the body is setup for a direct linear move,
			//we just add the thrust onto the position.
			ML_Vec3Add((ML_VEC3*)&pBody->m_matPos._41, (ML_VEC3*)&pBody->m_matPos._41, &pBody->m_v3Thrust);
		}
		else
		{
			#if 1
			//If the body is not moving directly, we update the velocity
			//by calculating the acceleration and adding that to the velocity.
			
			//The first thing we'll do is update the velocity (this is done
			//using standard calculus for physics, the acceleration is
			//simly (Force/Mass), then we just add that
			//acceleration onto the velocity.
			ML_Vec3Scale(&v3Temp, &pBody->m_v3Thrust, fOneOverMass*m_fTimeStep); //a*t=(F/M)*t
			ML_Vec3Add(&pBody->m_v3Vel, &pBody->m_v3Vel, &v3Temp); //V = V0 + a*t
			//We now have the velocity at which the object is moving (dV/dt)
			#else
			ML_Vec3Scale(&v3Temp, &pBody->m_v3Thrust, fOneOverMass);
			//ML_Vec3Add(&pBody->m_v3Accel, &pBody->m_v3Accel, &v3Temp);
			pBody->m_v3Accel=v3Temp;
			ML_Vec3Scale(&v3Temp, &pBody->m_v3Accel, m_fTimeStep);
			ML_Vec3Add(&pBody->m_v3Vel, &pBody->m_v3Vel, &v3Temp);
			#endif
		}
		
		if(LG_CheckFlag(pBody->m_nFlags, AI_PHYS_DIRECT_ANGULAR))
		{
			ML_MatRotationAxis(
				&matTemp,
				&pBody->m_v3Torque,
				ML_Vec3Length(&pBody->m_v3Torque));
				
			ML_MatMultiply(&pBody->m_matPos, &matTemp, &pBody->m_matPos);
		}
		else
		{
			//Update the angular velocity:
			//Typical physics torque/mass = angular acceleration.
			ML_Vec3Scale(&v3Temp, &pBody->m_v3Torque, m_fTimeStep*fOneOverMass); //(alpha)*t=(T/m)*(delta t)
			//w=w0+(alpha)*t
			ML_Vec3Add(
				&pBody->m_v3AngVel, 
				&pBody->m_v3AngVel, 
				&v3Temp);
		}
		
		//We'll now calculate a new AABB to reflect the potential move.
		SetPostMoveAABB(pBody);
	}
		
	//Do collision detection:
	//AABBs will be update an objects will
	//adjust their velocities as necessary.
	CollisionCheck();
	
	//Now update everything, and do some additional calcuations:
	for(CLListStack::LSItem*pNode=m_AwakeBodies.m_pFirst; pNode; pNode=pNode->m_pNext)
	{	
		CLPhysBody* pBody=(CLPhysBody*)pNode;
		#ifdef FORCE_METHOD
		if(ML_Vec3Length(&pBody->m_v3ClsnForces)>0.0f)
		{
			pBody->m_v3Vel=s_v3Zero;
		}
		#endif
		//Save some values:
		pBody->m_fAngSpeed=ML_Vec3Length(&pBody->m_v3AngVel); //Angular speed.
		pBody->m_fSpeed=ML_Vec3Length(&pBody->m_v3Vel); //Linear speed.
		
		//Update the position:
		//The position is updated by simply adding the velocity to the position.
		ML_Vec3Scale(&v3Temp, &pBody->m_v3Vel, m_fTimeStep);
		ML_Vec3Add((ML_VEC3*)&pBody->m_matPos._41, (ML_VEC3*)&pBody->m_matPos._41, &v3Temp);
		
		//Update the orientation:
		
		//For torque we simply rotate about the torque axis,
		//according to the magnitude of the torque (speed).
		//Technically for a matrix rotation about an axis we
		//should normalize the vector, but the ML_MatRotationAxis
		//normalizes it for us, we should also insure that
		//the angle is > 0, but ML_MatRotationAxis also checks
		//that for us.
		
		//We want to rotate around the body's center of graity.
		//Translate to the center of gravity:
		ML_MatTranslation(&matTemp, pBody->m_v3Center.x, pBody->m_v3Center.y, pBody->m_v3Center.z);
		//Do the rotation:
		ML_MatRotationAxis(
			&matTemp2,
			&pBody->m_v3AngVel,
			pBody->m_fAngSpeed*m_fTimeStep);	
		//Catenate matricies:
		ML_MatMultiply(&matTemp, &matTemp2, &matTemp);
		//Translate from the center of gravity:
		ML_MatTranslation(&matTemp2, -pBody->m_v3Center.x, -pBody->m_v3Center.y, -pBody->m_v3Center.z);
		//Catenate:
		ML_MatMultiply(&matTemp, &matTemp2, &matTemp);
		//Apply transformation:
		ML_MatMultiply(&pBody->m_matPos, &matTemp, &pBody->m_matPos);
		

		UpdateToSrv(pBody, pBody->m_pEnt);
	}
}

/* CLPhysLegacy::Init() */
void CLPhysLegacy::Init(lg_dword nMaxBodies)
{
	m_nHullCount=0;
	m_pGeoPlaneList=LG_NULL;
	m_pGeoHulls=LG_NULL;
	
	m_UnusedBodies.Clear();
	m_AwakeBodies.Clear();
	m_AsleepBodies.Clear();
	
	Err_Printf("Initializing Legacy Physics Engine...");
	//Should probably set max bodies as a parameter to init.
	m_nMaxBodies=nMaxBodies;
	Err_Printf("Creating body list of %u bodies...", m_nMaxBodies);
	//Create the bodie list and put it in the unused stack.
	m_pBodyList=new CLPhysBody[m_nMaxBodies];
	Err_Printf(
		"%u bytes allocated (%u MB).", 
		m_nMaxBodies*sizeof(CLPhysBody), 
		m_nMaxBodies*sizeof(CLPhysBody)/1048576);
		
	//Initialize all the bodies into the unused list.
	m_UnusedBodies.Init(m_pBodyList, m_nMaxBodies, sizeof(*m_pBodyList));

	#if 0
	//Create the lists.
	m_pSleepingBodies=new CLList<CLPhysBody*>(m_nMaxBodies);
	m_pAwakeBodies=new CLList<CLPhysBody*>(m_nMaxBodies);
	
	//To estimate the memory usage we take sizeof(lg_void*)
	//or the size of a pointer and multiply it by 3 (because
	//in the lists there are prev, next, and object pointers).
	//We know the object is a pointer because in the template
	//we specified it as (CLPhysBody*) not (CLPhysBody).
	Err_Printf(
		"Awake and sleeping lists created using %u bytes (%u MB)",
		(sizeof(lg_void*)*3)*m_nMaxBodies*2,
		(sizeof(lg_void*)*3)*m_nMaxBodies*2/1048576);
	#endif
	//Set some default values...
	m_v3Grav.x=m_v3Grav.y=m_v3Grav.z=0.0f;
	m_fDrag=0.0f;//0.1f;
	m_fTimeStep=0;
}

/* Shutdown
	PRE: Init should have been called.
	POST: Memory allocated to the physics engine is released,
	 after this method is called any created bodies that might
	 still ahve references on the server should not be used.
*/
void CLPhysLegacy::Shutdown()
{
	Err_Printf("Shutting down Legacy Physics Engine...");
	//Delete the lists.
	#if 0
	delete m_pSleepingBodies;
	delete m_pAwakeBodies;
	delete m_pBodyStack;
	#endif
	m_AsleepBodies.Clear();
	m_AwakeBodies.Clear();
	m_UnusedBodies.Clear();
	LG_SafeDeleteArray(m_pBodyList);
	SetupWorld(LG_NULL);
}

/* AddBody
	PRE: pEnt should be an entity in the server world,
	 pInfo should be set with all information about the
	 body to be created.
	POST: A new body is added to the world (or if a new
	 body could not be added, null is returned.  Note that
	 the return value is a pointer that should be used
	 whenever updates by the server need to be applied
	 to the physics engine.
*/
lg_void* CLPhysLegacy::AddBody(lg_void* pEnt, lp_body_info* pInfo)
{
	CLPhysBody* pBodyNew = (CLPhysBody*)m_UnusedBodies.Pop();//m_pBodyStack->Pop();
	if(!pBodyNew)
	{
		Err_Printf("CLPhysLegacy::AddBody ERROR: Could not obtain a new body.");
		return LG_NULL;
	}
	pBodyNew->m_pEnt=pEnt;
	pBodyNew->m_fMass=pInfo->m_fMass;
	pBodyNew->m_matPos=pInfo->m_matPos;
	pBodyNew->m_v3Thrust=s_v3Zero;
	pBodyNew->m_v3Vel=s_v3Zero;
	pBodyNew->m_v3Torque=s_v3Zero;
	pBodyNew->m_v3AngVel=s_v3Zero;
	pBodyNew->m_nFlags=pInfo->m_nAIPhysFlags;
	pBodyNew->m_nColBody=0;
	pBodyNew->m_nNumRegions=0;
	pBodyNew->m_v3Center.x=pInfo->m_fMassCenter[0];
	pBodyNew->m_v3Center.y=pInfo->m_fMassCenter[1];
	pBodyNew->m_v3Center.z=pInfo->m_fMassCenter[2];
	pBodyNew->m_fAngSpeed=0.0f;
	pBodyNew->m_fSpeed=0.0f;
	#ifdef FORCE_METHOD
	pBodyNew->m_v3ClsnForces=s_v3Zero;
	#endif
	LG_SetFlag(pBodyNew->m_nFlags, CLPhysBody::PHYS_BODY_AWAKE);
	//pBodyNew->m_nFlags=CLPhysBody::PHYS_BODY_AWAKE;
	pBodyNew->m_aabbBase=pInfo->m_aabbBody;
	//We always start a body in the awake list.
	m_AwakeBodies.Push(pBodyNew);
	return pBodyNew;
}

void CLPhysLegacy::RemoveBody(lg_void* pBody)
{
	CLPhysBody* pBodyA=(CLPhysBody*)pBody;
	if(LG_CheckFlag(pBodyA->m_nFlags, CLPhysBody::PHYS_BODY_AWAKE))
	{
		m_AwakeBodies.Remove(pBodyA);
	}
	else
	{
		m_AsleepBodies.Remove(pBodyA);
	}
	
	m_UnusedBodies.Push(pBodyA);
}

void CLPhysLegacy::SetupWorld(lg_void* pWorldMap)
{
	//We always clear the old data:
	LG_SafeDeleteArray(m_pGeoPlaneList);
	LG_SafeDeleteArray(m_pGeoHulls);
	m_nHullCount=0;
	
	CLMap* pMap=(CLMap*)pWorldMap;
	
	if(!pMap || !pMap->IsLoaded())
		return;
		
	m_nHullCount=pMap->m_nGeoBlockCount;
	m_pGeoHulls=new CLPhysHull[m_nHullCount];
	m_pGeoPlaneList=new ml_plane[pMap->m_nGeoFaceCount];
	
	for(lg_dword i=0; i<m_nHullCount; i++)
	{
		m_pGeoHulls[i].m_nFirstFace=pMap->m_pGeoBlocks[i].nFirstFace;
		m_pGeoHulls[i].m_nNumFaces=pMap->m_pGeoBlocks[i].nFaceCount;
		m_pGeoHulls[i].m_aabbHull=pMap->m_pGeoBlocks[i].aabbBlock;
		
		m_pGeoHulls[i].m_pFaces=&m_pGeoPlaneList[m_pGeoHulls[i].m_nFirstFace];
	}
	
	for(lg_dword i=0; i<pMap->m_nGeoFaceCount; i++)
	{
		m_pGeoPlaneList[i]=pMap->m_pGeoFaces[i];
	}
}

void CLPhysLegacy::SetGravity(ML_VEC3* pGrav)
{
	m_v3Grav=*pGrav;
}

/* Simulate
	PRE: Everything needs to be initialized, fTimeStepSec >= 0.0f and
	 represents how many seconds have elapsed since the last update
	 (usually a small number such as 1/30 seconds (at 30 fps) etc.).
	POST: The physics engine simulates all physical interactions.
	 This method updates the entities on the server.  After it
	 finishes any changes in a body's velocity, torque, position, etc.
	 will already be updated on the server, so there is no need to
	 loop through all the ents on the server and update their
	 physical information.  Note that the server's entity's thrust
	 and torque are set to zero, this is because if an entity wants
	 to continue accelerating it needs to keep setting the forces
	 (that is in order to speed up forces need to be applied over time,
	 remember that forces imply acceleration, but if the object only
	 accelerates a little then it will not have a big velocity, and if
	 there is a significant amount of friction, then the veloctiy will
	 be reduced to zero very quickly).
	 
	 Future Notes:
	 Additionally if a body's veloctiy becomes extremely low, that
	 body will be put to sleep and will not be effected by calls
	 to UpdateBody.  If another bodies collides with a sleeping body
	 it will then become awake.  Note that AI controlled bodies should
	 be set to never fall asleep, only dumb objects.
*/
void CLPhysLegacy::Simulate(lg_float fTimeStepSec)
{
	/*
	if(fTimeStepSec>=1.0f)
	{
		m_fTimeStep=0.0f;
	}
	*/
	//We'll clamp the elapsed time to less than
	//1 second (if we're at 1 frame per second
	//we've go serious problems anyway).
	m_fTimeStep=LG_Clamp(fTimeStepSec, 0.001f, 0.25f);
	
	//m_fTimeStep=fTimeStepSec>=0.25f?0.0f:fTimeStepSec;
	ProcessAwakeBodies();
}

void CLPhysLegacy::SetBodyFlag(lg_void* pBody, lg_dword nFlagID, lg_dword nValue)
{

}

void CLPhysLegacy::SetBodyVector(lg_void* pBody, lg_dword nVecID, ML_VEC3* pVec)
{

}

void CLPhysLegacy::SetBodyFloat(lg_void* pBody, lg_dword nFloatID, lg_float fValue)
{

}

/* SetBodyPosition
	PRE: pBody must point to a body created by the phys engine.
	POST: The new position is set, and the server's entity's
	 m_v3Face and m_v3Pos is updated as well (so it does not 
	 need to be updated on the server side).
	 
	NOTES: This should rarely be called, it may need to be called
	 in the case of teleports of some kind, or in the case that
	 precise rotation needs to be specifed (as in the case of a
	 first person shooter, if we just had the mouse do a certain
	 amount of torque rotation might not be as smooth as directly
	 updating the rotation, but in that case the translation part
	 of the matrix should not be changed, only the rotation part).
*/
void CLPhysLegacy::SetBodyPosition(lg_void* pBody, ML_MAT* pMat)
{
	CLPhysBody* pPhysBody=(CLPhysBody*)pBody;
	LEntitySrv* pEntSrv=(LEntitySrv*)pPhysBody->m_pEnt;
	
	pPhysBody->m_matPos=*pMat;
	pEntSrv->m_matPos=*pMat;
	
	//Calculate the face vectors, we set up face vector's for
	//ai purposes.  These aren't really usefull for inert objects
	//but intelligent objects can use them to decide what direction
	//they want to move, etc.  In the future it may be decided to
	//update these vectors only when the object is flagged as
	//intelligent.
	ML_Vec3TransformNormalArray(
		pEntSrv->m_v3Face, sizeof(ML_VEC3), 
		s_v3StdFace, sizeof(ML_VEC3), 
		pMat, 3);
}
	
lg_void* CLPhysLegacy::GetBodySaveInfo(lg_void* pBody, lg_dword* pSize)
{
	return LG_NULL;
}

lg_void* CLPhysLegacy::LoadBodySaveInfo(lg_void* pData, lg_dword nSize)
{
	return LG_NULL;
}

void CLPhysLegacy::UpdateFromSrv(CLPhysBody* pPhysBody, lg_void* pEntSrv)
{
	static ml_vec3 v3Temp;
	LEntitySrv* pEnt=(LEntitySrv*)pEntSrv;
	//ML_Vec3Add(&pPhysBody->m_v3Thrust, &pPhysBody->m_v3Thrust, &pEnt->m_v3Thrust);
	pPhysBody->m_v3Thrust=pEnt->m_v3Thrust;
	pPhysBody->m_v3Torque=pEnt->m_v3Torque;
	ML_Vec3Scale(&v3Temp, &pEnt->m_v3Impulse, 1.0f/m_fTimeStep);
	ML_Vec3Add(&pPhysBody->m_v3Thrust, &pPhysBody->m_v3Thrust, &v3Temp);
	//Set torque and thrust to zero, so they don't carry over to the next frame.
	pEnt->m_v3Torque=s_v3Zero;
	pEnt->m_v3Thrust=s_v3Zero;
	pEnt->m_v3Impulse=s_v3Zero;
}
void CLPhysLegacy::UpdateToSrv(CLPhysBody* pPhysBody, lg_void* pEntSrv)
{
	LEntitySrv* pEnt=(LEntitySrv*)pEntSrv;
	//We need to update the position, velocities
	pEnt->m_v3Vel=pPhysBody->m_v3Vel;
	pEnt->m_v3AngVel=pPhysBody->m_v3AngVel;
	pEnt->m_matPos=pPhysBody->m_matPos;
	//Calculate the face vectors.
	ML_Vec3TransformNormalArray(
		pEnt->m_v3Face, sizeof(ML_VEC3), 
		s_v3StdFace, sizeof(ML_VEC3), 
		&pEnt->m_matPos, 3);
		
	//Set the AABB (we use the PostMove AABB because this represents where the object is now:
	pEnt->m_aabbBody=pPhysBody->m_aabbPostMove;
	
	//Call the post physics AI routine.
	pEnt->m_pAI->PostPhys(pEnt);
}
