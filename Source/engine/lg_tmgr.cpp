#include <d3dx9.h>
#include "lg_tmgr.h"
#include "lg_func.h"
#include "lg_err.h"
#include "lg_err_ex.h"
#include "lf_sys2.h"
#include "lc_sys2.h"
#include "lg_cvars.h"

CLTMgr* CLTMgr::s_pTMgr=LG_NULL;

CLTMgr::CLTMgr(IDirect3DDevice9* pDevice, lg_dword nMaxTex):
	CLHashMgr(nMaxTex, LG_TEXT("TMgr")),
	m_pDevice(LG_NULL),
	m_nPool(D3DPOOL_MANAGED)
{
	//If the texture manager already exists then we made an error somewhere.
	if(s_pTMgr)
	{
		throw new CLError(
			LG_ERR_DEFAULT, __FILE__, __LINE__, 
			LG_TEXT("The Texture Manager can only be initialized once."));
	}
	
	//Store the global manager.
	s_pTMgr=this;
	
	//Save a copy of the device.
	m_pDevice=pDevice;
	m_pDevice->AddRef();
	
	//Load the default texture:
	//m_pDefItem=DoLoad(CV_Get(CVAR_tm_DefaultTexture)->szValue, 0);
	m_pItemList[0].pItem=DoLoad(CV_Get(CVAR_tm_DefaultTexture)->szValue, 0);
	LG_strncpy(m_pItemList[0].szName, CV_Get(CVAR_tm_DefaultTexture)->szValue, LG_MAX_PATH);
	
	if(m_pItemList[0].pItem)
		Err_Printf("TMgr: Default texture is \"%s\".", CV_Get(CVAR_tm_DefaultTexture)->szValue);
	else
		Err_Printf("TMgr: Could not load default texture.");
	
	Err_Printf("TMgr: Initialized with %u textures available.", m_nMaxItems);
}

CLTMgr::~CLTMgr()
{
	//Clear global manager.
	s_pTMgr=LG_NULL;
	//Release all textures, we have to call UnloadItems
	//from the destructor here, because the CLTMgr class
	//will be destroyed, and we can't call DoDestroy when
	//the class is destroyed.
	UnloadItems();
	//Destroy default item:
	DoDestroy(m_pItemList[0].pItem);
	//Release direct3d:
	LG_SafeRelease(m_pDevice);
	
	//The base class destructor will deallocate memory.
}

tm_tex CLTMgr::LoadTextureMem(lg_void* pData, lg_dword nSize, lg_dword nFlags)
{
	IDirect3DTexture9* pOut=LG_NULL;
	
	//Use the cvars and flags to set up the load
	//If the size is 0, then the default size is taken
	lg_dword nTexSize=CV_Get(CVAR_tm_TextureSizeLimit)->nValue;
	lg_dword nMips=LG_CheckFlag(nFlags, TM_FORCENOMIP) || !CV_Get(CVAR_tm_UseMipMaps)->nValue?1:0;
	D3DFORMAT nFormat=CV_Get(CVAR_tm_Force16BitTextures)?D3DFMT_R5G6B5:D3DFMT_UNKNOWN;
	
		
	//Now attempt to load the texture...
	//This is just temporyary, should check flags and stuff.
	HRESULT nResult=D3DXCreateTextureFromFileInMemoryEx(
		m_pDevice,
		pData,
		nSize,
		nTexSize,
		nTexSize,
		nMips,
		0,
		nFormat,
		m_nPool,
		D3DX_DEFAULT,
		D3DX_DEFAULT,
		0,
		LG_NULL,
		LG_NULL,
		&pOut);
		
	if(LG_FAILED(nResult))
	{		
		Err_Printf(LG_TEXT("TMgr ERROR: Could not obtain memory texture."));
		Err_Printf(LG_TEXT("TMgr: Obtained default texture."));
		
		return HM_DEFAULT_ITEM;
	}
	else
	{
		//We need to insert the texture into the list, so the reference is
		//valid, since we can't really hash memory we'll just use the first
		//available slot.  We start at 1 because 0 is default.
		for(lg_dword nLoc=1; nLoc<m_nMaxItems; nLoc++)
		{
			if(m_pItemList[nLoc].nHashCode==CLHashMgr::HM_EMPTY_NODE)
			{
				m_pItemList[nLoc].pItem=pOut;
				m_pItemList[nLoc].nHashCode=CLHashMgr::HM_MEMORY_NODE;
				m_pItemList[nLoc].szName[0]=0;
				m_pItemList[nLoc].nFlags=nFlags;
				Err_Printf(LG_TEXT("TMgr: Loaded memory texture."));
				return nLoc+1;
			}
		}
	}
	
	Err_Printf(LG_TEXT("TMgr ERROR: Could not find a spot, manager full."));
	return HM_DEFAULT_ITEM;
}

