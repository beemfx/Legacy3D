#ifndef ML_LIB_STATIC
#include <windows.h>
#endif ML_LIB_STATIC

#include "ML_lib.h"
#include "ML_vec3.h"
#include "ML_mat.h"

#define ML_DECL_FUNC(name, b) (__cdecl * name )(b)

extern void* ML_Vec3AddCB;
extern void* ML_Vec3CrossCB;
extern void* ML_Vec3DotCB;
extern void* ML_Vec3LengthCB;
extern void* ML_Vec3LengthSqCB;
extern void* ML_Vec3NormalizeCB;
extern void* ML_Vec3ScaleCB;
extern void* ML_Vec3DistanceCB;
extern void* ML_Vec3SubtractCB;
extern void* ML_Vec3TransformCB;
extern void* ML_Vec3TransformCoordCB;
extern void* ML_Vec3TransformNormalCB;

extern void* ML_MatMultiplyCB;
extern void* ML_MatInverseCB;

ml_bool ML_TestSupport(ML_INSTR nInstr);

#ifndef ML_LIB_STATIC
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
	/*
	printf("Floating Point: %i\n", ML_TestSupport(ML_INSTR_F));
	printf("MMX: %i\n", ML_TestSupport(ML_INSTR_MMX));
	printf("SSE: %i\n", ML_TestSupport(ML_INSTR_SSE));
	printf("SSE2: %i\n", ML_TestSupport(ML_INSTR_SSE2));
	printf("SSE3: %i\n", ML_TestSupport(ML_INSTR_SSE3));
	printf("AMD MMX: %i\n", ML_TestSupport(ML_INSTR_AMDMMX));
	printf("3DNOW!: %i\n", ML_TestSupport(ML_INSTR_3DNOW));
	printf("3DNOW!2: %i\n", ML_TestSupport(ML_INSTR_3DNOW2));
	*/
	
	//#define SET_ML_INSTR(name) if(ML_TestSupport(name)){ML_SetFuncs(name); OutputDebugString("Using "#name" instructions.\n");}
	
	//Always set instructions to FP first that way if a function isn't overriddent it won't be null.
	ML_SetSIMDSupport(ML_INSTR_F);
	
	if(fdwReason>=1 && fdwReason <=2)
	{
		ML_SetBestSIMDSupport();
		/* Choose which instruction set we want.*/
		/*
		SET_ML_INSTR(ML_INSTR_SSE3)
		else SET_ML_INSTR(ML_INSTR_SSE2)
		else SET_ML_INSTR(ML_INSTR_3DNOW2)
		else SET_ML_INSTR(ML_INSTR_3DNOW)
		else SET_ML_INSTR(ML_INSTR_SSE)
		else SET_ML_INSTR(ML_INSTR_F)
		*/
	}
		
	return TRUE;
}
#endif ML_LIB_STATIC

ML_INSTR ML_FUNC ML_SetBestSIMDSupport()
{
	#ifdef ML_LIB_STATIC
	#define SET_ML_INSTR(name) if(ML_TestSupport(name)){ML_SetSIMDSupport(name); return name;}
	#else ML_LIB_STATIC
	#define SET_ML_INSTR(name) if(ML_TestSupport(name)){ML_SetSIMDSupport(name); OutputDebugString("Using "#name" instructions.\n"); return name;}
	#endif ML_LIB_STATIC
	//Always set instructions to FP first that way if a function isn't overriddent it won't be null.
	ML_SetSIMDSupport(ML_INSTR_F);
	/* Choose which instruction set we want.*/
	SET_ML_INSTR(ML_INSTR_SSE3)
	else SET_ML_INSTR(ML_INSTR_SSE2)
	else SET_ML_INSTR(ML_INSTR_3DNOW2)
	else SET_ML_INSTR(ML_INSTR_3DNOW)
	else SET_ML_INSTR(ML_INSTR_SSE)
	else SET_ML_INSTR(ML_INSTR_F)
	
	return ML_INSTR_F;
}

ml_bool ML_TestSupport(ML_INSTR nInstr)
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

