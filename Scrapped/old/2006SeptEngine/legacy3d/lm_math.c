#include <d3dx9.h>
#include "lm_math.h"

L_dword __cdecl L_nextpow2_C(L_dword n)
{
	L_byte i=0;
	while(!L_CHECK_FLAG(n-1, 0x80000000l>>i))
		i++;
	return 1<<(0x80000000-i);
}

void* L_matslerp(void* pOut, void* pM1, void* pM2, float t)
{
	D3DXMATRIX *pXOut=pOut, *pXM1=pM1, *pXM2=pM2;
	D3DXQUATERNION Q1, Q2;
	float x=pXM1->_41+t*(pXM2->_41-pXM1->_41);
	float y=pXM1->_42+t*(pXM2->_42-pXM1->_42);
	float z=pXM1->_43+t*(pXM2->_43-pXM1->_43);
	D3DXQuaternionRotationMatrix(&Q1, pM1);
	D3DXQuaternionRotationMatrix(&Q2, pM2);
	D3DXQuaternionSlerp(&Q1, &Q1, &Q2, t);
	D3DXMatrixRotationQuaternion(pXOut, &Q1);
	pXOut->_41=x;
	pXOut->_42=y;
	pXOut->_43=z;
	return pOut;
}