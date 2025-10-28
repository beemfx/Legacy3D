#include <stdio.h>
#include <d3dx9.h>
#include <ML_lib.h>
#include <math.h>

#define DO_ALT_TEST

void MatIdentTest(ML_MAT* pMM, ML_MAT* pMD);
void MatMulTest(ML_MAT* pT, ML_MAT* pM1, ML_MAT* pM2);
void MatRotXTest(ML_MAT* pT, float fA);
void MatRotYTest(ML_MAT* pT, float fA);
void MatRotZTest(ML_MAT* pT, float fA);
void MatFromQuatTest(ML_MAT* pM, ML_QUAT* pQ);
void MatInverseTest(ML_MAT* pMT, ML_MAT* pM);
void QuatFromMatTest(ML_QUAT* pQT, ML_MAT* pM);
void QuatSlerpTest(ML_QUAT* pQT, ML_QUAT* pQ1, ML_QUAT* pQ2, float t);
void MatDetTest(ML_MAT* pM);
void MatFovPerspectiveTest(ML_MAT* pMT);
void MatLookAtTest(ML_MAT* pM, ML_VEC3* pEye, ML_VEC3* pAt, ML_VEC3* pUp);
void MatYawPitchRollTest(ML_MAT* pM, float Yaw, float Pitch, float Roll);
void PrintMat(void*);
void PrintVec4(void*);

/*
ML_QUAT* ML_FUNC ML_QuatRotationMat2(ML_QUAT* pOut, ML_MAT* pM)
{
	float fTrace=pM->_11+pM->_22+pM->_33+1.0f;
	if(fTrace > 0.00000001f)
	{
		//MessageBox(0, "W", 0, 0);
		pOut->w=sqrtf(fTrace)*0.5f;
		fTrace=0.25f/pOut->w;
		pOut->x=(pM->_23-pM->_32)*fTrace;
		pOut->y=(pM->_31-pM->_13)*fTrace;
		pOut->z=(pM->_12-pM->_21)*fTrace;
		return pOut;
	}

	if( (pM->_11>pM->_22) && (pM->_11>pM->_33))
	{
		//MessageBox(0, "X", 0, 0);
		pOut->x=sqrtf(pM->_11-pM->_22-pM->_33+1.0f)*0.5f;
		fTrace=0.25f/pOut->x;
		pOut->w=(pM->_23-pM->_32)*fTrace;
		pOut->y=(pM->_12+pM->_21)*fTrace;
		pOut->z=(pM->_31+pM->_13)*fTrace;
		return pOut;
	}
	else if(pM->_22>pM->_33)
	{
		//MessageBox(0, "Y", 0, 0);
		pOut->y=sqrtf(pM->_22-pM->_11-pM->_33+1.0f)*0.5f;
		fTrace=0.25f/pOut->y;
		pOut->w=(pM->_31-pM->_13)*fTrace;
		pOut->x=(pM->_12+pM->_21)*fTrace;
		pOut->z=(pM->_23+pM->_32)*fTrace;
		return pOut;
	}
	else
	{
		//MessageBox(0, "Z", 0, 0);
		pOut->z=sqrtf(pM->_33-pM->_11-pM->_22+1.0f)*0.5f;
		fTrace=0.25f/pOut->z;
		pOut->w=(pM->_12-pM->_21)*fTrace;
		pOut->x=(pM->_31+pM->_13)*fTrace;
		pOut->y=(pM->_23+pM->_32)*fTrace;
		return pOut;
	}
}
*/



