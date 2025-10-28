/* lv_tex.h - Header for texture loading functions. */
#ifndef __LV_TEX_H__
#define __LV_TEX_H__
#include "common.h"
#include <d3d9.h>
#include "lv_init.h"

#ifdef __cplusplus
extern "C"{
#endif /*__cplusplus*/

L_bool Tex_Load(
	IDirect3DDevice9* lpDevice, 
	L_lpstr szFilename,
	D3DPOOL Pool, 
	L_dword dwTransparent,
	L_bool bForceNoMip,
	IDirect3DTexture9** lppTex);


#define TEXLOAD_GENMIPMAP       0x00000001
#define TEXLOAD_HWMIPMAP        0x00000002
#define TEXLOAD_LINEARMIPFILTER 0x00000004
#define TEXLOAD_POINTMIPFILTER  0x00000000
#define TEXLOAD_FORCE16BIT      0x00000010
#define TEXLOAD_16BITALPHA      0x00000020

L_bool Tex_LoadIMG2(
	IDirect3DDevice9* lpDevice,
	L_lpstr szFilename,
	D3DPOOL Pool,
	L_dword Flags,
	L_dword SizeLimit,
	IDirect3DTexture9** lppTex);

#define SYSTOVID_AUTOGENMIPMAP   0x00000001
#define SYSTOVID_POINTMIPFILTER  0x00000000
#define SYSTOVID_LINEARMIPFILTER 0x00000002

L_bool Tex_SysToVid(
	IDirect3DDevice9* lpDevice,
	IDirect3DTexture9** lppTex,
	L_dword Flags);

L_bool Tex_CanAutoGenMipMap(IDirect3DDevice9* lpDevice, D3DFORMAT Format);


#ifdef __cplusplus
}
#endif /*__cplusplus*/
#endif /*__LV_TEX_H__*/