#pragma once
#include "common.h"
#ifdef __cplusplus
extern "C"
#endif 
lg_bool Tex_Load(
	IDirect3DDevice9* lpDevice, 
	lg_lpstr szFilename,
	D3DPOOL Pool, 
	lg_dword dwTransparent,
	lg_bool bForceNoMip,
	IDirect3DTexture9** lppTex);
	
#ifdef __cplusplus
extern "C"
#endif 
IDirect3DTexture9* Tex_Load2(
	lg_lpstr szFilename,
#ifdef __cplusplus
	lg_bool bForceNoMip=LG_FALSE);
#else 
	lg_bool bForceNoMip);
#endif

#ifdef __cplusplus
extern "C"
#endif 
void Tex_SetDevice(IDirect3DDevice9* pDevice);