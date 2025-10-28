/* lv_tex.c texture loading functions. */

#include <d3d9.h>
#include <d3dx9.h>
#include <lf_sys.h>
#include "lv_tex.h"
#include "lg_err.h"
#include "lg_sys.h"
#include "lv_init.h"


L_bool Tex_SysToVid(
	IDirect3DDevice9* lpDevice,
	IDirect3DTexture9** lppTex,
	L_dword Flags)
	/*
	L_bool bAutoGenMipMap,
	D3DTEXTUREFILTERTYPE AutoGenFilter)
	*/
{
	IDirect3DTexture9* lpTexSys=*lppTex;
	IDirect3DTexture9* lpTexVid=L_null;

	L_result nResult=0;
	D3DSURFACE_DESC sysdesc;
	L_dword nLevelCount=0;


	if(!lpTexSys || !lpDevice)
		return L_false;

	/* Get the description of the system texture. 
		Make sure that the source is actually in system
		memory.  Also get the level count.*/
	memset(&sysdesc, 0, sizeof(sysdesc));
	nResult=IDirect3DTexture9_GetLevelDesc(lpTexSys, 0, &sysdesc);
	if(L_failed(nResult))
		return L_false;
	if(sysdesc.Pool!=D3DPOOL_SYSTEMMEM)
		return L_false;
	nLevelCount=IDirect3DTexture9_GetLevelCount(lpTexSys);
	/* Create the new texture in video memory. */
	nResult=lpDevice->lpVtbl->CreateTexture(
		lpDevice,
		sysdesc.Width, 
		sysdesc.Height,
		L_CHECK_FLAG(Flags, SYSTOVID_AUTOGENMIPMAP)?0:nLevelCount,
		L_CHECK_FLAG(Flags, SYSTOVID_AUTOGENMIPMAP)?D3DUSAGE_AUTOGENMIPMAP:0,
		sysdesc.Format,
		D3DPOOL_DEFAULT,
		&lpTexVid,
		L_null);

	if(L_failed(nResult))
	{
		Err_Printf("Tex_Load Error: Could not open texture.");
		Err_PrintDX("   IDirect3DDevice9::CreateTexture", nResult);
		return L_false;
	}

	/* Use the device function to update the surface in video memory,
		with the data in system memory, then release the system memory
		texture, and adjust the pointer to point at the video memory
		texture. */
	if(L_CHECK_FLAG(Flags, SYSTOVID_AUTOGENMIPMAP) && nLevelCount==1)
	{
		D3DTEXTUREFILTERTYPE nFilter=0;
		nFilter=L_CHECK_FLAG(Flags, SYSTOVID_LINEARMIPFILTER)?D3DTEXF_LINEAR:D3DTEXF_POINT;
		lpTexVid->lpVtbl->SetAutoGenFilterType(lpTexVid, nFilter);
	}
	lpTexVid->lpVtbl->AddDirtyRect(lpTexVid, L_null);
	lpDevice->lpVtbl->UpdateTexture(lpDevice, (void*)lpTexSys, (void*)lpTexVid);
	lpTexSys->lpVtbl->Release(lpTexSys);
	*lppTex=lpTexVid;
	return L_true;
}

L_bool Tex_LoadD3D(
	IDirect3DDevice9* lpDevice,
	L_lpstr szFilename,
	D3DPOOL Pool,
	L_dword Flags,
	L_dword nSizeLimit,
	IDirect3DTexture9** lppTex)
{
	LF_FILE2 fin=L_null;
	L_result nResult=0;
	L_uint nMipLevels=0;
	L_dword dwTexFilter=0;
	L_dword dwTexFormat=0;
	L_dword dwWidth=0, dwHeight=0;


	fin=File_Open(szFilename, 0, LF_ACCESS_READ|LF_ACCESS_MEMORY, LFCREATE_OPEN_EXISTING);
	if(!fin)
	{
		Err_Printf("Tex_Load Error: Could not create texture from \"%s\".", szFilename);
		Err_Printf("   Could not open source file.");
		return L_false;
	}

	if(L_CHECK_FLAG(Flags, TEXLOAD_GENMIPMAP))
		nMipLevels=0;
	else
		nMipLevels=1;

	if(L_CHECK_FLAG(Flags, TEXLOAD_LINEARMIPFILTER))
		dwTexFilter=D3DX_FILTER_TRIANGLE|D3DX_FILTER_DITHER;
	else
		dwTexFilter=D3DX_FILTER_POINT;

	if(nSizeLimit)
	{
		dwWidth=dwHeight=nSizeLimit;
	}
	else
	{
		dwWidth=dwHeight=D3DX_DEFAULT;
	}

	if(L_CHECK_FLAG(Flags, TEXLOAD_FORCE16BIT))
	{
		if(L_CHECK_FLAG(Flags, TEXLOAD_16BITALPHA))
			dwTexFormat=D3DFMT_A1R5G5B5;
		else
			dwTexFormat=D3DFMT_R5G6B5;
	}
	else
	{
		dwTexFormat=D3DFMT_UNKNOWN;
	}

	/* Because we opened the file in membuf mode, 
		we can just pas the pFileData member, and the 
		File Size member to create the file in memory. */
	nResult=D3DXCreateTextureFromFileInMemoryEx(
		lpDevice,
		File_GetMemPointer(fin, L_null),
		File_GetSize(fin),
		dwWidth,
		dwHeight,
		nMipLevels,
		0,
		dwTexFormat,
		Pool,
		dwTexFilter,
		dwTexFilter,
		0,
		L_null,
		L_null,
		lppTex);

	File_Close(fin);

	if(L_failed(nResult))
	{
		Err_Printf("Tex_Load Error: Could not create texture from \"%s\".", szFilename);
		Err_PrintDX("   D3DXCreateTextureFromFileInMemoryEx", nResult);
		return L_false;
	}

	return L_true;
}

