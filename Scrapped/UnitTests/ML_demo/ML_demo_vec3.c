#include <stdio.h>
#include <ML_lib.h>
#include <d3dx9.h>

/* Note: Both the SSE, SSE3, and F functions don't know how to normalize
	a 0.0f, 0.0f, 0.0f vector.*/
	
#define DO_ALT_TEST

void Vec3AddTest(ML_VEC3* pVT, ML_VEC3* pV1, ML_VEC3* pV2);
void Vec3CrossTest(ML_VEC3* pVT, ML_VEC3* pV1, ML_VEC3* pV2);
void Vec3DotTest(ML_VEC3* pV1, ML_VEC3* pV2);
void Vec3LengthTest(ML_VEC3* pV1);
void Vec3NormalizeTest(ML_VEC3* pVT, ML_VEC3* pV1);
void Vec3SubTest(ML_VEC3* pVT, ML_VEC3* pV1, ML_VEC3* pV2);
void Vec3ScaleTest(ML_VEC3* pVT, ML_VEC3* pV1, float fS);
void Vec3DistanceTest(ML_VEC3* pV1, ML_VEC3* pV2);
void Vec3TransformTest(ML_VEC4* pVQ, ML_VEC3* pV, ML_MAT* pMT);
void Vec3TransformCoordTest(ML_VEC3* pVT, ML_VEC3* pV, ML_MAT* pMT);
void Vec3TransformNormalTest(ML_VEC3* pVT, ML_VEC3* pV, ML_MAT* pMT);
void Vec3LengthSqTest(ML_VEC3* pV1);

void PrintVec3(void* pVec3);
void PrintVec4(void* pVec4);

int Vec3Test()
{
	ML_VEC4 VQ={0.0f, 0.0f, 0.0f};
	ML_VEC3 VT={0.0f, 0.0f, 0.0f};
	
	ML_VEC3 V1={12.34f, 45.67f, 89.1011f};
	ML_VEC3 V2={343.324f, 765.443f, 78.45f};
	
	ML_MAT M1={1.2f, 56.7f, 2.0f, 3.5f,
				  5.6f, 7.0f, 8.0f, 90.0f,
				  10.0f, 785.0f, 4.3f, 2.1f,
				  3.0f, 4.0f, 5.0f, 7.0f};
						
	float fS=12.3f;
	
	
	printf("Data:\n");
	printf("VT:\t");PrintVec3(&VT);
	printf("V1:\t");PrintVec3(&V1);
	printf("V2:\t");PrintVec3(&V2);
	printf("Scale:\t%f\n\n", fS);
	

	printf("Tests:\n\n");
	Vec3AddTest(&VT, &V1, &V2);
	Vec3CrossTest(&VT, &V1, &V2);
	Vec3DotTest(&V1, &V2);
	Vec3LengthTest(&V1);
	Vec3NormalizeTest(&VT, &V1);
	Vec3SubTest(&VT, &V1, &V2);
	Vec3ScaleTest(&VT, &V1, fS);
	Vec3DistanceTest(&V1, &V2);
	Vec3TransformTest(&VQ, &V1, &M1);
	Vec3TransformCoordTest(&VT, &V1, &M1);
	Vec3TransformNormalTest(&VT, &V1, &M1);
	Vec3LengthSqTest(&V1);
	
	printf("\n");
	return 0;
}

void Vec3TransformNormalTest(ML_VEC3* pVT, ML_VEC3* pV, ML_MAT* pMT)
{
	printf("Vec3TransformNormal:\n");
	printf("ML_lib:\t");
	PrintVec3(ML_Vec3TransformNormal(pVT, pV, pMT));
	#ifdef DO_ALT_TEST
	memcpy(pVT, pV, sizeof(ML_VEC3));
	printf("PEQU:\t");
	PrintVec3(ML_Vec3TransformNormal(pVT, pVT, pMT));
	#endif
	printf("D3DX:\t");
	PrintVec3((ML_VEC3*)D3DXVec3TransformNormal((void*)pVT, (void*)pV, (void*)pMT));
	printf("\n\n");
}

void Vec3TransformCoordTest(ML_VEC3* pVT, ML_VEC3* pV, ML_MAT* pMT)
{
	printf("Vec3TransformCoord:\n");
	printf("ML_lib:\t");
	PrintVec3(ML_Vec3TransformCoord(pVT, pV, pMT));
	#ifdef DO_ALT_TEST
	printf("PEQU:\t");
	memcpy(pVT, pV, sizeof(ML_VEC3));
	PrintVec3(ML_Vec3TransformCoord(pVT, pVT, pMT));
	#endif
	printf("D3DX:\t");
	PrintVec3((ML_VEC3*)D3DXVec3TransformCoord((void*)pVT, (void*)pV, (void*)pMT));
	printf("\n\n");
}

void Vec3TransformTest(ML_VEC4* pVQ, ML_VEC3* pV, ML_MAT* pMT)
{
	ML_VEC4 V4;
	memcpy(&V4, pV, 12);
	printf("Vec3Transform:\n");
	printf("ML_lib:\t");
	PrintVec4(ML_Vec3Transform(pVQ, pV, pMT));
	#ifdef DO_ALT_TEST
	printf("PEQU:\t");
	memcpy(&V4, pV, sizeof(ML_VEC3));
	PrintVec4(ML_Vec3Transform(&V4, (void*)&V4, pMT));
	#endif
	printf("D3DX:\t");
	PrintVec4((ML_VEC4*)D3DXVec3Transform((void*)pVQ, (void*)pV, (void*)pMT));
	printf("\n\n");
}

