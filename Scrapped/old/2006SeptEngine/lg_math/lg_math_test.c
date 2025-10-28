#include <stdio.h>
#include <conio.h>
#include <d3d9.h>
#include <d3dx9.h>

#include "lg_math.h"

#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "winmm.lib")



void printmatrix(L_matrix* pM1)
{
	printf("%f\t%f\t%f\t%f\n", pM1->_11, pM1->_12, pM1->_13, pM1->_14);
	printf("%f\t%f\t%f\t%f\n", pM1->_21, pM1->_22, pM1->_23, pM1->_24);
	printf("%f\t%f\t%f\t%f\n", pM1->_31, pM1->_32, pM1->_33, pM1->_34);
	printf("%f\t%f\t%f\t%f", pM1->_41, pM1->_42, pM1->_43, pM1->_44);
	printf("\n\n");
}

void printvec3(L_vector3* pV1)
{
	printf("%f\t%f\t%f\t", pV1->x, pV1->y, pV1->z);
	printf("\n");
}

void testvec3(int test)
{
	L_vector3 V1={11.3f, 438.71f, 2.345f};
	L_vector3 V2={325.34f, 26.4f, 64.87f};

	//printf("%f\n", L_vec3normalize(&V1, &V1));
	//printf("%f\n", sqrt(V1.x*V1.x+V1.y*V1.y+V1.z*V1.z));

	if(test==1)
	{
		printf("D3DX:\n");
		printf("Length: %f\n", D3DXVec3Length((void*)&V1));
		printf("Normal: ");
		printvec3((void*)D3DXVec3Normalize((void*)&V1, (void*)&V1));
		printf("Scale: ");
		printvec3((void*)D3DXVec3Scale((void*)&V1, (void*)&V1, 89.25f));
		printf("Scale again: ");
		printvec3((void*)D3DXVec3Scale((void*)&V1, (void*)&V1, 2.0f));
	}
	else if(test==2)
	{
		printf("L_math:\n");
		printf("Length: %f\n", L_vec3length(&V1));
		printf("Normal: ");
		printvec3(L_vec3normalize(&V1, &V1));
		printf("Scale: ");
		printvec3((void*)L_vec3scale((void*)&V1, (void*)&V1, 89.25f));
		printf("Scale again: ");
		printvec3((void*)L_vec3scale((void*)&V1, (void*)&V1, 2.0f));
	}
}

void test(int d3d)
{
	L_matrix Mat1={1.2f, 56.7f, 2.0f, 3.5f,
						5.6f, 7.0f, 8.0f, 90.0f,
						10.0f, 5.0f, 4.3f, 2.1f,
						3.0f, 4.0f, 5.0f, 7.0f};

	L_matrix Mat2={5.0f, 6.0f, 7.0f, 20.2f,
						13.0f, 12.56f, 7.5f, 2.1f,
						5.6f, 7.0f, 3.0f, 2.0f,
						7.0f, 5.6f, 7.0f, 2.4f};

		
	if(d3d==1)
	{
		printf("D3DX\n");
		printmatrix((void*)D3DXMatrixMultiply((void*)&Mat1, (void*)&Mat1, (void*)&Mat2));
	}
	else if(d3d==2)
	{
		printf("L_math\n");
		printmatrix(L_matmultiply(&Mat1, &Mat1, &Mat2));
	}
	else if(d3d==3)
	{
		printf("L_mathC\n");
		printmatrix(L_matmultiply_C(&Mat1, &Mat1, &Mat2));
	}
	else
		return;
}

#define TEST_FUNC(label, func){dwTime=timeGetTime();for(i=0; i<dwNumTimes; i++)func;dwTime=timeGetTime()-dwTime;printf("%s: Performed %i in %i milliseconds\n", label, dwNumTimes, dwTime);}

int main(void)
{
	L_matrix Mat1={1.2f, 56.7f, 2.0f, 3.5f,
						5.6f, 7.0f, 8.0f, 90.0f,
						10.0f, 5.0f, 4.3f, 2.1f,
						3.0f, 4.0f, 5.0f, 7.0f};
	L_matrix Mat2={5.0f, 6.0f, 7.0f, 20.2f,
						13.0f, 12.56f, 7.5f, 2.1f,
						5.6f, 7.0f, 3.0f, 2.0f,
						7.0f, 5.6f, 7.0f, 2.4f};


	L_vector3 V1={1.3f, 56.7f, 28.345f};
	L_vector3 V2={345.34f, 2.4f, 67.87f};


	unsigned long dwTime=0;
	unsigned long dwNumTimes=1000000;
	unsigned long i=0;

	/*
	testvec3(1);
	testvec3(2);
	getch();
	return 0;
	*/
	//i=0xFFFFFFFF;
	//printf("The number is: %i (0x%08X)\n", L_nextpow2_ASM(i), L_nextpow2_ASM(i));
	//return 0;
	/*
	TEST_FUNC("L_mathC", L_vec3distance_C(&V1, &V2));
	TEST_FUNC("L_math", L_vec3distance(&V1, &V2));
	*/
	/*
	test(1);
	test(2);
	test(3);
	*/
	
	TEST_FUNC("D3DX", D3DXMatrixMultiply((void*)&Mat1, (void*)&Mat1, (void*)&Mat2));
	TEST_FUNC("L_math", L_matmultiply(&Mat1, &Mat1, &Mat2));
	TEST_FUNC("L_mathC", L_matmultiply_C(&Mat1, &Mat1, &Mat2));
	
	/*
	TEST_FUNC("L_mathC", L_nextpow2_C(5000));
	TEST_FUNC("L_math", L_nextpow2(5000));
	*/
	/*
	memcpy(&dwTime, &fTest, 4);
	printf("%f is 0x%8X\n", fTest, dwTime);
	*/
	getch();
	return 0;
}




/*
void testdump()
{
	int row=0, column=0;

	FILE* fout=fopen("out.txt", "w");
	for(row=0; row<4; row++)
	{
		for(column=0; column<4; column++)
		{
			fprintf(fout, ";_%i%i\n", row+1, column+1);
			fprintf(fout, "fld   dword [ecx+%i]\n", 16*row+0);
			fprintf(fout, "fmul  dword [edx+%i]\n", 4*column+0);

			fprintf(fout, "fld   dword [ecx+%i]\n", 16*row+4);
			fprintf(fout, "fmul  dword [edx+%i]\n", 4*column+16);
			fprintf(fout, "faddp st1\n");

			fprintf(fout, "fld   dword [ecx+%i]\n", 16*row+8);
			fprintf(fout, "fmul  dword [edx+%i]\n", 4*column+32);
			fprintf(fout, "faddp st1\n");

			fprintf(fout, "fld   dword [ecx+%i]\n", 16*row+12);
			fprintf(fout, "fmul  dword [edx+%i]\n", 4*column+48);
			fprintf(fout, "faddp st1\n");
			fprintf(fout, "fstp  dword [eax+%i]\n\n", 16*row+4*column);
		}
	}

	fclose(fout);
}
*/