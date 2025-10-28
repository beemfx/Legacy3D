//ld_sys.cpp - Debugging functions - note that all graphics drawing functions
//here are bo no means optimized.
#include <d3d9.h>
#include "ML_lib.h"
#include "common.h"

typedef struct COLOR_VERTEX{
	float x, y, z;
	lg_dword color;
}COLOR_VERTEX;
typedef struct COLOR_VERTEX2{
	ml_vec3 pos;
	lg_dword color;
}COLOR_VERTEX2;
#define COLOR_VERTEX_FVF (D3DFVF_XYZ|D3DFVF_DIFFUSE)

void LD_DrawTris(IDirect3DDevice9*  pDevice, ml_vec3* pPoints, lg_dword nTriCount)
{
	pDevice->SetFVF(COLOR_VERTEX_FVF);
	//pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	COLOR_VERTEX2 tri[3];
	for(lg_dword i=0; i<nTriCount; i++)
	{
		const lg_dword CLR_COUNT=4;
		lg_dword clr;
		
		if((i%CLR_COUNT)==0)
		{
			clr=0xFFFF0000;
		}
		else if((i%CLR_COUNT)==1)
		{
			clr=0xFF00FF00;
		}
		else if((i%CLR_COUNT)==2)
		{
			clr=0xFF0000FF;
		}
		else if((i%CLR_COUNT)==3)
		{
			clr=0xFF00FFFF;
		}
		
		lg_dword clr1=0xFF00FF00, clr2=0xFFFF0000, clr3=0xFF0000FF;
		
		tri[0].pos=pPoints[i*3];
		tri[0].color=clr;
		tri[1].pos=pPoints[i*3+1];
		tri[1].color=clr;
		tri[2].pos=pPoints[i*3+2];
		tri[2].color=clr;
		
		pDevice->DrawPrimitiveUP(
			D3DPT_TRIANGLELIST,
			1,
			tri,
			sizeof(COLOR_VERTEX2));
	}
	/*
	pDevice->DrawPrimitiveUP(
		D3DPT_TRIANGLELIST,
		nTriCount,
		pPoints,
		sizeof(ML_VEC3));
	*/
}

void LD_DrawVec(IDirect3DDevice9* pDevice, ML_VEC3* pOrigin, ML_VEC3* pVec)
{
	COLOR_VERTEX vVec[2];
	vVec[0].x=pOrigin->x;
	vVec[0].y=pOrigin->y;
	vVec[0].z=pOrigin->z;
	vVec[0].color=0xFFFFFFFF;
	
	vVec[1].x=pOrigin->x+pVec->x;
	vVec[1].y=pOrigin->y+pVec->y;
	vVec[1].z=pOrigin->z+pVec->z;
	vVec[1].color=0xFFFFFFFF;
	
	//Render the Line.
	ML_MAT matIdent;
	ML_MatIdentity(&matIdent);
	DWORD dwAlpha;
	
	pDevice->GetRenderState(D3DRS_ALPHABLENDENABLE, &dwAlpha);
	if(dwAlpha)
		pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	pDevice->SetTexture(0, NULL);
	pDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX*)&matIdent);
	pDevice->SetFVF(COLOR_VERTEX_FVF);
	pDevice->DrawPrimitiveUP(D3DPT_LINESTRIP, 1, &vVec, sizeof(COLOR_VERTEX)); 
	if(dwAlpha)
		pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, dwAlpha);
}

void LD_DrawAABB(IDirect3DDevice9* pDevice, ML_AABB* pBox, D3DCOLOR Color)
{		
	COLOR_VERTEX vAABB[16];
	
	for(lg_dword i=0; i<16; i++)
	{
		vAABB[i].color=Color;
	}
	
	vAABB[0].x=pBox->v3Min.x;
	vAABB[0].y=pBox->v3Min.y;
	vAABB[0].z=pBox->v3Min.z;
	
	vAABB[1].x=pBox->v3Min.x;
	vAABB[1].y=pBox->v3Max.y;
	vAABB[1].z=pBox->v3Min.z;
	
	vAABB[2].x=pBox->v3Min.x;
	vAABB[2].y=pBox->v3Max.y;
	vAABB[2].z=pBox->v3Max.z;

	vAABB[3].x=pBox->v3Max.x;
	vAABB[3].y=pBox->v3Max.y;
	vAABB[3].z=pBox->v3Max.z;

	vAABB[4].x=pBox->v3Max.x;
	vAABB[4].y=pBox->v3Max.y;
	vAABB[4].z=pBox->v3Min.z;

	vAABB[5].x=pBox->v3Max.x;
	vAABB[5].y=pBox->v3Min.y;
	vAABB[5].z=pBox->v3Min.z;

	vAABB[6].x=pBox->v3Max.x;
	vAABB[6].y=pBox->v3Min.y;
	vAABB[6].z=pBox->v3Max.z;

	vAABB[7].x=pBox->v3Min.x;
	vAABB[7].y=pBox->v3Min.y;
	vAABB[7].z=pBox->v3Max.z;

	vAABB[8].x=pBox->v3Min.x;
	vAABB[8].y=pBox->v3Min.y;
	vAABB[8].z=pBox->v3Min.z;

	vAABB[9].x=pBox->v3Max.x;
	vAABB[9].y=pBox->v3Min.y;
	vAABB[9].z=pBox->v3Min.z;

	vAABB[10].x=pBox->v3Max.x;
	vAABB[10].y=pBox->v3Min.y;
	vAABB[10].z=pBox->v3Max.z;

	vAABB[11].x=pBox->v3Max.x;
	vAABB[11].y=pBox->v3Max.y;
	vAABB[11].z=pBox->v3Max.z;

	vAABB[12].x=pBox->v3Max.x;
	vAABB[12].y=pBox->v3Max.y;
	vAABB[12].z=pBox->v3Min.z;

	vAABB[13].x=pBox->v3Min.x;
	vAABB[13].y=pBox->v3Max.y;
	vAABB[13].z=pBox->v3Min.z;

	vAABB[14].x=pBox->v3Min.x;
	vAABB[14].y=pBox->v3Max.y;
	vAABB[14].z=pBox->v3Max.z;

	vAABB[15].x=pBox->v3Min.x;
	vAABB[15].y=pBox->v3Min.y;
	vAABB[15].z=pBox->v3Max.z;
	
	//Render the AABB.
	ML_MAT matIdent;
	ML_MatIdentity(&matIdent);
	DWORD dwAlpha;
	pDevice->GetRenderState(D3DRS_ALPHABLENDENABLE, &dwAlpha);
	if(dwAlpha)
		pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	pDevice->SetTexture(0, NULL);
	pDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX*)&matIdent);
	pDevice->SetFVF(COLOR_VERTEX_FVF);
	pDevice->DrawPrimitiveUP(D3DPT_LINESTRIP, 15, &vAABB, sizeof(COLOR_VERTEX)); 
	if(dwAlpha)
		pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, dwAlpha);
}
