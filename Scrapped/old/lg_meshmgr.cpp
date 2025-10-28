#include "lf_sys2.h"
#include "lg_meshmgr.h"
#include "lg_err.h"
#include "lg_err_ex.h"
#include "lg_func.h"


CLMeshMgr* CLMeshMgr::s_pMeshMgr=LG_NULL;

CLMeshD3D* CLMeshMgr::GlobalLoadMesh(lf_path szFilename, lg_dword Flags)
{
	if(s_pMeshMgr)
		return s_pMeshMgr->LoadMesh(szFilename, Flags);
	else
	{
		Err_Printf(
			"LG_LoadMesh ERROR: The Mesh Manager has not been initialized, cannot load \"%s\"", 
			szFilename);
		
		return LG_NULL;
	}
}

CLSkel* CLMeshMgr::GlobalLoadSkel(lf_path szFilename, lg_dword Flags)
{
	if(s_pMeshMgr)
		return s_pMeshMgr->LoadSkel(szFilename, Flags);
	else
	{
		Err_Printf(
			"LG_LoadSkel ERROR: The Mesh Manager has not been initialized, cannot load \"%s\"", 
			szFilename);
		
		return LG_NULL;
	}
}

CLMeshMgr::CLMeshMgr(IDirect3DDevice9 *pDevice, lg_dword nMaxMesh, lg_dword nMaxSkel):
	m_nNumMeshes(0),
	m_pFirstMesh(LG_NULL),
	m_nNumSkels(0),
	m_pFirstSkel(LG_NULL)
{
	//If the Mesh Manager already exists, then we have failed, because
	//it should only be initialized once.
	if(s_pMeshMgr)
	{
		throw new CLError(
			LG_ERR_DEFAULT, __FILE__, __LINE__, 
			LG_TEXT("The Mesh Manager can only be initialized once."));
	}
	//To initialze D3D we are going to save a reference to
	//the device for the purpose of loading meshes.
	//This function should be called before any meshes are
	//loaded or the meshes will not be able to get d3d
	//interfaces to render themselves with.  Note that if
	//a null point is passed it effectively uninitializes
	//the device interace, but all meshes that have been
	//created so far will still have references to the
	//device they were created with.
	//L_safe_release(m_pDevice);
	//m_pDevice=pDevice;
	CLMeshD3D::InitMeshDevice(pDevice);
	s_pMeshMgr=this;
}

CLMeshMgr::~CLMeshMgr()
{
	if(!UnloadAll(LMMGR_UNLOAD_FORCE_ALL))
		OutputDebugString("CLMeshMgr::ERROR Was not able to delete all stored items!\n");
	else
		OutputDebugString("CLMeshMgr::NOTE Was able to delete all stored items!\n");
		
	if(m_nNumSkels>0)
		OutputDebugString("CLMeshMgr::ERROR There were skeletons left over!\n");
	if(m_nNumMeshes>0)
		OutputDebugString("CLMeshMgr::ERROR There were meshes left over!\n");
		
	CLMeshD3D::UnInitMeshDevice();
	s_pMeshMgr=LG_NULL;
}

CLMeshD3D* CLMeshMgr::GetMesh(lg_char* szName)
{
	LMESH_LINK* pLink=m_pFirstMesh;
	while(pLink)
	{
		if(L_strnicmp(szName, pLink->szName, LMESH_MAX_NAME))
			return pLink->pMesh;
		pLink=pLink->pNext;
	}
	return LG_NULL;
}


