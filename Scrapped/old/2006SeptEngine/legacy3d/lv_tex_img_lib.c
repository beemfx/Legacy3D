#include <common.h>
#include <d3d9.h>
#include <lf_sys.h>
#include "lv_tex.h"
#include <img_lib.h>
#include "lg_err.h"
#include "lv_init.h"
#include "lm_math.h"

typedef struct _TEX_IMG_COPY_DESC2{
	L_dword   nWidth;
	L_dword   nHeight;
	IMGFMT    nFormat;
	D3DFORMAT nD3DFormat;
	LV_TEXFILTER_MODE nMipFilter;
	L_bool    bGenMipMap;
	L_bool    bUseHWMipMap;
	D3DPOOL   nPool;
	char      szFilename[MAX_F_PATH];
}TEX_IMG_COPY_DESC2;

__inline L_bool Tex_CopyIMGToTex(
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
L_bool Tex_LoadIMG2(
	IDirect3DDevice9* lpDevice,
	L_lpstr szFilename,
	D3DPOOL Pool,
	L_dword Flags,
	L_dword nSizeLimit,
	IDirect3DTexture9** lppTex)
{
	TEX_IMG_COPY_DESC2 copydesc;
	HIMG hImage=L_null;
	IMG_CALLBACKS cb;
	LF_FILE2 fin=L_null;
	IMG_DESC imgdesc;
	D3DCAPS9 Caps;

	L_bool bResult=L_false;

	if(!lpDevice || !szFilename)
		return L_false;

	/* First thing first we open the tga file, we are using the callback
		function to open the file so that we can use the Legacy File
		system. */
	cb.close=img_close;
	cb.read=img_read;
	cb.seek=img_seek;
	cb.tell=img_tell;

	/* If we can't open the file, or if the tga file can't be opened
		then it was probably an invalid texture.  Note that TGA_OpenCallbacks
		will close the file for us, so we don't need to worry about closing
		it ourselves. */
	fin=File_Open(szFilename, 0, LF_ACCESS_READ, LFCREATE_OPEN_EXISTING);
	if(!fin)
	{
		Err_Printf("Tex_Load Error: Could not open \"%s\".", szFilename);
		Err_Printf("   Could not open source file.");
		return L_false;
	}
	hImage=IMG_OpenCallbacks(fin, &cb);
	if(!hImage)
	{
		Err_Printf("Tex_Load Error: Could not open \"%s\".", szFilename);
		Err_Printf("   Could not acquire Truevision Targa from file.");
		return L_false;
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
		copydesc.nWidth=L_min(copydesc.nWidth, nSizeLimit);
		copydesc.nHeight=L_min(copydesc.nHeight, nSizeLimit);
	}

	memset(&Caps, 0, sizeof(Caps));
	lpDevice->lpVtbl->GetDeviceCaps(lpDevice, &Caps);

	/* We need to make sure the texture is not larger than the device limits. */
	copydesc.nWidth=L_min(copydesc.nWidth, Caps.MaxTextureWidth);
	copydesc.nHeight=L_min(copydesc.nHeight, Caps.MaxTextureHeight);
	/* If required (which it usually is) we need to make sure the texture
		is a power of 2. */
	if(L_CHECK_FLAG(Caps.TextureCaps, D3DPTEXTURECAPS_POW2))
	{
		copydesc.nWidth=L_nextpow2(copydesc.nWidth);
		copydesc.nHeight=L_nextpow2(copydesc.nHeight);
	}
	/* If required we need to make sure the texture is square.*/
	if(L_CHECK_FLAG(Caps.TextureCaps, D3DPTEXTURECAPS_SQUAREONLY))
	{
		copydesc.nWidth=copydesc.nHeight=L_max(copydesc.nWidth, copydesc.nHeight);
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

	if(L_CHECK_FLAG(Flags, TEXLOAD_FORCE16BIT))//CVar_GetValue(LG_GetGame()->m_cvars, "v_Force16BitTextures", L_null))
	{
		if(imgdesc.BitsPerPixel==32 && L_CHECK_FLAG(Flags, TEXLOAD_16BITALPHA))//(L_bool)CVar_GetValue(LG_GetGame()->m_cvars, "v_16BitTextureAlpha", L_null))
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

	L_strncpy(copydesc.szFilename, szFilename, MAX_F_PATH);
	bResult=Tex_CopyIMGToTex(lpDevice, hImage, lppTex, &copydesc);

	IMG_Delete(hImage);
	hImage=L_null;

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
__inline L_bool Tex_CopyIMGToTex(
	IDirect3DDevice9* lpDevice, 
	HIMG hImage, 
	IDirect3DTexture9** lppTex,
	TEX_IMG_COPY_DESC2* pDesc)
{
	IDirect3DTexture9* lpTexTemp=L_null;

	L_dword nMipLevels=0;
	L_dword nUsage=0;
	L_dword i=0;
	L_result nResult=0;


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
		
		i=L_min(pDesc->nWidth, pDesc->nHeight);
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
		L_null);

	if(L_failed(nResult))
	{
		*lppTex=L_null;
		Err_Printf("Tex_Load Error: Could not open \"%s\".", pDesc->szFilename);
		Err_PrintDX("   IDirect3DDevice9", nResult);
		return L_false;
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
		nResult=lpTexTemp->lpVtbl->LockRect(lpTexTemp, i, &rc, L_null, 0);
	
		if(L_failed(nResult))
		{
			Err_Printf("Tex_Load Error: Could not open \"%s\".", pDesc->szFilename);
			Err_PrintDX("   IDirect3DTexture9::LockRect", nResult);
			lpTexTemp->lpVtbl->Release(lpTexTemp);
			return L_false;
		}

		nResult=IDirect3DTexture9_GetLevelDesc(lpTexTemp, i, &sdesc);
		if(L_failed(nResult))
		{
			Err_Printf("Tex_Load Error: Could not open \"%s\".", pDesc->szFilename);
			Err_PrintDX("   IDirect3DTexture9::GetLevelDesc", nResult);
			lpTexTemp->lpVtbl->UnlockRect(lpTexTemp, i);
			lpTexTemp->lpVtbl->Release(lpTexTemp);
			return L_false;
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
			L_null,
			0xFF);

		lpTexTemp->lpVtbl->UnlockRect(lpTexTemp, i);
	}

	if(pDesc->nPool==D3DPOOL_DEFAULT)
	{
		L_dword nFlags=0;
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
			return L_false;
		}
	}

	*lppTex=lpTexTemp;
	return L_true;
	
}

/*********************************************
	Callback functions for reading tga file.
*********************************************/
int __cdecl img_close(void* stream)
{
	return File_Close(stream);
}
int __cdecl img_seek(void* stream, long offset, int origin)
{
	return File_Seek(stream, offset, origin);
}
int __cdecl img_tell(void* stream)
{
	return File_Tell(stream);
}
unsigned int __cdecl img_read(void* buffer, unsigned int size, unsigned int count, void* stream)
{
	return File_Read(stream, size*count, buffer);
}