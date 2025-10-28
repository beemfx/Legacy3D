#define ML_LIB_MAIN
#include "ML_lib.h"
#include "ML_vec3.h"
#include "ML_mat.h"

ml_dword FLOAT_SSE_NO_W_MASK[]   = {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000};
ml_dword FLOAT_3DNOW_NO_W_MASK[] = {0xFFFFFFFF, 0x00000000};

const ml_mat ML_matIdentity = {1.0f, 0.0f, 0.0f, 0.0f,
                               0.0f, 1.0f, 0.0f, 0.0f,
                               0.0f, 0.0f, 1.0f, 0.0f,
                               0.0f, 0.0f, 0.0f, 1.0f};
const ml_vec3 ML_v3Zero = {0.0f, 0.0f, 0.0f};


ml_bool ML_FUNC ML_Init(ML_INSTR nInstr)
{
	//If the instruction set is not supported, or if we
	//asked for the best support, we get the best support
	//available...
	if(nInstr==ML_INSTR_BEST || !ML_InstrSupport(nInstr))
		nInstr=ML_GetBestSupport();
		
	//We'll now check to make sure the support we got is supported:
	if(!ML_InstrSupport(nInstr))
		return ML_FALSE;
		
	//Our library only really uses FP, SSE, and 3DNOW.
	//If something like MMX was passed then the library
	//will default to FP.
	
	#define SET_FUNC(fn, fl, se1, se2, se3, now1, now2) { \
				if(nInstr==ML_INSTR_F){fn=fn##_##fl;} \
				else if(nInstr==ML_INSTR_SSE){fn=fn##_##se1;} \
				else if(nInstr==ML_INSTR_SSE2){fn=fn##_##se2;} \
				else if(nInstr==ML_INSTR_SSE3){fn=fn##_##se3;} \
				else if(nInstr==ML_INSTR_3DNOW){fn=fn##_##now1;} \
				else if(nInstr==ML_INSTR_3DNOW2){fn=fn##_##now2;} \
				else {fn=fn##_##fl;} \
				}
				
	//Fuctions declare with ML_DECLARE_FUNC must be initialzed
	//here, functions that only have one method do not need to
	//be initialized.
				
	SET_FUNC(ML_Vec3Add,             F, SSE, SSE, SSE, 3DNOW, 3DNOW);
	SET_FUNC(ML_Vec3Subtract,        F, SSE, SSE, SSE, 3DNOW, 3DNOW);
	SET_FUNC(ML_Vec3Cross,           F, SSE, SSE, SSE, 3DNOW, 3DNOW);
	SET_FUNC(ML_Vec3Dot,             F, SSE, SSE, SSE3, 3DNOW, 3DNOW);
	SET_FUNC(ML_Vec3Length,          F, SSE, SSE, SSE3, 3DNOW, 3DNOW);
	SET_FUNC(ML_Vec3LengthSq,        F, SSE, SSE, SSE3, 3DNOW, 3DNOW);
	SET_FUNC(ML_Vec3Normalize,       F, SSE, SSE, SSE3, 3DNOW, 3DNOW);
	SET_FUNC(ML_Vec3Scale,           F, SSE, SSE, SSE, 3DNOW, 3DNOW);
	SET_FUNC(ML_Vec3Distance,        F, SSE, SSE, SSE3, 3DNOW, 3DNOW);
	SET_FUNC(ML_Vec3DistanceSq,      F, SSE, SSE, SSE3, 3DNOW, 3DNOW);
	SET_FUNC(ML_Vec3Transform,       F, SSE, SSE, SSE, 3DNOW, 3DNOW);
	SET_FUNC(ML_Vec3TransformCoord,  F, SSE, SSE, SSE, 3DNOW, 3DNOW);
	SET_FUNC(ML_Vec3TransformNormal, F, SSE, SSE, SSE, 3DNOW, 3DNOW);
	
	SET_FUNC(ML_MatMultiply,         F, SSE, SSE, SSE, 3DNOW, 3DNOW);
	SET_FUNC(ML_MatInverse,          F, F, F, F, F, F);
	
	return ML_TRUE;
}

ml_bool  ML_FUNC ML_InstrSupport(ML_INSTR nInstr)
{
	ml_bool bSupport=ML_FALSE;
	int neax, nebx, necx, nedx;
	
	if(nInstr>=ML_INSTR_AMDMMX)
		neax=0x80000001;
	else
		neax=0x00000001;
		
	__asm
	{
		push ebx
		mov eax, neax;
		cpuid
		mov neax, eax
		mov nebx, ebx
		mov necx, ecx
		mov nedx, edx
		pop ebx
	}
	
	switch(nInstr)
	{
	case ML_INSTR_F:
		return (nedx>>0)&1;
	case ML_INSTR_MMX:
		return (nedx>>23)&1;
	case ML_INSTR_SSE:
		return (nedx>>25)&1;
	case ML_INSTR_SSE2:
		return (nedx>>26)&1;
	case ML_INSTR_SSE3:
		return (necx>>0)&1;
	case ML_INSTR_AMDMMX:
		return (nedx>>22)&0x1;
	case ML_INSTR_3DNOW:
		return (nedx>>30)&0x1;
	case ML_INSTR_3DNOW2:
		return (nedx>>31)&0x1;
	default:
		return 0;
	}
	
	return bSupport;
}

ML_INSTR ML_FUNC ML_GetBestSupport()
{
	#define INSTR_START ML_INSTR nInstr=ML_INSTR_BEST; if(0) {}
	#define INSTR_END else {nInstr=ML_INSTR_BEST;} return nInstr;
	#define INSTR_SUPPORT(name) else if(ML_InstrSupport(ML_INSTR_##name)){nInstr=ML_INSTR_##name;}
	
	//We put the support in order of
	//priority, we ignore support that isn't used at all
	//such as MMX and what not.
	INSTR_START
		INSTR_SUPPORT(SSE3)
		INSTR_SUPPORT(SSE2)
		INSTR_SUPPORT(SSE)
		INSTR_SUPPORT(3DNOW2)
		INSTR_SUPPORT(3DNOW)
		INSTR_SUPPORT(F)
	INSTR_END
}
