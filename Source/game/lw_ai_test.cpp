#include "lw_ent_sdk.h"
#include "lg_func.h"
#include "lw_ai_test.h"

#include <memory.h>

extern GAME_AI_GLOBALS g_Globals;

/***************************
*** The Test Jack AI class ***
***************************/

lg_dword CLWAIJack::PhysFlags()
{
	return AI_PHYS_INT|AI_PHYS_DIRECT_ANGULAR;
}

void CLWAIJack::Init(LEntitySrv *pEnt)
{
	AIJACK_DATA* pData=(AIJACK_DATA*)pEnt->m_pData;
	pEnt->m_fLook[1]=0.0f;
	LWAI_SetAnim(pEnt->m_nAnimFlags1, 0, 0, 0, 1.0f, 0.0f);
	pData->nMode=0;
}

void CLWAIJack::PrePhys(LEntitySrv *pEnt)
{
	AIJACK_DATA* pData=(AIJACK_DATA*)pEnt->m_pData;
	lg_float fThrust=pEnt->m_fMass*3.0f;
	
	//AI_DATA* pAIData=(AI_DATA*)pAIDataArg;
	//Technically a server ai should not take input, but
	//this is strictly for development testing.
	ML_VEC3 v3Temp;
	
	if(LWAI_CmdActive(pEnt, 0))
	{
		//The general idea for movement is to create a vector 
		//in the direction of movement, note that the vector
		//is a force so to get it to be acceleration we need
		//to multiply it by the object's mass, that way 2.0f
		//will be 2 m/s^2.
		ML_Vec3Scale(&v3Temp, &pEnt->m_v3Face[0], fThrust);
		ML_Vec3Add(&pEnt->m_v3Thrust, &pEnt->m_v3Thrust, &v3Temp);
	}
	if(LWAI_CmdActive(pEnt, 1))
	{
		ML_Vec3Scale(&v3Temp, &pEnt->m_v3Face[0], -fThrust);
		ML_Vec3Add(&pEnt->m_v3Thrust, &pEnt->m_v3Thrust, &v3Temp);
	}
	if(LWAI_CmdActive(pEnt, 2))
	{
		ML_Vec3Scale(&v3Temp, &pEnt->m_v3Face[2], fThrust);
		ML_Vec3Add(&pEnt->m_v3Thrust, &pEnt->m_v3Thrust, &v3Temp);
	}
	if(LWAI_CmdActive(pEnt, 3))
	{
		ML_Vec3Scale(&v3Temp, &pEnt->m_v3Face[2], -fThrust);
		ML_Vec3Add(&pEnt->m_v3Thrust, &pEnt->m_v3Thrust, &v3Temp);
	}
	#if 0
	if(cInput.s_pCmds[COMMAND_MOVEUP].IsActive())
	{
		ML_Vec3Scale(&v3Temp, &pEnt->m_v3Face[1], fThrust);
		ML_Vec3Add(&pEnt->m_v3Thrust, &pEnt->m_v3Thrust, &v3Temp);
	}
	#else
	if(LWAI_CmdPressed(pEnt, 6))
	{
		ML_Vec3Scale(&v3Temp, &pEnt->m_v3Face[1], 5.0f*pEnt->m_fMass);
		ML_Vec3Add(&pEnt->m_v3Impulse, &pEnt->m_v3Impulse, &v3Temp);
	}
	#endif
	if(LWAI_CmdActive(pEnt, 7))
	{
		ML_Vec3Scale(&v3Temp, &pEnt->m_v3Face[1], -fThrust);
		ML_Vec3Add(&pEnt->m_v3Thrust, &pEnt->m_v3Thrust, &v3Temp);
	}
	//This AI is setup so that it will rotate directly,
	//that means that the object will rotate by the amount
	//specified as the torque, for that reason the number should
	//be either attached to the mouse, or very small (and time adjusted) so 
	//the transition apears smooth.
	lg_float fRot = pEnt->m_fAxis[0];
	pEnt->m_v3Torque.y=fRot;
	
	pEnt->m_fLook[1]-=pEnt->m_fAxis[1];
	pEnt->m_fLook[1]=LG_Clamp(pEnt->m_fLook[1], -ML_HALFPI, ML_HALFPI);

	
	ML_VEC3 v3Look[3]={
		0.0f, 0.0f, 1.0f,
		0.0f, 1.0f, 0.0f,
		1.0f, 0.0f, 0.0f};
		
	ML_MAT matTemp;
	ML_MatRotationX(&matTemp, pEnt->m_fLook[1]);
	ML_Vec3TransformNormalArray(
		v3Look, sizeof(ML_VEC3),
		v3Look, sizeof(ML_VEC3),
		&matTemp,
		3);
	//memcpy(pEnt->m_v3Look, pEnt->m_v3Face, sizeof(ML_VEC3)*3);
	ML_Vec3TransformNormalArray(
		pEnt->m_v3Look, sizeof(ML_VEC3),
		v3Look, sizeof(ML_VEC3),
		&pEnt->m_matPos,
		3);
		
	//Setup the animation
	if(ML_Vec3LengthSq(&pEnt->m_v3Thrust)>0.0f)
	{
		if(pData->nMode!=1)
		{
			LWAI_SetAnim(pEnt->m_nAnimFlags1, 0, 1, 0, 1.0f, 0.3f);
			pData->nMode=1;
		}
	}
	else
	{
		if(pData->nMode!=0)
		{
			LWAI_SetAnim(pEnt->m_nAnimFlags1, 0, 0, 0, 2.0f, 0.3f);
			pData->nMode=0;
		}
	}
}

