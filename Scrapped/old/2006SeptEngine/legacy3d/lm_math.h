#ifndef __LM_MATH_H__
#define __LM_MATH_H__

#include "common.h"

#ifdef __cplusplus
extern "C"{
#endif /*__cplusplus*/

L_dword __cdecl L_nextpow2_ASM(L_dword n);
L_dword __cdecl L_nextpow2_C(L_dword n);

//#define __USEASM__
#ifdef __USEASM__
#define L_nextpow2 L_nextpow2_ASM
#else
#define L_nextpow2 L_nextpow2_C
#endif

void* L_matslerp(void* pOut, void* pM1, void* pM2, float t);

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*__LM_MATH_H__*/