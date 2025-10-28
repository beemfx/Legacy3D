#include <math.h>
#include "lg_math.h"

L_dword __cdecl L_nextpow2_C(const L_dword n) 
{
	L_dword result = 1;
	if(n>=0x80000000)
		return 0x80000000;
	while(result<n) 
	{
		result <<= 1;
	}
	return result;
}

L_vector3* __cdecl L_vec3add_C(L_vector3* pOut, const L_vector3* pV1, const L_vector3* pV2)
{
	pOut->x=pV1->x+pV2->x;
	pOut->y=pV1->y+pV2->y;
	pOut->z=pV1->z+pV2->z;
	return pOut;
}

float __cdecl L_vec3distance_C(const L_vector3* pV1, const L_vector3* pV2)
{
	return (float)sqrt(((pV1->x-pV2->x)*(pV1->x-pV2->x)+(pV1->y-pV2->y)*(pV1->y-pV2->y)+(pV1->z-pV2->z)*(pV1->z-pV2->z)));
}



L_matrix* __cdecl L_matmultiply_C(L_matrix* pOut, const L_matrix* pM1, const L_matrix* pM2)
{
	L_matrix out;
	
	out._11 = (pM1->_11 * pM2->_11) + (pM1->_12 * pM2->_21) + (pM1->_13 * pM2->_31) + (pM1->_14 * pM2->_41);
	out._12 = (pM1->_11 * pM2->_12) + (pM1->_12 * pM2->_22) + (pM1->_13 * pM2->_32) + (pM1->_14 * pM2->_42);
	out._13 = (pM1->_11 * pM2->_13) + (pM1->_12 * pM2->_23) + (pM1->_13 * pM2->_33) + (pM1->_14 * pM2->_43);
	out._14 = (pM1->_11 * pM2->_14) + (pM1->_12 * pM2->_24) + (pM1->_13 * pM2->_34) + (pM1->_14 * pM2->_44);	
	out._21 = (pM1->_21 * pM2->_11) + (pM1->_22 * pM2->_21) + (pM1->_23 * pM2->_31) + (pM1->_24 * pM2->_41);
	out._22 = (pM1->_21 * pM2->_12) + (pM1->_22 * pM2->_22) + (pM1->_23 * pM2->_32) + (pM1->_24 * pM2->_42);
	out._23 = (pM1->_21 * pM2->_13) + (pM1->_22 * pM2->_23) + (pM1->_23 * pM2->_33) + (pM1->_24 * pM2->_43);
	out._24 = (pM1->_21 * pM2->_14) + (pM1->_22 * pM2->_24) + (pM1->_23 * pM2->_34) + (pM1->_24 * pM2->_44);	
	out._31 = (pM1->_31 * pM2->_11) + (pM1->_32 * pM2->_21) + (pM1->_33 * pM2->_31) + (pM1->_34 * pM2->_41);
	out._32 = (pM1->_31 * pM2->_12) + (pM1->_32 * pM2->_22) + (pM1->_33 * pM2->_32) + (pM1->_34 * pM2->_42);
	out._33 = (pM1->_31 * pM2->_13) + (pM1->_32 * pM2->_23) + (pM1->_33 * pM2->_33) + (pM1->_34 * pM2->_43);
	out._34 = (pM1->_31 * pM2->_14) + (pM1->_32 * pM2->_24) + (pM1->_33 * pM2->_34) + (pM1->_34 * pM2->_44);	
	out._41 = (pM1->_41 * pM2->_11) + (pM1->_42 * pM2->_21) + (pM1->_43 * pM2->_31) + (pM1->_44 * pM2->_41);
	out._42 = (pM1->_41 * pM2->_12) + (pM1->_42 * pM2->_22) + (pM1->_43 * pM2->_32) + (pM1->_44 * pM2->_42);
	out._43 = (pM1->_41 * pM2->_13) + (pM1->_42 * pM2->_23) + (pM1->_43 * pM2->_33) + (pM1->_44 * pM2->_43);
	out._44 = (pM1->_41 * pM2->_14) + (pM1->_42 * pM2->_24) + (pM1->_43 * pM2->_34) + (pM1->_44 * pM2->_44);	
	*pOut = out;
	return pOut;
}

L_matrix* __cdecl L_matmultiply3DNow(L_matrix* pOut, const L_matrix* pM1, const L_matrix* pM2)
{
	return L_null;
	
	#ifdef GPP
	asm(
	"movl $4, %%ecx\n"
	"MatrixMultiplyOf_3DNow_Loop%=:\n"
	"movd   (%0), %%mm0\n"			/* mm0 = ? | x */
	"movd  4(%0), %%mm2\n"			/* mm2 = ? | y */
	"movd  8(%0), %%mm4\n"			/* mm4 = ? | z */
	"movd 12(%0), %%mm6\n"			/* mm6 = ? | w */
	"prefetch 16(%0)\n"				/* prefetch_for_reading(in+4);  */
	"prefetchw 16(%2)\n"			/* prefetch_for_writing(out+4); */
	"punpckldq %%mm0, %%mm0\n"		/* mm0 = x | x */
	"punpckldq %%mm2, %%mm2\n"		/* mm2 = y | y */
	"punpckldq %%mm4, %%mm4\n"		/* mm4 = z | z */
	"punpckldq %%mm6, %%mm6\n"		/* mm6 = w | w */
	"movq %%mm0, %%mm1\n"			/* mm1 = x | x */
	"movq %%mm2, %%mm3\n"			/* mm3 = y | y */
	"movq %%mm4, %%mm5\n"			/* mm5 = z | z */
	"movq %%mm6, %%mm7\n"			/* mm7 = w | w */
	"pfmul   (%1), %%mm0\n"			/* mm0 = x*m[1]  | x*m[0] */
	"pfmul  8(%1), %%mm1\n"			/* mm1 = x*m[3]  | x*m[2] */
	"pfmul 16(%1), %%mm2\n" 		/* mm2 = y*m[5]  | y*m[4] */
	"pfmul 24(%1), %%mm3\n" 		/* mm3 = y*m[7]  | y*m[6] */
	"pfmul 32(%1), %%mm4\n" 		/* mm4 = z*m[9]  | z*m[8] */
	"pfmul 40(%1), %%mm5\n" 		/* mm5 = z*m[11] | z*m[10] */
	"pfmul 48(%1), %%mm6\n" 		/* mm6 = w*m[13] | w*m[12] */
	"pfmul 56(%1), %%mm7\n" 		/* mm7 = w*m[15] | w*m[14] */
	"pfadd %%mm0, %%mm2\n"
	"pfadd %%mm1, %%mm3\n"
	"pfadd %%mm4, %%mm6\n"
	"pfadd %%mm5, %%mm7\n"
	"pfadd %%mm2, %%mm6\n"
	"pfadd %%mm3, %%mm7\n"
	"movq %%mm6,  (%2)\n"
	"movq %%mm7, 8(%2)\n"
	"addl $16, %0\n"
	"addl $16, %2\n"
	"loop MatrixMultiplyOf_3DNow_Loop%=\n"
	:
	: "r" (pLeft), "r" (pRight), "r" (pOut)
	: "%ecx"
	);
	#endif
	
}