void CLWAIJack::PostPhys(LEntitySrv *pEnt)
{

}

/*******************************
*** The Test Blaine AI class ***
*******************************/

void CLWAIBlaine::Init(LEntitySrv *pEnt)
{
	AIBLAINE_DATA* pData=(AIBLAINE_DATA*)pEnt->m_pData;
	pData->fElapsedTime=0.0f;
	pData->bForward=LG_TRUE;
	pData->fCurThrust=0.0f;
	pData->fRotSpeed=0.0f;
	pData->nTicks=0;
}

lg_dword CLWAIBlaine::PhysFlags()
{
	return AI_PHYS_INT|AI_PHYS_DIRECT_ANGULAR;
}

void CLWAIBlaine::PrePhys(LEntitySrv *pEnt)
{
	AIBLAINE_DATA* pData=(AIBLAINE_DATA*)pEnt->m_pData;
	
	
	pData->fElapsedTime+=g_Globals.fTimeStep;
	//Err_MsgPrintf("%f", pData->fElapsedTime);
	LG_UnsetFlag(pEnt->m_nAnimFlags1, LWAI_ANIM_CHANGE);
	if(pData->fElapsedTime>5.0f)
	{
		pData->fCurThrust=pEnt->m_fMass*3.0f;;
		pData->fRotSpeed=LG_RandomFloat(-1.0f, 1.0f);
		pData->fElapsedTime=0.0f;
		LWAI_SetAnim(pEnt->m_nAnimFlags1, 0, 1, 0, 1.0f, 0.5f);
		pData->nTicks++;
		
		//Esentially cuase the entity to kill itself after
		//15 seconds of existence.
		if(pData->nTicks>2)
		{
			LWAI_QueueFunc(pEnt, AI_FUNC_DELETE);
		}
	}
	
	ml_vec3 v3Temp={0.0f, 0.0f, 0.0f};
	
	ML_Vec3Scale(&v3Temp, &pEnt->m_v3Face[0], pData->fCurThrust);
	ML_Vec3Add(&pEnt->m_v3Thrust, &pEnt->m_v3Thrust, &v3Temp);
	
	pEnt->m_v3Torque.y=pData->fRotSpeed*g_Globals.fTimeStep;
}

void CLWAIBlaine::PostPhys(LEntitySrv *pEnt)
{
	
}