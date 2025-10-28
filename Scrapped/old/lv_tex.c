/* lv_tex.c texture loading functions. */
#ifndef D3DX_SUPPORT
#define D3DX_SUPPORT
#endif //D3DX_SUPPORT


#include "lv_tex.h"

#ifdef D3DX_SUPPORT
#include <d3dx9.h>
#endif D3DX_SUPPORT

#include "lg_err.h"
#include "img_lib.h"
#include "lf_sys2.h"
#include "ML_lib.h"
#include "lg_func.h"



lg_bool Tex_LoadD3D(
	IDirect3DDevice9* lpDevice,
	lg_lpstr szFilename,
	D3DPOOL Pool,
	lg_dword Flags,
	lg_dword nSizeLimit,
	IDirect3DTexture9** lppTex);
	
lg_bool Tex_CanAutoGenMipMap(IDirect3DDevice9* lpDevice, D3DFORMAT Format);
	
//lg_bool g_bUseMipMaps=LG_TRUE;
//lg_bool g_bUseHWMipMaps=LG_TRUE;
//lg_bool g_MipFilter=FILTER_LINEAR;
lg_bool g_nSizeLimit=0;
//lg_bool g_bForce16Bit=LG_FALSE;
#ifdef _DEBUG
lg_bool g_bLoadOptionsSet=LG_FALSE;
#endif _DEBUG

lg_dword g_TexLdFlags=TEXLOAD_GENMIPMAP|TEXLOAD_HWMIPMAP|TEXLOAD_LINEARMIPFILTER;
	
void Tex_SetLoadOptions(
	lg_bool bUseMipMaps, //If false all mip-mapping will be disabled.
	lg_bool bUseHWMips,  //If true hardware mip-maps will be generated if possible.
	TEXFILTER_MODE MipFilter, //If the image has to be resisized at all this will be the filter applied.
	lg_uint nSizeLimit, //Textures will not excede this size.
	lg_bool bForce16Bit,//If ture all textures will be limited to 16 bits.
	lg_bool b16Alpha)
{
	g_TexLdFlags=0;
	if(bUseMipMaps)
		g_TexLdFlags|=TEXLOAD_GENMIPMAP;
	if(bUseHWMips)
		g_TexLdFlags|=TEXLOAD_HWMIPMAP;
	if(MipFilter>=FILTER_LINEAR)
		g_TexLdFlags|=TEXLOAD_LINEARMIPFILTER;
	if(bForce16Bit)
		g_TexLdFlags|=TEXLOAD_FORCE16BIT;
	if(b16Alpha)
		g_TexLdFlags|=TEXLOAD_16BITALPHA;
		
	g_nSizeLimit=nSizeLimit;
	#ifdef _DEBUG
	g_bLoadOptionsSet=LG_TRUE;
	#endif _DEBUG
}

void Tex_SetLoadOptionsFlag(lg_dword Flags, lg_uint nSizeLimit)
{
	g_TexLdFlags=Flags;
	g_nSizeLimit=nSizeLimit;
	#ifdef _DEBUG
	g_bLoadOptionsSet=LG_TRUE;
	#endif _DEBUG
}


