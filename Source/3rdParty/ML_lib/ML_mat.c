// (c) Beem Media. All rights reserved.

#include "ML_mat.h"
#include "d3dx9math.h"

ml_mat* ML_MatMultiply(ml_mat* pOut, const ml_mat* pM1, const ml_mat* pM2)
{
	return (ml_mat*)D3DXMatrixMultiply((D3DXMATRIX*)(pOut), (const D3DXMATRIX*)(pM1), (const D3DXMATRIX*)(pM2));
}

ml_mat* ML_MatIdentity(ml_mat* pOut)
{
	return (ml_mat*)D3DXMatrixIdentity((D3DXMATRIX*)pOut);
}

ml_mat* ML_MatRotationQuat(ml_mat* pOut, const ML_QUAT* pQ)
{
	return (ml_mat*)D3DXMatrixRotationQuaternion(pOut, pQ);
}