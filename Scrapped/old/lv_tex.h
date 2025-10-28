/* lv_tex.h - Header for texture loading functions. */
#ifndef __LV_TEX_H__
#define __LV_TEX_H__
#include <d3d9.h>
#include "common.h"

#ifdef __cplusplus
extern "C"{
#endif /*__cplusplus*/


typedef enum _TEXFILTER_MODE{
	FILTER_UNKNOWN     = 0,
	FILTER_NONE        = 0,
	FILTER_POINT       = 1,
	FILTER_LINEAR      = 2,
	FILTER_BILINEAR    = 2,
	FILTER_TRILINEAR   = 3,
	FILTER_ANISOTROPIC = 4,
	FILTER_FORCE_DWORD = 0xFFFFFFFF
}TEXFILTER_MODE;
//#endif FILTER_MODE

lg_bool Tex_Load(
	IDirect3DDevice9* lpDevice, 
	lg_lpstr szFilename,
	D3DPOOL Pool, 
	lg_dword dwTransparent,
	lg_bool bForceNoMip,
	IDirect3DTexture9** lppTex);
	
lg_bool Tex_Load_Memory(
	IDirect3DDevice9* lpDevice, 
	lg_void* pBuffer,
	lg_dword nSize,
	D3DPOOL Pool, 
	lg_dword dwTransparent,
	lg_bool bForceNoMip,
	IDirect3DTexture9** lppTex);
	
void Tex_SetLoadOptions(
	lg_bool bUseMipMaps, //If false all mip-mapping will be disabled.
	lg_bool bUseHWMips,  //If true hardware mip-maps will be generated if possible.
	TEXFILTER_MODE MipFilter, //If the image has to be resisized at all this will be the filter applied.
	lg_uint nSizeLimit, //Textures will not excede this size.
	lg_bool bForce16Bit,//If ture all textures will be limited to 16 bits.
	lg_bool b16Alpha);
	
#define TEXLOAD_GENMIPMAP       0x00000001
#define TEXLOAD_HWMIPMAP        0x00000002
#define TEXLOAD_LINEARMIPFILTER 0x00000004
#define TEXLOAD_POINTMIPFILTER  0x00000000
#define TEXLOAD_FORCE16BIT      0x00000010
#define TEXLOAD_16BITALPHA      0x00000020

void Tex_SetLoadOptionsFlag(lg_dword Flags, lg_uint nSizeLimit);


#define SYSTOVID_AUTOGENMIPMAP   0x00000001
#define SYSTOVID_POINTMIPFILTER  0x00000000
#define SYSTOVID_LINEARMIPFILTER 0x00000002

lg_bool Tex_SysToVid(
	IDirect3DDevice9* lpDevice,
	IDirect3DTexture9** lppTex,
	lg_dword Flags);
	
lg_bool Tex_LoadIMG2(
	IDirect3DDevice9* lpDevice,
	lg_lpstr szFilename,
	D3DPOOL Pool,
	lg_dword Flags,
	lg_dword SizeLimit,
	IDirect3DTexture9** lppTex);
	
lg_bool Tex_LoadIMG2_Memory(
	IDirect3DDevice9* lpDevice,
	const lg_byte* pBuffer,
	lg_dword nBufferSize,
	D3DPOOL Pool,
	lg_dword Flags,
	lg_dword nSizeLimit,
	IDirect3DTexture9** lppTex);

#ifdef __cplusplus
}
#endif /*__cplusplus*/
#endif /*__LV_TEX_H__*/