lg_bool Tex_Load(
	IDirect3DDevice9* lpDevice,
	lg_lpstr szFilename,
	D3DPOOL Pool,
	lg_dword dwTransparent,
	lg_bool bForceNoMip,
	IDirect3DTexture9** lppTex)
{
	//void* lpGame=LG_NULL;//LG_GetGame();
	lg_dword nFlags=0;

	lg_bool (*pTexLoadCB)(
		IDirect3DDevice9* lpDevice,
		lg_lpstr szFilename,
		D3DPOOL Pool,
		lg_dword Flags,
		lg_dword SizeLimit,
		IDirect3DTexture9** lppTex)=LG_NULL;

	lg_bool bGenMipMap=LG_FALSE;
	TEXFILTER_MODE nMipFilter=FILTER_POINT;
	lg_dword dwSizeLimit=0;

	char* szExt=LG_NULL;
	lg_dword nLen=L_strlen(szFilename);
	lg_dword i=0;
	
	#ifdef _DEBUG
	if(!g_bLoadOptionsSet)
		Err_PrintfDebug("Tex_Load WARNING: Using Tex_Load before calling Tex_SetLoadOptions...\n");
	#endif _DEBUG

	if(!lpDevice || !szFilename)
	{
		Err_Printf("Tex_Load Error: Could not open \"%s\".", szFilename);
		Err_Printf("   No device or no filename specified.");
		*lppTex=LG_NULL;
		return LG_FALSE;
	}
	nFlags=g_TexLdFlags;
	dwSizeLimit=g_nSizeLimit;
	/*
	#ifdef _DEBUG
	Err_PrintfDebug("Loading with the following options:\n%s%s%s%s%s\n\tSize Limit: %d",
		L_CHECK_FLAG(nFlags, TEXLOAD_GENMIPMAP)?"\tGENMIPMAP\n":"",
		L_CHECK_FLAG(nFlags, TEXLOAD_HWMIPMAP)?"\tHWMIPMAP\n":"",
		L_CHECK_FLAG(nFlags, TEXLOAD_LINEARMIPFILTER)?"\tLINEARMIPFILTER\n":"",
		L_CHECK_FLAG(nFlags, TEXLOAD_FORCE16BIT)?"\tFORCE16BIT\n":"",
		L_CHECK_FLAG(nFlags, TEXLOAD_16BITALPHA)?"\t16BITALPHS\n":"",
		g_nSizeLimit);
	#endif
	*/

	for(i=nLen; i>0; i--)
	{
		if(szFilename[i]=='.')
		{
			szExt=&szFilename[i+1];
			break;
		}
	}
	
	#ifdef D3DX_SUPPORT
	if(L_strnicmp(szExt, "tga", 0) || 
		L_strnicmp(szExt, "bmp", 0) || 
		L_strnicmp(szExt, "dib", 0) || 
		L_strnicmp(szExt, "jpg", 0) ||
		L_strnicmp(szExt, "gif", 0) ||
		L_strnicmp(szExt, "png", 0))
		pTexLoadCB=Tex_LoadIMG2;
	else
		pTexLoadCB=Tex_LoadD3D;
	#else D3DX_SUPPORT
	pTexLoadCB=Tex_LoadIMG2;
	#endif D3DX_SUPPORT

	return pTexLoadCB(
			lpDevice, 
			szFilename, 
			Pool, 
			nFlags, 
			dwSizeLimit, 
			lppTex);

}

lg_bool Tex_Load_Memory(
	IDirect3DDevice9* lpDevice, 
	lg_void* pBuffer,
	lg_dword nSize,
	D3DPOOL Pool, 
	lg_dword dwTransparent,
	lg_bool bForceNoMip,
	IDirect3DTexture9** lppTex)
{
	//void* lpGame=LG_NULL;//LG_GetGame();
	lg_dword nFlags=0;

	lg_bool (*pTexLoadCB)(
		IDirect3DDevice9* lpDevice,
		lg_void* pBuffer,
		lg_dword nSize,
		D3DPOOL Pool,
		lg_dword Flags,
		lg_dword SizeLimit,
		IDirect3DTexture9** lppTex)=LG_NULL;

	lg_bool bGenMipMap=LG_FALSE;
	TEXFILTER_MODE nMipFilter=FILTER_POINT;
	lg_dword dwSizeLimit=0;

	#ifdef _DEBUG
	if(!g_bLoadOptionsSet)
		Err_PrintfDebug("Tex_Load WARNING: Useing Tex_Load before calling Tex_SetLoadOptions...\n");
	#endif _DEBUG

	if(!lpDevice)
	{
		Err_Printf("Tex_Load_Memory Error: Could not open texture.");
		Err_Printf("   No device or no filename specified.");
		*lppTex=LG_NULL;
		return LG_FALSE;
	}

	nFlags=g_TexLdFlags;
	dwSizeLimit=g_nSizeLimit;

	pTexLoadCB=Tex_LoadIMG2_Memory;

	return pTexLoadCB(
			lpDevice, 
			pBuffer,
			nSize, 
			Pool, 
			nFlags, 
			dwSizeLimit, 
			lppTex);
}

