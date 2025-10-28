#include "lv_texmgr.h"
#include "lv_tex.h"
#include "lg_err_ex.h"
#include "lg_err.h"
#include "lf_sys2.h"
#include "lg_func.h"

#if 0
extern "C" IDirect3DTexture9* Tex_Load2(lg_cstr szFilename, lg_bool bForceNoMip)
{
	if(!CLTexMgr::s_pTexMgr)
	{
		Err_Printf("Tex_Load Error: Texture manager not initialized.");
		return LG_NULL;
	}
	return CLTexMgr::s_pTexMgr->LoadTexture(szFilename, bForceNoMip);
}


extern "C" IDirect3DTexture9* Tex_Load2_Memory(lg_void* pBuffer, lg_dword nSize, lg_bool bForceNoMip)
{
	if(!CLTexMgr::s_pTexMgr)
	{
		Err_Printf("Tex_Load_Memory Error: Texture manager not initialized.");
		return LG_NULL;
	}
	return CLTexMgr::s_pTexMgr->LoadTexture(pBuffer, nSize, bForceNoMip);
}
#endif
class CLTexMgr* CLTexMgr::s_pTexMgr=LG_NULL;

CLTexMgr::CLTexMgr():
	m_pDevice(LG_NULL),
	m_pDefaultTex(LG_NULL),
	m_pFirst(LG_NULL)
{
	s_pTexMgr=this;
}

CLTexMgr::~CLTexMgr()
{
	UnInit();
}

void CLTexMgr::Init(IDirect3DDevice9* pDevice, lg_cstr szDefaultTex, lg_dword Flags, lg_uint nSizeLimit)
{
	if(m_pDevice)
		UnInit();
		
	Tex_SetLoadOptionsFlag(Flags, nSizeLimit);	
	m_pDevice=pDevice;
	m_pDevice->AddRef();
	//Should create a default texture.
	lg_bool bResult=Tex_Load(
		m_pDevice,
		(lg_lpstr)szDefaultTex,
		D3DPOOL_DEFAULT,
		0x00000000,
		LG_FALSE,
		&m_pDefaultTex);
		
	if(!bResult)
	{
		m_pDefaultTex=LG_NULL;
		Err_Printf("Could not load default texture \"%s\"", szDefaultTex);
		MessageBox(0, "No default texture", 0, 0);
	}
		
}
void CLTexMgr::UnInit()
{
	ClearCache();
	L_safe_release(m_pDefaultTex);
	L_safe_release(m_pDevice);
}

IDirect3DTexture9* CLTexMgr::GetTexture(lg_cstr szFilename)
{
	lg_char szName[MAX_TEX_NAME];
	LF_GetFileNameFromPath(szName, szFilename);
	
	TEX_LINK* pLink=m_pFirst;
	while(pLink)
	{
		if(L_strnicmp(szName, pLink->szName, 0))
			return pLink->pTex;
			
		pLink=pLink->pNext;
	}
	return m_pDefaultTex;
}

IDirect3DTexture9* CLTexMgr::LoadTexture(lg_void* pData, lg_dword nSize, lg_bool bForceNoMipMap)
{
	//The texture loading function that laods from memory is a little
	//different because it only loads the texture, it doesn't
	//save a copy of the texture.
	
	IDirect3DTexture9* pNewTex=LG_NULL;
	
	//Attempt to load the texture.
	if(!Tex_Load_Memory(
		m_pDevice,
		pData,
		nSize,
		D3DPOOL_DEFAULT,
		0x00000000,
		bForceNoMipMap,
		&pNewTex))
	{	
		if(m_pDefaultTex)
			m_pDefaultTex->AddRef();
		return m_pDefaultTex;
	}
	
	return pNewTex;
}

IDirect3DTexture9* CLTexMgr::LoadTexture(lg_cstr szFilename, lg_bool bForceNoMipMap)
{
	lg_char szName[MAX_TEX_NAME];
	LF_GetFileNameFromPath(szName, szFilename);
	//First try to get the texture if it already has been loaded.
	IDirect3DTexture9* pNewTex=GetTexture(szFilename);
	if(pNewTex!=m_pDefaultTex)
	{
		pNewTex->AddRef();
		return pNewTex;
	}
	//Then attempt to load the texture.
	if(!Tex_Load(
		m_pDevice,
		(lg_lpstr)szFilename,
		D3DPOOL_DEFAULT,
		0x00000000,
		bForceNoMipMap,
		&pNewTex))
	{
		
		if(m_pDefaultTex)
			m_pDefaultTex->AddRef();
		return m_pDefaultTex;
	}
		
	TEX_LINK* pNewLink=new TEX_LINK;
	//If we're out of memory we have some serious problems.
	if(!pNewLink)
		throw CLError(LG_ERR_OUTOFMEMORY, __FILE__, __LINE__);
		
	pNewLink->pNext=m_pFirst;
	m_pFirst=pNewLink;
	pNewLink->pTex=pNewTex;
	LG_strncpy(pNewLink->szName, szName, MAX_TEX_NAME);
	pNewTex->AddRef();
	return pNewTex;
}

void CLTexMgr::ClearCache()
{
	TEX_LINK* pLink=m_pFirst;
	TEX_LINK* pNext=LG_NULL;
	while(pLink)
	{
		pNext=pLink->pNext;
		L_safe_release(pLink->pTex);
		L_safe_delete(pLink);
		pLink=pNext;
	}
	m_pFirst=LG_NULL;
}