CLMeshD3D* CLMeshMgr::LoadMesh(lf_path szFilename, lg_dword nFlags)
{
	//The first thing we do is make sure that
	//the mesh isn't already loaded.  Note that if two meshes
	//have the same name, even if they are different meshes only one
	//mesh will be loaded, for this reason no two meshes used
	//in a game should have the same name.
	lf_path szTemp;
	CLMeshD3D* pExistingMesh=GetMesh(LF_GetShortNameFromPath(szTemp, szFilename));
	if(pExistingMesh)
	{
		//Err_Printf("CLMeshMgr::LoadMesh WARNING: \"%s\" mesh is already loaded (or if a different mesh is desired the filename may need to be changed).", szTemp);
		Err_Printf("\"%s\" mesh acquired (%s).", szTemp, pExistingMesh->Validate()?"D3D READY":"NO RENDER");
		return pExistingMesh;
	}
	//Allocate memory for the new link in the chain...
	//Note that the new link will be placed at the
	//beginning of the chain, we are simply creating
	//a linked list here.
	LMESH_LINK* pNew=new LMESH_LINK;
	if(!pNew)
	{
		Err_Printf("CLMeshMgr::LoadMesh ERROR: Could not allocate memory for linked list.");
		return LG_NULL;
	}
	
	pNew->pMesh=new CLMeshD3D();
	if(!pNew->pMesh)
	{
		delete pNew;
		Err_Printf("CLMeshMgr::LoadMesh ERROR: Could not allocate memory for CLMeshD3D.");
		return LG_NULL;
	}
	if(!pNew->pMesh->Load(szFilename))
	{
		delete pNew->pMesh;
		delete pNew;
		Err_Printf("CLMeshMgr::LoadMesh ERROR: Could not load \"%s\".", szFilename);
		return LG_NULL;
	}
	//Set the flags for this mesh.
	pNew->nFlags=nFlags;
	//For now the name will simply be the short version of
	//the file without the extensions (in the future this will
	//actually be a name stored in he file itself).
	LF_GetShortNameFromPath(pNew->szName, szFilename);
	pNew->pNext=m_pFirstMesh;
	m_pFirstMesh=pNew;
	m_nNumMeshes++;

	lg_bool bD3D=pNew->pMesh->CreateD3DComponents();
	
	Err_Printf("\"%s\" mesh loaded (%s).", pNew->szName, bD3D?"D3D READY":"NO RENDER");
	return pNew->pMesh;
}

lg_bool CLMeshMgr::UnloadMesh(lg_char* szName)
{
	LMESH_LINK* pNext,* pCur;
	LMESH_LINK* pNewList=LG_NULL;
	lg_bool bFound=LG_FALSE;
	//The easiest way to delete a mesh is to create
	//a new list (which will be in reverse ord)
	//and while so doing eliminate the desired mesh.
	pCur=m_pFirstMesh;
	while(pCur)
	{
		pNext=pCur->pNext;
		if(bFound || !L_strnicmp(pCur->szName, szName, LMESH_MAX_NAME))
		{
			pCur->pNext=pNewList;
			pNewList=pCur;
		}
		else
		{
			OutputDebugString("Removing ");OutputDebugString(pCur->szName);OutputDebugString(" mesh.\n");
			Err_Printf("\"%s\" mesh unloaded.", pCur->szName);
			L_safe_delete(pCur->pMesh);
			delete pCur;
			bFound=LG_TRUE;
			m_nNumMeshes--;
		}
		pCur=pNext;
	}
	m_pFirstMesh=pNewList;
	return bFound;
}

lg_bool CLMeshMgr::UnloadAll(lg_dword nFlags)
{
	lg_bool bResult;
	bResult=UnloadMeshes(nFlags);
	bResult=bResult&&UnloadSkels(nFlags);
	return bResult;
}

lg_bool CLMeshMgr::UnloadMeshes(lg_dword nFlags)
{
	LMESH_LINK* pItem=m_pFirstMesh,* pTemp=LG_NULL;
	LMESH_LINK* pNewList=LG_NULL, *pNewItem=LG_NULL;
	
	while(pItem)
	{
		pTemp=pItem;
		pItem=pTemp->pNext;
		
		//If the model needs to be saved we will put it in the new list.
		if(L_CHECK_FLAG(pTemp->nFlags, LMMGR_LOAD_RETAIN) && !L_CHECK_FLAG(nFlags, LMMGR_UNLOAD_FORCE_ALL))
		{
			pNewItem=pNewList;
			pNewList=pTemp;
			pNewList->pNext=pNewItem;
			continue;
		}
		
	
		OutputDebugString("Removing ");OutputDebugString(pTemp->szName);OutputDebugString(" Mesh.\n");
		Err_Printf("\"%s\" mesh unloaded.", pTemp->szName);
		L_safe_delete(pTemp->pMesh);
		delete pTemp;
		m_nNumMeshes--;

	}
	m_pFirstMesh=pNewList;
	return LG_TRUE;
}

lg_bool CLMeshMgr::UnloadSkels(lg_dword nFlags)
{
	LSKEL_LINK* pItem=m_pFirstSkel,* pTemp=LG_NULL;
	LSKEL_LINK* pNewList=LG_NULL, *pNewItem=LG_NULL;
	
	while(pItem)
	{
		pTemp=pItem;
		pItem=pTemp->pNext;
		
		//If the model needs to be saved we will put it in the new list.
		if(L_CHECK_FLAG(pTemp->nFlags, LMMGR_LOAD_RETAIN) && !L_CHECK_FLAG(nFlags, LMMGR_UNLOAD_FORCE_ALL))
		{
			pNewItem=pNewList;
			pNewList=pTemp;
			pNewList->pNext=pNewItem;
			continue;
		}
		
	
		OutputDebugString("Removing ");OutputDebugString(pTemp->szName);OutputDebugString(" skeleton.\n");
		Err_Printf("\"%s\" skeleton unloaded.", pTemp->szName);
		L_safe_delete(pTemp->pSkel);
		delete pTemp;
		m_nNumSkels--;

	}
	m_pFirstSkel=pNewList;
	return LG_TRUE;
}