lg_bool Tex_CanAutoGenMipMap(IDirect3DDevice9* lpDevice, D3DFORMAT Format)
{
	D3DCAPS9 Caps;
	D3DDEVICE_CREATION_PARAMETERS d3ddc;
	D3DDISPLAYMODE d3ddm;
	IDirect3D9* lpD3D=LG_NULL;
	lg_result nResult=0;

	if(!lpDevice)
		return LG_FALSE;


	memset(&Caps, 0, sizeof(D3DCAPS9));

	lpDevice->lpVtbl->GetDeviceCaps(lpDevice, &Caps);
	if(!L_CHECK_FLAG(Caps.Caps2, D3DCAPS2_CANAUTOGENMIPMAP))
		return LG_FALSE;

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
	
	if(LG_SUCCEEDED(nResult))
		return LG_TRUE;
	else
		return LG_FALSE;
}

lg_bool Tex_SysToVid(
	IDirect3DDevice9* lpDevice,
	IDirect3DTexture9** lppTex,
	lg_dword Flags)
	/*
	lg_bool bAutoGenMipMap,
	D3DTEXTUREFILTERTYPE AutoGenFilter)
	*/
{
	IDirect3DTexture9* lpTexSys=*lppTex;
	IDirect3DTexture9* lpTexVid=LG_NULL;

	lg_result nResult=0;
	D3DSURFACE_DESC sysdesc;
	lg_dword nLevelCount=0;


	if(!lpTexSys || !lpDevice)
		return LG_FALSE;

	/* Get the description of the system texture. 
		Make sure that the source is actually in system
		memory.  Also get the level count.*/
	memset(&sysdesc, 0, sizeof(sysdesc));
	nResult=IDirect3DTexture9_GetLevelDesc(lpTexSys, 0, &sysdesc);
	if(LG_FAILED(nResult))
		return LG_FALSE;
	if(sysdesc.Pool!=D3DPOOL_SYSTEMMEM)
		return LG_FALSE;
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
		LG_NULL);

	if(LG_FAILED(nResult))
	{
		Err_Printf("Tex_Load Error: Could not open texture.");
		Err_PrintDX("   IDirect3DDevice9::CreateTexture", nResult);
		return LG_FALSE;
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
	lpTexVid->lpVtbl->AddDirtyRect(lpTexVid, LG_NULL);
	lpDevice->lpVtbl->UpdateTexture(lpDevice, (void*)lpTexSys, (void*)lpTexVid);
	lpTexSys->lpVtbl->Release(lpTexSys);
	*lppTex=lpTexVid;
	return LG_TRUE;
}

#ifdef D3DX_SUPPORT
lg_bool Tex_LoadD3D(
	IDirect3DDevice9* lpDevice,
	lg_lpstr szFilename,
	D3DPOOL Pool,
	lg_dword Flags,
	lg_dword nSizeLimit,
	IDirect3DTexture9** lppTex)
{
	LF_FILE3 fin=LG_NULL;
	lg_result nResult=0;
	lg_uint nMipLevels=0;
	lg_dword dwTexFilter=0;
	lg_dword dwTexFormat=0;
	lg_dword dwWidth=0, dwHeight=0;


	fin=LF_Open(szFilename, LF_ACCESS_READ|LF_ACCESS_MEMORY, LF_OPEN_EXISTING);
	if(!fin)
	{
		Err_Printf("Tex_Load Error: Could not create texture from \"%s\".", szFilename);
		Err_Printf("   Could not open source file.");
		return LG_FALSE;
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
		LF_GetMemPointer(fin),
		LF_GetSize(fin),
		dwWidth,
		dwHeight,
		nMipLevels,
		0,
		dwTexFormat,
		Pool,
		dwTexFilter,
		dwTexFilter,
		0,
		LG_NULL,
		LG_NULL,
		lppTex);

	LF_Close(fin);

	if(LG_FAILED(nResult))
	{
		Err_Printf("Tex_Load Error: Could not create texture from \"%s\".", szFilename);
		Err_PrintDX("   D3DXCreateTextureFromFileInMemoryEx", nResult);
		return LG_FALSE;
	}

	return LG_TRUE;
}
#endif D3DX_SUPPORT



