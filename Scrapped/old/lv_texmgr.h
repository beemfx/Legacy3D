#ifndef __LV_TEXMGR_H__
#define __LV_TEXMGR_H__

#include <d3d9.h>
#include "common.h"
#ifndef __cplusplus
IDirect3DTexture9* Tex_Load2(lg_cstr szFilename, lg_bool bForceNoMip);
IDirect3DTexture9* Tex_Load2_Memory(lg_void* pBuffer, lg_dword nSize, lg_bool bForceNoMip);

#else __cplusplus
//extern "C" IDirect3DTexture9* Tex_Load2(lg_cstr szFilename, lg_bool bForceNoMip=LG_FALSE);
//extern "C" IDirect3DTexture9* Tex_Load2_Memory(lg_void* pBuffer, lg_dword nSize, lg_bool bForceNoMip);
class CLTexMgr
{
//friend IDirect3DTexture9* Tex_Load2(lg_cstr, lg_bool);
//friend IDirect3DTexture9* Tex_Load2_Memory(lg_void*, lg_dword, lg_bool);
private:
	static const lg_dword MAX_TEX_NAME=32;
	typedef struct _TEX_LINK
	{
		IDirect3DTexture9* pTex;
		lg_char szName[MAX_TEX_NAME+1];
		struct _TEX_LINK*  pNext;
	}TEX_LINK;

private:
	IDirect3DDevice9* m_pDevice;
	IDirect3DTexture9* m_pDefaultTex;
	
	TEX_LINK* m_pFirst;
	
	IDirect3DTexture9* GetTexture(lg_cstr szFilename);
public:
	CLTexMgr();
	~CLTexMgr();
	void Init(
		IDirect3DDevice9* pDevice,
		lg_cstr szDefaultTex, 
		lg_dword Flags, 
		lg_uint nSizeLimit);
	void UnInit();
	IDirect3DTexture9* LoadTexture(lg_cstr szFilename, lg_bool bForceNoMipMap=LG_FALSE);
	IDirect3DTexture9* LoadTexture(lg_void* pData, lg_dword nSize, lg_bool bForceNoMipMap=LG_FALSE);
	void ClearCache();
private:
	static class CLTexMgr* s_pTexMgr;
};

#endif __cplusplus

#endif __LV_TEXMGR_H__