lg_bool CLMeshMgr::UnloadSkel(lg_char* szName)
{
	LSKEL_LINK* pNext,* pCur;
	LSKEL_LINK* pNewList=LG_NULL;
	lg_bool bFound=LG_FALSE;
	//The easiest way to delete a mesh is to create
	//a new list (which will be in reverse ord)
	//and while so doing eliminate the desired mesh.
	pCur=m_pFirstSkel;
	while(pCur)
	{
		pNext=pCur->pNext;
		if(bFound || !L_strnicmp(pCur->szName, szName, LMESH_MAX_NAME))
		{
			pCur->pNext=pNewList;
			pNewList=pCur;
		}
		else
		{
			OutputDebugString("Removing ");OutputDebugString(pCur->szName);OutputDebugString(" skeleton.\n");
			Err_Printf("\"%s\" skeleton unloaded.", pCur->szName);
			L_safe_delete(pCur->pSkel);
			delete pCur;
			bFound=LG_TRUE;
			m_nNumMeshes--;
		}
		pCur=pNext;
	}
	m_pFirstSkel=pNewList;
	return bFound;
}

void CLMeshMgr::InvalidateMeshes()
{
	LMESH_LINK* pLink=m_pFirstMesh;
	
	while(pLink)
	{
		pLink->pMesh->Invalidate();
		pLink=pLink->pNext;
	}
}

lg_bool CLMeshMgr::ValidateMeshes()
{
	LMESH_LINK* pLink=m_pFirstMesh;
	lg_bool bResult=LG_TRUE;
	while(pLink)
	{
		bResult=bResult&&pLink->pMesh->Validate();
		pLink=pLink->pNext;
	}
	return bResult;
}


CLSkel* CLMeshMgr::LoadSkel(lf_path szFilename, lg_dword nFlags)
{
	//The first thing we do is make sure that
	//the mesh isn't already loaded.  Note that if two meshes
	//have the same name, even if they are different meshes only one
	//mesh will be loaded, for this reason no two meshes used
	//in a game should have the same name.
	lf_path szTemp;
	CLSkel* pExistingSkel=GetSkel(LF_GetShortNameFromPath(szTemp, szFilename));
	if(pExistingSkel)
	{
		//Err_Printf("CLMeshMgr::LoadSkel WARNING: \"%s\" skeleton is already loaded (or if a different mesh is desired the filename may need to be changed).", szTemp);
		Err_Printf("\"%s\" skeleton acquired.", szTemp);
		return pExistingSkel;
	}	
	//Allocate memory for the new link in the chain...
	//Note that the new link will be placed at the
	//beginning of the chain, we are simply creating
	//a linked list here.
	LSKEL_LINK* pNew=new LSKEL_LINK;
	if(!pNew)
	{
		Err_Printf("CLMeshMgr::LoadSkel ERROR: Could not allocate memory for linked list.");
		return LG_NULL;
	}
	//Load the skel (if it exists).
	pNew->pSkel=new CLSkel();
	if(!pNew->pSkel)
	{
		delete pNew;
		Err_Printf("CLMeshMgr::LoadSkel ERROR: Could not allocate memory for CLSkel.");
		return LG_NULL;
	}
	if(!pNew->pSkel->Load(szFilename))
	{
		delete pNew->pSkel;
		delete pNew;
		Err_Printf("CLMeshMgr::LoadSkel ERROR: Could not load \"%s\".", szFilename);
		return LG_NULL;
	}
	
	//Set the flags for this skeleton.
	pNew->nFlags=nFlags;
	//For now the name will simply be the short version of
	//the file without the extensions (in the future this will
	//actually be a name stored int he file itself).
	LF_GetShortNameFromPath(pNew->szName, szFilename);
	pNew->pNext=m_pFirstSkel;
	m_pFirstSkel=pNew;
	m_nNumSkels++;
	Err_Printf("\"%s\" skeleton loaded.", pNew->szName);
	return pNew->pSkel;
}

CLSkel* CLMeshMgr::GetSkel(lg_char* szName)
{
	LSKEL_LINK* pLink=m_pFirstSkel;
	while(pLink)
	{
		if(L_strnicmp(szName, pLink->szName, LMESH_MAX_NAME))
			return pLink->pSkel;
		pLink=pLink->pNext;
	}
	return LG_NULL;
}