typedef struct _TEX_IMG_COPY_DESC2{
	lg_dword   nWidth;
	lg_dword   nHeight;
	IMGFMT    nFormat;
	D3DFORMAT nD3DFormat;
	TEXFILTER_MODE nMipFilter;
	lg_bool    bGenMipMap;
	lg_bool    bUseHWMipMap;
	D3DPOOL   nPool;
	lf_path   szFilename;
}TEX_IMG_COPY_DESC2;

__inline lg_bool Tex_CopyIMGToTex(
	IDirect3DDevice9* lpDevice, 
	HIMG hImage, 
	IDirect3DTexture9** lppTex,
	TEX_IMG_COPY_DESC2* pDesc);


int __cdecl img_close(void* stream);
int __cdecl img_seek(void* stream, long offset, int origin);
int __cdecl img_tell(void* stream);
unsigned int __cdecl img_read(void* buffer, unsigned int size, unsigned int count, void* stream);


/***************************************************************************
	Tex_LoadIMG()

	Loads a texture into a texture interface in system memory.  This is an 
	example of how to manually load a texture into system memory.  The idea
	behind this function is that it loads the HIMG then prepares the 
	TEX_IMG_COPY_DESC2 structure based on the loaded texture and various
	paramters and cvars, then calls Tex_CopyIMGToTex using the
	TEX_IMG_COPY_DESC2 structure to create the texture.  Note that this
	functions insures that all values passed to Tex_CopyIMGToTex are valid.

	What the different Flags parameters will do:
	TEXLOAD_GENMIPMAP       - A mip map will be generated for the texture.
	TEXLOAD_HWMIPMAP        - The texture will use hardware generated mip 
		maps (if available)
	TEXLOAD_LINEARMIPFILTER - The device will use a linear filter to 
		genreate mip sublevels.
	TEXLOAD_POINTMIPFILTER  - The device will use point filterting to
		generate mip sublevels.
	TEXLOAD_FORCE16BIT      - The texture will be forced to a bitdepth of 16.
	TEXLOAD_16BITALPHA      - If the texture is being converted from 32 bit
		to 16 bit an alpha channel will be used for the 16 bit texture
		allowing the retention of some transparency.  Note that 16 bit alpha
		channels are only 1 bit so they are either on or off, the texture
		loader typically converts even the remotest amount of transparency
		to off.

	Notes:  The function will limit the texture to the specified size limit.
	It also insures that the final texture dimensions will be valid (a power
	of 2, square, etc) depending on the device capabilities.  Note that it
	assumes that the final texture format will be valid so if a video card
	that only supports 16 bit textures is being used then when Tex_LoadIMG2
	is called the TEXLOAD_FORCE16BIT flag needs to be set or the texture
	creation will fail.
***************************************************************************/
lg_bool Tex_LoadIMG2(
	IDirect3DDevice9* lpDevice,
	lg_lpstr szFilename,
	D3DPOOL Pool,
	lg_dword Flags,
	lg_dword nSizeLimit,
	IDirect3DTexture9** lppTex)
{
	LF_FILE3 fin=LG_NULL;
	lg_bool bResult;
	
	if(!lpDevice || !szFilename)
	{
		*lppTex=LG_NULL;
		return LG_FALSE;
	}

	/* First thing first we open the tga file, we are using the callback
		function to open the file so that we can use the Legacy File
		system. */

	/* If we can't open the file, or if the tga file can't be opened
		then it was probably an invalid texture.  Note that TGA_OpenCallbacks
		will close the file for us, so we don't need to worry about closing
		it ourselves. */
		
	/* We use the LF_ACCESS_MEMORY flag to insure that we can use
		File_GetMemPointer. */
	fin=LF_Open(szFilename, LF_ACCESS_READ|LF_ACCESS_MEMORY, LF_OPEN_EXISTING);
	if(!fin)
	{
		Err_Printf("Tex_Load Error: Could not open \"%s\".", szFilename);
		Err_Printf("   Could not open source file.");
		*lppTex=LG_NULL;
		return LG_FALSE;
	}
	bResult=Tex_LoadIMG2_Memory(lpDevice, LF_GetMemPointer(fin), LF_GetSize(fin), Pool, Flags, nSizeLimit, lppTex);
	LF_Close(fin);
	
	return bResult;
}