void MatTest()
{
	ML_MAT MT;
	
	ML_MAT M1={32.2f, 56.7f, -2.0f, 3.5f,
						5.6f, 7.0f, 8.0f, 90.0f,
						10.0f, -785.0f, 4.3f, 2.1f,
						3.0f, 4.0f, 5.0f, -7.0f};
						
	ML_MAT M2={500.23f, 6.0f, 7.0f, 20.2f,
						13.0f, -2.56f, -7.5f, 2.1f,
						5.6f, 7.0f, 1.0f, -2.0f,
						7.0f, -5.6f, 7.0f, 2.4f};
						
	ML_MAT M3={-5.0f, 6.0f, 7.0f, 20.2f,
				  13.0f, 1.0f, -7.5f, 2.1f,
				  5.6f, 7.0f, 2.0f, -2.0f,
				  7.0f, -5.6f, 7.0f, 2.4f};
				  
	ML_MAT M4={-5.0f, 6.0f, 7.0f, 0.2f,
				  13.0f, 1.0f, -7.5f, 0.0f,
				  5.6f, 7.0f, 2.0f, 0.0f,
				 0.0f, 0.0f, 0.0f, 1.0f};
						
	ML_QUAT Q1={345.21f, 78.65f, 21.20f, 34.56f};
	ML_QUAT Q2={23.23f, -2.4f, 34.4f, 0.56f};
	ML_QUAT QT={0.0f, 0.0f, 0.0f, 0.0f};
	
	ML_VEC3 v1={0.0f, 0.0f, -1.0f};
	ML_VEC3 v2={0.0f, 0.0f, 1.0f};
	ML_VEC3 v3={0.0f, 1.0f, 0.0f};
	
	ML_MatIdentity(&MT);					
	printf("MT:\n");PrintMat(&MT);
	printf("M1:\n");PrintMat(&M1);
	printf("M2:\n");PrintMat(&M2);
	printf("M3:\n");PrintMat(&M3);
	
	printf("Q1:\t");PrintVec4(&Q1);
	printf("Q2:\t");PrintVec4(&Q2);
	printf("\n\n");
	
						
	#if 1
	MatIdentTest(&MT, &MT);
	MatMulTest(&MT, &M1, &M2);
	MatRotXTest(&MT, 3.76f);
	MatRotYTest(&MT, 3.76f);
	MatRotZTest(&MT, 3.76f);
	MatFromQuatTest(&MT, &Q1);
	QuatFromMatTest(&QT, &M3);
	MatDetTest(&M4);
	#endif
	MatInverseTest(&MT, &M4);
	#if 1
	MatFovPerspectiveTest(&MT);
	
	QuatSlerpTest(&QT, &Q1, &Q2, 1.24f);
	MatLookAtTest(&MT, &v1, &v2, &v3);
	MatYawPitchRollTest(&MT, 3.012f, 1.24f, 4.566f);
	#endif
}

void MatYawPitchRollTest(ML_MAT* pM, float Yaw, float Pitch, float Roll)
{
	printf("YawPitchRoll Test:\n");
	printf("ML_lib:\n");
	PrintMat((void*)ML_MatRotationYawPitchRoll(pM, Yaw, Pitch, Roll));
	printf("D3DX:\n");
	PrintMat((void*)D3DXMatrixRotationYawPitchRoll(pM, Yaw, Pitch, Roll));
	printf("\n\n");
}

void MatLookAtTest(ML_MAT* pM, ML_VEC3* pEye, ML_VEC3* pAt, ML_VEC3* pUp)
{
	printf("MatLookAt Test:\n");
	printf("ML_lib:\n");
	PrintMat((void*)ML_MatLookAtLH(pM, pEye, pAt, pUp));
	printf("D3DX:\n");
	PrintMat((void*)D3DXMatrixLookAtLH(pM, pEye, pAt, pUp));
	printf("\n\n");
}

void MatFovPerspectiveTest(ML_MAT* pMT)
{
	printf("MatPerspectiveFovLH Test:\n");
	printf("ML_lib:\n");
	PrintMat((void*)ML_MatPerspectiveFovLH(pMT, 3.14159f*0.25f, 4.0f/3.0f, 1.0f, 1000.0f));
	printf("D3DX:\n");
	PrintMat((void*)D3DXMatrixPerspectiveFovLH((ML_MAT*)pMT, 3.14159f*0.25f, 4.0f/3.0f, 1.0f, 1000.0f));
	printf("\n\n");
}

void MatInverseTest(ML_MAT* pMT, ML_MAT* pM)
{
	printf("MatInverse Test:\n");
	printf("ML_lib:\n");
	PrintMat((void*)ML_MatInverse((void*)pMT, (void*)0, pM));
	printf("D3DX:\n");
	PrintMat((void*)D3DXMatrixInverse((void*)pMT, (void*)0, pM));
	printf("\n\n");
}