void CLTMgr::Invalidate()
{
	//Do nothing because textures are managed.
}

void CLTMgr::Validate()
{
	//Do nothing because textures are managed.
}

IDirect3DTexture9* CLTMgr::DoLoad(lg_path szFilename, lg_dword nFlags)
{
	//Open the file, we open with memory access.
	LF_FILE3 fileTex=LF_Open(szFilename, LF_ACCESS_MEMORY|LF_ACCESS_READ, LF_OPEN_EXISTING);
	if(!fileTex)
		return LG_NULL;
		
	IDirect3DTexture9* pOut=LG_NULL;
	
	//Use the cvars and flags to set up the load
	//If the size is 0, then the default size is taken
	lg_dword nTexSize=CV_Get(CVAR_tm_TextureSizeLimit)->nValue;
	lg_dword nMips=LG_CheckFlag(nFlags, TM_FORCENOMIP) || !CV_Get(CVAR_tm_UseMipMaps)->nValue?1:0;
	D3DFORMAT nFormat=CV_Get(CVAR_tm_Force16BitTextures)?D3DFMT_R5G6B5:D3DFMT_UNKNOWN;
	
		
	//Now attempt to load the texture...
	//This is just temporyary, should check flags and stuff.
	HRESULT hRes=D3DXCreateTextureFromFileInMemoryEx(
		m_pDevice,
		LF_GetMemPointer(fileTex),
		LF_GetSize(fileTex),
		nTexSize,
		nTexSize,
		nMips,
		0,
		nFormat,
		m_nPool,
		D3DX_DEFAULT,
		D3DX_DEFAULT,
		0,
		LG_NULL,
		LG_NULL,
		&pOut);
		
	//Close the source file.
	LF_Close(fileTex);
	
	if(LG_FAILED(hRes))
	{
		Err_Printf("TMgr ERROR: \"%s\" was not a valid texture.", szFilename);
		pOut=LG_NULL;
	}
	
	
	return pOut;
}

void CLTMgr::DoDestroy(IDirect3DTexture9* pItem)
{
	LG_SafeRelease(pItem);
}


void CLTMgr::SetTexture(tm_tex texture, lg_dword nStage)
{
	m_pDevice->SetTexture(nStage, texture?m_pItemList[texture-1].pItem:0);
}


tm_tex CLTMgr::TM_LoadTex(lg_path szFilename, lg_dword nFlags)
{
	return s_pTMgr->Load(szFilename, nFlags);
}

tm_tex CLTMgr::TM_LoadTexMem(lg_void* pData, lg_dword nSize, lg_dword nFlags)
{
	return s_pTMgr->LoadTextureMem(pData, nSize, nFlags);
}

void CLTMgr::TM_SetTexture(tm_tex texture, lg_dword nStage)
{
	return s_pTMgr->SetTexture(texture, nStage);
}
#if 0
//Test manager
CLTMgr::CLTMgr(IDirect3DDevice9* pDevice, lg_dword nMaxTex)
{

}

CLTMgr::~CLTMgr()
{

}

tm_tex CLTMgr::TM_LoadTex(lg_path szFilename, lg_dword nFlags)
{
	return 0;
}

tm_tex CLTMgr::TM_LoadTexMem(lg_void *pData, lg_dword nSize, lg_dword nFlags)
{
	return 0;
}

void CLTMgr::Validate()
{
}

void CLTMgr::Invalidate()
{
}

void CLTMgr::TM_SetTexture(tm_tex texture, lg_dword nStage)
{
}

void CLTMgr::PrintTextures()
{
	Err_Printf("TMgr: The test texture manager is operating.");
}

#endif