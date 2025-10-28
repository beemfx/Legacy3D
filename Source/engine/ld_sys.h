#pragma once
#include <d3d9.h>
#include "ML_lib.h"

void LD_DrawAABB(IDirect3DDevice9* pDevice, ML_AABB* pBox, D3DCOLOR Color);
void LD_DrawVec(IDirect3DDevice9* pDevice, ML_VEC3* pOrigin, ML_VEC3* pVec);
void LD_DrawTris(IDirect3DDevice9*  pDevice, ml_vec3* pPoints, lg_dword nTriCount);