void MatDetTest(ML_MAT* pM)
{
	printf("MatDet Test:\n");
	//ML_MatRotationX(pM, 3.5f);
	printf("ML_lib: %f\n", ML_MatDeterminant(pM));
	printf("D3DX: %f\n", D3DXMatrixDeterminant(pM));
	printf("\n\n");
}

void QuatSlerpTest(ML_QUAT* pQT, ML_QUAT* pQ1, ML_QUAT* pQ2, float t)
{
	printf("QuatSlerp Test:\n");
	printf("ML_lib:\t");
	PrintVec4((void*)ML_QuatSlerp(pQT, pQ1, pQ2, t));//(pQT, pM));
	printf("D3DX:\t");
	PrintVec4((void*)D3DXQuaternionSlerp((void*)pQT, (void*)pQ1, (void*)pQ2, t));
	printf("\n\n");
}

void QuatFromMatTest(ML_QUAT* pQT, ML_MAT* pM)
{
	printf("QuatFromMat Test:\n");
	printf("ML_lib:\t");
	PrintVec4((void*)ML_QuatRotationMat(pQT, pM));
	printf("D3DX:\t");
	PrintVec4((void*)D3DXQuaternionRotationMatrix((void*)pQT, (void*)pM));
	printf("\n\n");
}

void MatFromQuatTest(ML_MAT* pM, ML_QUAT* pQ)
{
	printf("MatFromQuat Test:\n");
	printf("ML_lib:\n");
	PrintMat((void*)ML_MatRotationQuat(pM, pQ));
	printf("D3DX:\n");
	PrintMat((void*)D3DXMatrixRotationQuaternion((void*)pM, (void*)pQ));
	printf("\n\n");
}

void MatIdentTest(ML_MAT* pMM, ML_MAT* pMD)
{
	printf("MatIdentity Test:\n");
	printf("ML_lib:\n");
	PrintMat(ML_MatIdentity(pMM));
	printf("D3DX:\n");
	PrintMat(D3DXMatrixIdentity((void*)pMD));
	printf("\n\n");
}

void MatMulTest(ML_MAT* pMT, ML_MAT* pM1, ML_MAT* pM2)
{
	printf("MatMultiply Test:\n");
	printf("ML_lib:\n");
	PrintMat(ML_MatMultiply(pMT, pM1, pM2));
	#ifdef DO_ALT_TEST
	printf("PEQU1:\n");
	memcpy(pMT, pM1, sizeof(ML_MAT));
	PrintMat(ML_MatMultiply(pMT, pMT, pM2));
	printf("PEQU2:\n");
	memcpy(pMT, pM2, sizeof(ML_MAT));
	PrintMat(ML_MatMultiply(pMT, pM1, pMT));
	#endif
	printf("D3DX:\n");
	PrintMat((ML_MAT*)D3DXMatrixMultiply((void*)pMT, (void*)pM1, (void*)pM2));
	printf("\n\n");
}

void MatRotXTest(ML_MAT* pT, float fA)
{
	printf("MatRotateX Test:\n");
	printf("ML_lib:\n");
	PrintMat(ML_MatRotationX(pT, fA));
	printf("D3DX:\n");
	PrintMat((ML_MAT*)D3DXMatrixRotationX((void*)pT, fA));
	printf("\n\n");
}

void MatRotYTest(ML_MAT* pT, float fA)
{
	printf("MatRotateY Test:\n");
	printf("ML_lib:\n");
	PrintMat(ML_MatRotationY(pT, fA));
	printf("D3DX:\n");
	PrintMat((ML_MAT*)D3DXMatrixRotationY((void*)pT, fA));
	printf("\n\n");
}
void MatRotZTest(ML_MAT* pT, float fA)
{
	printf("MatRotateZ Test:\n");
	printf("ML_lib:\n");
	PrintMat(ML_MatRotationY(pT, fA));
	printf("D3DX:\n");
	PrintMat((ML_MAT*)D3DXMatrixRotationY((void*)pT, fA));
	printf("\n\n");
}