L_bool Tex_Load(
	IDirect3DDevice9* lpDevice,
	L_lpstr szFilename,
	D3DPOOL Pool,
	L_dword dwTransparent,
	L_bool bForceNoMip,
	IDirect3DTexture9** lppTex)
{
	L3DGame* lpGame=LG_GetGame();
	L_dword nFlags=0;

	L_bool (*pTexLoadCB)(
		IDirect3DDevice9* lpDevice,
		L_lpstr szFilename,
		D3DPOOL Pool,
		L_dword Flags,
		L_dword SizeLimit,
		IDirect3DTexture9** lppTex)=L_null;

	L_bool bGenMipMap=L_false;
	LV_TEXFILTER_MODE nMipFilter=FILTER_POINT;
	L_dword dwSizeLimit=0;

	char* szExt=L_null;
	L_dword nLen=L_strlen(szFilename);
	L_dword i=0;

	if(!lpDevice || !szFilename)
	{
		Err_Printf("Tex_Load Error: Could not open \"%s\".", szFilename);
		Err_Printf("   No device or no filename specified.");
		return L_false;
	}

	nFlags=0;
	if(lpGame)
	{
		if(CVar_GetValue(lpGame->m_cvars, "v_UseMipMaps", L_null) && !bForceNoMip)
			nFlags|=TEXLOAD_GENMIPMAP;

		if(CVar_GetValue(lpGame->m_cvars, "v_MipGenFilter", L_null)>=FILTER_LINEAR)
			nFlags|=TEXLOAD_LINEARMIPFILTER;

		dwSizeLimit=(L_dword)CVar_GetValue(lpGame->m_cvars, "v_TextureSizeLimit", L_null);

		if(CVar_GetValue(lpGame->m_cvars, "v_HWMipMaps", L_null))
			nFlags|=TEXLOAD_HWMIPMAP;

		if(CVar_GetValue(lpGame->m_cvars, "v_Force16BitTextures", L_null))
			nFlags|=TEXLOAD_FORCE16BIT;

		if(CVar_GetValue(lpGame->m_cvars, "v_16BitTextureAlpha", L_null))
			nFlags|=TEXLOAD_16BITALPHA;

	}
	else
	{
		if(!bForceNoMip)
			nFlags|=TEXLOAD_GENMIPMAP;
		nFlags|=TEXLOAD_LINEARMIPFILTER;
		dwSizeLimit=0;
	}

	for(i=nLen; i>0; i--)
	{
		if(szFilename[i]=='.')
		{
			szExt=&szFilename[i+1];
			break;
		}
	}

	if(L_strnicmp(szExt, "tga", 0) || L_strnicmp(szExt, "bmp", 0) || L_strnicmp(szExt, "dib", 0))
		pTexLoadCB=Tex_LoadIMG2;
	else
		pTexLoadCB=Tex_LoadD3D;

	return pTexLoadCB(
			lpDevice, 
			szFilename, 
			Pool, 
			nFlags, 
			dwSizeLimit, 
			lppTex);

}

L_bool Tex_CanAutoGenMipMap(IDirect3DDevice9* lpDevice, D3DFORMAT Format)
{
	D3DCAPS9 Caps;
	D3DDEVICE_CREATION_PARAMETERS d3ddc;
	D3DDISPLAYMODE d3ddm;
	IDirect3D9* lpD3D=L_null;
	L_result nResult=0;

	if(!lpDevice)
		return L_false;


	memset(&Caps, 0, sizeof(D3DCAPS9));

	lpDevice->lpVtbl->GetDeviceCaps(lpDevice, &Caps);
	if(!L_CHECK_FLAG(Caps.Caps2, D3DCAPS2_CANAUTOGENMIPMAP))
		return L_false;

	lpDevice->lpVtbl->GetDirect3D(lpDevice, &lpD3D);
	memset(&d3ddc, 0, sizeof(d3ddc));
	lpDevice->lpVtbl->GetCreationParameters(lpDevice, &d3ddc);
	memset(&d3ddm, 0, sizeof(d3ddm));
		
	nResult=lpD3D->lpVtbl->CheckDeviceFormat(
		lpD3D,
		d3ddc.AdapterOrdinal, 
		d3ddc.DeviceType, 
		D3DFMT_R5G6B5,
		D3DUSAGE_AUTOGENMIPMAP, 
		D3DRTYPE_TEXTURE,  
		Format);
	L_safe_release(lpD3D);
	
	if(L_succeeded(nResult))
		return L_true;
	else
		return L_false;
}