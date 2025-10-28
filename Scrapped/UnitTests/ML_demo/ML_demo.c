#include <math.h>
#include <stdio.h>
#include <conio.h>
#include <d3d9.h>
#include <d3dx9.h>

#pragma warning(disable: 4267)
#pragma warning(disable: 4996)

#include "ML_lib.h"

#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "winmm.lib")

void PrintMat(ML_MAT* pM1);
void PrintVec3(ML_VEC3* pV1);
void PrintVec4(ML_VEC4* pV1);
void CRT_Test();

void Vec3Test();

int main(void)
{
	//SSEVersion();
	//DoSomething();
	int i=0;
	
	#define F_STACK_TEST
	#ifdef F_STACK_TEST
	float fValue=2.3f;
	__asm fld fValue
	#endif F_STACK_TEST
	
	//ML_SetSIMDSupport(ML_INSTR_F);
	ML_Init(ML_INSTR_F);
	//ML_SetFuncs(ML_INSTR_SSE3);
	//ML_SetFuncs(ML_INSTR_3DNOW);
	
	printf("Conducting tests...\n");
	
	//#define TEXT_OUTPUT
	#ifdef TEXT_OUTPUT
	freopen("out.txt", "w", stdout);
	#endif TEST_OUTPUT
	
	Vec3Test();
	MatTest();
	CRT_Test();
	#ifdef F_STACK_TEST
	__asm fstp fValue
	printf("FVALUE=%f\n", fValue);
	#endif F_STACK_TEST
	
	#ifdef TEXT_OUTPUT
	freopen( "CON", "w", stdout );
	#endif TEXT_OUTPUT

	//MatTest();
	printf("Tests complete.\nPress any key to exit.\n");
	getch();
	return 0;
}

void CRT_Test()
{
	float f=2.5f;
	ml_float fSin, fCos;
	printf("ML_lib:\tcosf(%f)=%f\n", f, ML_cosf(f));
	printf("CRT:\tcosf(%f)=%f\n", f, cosf(f));
	printf("\n\n");
	printf("ML_lib:\tsinf(%f)=%f\n", f, ML_sinf(f));
	printf("CRT:\tsinf(%f)=%f\n", f, sinf(f));
	printf("\n\n");
	printf("ML_lib:\ttanf(%f)=%f\n", f, ML_tanf(f));
	printf("CRT:\ttanf(%f)=%f\n", f, tanf(f));
	printf("\n\n");
	f=0.5f;
	printf("ML_lib:\tacosf(%f)=%f\n", f, ML_acosf(f));
	printf("CRT:\tacosf(%f)=%f\n", f, acosf(f));
	printf("\n\n");
	printf("ML_lib:\tasinf(%f)=%f\n", f, ML_asinf(f));
	printf("CRT:\tasinf(%f)=%f\n", f, asinf(f));
	printf("\n\n");
	//3.012f, 1.24f, 4.566f
	f=4.566f;
	fSin=ML_sincosf(f, &fCos);
	printf("ML_lib:\tsincosf(%f)=%f, %f\n", f, fSin, fCos);
	fSin=sinf(f);
	fCos=cosf(f);
	printf("CRT:\tsincosf(%f)=%f, %f\n", f, fSin, fCos);
	f=1.24f;
	fSin=ML_sincosf(f, &fCos);
	printf("ML_lib:\tsincosf(%f)=%f, %f\n", f, fSin, fCos);
	fSin=sinf(f);
	fCos=cosf(f);
	printf("CRT:\tsincosf(%f)=%f, %f\n", f, fSin, fCos);
	f=3.012f;
	fSin=ML_sincosf(f, &fCos);
	printf("ML_lib:\tsincosf(%f)=%f, %f\n", f, fSin, fCos);
	fSin=sinf(f);
	fCos=cosf(f);
	printf("CRT:\tsincosf(%f)=%f, %f\n", f, fSin, fCos);
	printf("\n\n");
	
}

void PrintMat(ML_MAT* pM1)
{
	printf("%f\t%f\t%f\t%f\n", pM1->_11, pM1->_12, pM1->_13, pM1->_14);
	printf("%f\t%f\t%f\t%f\n", pM1->_21, pM1->_22, pM1->_23, pM1->_24);
	printf("%f\t%f\t%f\t%f\n", pM1->_31, pM1->_32, pM1->_33, pM1->_34);
	printf("%f\t%f\t%f\t%f", pM1->_41, pM1->_42, pM1->_43, pM1->_44);
	printf("\n\n");
}

void PrintVec3(ML_VEC3* pV1)
{
	printf("%f\t%f\t%f\t", pV1->x, pV1->y, pV1->z);
	printf("\n");
}

void PrintVec4(ML_VEC4* pV1)
{
	printf("%f\t%f\t%f\t%f", pV1->x, pV1->y, pV1->z, pV1->w);
	printf("\n");
}