void Vec3AddTest(ML_VEC3* pVT, ML_VEC3* pV1, ML_VEC3* pV2)
{
	printf("Vec3Add:\n");
	printf("ML_lib:\t");
	memcpy(pVT, pV1, sizeof(ML_VEC3));
	PrintVec3(ML_Vec3Add(pVT, pV1, pV2));
	#ifdef DO_ALT_TEST
	printf("PEQU1:\t");
	memcpy(pVT, pV1, sizeof(ML_VEC3));
	PrintVec3(ML_Vec3Add(pVT, pVT, pV2));
	printf("PEQU2:\t");
	memcpy(pVT, pV2, sizeof(ML_VEC3));
	PrintVec3(ML_Vec3Add(pVT, pV1, pVT));
	#endif
	printf("D3DX:\t");
	PrintVec3((ML_VEC3*)D3DXVec3Add((void*)pVT, (void*)pV1, (void*)pV2));
	printf("\n\n");
}

void Vec3ScaleTest(ML_VEC3* pVT, ML_VEC3* pV1, float fS)
{
	printf("Vec3Scale:\n");
	printf("ML_lib:\t");
	PrintVec3(ML_Vec3Scale(pVT, pV1, fS));
	#ifdef DO_ALT_TEST
	printf("PEQU:\t");
	memcpy(pVT, pV1, sizeof(ML_VEC3));
	PrintVec3(ML_Vec3Scale(pVT, pVT, fS));
	#endif
	printf("D3DX:\t");
	PrintVec3((ML_VEC3*)D3DXVec3Scale((void*)pVT, (void*)pV1, fS));
	printf("\n\n");
}

void Vec3SubTest(ML_VEC3* pVT, ML_VEC3* pV1, ML_VEC3* pV2)
{
	printf("Vec3Sub:\n");
	printf("ML_lib:\t");
	PrintVec3(ML_Vec3Subtract(pVT, pV1, pV2));
	#ifdef DO_ALT_TEST
	printf("PEQU1:\t");
	memcpy(pVT, pV1, sizeof(ML_VEC3));
	PrintVec3(ML_Vec3Subtract(pVT, pVT, pV2));
	printf("PEQU2:\t");
	memcpy(pVT, pV2, sizeof(ML_VEC3));
	PrintVec3(ML_Vec3Subtract(pVT, pV1, pVT));
	#endif
	printf("D3DX:\t");
	PrintVec3((ML_VEC3*)D3DXVec3Subtract((void*)pVT, (void*)pV1, (void*)pV2));
	printf("\n\n");
}


void Vec3CrossTest(ML_VEC3* pVT, ML_VEC3* pV1, ML_VEC3* pV2)
{
	printf("Vec3Cross:\n");
	printf("ML_lib:\t");
	PrintVec3(ML_Vec3Cross(pVT, pV1, pV2));
	#ifdef DO_ALT_TEST
	printf("PEQU1:\t");
	memcpy(pVT, pV1, sizeof(ML_VEC3));
	PrintVec3(ML_Vec3Cross(pVT, pVT, pV2));
	printf("PEQU2:\t");
	memcpy(pVT, pV2, sizeof(ML_VEC3));
	PrintVec3(ML_Vec3Cross(pVT, pV1, pVT));
	#endif
	printf("D3DX:\t");
	PrintVec3((ML_VEC3*)D3DXVec3Cross((void*)pVT, (void*)pV1, (void*)pV2));
	printf("\n\n");
}


void Vec3DotTest(ML_VEC3* pV1, ML_VEC3* pV2)
{
	printf("Vec3Dot:\n");
	printf("ML_lib:\t");
	printf("%f\n", ML_Vec3Dot(pV1, pV2));
	printf("D3DX:\t");
	printf("%f\n", D3DXVec3Dot((void*)pV1, (void*)pV2));
	printf("\n\n");
}

void Vec3LengthTest(ML_VEC3* pV1)
{
	printf("Vec3Length:\n");
	printf("ML_lib:\t");
	printf("%f\n", ML_Vec3Length(pV1));
	printf("D3DX:\t");
	printf("%f\n", D3DXVec3Length((void*)pV1));
	printf("\n\n");
}

void Vec3LengthSqTest(ML_VEC3* pV1)
{
	printf("Vec3LengthSq:\n");
	printf("ML_lib:\t");
	printf("%f\n", ML_Vec3LengthSq(pV1));
	printf("D3DX:\t");
	printf("%f\n", D3DXVec3LengthSq((void*)pV1));
	printf("\n\n");
}

void Vec3DistanceTest(ML_VEC3* pV1, ML_VEC3* pV2)
{
	printf("Vec3Distance:\n");
	printf("ML_lib:\t");
	printf("%f\n", ML_Vec3Distance(pV1, pV2));
	printf("D3DX:\t");
	printf("%f\n", D3DXVec3Length(D3DXVec3Subtract((void*)pV1, (void*)pV1, (void*)pV2)));
	printf("\n\n");
}

void Vec3NormalizeTest(ML_VEC3* pVT, ML_VEC3* pV1)
{
	printf("Vec3Normalize:\n");
	printf("ML_lib:\t");
	PrintVec3(ML_Vec3Normalize(pVT, pV1));
	#ifdef DO_ALT_TEST
	printf("PEQU:\t");
	memcpy(pVT, pV1, sizeof(ML_VEC3));
	PrintVec3(ML_Vec3Normalize(pVT, pVT));
	#endif
	printf("D3DX:\t");
	PrintVec3((ML_VEC3*)D3DXVec3Normalize((void*)pVT, (void*)pV1));
	printf("\n\n");
}
