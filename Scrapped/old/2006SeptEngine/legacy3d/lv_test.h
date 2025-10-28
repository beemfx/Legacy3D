#ifndef __LV_TEST_H__
#define __LV_TEST_H__
#include <d3d9.h>

/*
typedef struct _L3DMVERTEX{
	float x, y, z;
	float tu, tv;
}L3DMVERTEX;

#define L3DMVERTEX_TYPE (D3DFVF_XYZ|D3DFVF_TEX1)

typedef struct _L3DM_MODEL{
	L_dword     dwTriangleCount;
	L3DMVERTEX* pVertices;

	IDirect3DVertexBuffer9* lpVB;
	IDirect3DTexture9*      lpSkin;
}L3DM_MODEL;
*/


typedef struct _LVT_OBJ{
	IDirect3DVertexBuffer9* lpVB;
	IDirect3DTexture9* lpTex;
	IDirect3DTexture9* lpLM;

	//L3DM_MODEL         l3dmTest;
}LVT_OBJ;


#ifdef __cplusplus
#define LVT_EXPORT extern "C"
#else
#define LVT_EXPORT
#endif

LVT_EXPORT int LVT_Delete(LVT_OBJ* lpObj);
LVT_EXPORT void LVT_Render(LVT_OBJ* lpObj, IDirect3DDevice9* lpDevice);
LVT_EXPORT int LVT_ValidateInvalidate(LVT_OBJ* lpObj, IDirect3DDevice9* lpDevice, L_bool bValidate);
LVT_EXPORT LVT_OBJ* LVT_CreateTest(IDirect3DDevice9* lpDevice);

LVT_EXPORT void LVT_LoadModel(char* szFilename);

#endif __LV_TEST_H__