/* Tex_CopyIMGToTex()

	This funciton actually creates the IDirect3DTexture9 interface,
	and copies the data located in hImage to the texture, it uses
	parameters set up in the TEX_IMG_COPY_DESC2 structure.  Note that
	it does not check to make sure that the values in that structure
	are valid, as Tex_LoadIMG builds that structure and insures that
	it is valid.  This function is only intended to be called by
	Tex_LoadImage and not any other function.*/
__inline lg_bool Tex_CopyIMGToTex(
	IDirect3DDevice9* lpDevice, 
	HIMG hImage, 
	IDirect3DTexture9** lppTex,
	TEX_IMG_COPY_DESC2* pDesc)
{
	IDirect3DTexture9* lpTexTemp=LG_NULL;

	lg_dword nMipLevels=0;
	lg_dword nUsage=0;
	lg_dword i=0;
	lg_result nResult=0;


	//Find out how many mip levels need to be generated.
	//1 if there will be no mip map.
	//0 if mip levels will be generated by the video card.
	//variable amount if we are generating them ourselves.
	if(!pDesc->bGenMipMap)
		nMipLevels=1;
	else if(pDesc->bGenMipMap && pDesc->bUseHWMipMap)
		nMipLevels=1;
	else if(pDesc->bGenMipMap && !pDesc->bUseHWMipMap)
	{
		nMipLevels=0;
		
		i=LG_Min(pDesc->nWidth, pDesc->nHeight);
		do
		{
			nMipLevels++;
		}while((i/=2)>=1);
	}

	//If we are automatically generating mip maps
	//we specify so.
	nUsage=0;
	if(pDesc->bGenMipMap && pDesc->bUseHWMipMap && (pDesc->nPool!=D3DPOOL_DEFAULT))
	{
		nUsage|=D3DUSAGE_AUTOGENMIPMAP;
	}

	nResult=lpDevice->lpVtbl->CreateTexture(
		lpDevice,
		pDesc->nWidth,
		pDesc->nHeight,
		nMipLevels,
		nUsage,
		pDesc->nD3DFormat,
		pDesc->nPool==D3DPOOL_DEFAULT?D3DPOOL_SYSTEMMEM:pDesc->nPool,
		&lpTexTemp,
		LG_NULL);

	if(LG_FAILED(nResult))
	{
		*lppTex=LG_NULL;
		Err_Printf("Tex_Load Error: Could not open \"%s\".", pDesc->szFilename);
		Err_PrintDX("   IDirect3DDevice9", nResult);
		return LG_FALSE;
	}

	//Now for each mip level we will lock and copy the source image.
	nMipLevels=lpTexTemp->lpVtbl->GetLevelCount(lpTexTemp);
	for(i=0; i<nMipLevels; i++)
	{
		D3DLOCKED_RECT rc;
		D3DSURFACE_DESC sdesc;
		IMG_DEST_RECT rcDest;

		memset(&rc, 0, sizeof(rc));
		memset(&sdesc, 0, sizeof(sdesc));
		nResult=lpTexTemp->lpVtbl->LockRect(lpTexTemp, i, &rc, LG_NULL, 0);
	
		if(LG_FAILED(nResult))
		{
			Err_Printf("Tex_Load Error: Could not open \"%s\".", pDesc->szFilename);
			Err_PrintDX("   IDirect3DTexture9::LockRect", nResult);
			lpTexTemp->lpVtbl->Release(lpTexTemp);
			return LG_FALSE;
		}

		nResult=IDirect3DTexture9_GetLevelDesc(lpTexTemp, i, &sdesc);
		if(LG_FAILED(nResult))
		{
			Err_Printf("Tex_Load Error: Could not open \"%s\".", pDesc->szFilename);
			Err_PrintDX("   IDirect3DTexture9::GetLevelDesc", nResult);
			lpTexTemp->lpVtbl->UnlockRect(lpTexTemp, i);
			lpTexTemp->lpVtbl->Release(lpTexTemp);
			
			return LG_FALSE;
		}

		rcDest.pImage=rc.pBits;
		rcDest.nOrient=IMGORIENT_TOPLEFT;
		rcDest.nFormat=pDesc->nFormat;
		rcDest.nWidth=sdesc.Width;
		rcDest.nHeight=sdesc.Height;
		rcDest.nPitch=rc.Pitch;
		rcDest.rcCopy.left=rcDest.rcCopy.top=0;
		rcDest.rcCopy.right=sdesc.Width;
		rcDest.rcCopy.bottom=sdesc.Height;

		IMG_CopyBits(
			hImage,
			&rcDest,
			pDesc->nMipFilter>FILTER_POINT?IMGFILTER_LINEAR:IMGFILTER_POINT,
			LG_NULL,
			0xFF);

		lpTexTemp->lpVtbl->UnlockRect(lpTexTemp, i);
	}

	if(pDesc->nPool==D3DPOOL_DEFAULT)
	{
		lg_dword nFlags=0;
		if(pDesc->nMipFilter>FILTER_POINT)
			nFlags|=SYSTOVID_LINEARMIPFILTER;
		if(pDesc->bGenMipMap && pDesc->bUseHWMipMap)
		{
			nFlags|=SYSTOVID_AUTOGENMIPMAP;
		}

		if(!Tex_SysToVid(lpDevice, &lpTexTemp, nFlags))
		{
			Err_Printf("Tex_Load Error: Could not open \"%s\".", pDesc->szFilename);
			Err_Printf("   Could not move texture to video memory.");
			L_safe_release(lpTexTemp);
			return LG_FALSE;
		}
	}

	*lppTex=lpTexTemp;
	return LG_TRUE;
}