ml_bool ML_SetSIMDSupport(ML_INSTR nInstr)
{
	ML_MatInverseCB=ML_MatInverse_F;
	
	switch(nInstr)
	{
	default:
	case ML_INSTR_F:
		//Vec3
		ML_Vec3AddCB=(void*)ML_Vec3Add_F;
		ML_Vec3CrossCB=(void*)ML_Vec3Cross_F;
		ML_Vec3DotCB=ML_Vec3Dot_F;
		ML_Vec3LengthCB=ML_Vec3Length_F;
		ML_Vec3LengthSqCB=ML_Vec3LengthSq_F;
		ML_Vec3NormalizeCB=ML_Vec3Normalize_F;
		ML_Vec3ScaleCB=ML_Vec3Scale_F;
		ML_Vec3DistanceCB=ML_Vec3Distance_F;
		ML_Vec3SubtractCB=ML_Vec3Subtract_F;
		ML_Vec3TransformCB=ML_Vec3Transform_F;
		ML_Vec3TransformCoordCB=ML_Vec3TransformCoord_F;
		ML_Vec3TransformNormalCB=ML_Vec3TransformNormal_F;
		
		//Mat
		ML_MatMultiplyCB=ML_MatMultiply_F;
		break;
	case ML_INSTR_SSE3:
		//Vec3
		ML_Vec3AddCB=ML_Vec3Add_SSE;
		ML_Vec3CrossCB=ML_Vec3Cross_SSE;
		ML_Vec3DotCB=ML_Vec3Dot_SSE3;
		ML_Vec3LengthCB=ML_Vec3Length_SSE3;
		ML_Vec3LengthSqCB=ML_Vec3LengthSq_SSE3;
		ML_Vec3NormalizeCB=ML_Vec3Normalize_SSE3;
		ML_Vec3ScaleCB=ML_Vec3Scale_SSE;
		ML_Vec3DistanceCB=ML_Vec3Distance_SSE3;
		ML_Vec3SubtractCB=ML_Vec3Subtract_SSE;
		ML_Vec3TransformCB=ML_Vec3Transform_SSE;
		ML_Vec3TransformCoordCB=ML_Vec3TransformCoord_SSE;
		ML_Vec3TransformNormalCB=ML_Vec3TransformNormal_SSE;
		
		//Mat
		ML_MatMultiplyCB=ML_MatMultiply_SSE;
		break;
	case ML_INSTR_SSE2:
	case ML_INSTR_SSE:
		//Vec3
		ML_Vec3AddCB=ML_Vec3Add_SSE;
		ML_Vec3CrossCB=ML_Vec3Cross_SSE;
		ML_Vec3DotCB=ML_Vec3Dot_SSE;
		ML_Vec3LengthCB=ML_Vec3Length_SSE;
		ML_Vec3LengthSqCB=ML_Vec3LengthSq_SSE;
		ML_Vec3NormalizeCB=ML_Vec3Normalize_SSE;
		ML_Vec3ScaleCB=ML_Vec3Scale_SSE;
		ML_Vec3DistanceCB=ML_Vec3Distance_SSE;
		ML_Vec3SubtractCB=ML_Vec3Subtract_SSE;
		ML_Vec3TransformCB=ML_Vec3Transform_SSE;
		ML_Vec3TransformCoordCB=ML_Vec3TransformCoord_SSE;
		ML_Vec3TransformNormalCB=ML_Vec3TransformNormal_SSE;
		
		//Mat
		ML_MatMultiplyCB=ML_MatMultiply_SSE;
		break;
	case ML_INSTR_3DNOW:
		//Vec3
		ML_Vec3AddCB=ML_Vec3Add_3DNOW;
		ML_Vec3CrossCB=ML_Vec3Cross_3DNOW;
		ML_Vec3DotCB=ML_Vec3Dot_3DNOW;
		ML_Vec3LengthCB=ML_Vec3Length_3DNOW;
		ML_Vec3LengthSqCB=ML_Vec3LengthSq_3DNOW;
		ML_Vec3SubtractCB=ML_Vec3Subtract_3DNOW;
		ML_Vec3NormalizeCB=ML_Vec3Normalize_3DNOW;
		ML_Vec3ScaleCB=ML_Vec3Scale_3DNOW;
		ML_Vec3DistanceCB=ML_Vec3Distance_3DNOW;
		ML_Vec3TransformCB=ML_Vec3Transform_3DNOW;
		ML_Vec3TransformCoordCB=ML_Vec3TransformCoord_3DNOW;
		ML_Vec3TransformNormalCB=ML_Vec3TransformNormal_3DNOW;
		
		//Mat.
		ML_MatMultiplyCB=ML_MatMultiply_3DNOW;
		/*
		//TODO: test all 3DNOW functions and convert the
		//following:
		
		//Mat
		break;
		*/
	};
	
	return ML_TRUE;
}