lg_bool Tex_LoadIMG2_Memory(
	IDirect3DDevice9* lpDevice,
	const lg_byte* pBuffer,
	lg_dword nBufferSize,
	D3DPOOL Pool,
	lg_dword Flags,
	lg_dword nSizeLimit,
	IDirect3DTexture9** lppTex)
{
	TEX_IMG_COPY_DESC2 copydesc;
	HIMG hImage=LG_NULL;
	LF_FILE3 fin=LG_NULL;
	IMG_DESC imgdesc;
	D3DCAPS9 Caps;
	lg_void* buffer=LG_NULL;

	lg_bool bResult=LG_FALSE;

	if(!lpDevice || !pBuffer || !nBufferSize)
	{
		*lppTex=LG_NULL;
		return LG_FALSE;
	}
	
	hImage=IMG_OpenMemory((lg_pvoid)pBuffer, nBufferSize);

	if(!hImage)
	{
		Err_Printf("Tex_Load Error: Could not open texture at 0x%08X.", pBuffer);
		Err_Printf("   Could not acquire image from file.");
		*lppTex=LG_NULL;
		return LG_FALSE;
	}

	/* Next we need to get a description of the tga file, so that
		we can create an appropriate texture. Note that we could
		set the final texture format to whatever we want, but we'll
		try to match to original format.*/
	memset(&imgdesc, 0, sizeof(imgdesc));
	memset(&copydesc, 0, sizeof(copydesc));
	IMG_GetDesc(hImage, &imgdesc);
	copydesc.nWidth=imgdesc.Width;
	copydesc.nHeight=imgdesc.Height;
	copydesc.nPool=Pool;
	//copydesc.dwSizeLimit=dwSizeLimit;

	/* If there is a size limit, we need to limit the size of the texture. */
	if(nSizeLimit)
	{
		copydesc.nWidth=LG_Min(copydesc.nWidth, nSizeLimit);
		copydesc.nHeight=LG_Min(copydesc.nHeight, nSizeLimit);
	}

	memset(&Caps, 0, sizeof(Caps));
	lpDevice->lpVtbl->GetDeviceCaps(lpDevice, &Caps);

	/* We need to make sure the texture is not larger than the device limits. */
	copydesc.nWidth=LG_Min(copydesc.nWidth, Caps.MaxTextureWidth);
	copydesc.nHeight=LG_Min(copydesc.nHeight, Caps.MaxTextureHeight);
	/* If required (which it usually is) we need to make sure the texture
		is a power of 2. */
	if(L_CHECK_FLAG(Caps.TextureCaps, D3DPTEXTURECAPS_POW2))
	{
		copydesc.nWidth=ML_NextPow2(copydesc.nWidth);
		copydesc.nHeight=ML_NextPow2(copydesc.nHeight);
	}
	/* If required we need to make sure the texture is square.*/
	if(L_CHECK_FLAG(Caps.TextureCaps, D3DPTEXTURECAPS_SQUAREONLY))
	{
		copydesc.nWidth=copydesc.nHeight=LG_Max(copydesc.nWidth, copydesc.nHeight);
	}

	if(L_CHECK_FLAG(Flags, TEXLOAD_LINEARMIPFILTER))
		copydesc.nMipFilter=FILTER_LINEAR;
	else
		copydesc.nMipFilter=FILTER_POINT;
	
	switch(imgdesc.BitsPerPixel)
	{
	default:
	case 16:
		copydesc.nD3DFormat=D3DFMT_R5G6B5;
		copydesc.nFormat=IMGFMT_R5G6B5;
		break;
	case 8:
	case 24:
		copydesc.nD3DFormat=D3DFMT_X8R8G8B8;
		copydesc.nFormat=IMGFMT_A8R8G8B8;
		break;
	case 32:
		copydesc.nD3DFormat=D3DFMT_A8R8G8B8;
		copydesc.nFormat=IMGFMT_A8R8G8B8;
		break;
	}

	if(L_CHECK_FLAG(Flags, TEXLOAD_FORCE16BIT))//CVar_GetValue(LG_GetGame()->m_cvars, "v_Force16BitTextures", LG_NULL))
	{
		if(imgdesc.BitsPerPixel==32 && L_CHECK_FLAG(Flags, TEXLOAD_16BITALPHA))//(lg_bool)CVar_GetValue(LG_GetGame()->m_cvars, "v_16BitTextureAlpha", LG_NULL))
		{
			copydesc.nD3DFormat=D3DFMT_A1R5G5B5;
			copydesc.nFormat=IMGFMT_X1R5G5B5;
		}
		else
		{
			copydesc.nD3DFormat=D3DFMT_R5G6B5;
			copydesc.nFormat=IMGFMT_R5G6B5;
		}
	}

	copydesc.bGenMipMap=L_CHECK_FLAG(Flags, TEXLOAD_GENMIPMAP);//bGenMipMap;
	copydesc.bUseHWMipMap=Tex_CanAutoGenMipMap(lpDevice, copydesc.nD3DFormat) && 
		                   L_CHECK_FLAG(Flags, TEXLOAD_HWMIPMAP);

	//LG_strncpy(copydesc.szFilename, LG_NULL, LF_MAX_PATH);
	//copydesc.szFilename[0]=0;
	bResult=Tex_CopyIMGToTex(lpDevice, hImage, lppTex, &copydesc);

	IMG_Delete(hImage);
	hImage=LG_NULL;

	return bResult;
}
