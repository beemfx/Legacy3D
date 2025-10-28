#include "lg_mmgr.h"
#include "lg_err.h"
#include "lg_err_ex.h"
#include "lg_func.h"

CLMMgr* CLMMgr::s_pMMgr=LG_NULL;

CLMeshLG* CLMMgr::MM_LoadMesh(lg_path szFilename, lg_dword nFlags)
{
	return s_pMMgr->LoadMesh(szFilename, nFlags);
}
CLSkelLG* CLMMgr::MM_LoadSkel(lg_path szFilename, lg_dword nFlags)
{
	return s_pMMgr->LoadSkel(szFilename, nFlags);
}

/*Constructor
	PRE: Direct3D must also be initialized,
	and we must decide the maximum number of 
	skeletons and meshes, a mesh manager should not
	have been previously initialized.
	POST: The mesh manager is initialized, and the
	global manager is set.
*/
CLMMgr::CLMMgr(IDirect3DDevice9 *pDevice, lg_dword nMaxMesh, lg_dword nMaxSkel)
	: m_MeshMgr(pDevice, nMaxMesh)
	, m_SkelMgr(nMaxSkel)
{
	//If the Mesh Manager already exists, then we have failed, because
	//it should only be initialized once.
	if(s_pMMgr)
	{
		throw LG_ERROR(LG_ERR_DEFAULT, LG_TEXT("The Mesh Manager can only be initialized once."));
	}
	//Set the global manager
	s_pMMgr=this;
	
	Err_Printf(
		"MMgr: Initialized with %u meshes & %u skels available.", 
		nMaxMesh, nMaxSkel);
}

/* Destructor
	PRE: The mesh manager should be deleted before
	Direct3D is uninitialized.
	POST: Deletes everything associated with the
	manager, no meshes should be used after it is
	deleted, or memory error will occur.
*/
CLMMgr::~CLMMgr()
{
	//Clear the global manager.
	s_pMMgr=LG_NULL;
	//Delete all meshes
	UnloadMeshes(MM_FORCE_UNLOAD);
	UnloadSkels(MM_FORCE_UNLOAD);
	m_MeshMgr.UnloadItems();
	m_SkelMgr.UnloadItems();
}

/* PRE: Initialized
	POST: Returns the mesh if it exists, else returns
	NULL, also sets flags for the mesh.
*/
CLMeshLG* CLMMgr::LoadMesh(lg_path szFilename, lg_dword nFlags)
{
	hm_item mesh=m_MeshMgr.Load(szFilename, nFlags);
	CLMeshLG* pOut=m_MeshMgr.GetInterface(mesh);
	
	//Just in case we loaded the mesh previously without
	//the MM_NOD3D flag, and we want the mesh to be renderable
	//now we'll do a check.
	if(pOut && !LG_CheckFlag(nFlags, MM_NOD3D))
	{
		pOut->MakeD3DReady(pOut->D3DR_FLAG_LOAD_DEF_TEX);
	}
	
	return pOut;
}

/* PRE: Initialized
	POST: Returns the mesh if it exists, else returns
	NULL, also sets flags for the mesh.
*/
CLSkelLG* CLMMgr::LoadSkel(lg_path szFilename, lg_dword nFlags)
{
	hm_item skel=m_SkelMgr.Load(szFilename, nFlags);
	CLSkelLG* pOut=m_SkelMgr.GetInterface(skel);
	return pOut;
}


void CLMMgr::UnloadMeshes(lg_dword nFlags)
{
	m_MeshMgr.UnloadItems();
	#if 0
	for(lg_dword i=0; i<m_nMaxMeshes; i++)
	{
		if(LG_CheckFlag(nFlags, this->MM_FORCE_UNLOAD) || !LG_CheckFlag(m_pMeshMem[i].nFlags, this->MM_RETAIN))
		{
			m_pMeshMem[m_pMeshMem[i].nHashCode].nHashCount--;
			m_pMeshMem[i].nHashCode=MM_EMPTY_NODE;
			m_pMeshMem[i].Mesh.Unload();
			m_pMeshMem[i].nFlags=0;
			m_pMeshMem[i].szName[0]=0;
		}
	}
	#endif
	
}
void CLMMgr::UnloadSkels(lg_dword nFlags)
{
	m_SkelMgr.UnloadItems();
	#if 0
	for(lg_dword i=0; i<m_nMaxSkels; i++)
	{
		if(LG_CheckFlag(nFlags, this->MM_FORCE_UNLOAD) || !LG_CheckFlag(m_pSkelList[i].nFlags, this->MM_RETAIN))
		{
			m_pSkelList[m_pSkelList[i].nHashCode].nHashCount--;
			m_pSkelList[i].nHashCode=MM_EMPTY_NODE;
			m_pSkelList[i].Skel.Unload();
			m_pSkelList[i].nFlags=0;
			m_pSkelList[i].szName[0]=0;
		}
	}
	#endif
}

void CLMMgr::ValidateMeshes()
{
	m_MeshMgr.Validate();
	#if 0
	for(lg_dword i=0; i<m_nMaxMeshes; i++)
	{
		if(m_pMeshMem[i].nHashCode!=MM_EMPTY_NODE)
			m_pMeshMem[i].Mesh.Validate();
	}
	#endif
}

void CLMMgr::InvalidateMeshes()
{
	m_MeshMgr.Invalidate();
	#if 0
	for(lg_dword i=0; i<m_nMaxMeshes; i++)
	{
		if(m_pMeshMem[i].nHashCode!=MM_EMPTY_NODE)
			m_pMeshMem[i].Mesh.Invalidate();
	}
	#endif
}

void CLMMgr::PrintMeshes()
{
	/*
	for(lg_dword i=0; i<m_nMaxMeshes; i++)
	{
		if(m_pMeshMem[i].nHashCode!=MM_EMPTY_NODE)
			Err_Printf("%i \"%s\" %i@%i (%s) (%i)",
				i,
				m_pMeshMem[i].szName,
				m_pMeshMem[i].nHashCode, i,
				m_pMeshMem[i].Mesh.IsD3DReady()?"D3D Ready":"No Render",
				m_pMeshMem[i].nHashCount);
		else
			Err_Printf("%i Empty (%i)", i, m_pMeshMem[i].nHashCount);
	}
	*/
	m_MeshMgr.PrintDebugInfo();
}

void CLMMgr::PrintSkels()
{
	m_SkelMgr.PrintDebugInfo();
	#if 0
	for(lg_dword i=0; i<m_nMaxSkels; i++)
	{
		if(m_pSkelList[i].nHashCode!=MM_EMPTY_NODE)
			Err_Printf("%i \"%s\" %i@%i (%i)",
				i,
				m_pSkelList[i].szName,
				m_pSkelList[i].nHashCode, i,
				m_pSkelList[i].nHashCount);
		else
			Err_Printf("%i Empty (%i)", i, m_pSkelList[i].nHashCount);
	}
	#endif
}


/***********************
*** The Mesh Manager ***
***********************/


CLMeshMgr::CLMeshMgr(IDirect3DDevice9* pDevice, lg_dword nMaxMeshes)
	: CLHashMgr(nMaxMeshes, "MeshMgr")
	, m_pMeshMem(LG_NULL)
	, m_stkMeshes()
{
	//We save a reference to the device,
	//if the device doesn't exist, then we can't
	//create D3D meshes.
	CLMeshLG::LM_Init(pDevice);
	
	//Create and initialize the mesh stack.
	//A list of meshes is stored in a stack, that
	//way when we need a new mesh we can retrive one from
	//the stack without creating a new instance.
	m_pMeshMem=new MeshItem[m_nMaxItems];
	if(!m_pMeshMem)
		throw LG_ERROR(LG_ERR_OUTOFMEMORY, LG_NULL);
	for(lg_dword i=0; i<m_nMaxItems; i++)
	{
		m_stkMeshes.Push(&m_pMeshMem[i]);
	}
	
	
	//The default item is simply nothing for the mesh manager.
	//m_pDefItem=LG_NULL;
	
	Err_Printf(LG_TEXT("MeshMgr: Initialized with %d meshes available."), m_nMaxItems);
}

CLMeshMgr::~CLMeshMgr()
{
	UnloadItems();
	//m_pDefItem=LG_NULL;
	m_stkMeshes.Clear();
	LG_SafeDeleteArray(m_pMeshMem);
	CLMeshLG::LM_Shutdown();
}

MeshItem* CLMeshMgr::DoLoad(lg_path szFilename, lg_dword nFlags)
{
	MeshItem* pOut = (MeshItem*)m_stkMeshes.Pop();
	if(!pOut)
		return LG_NULL;
	
	if(pOut->Mesh.Load(szFilename))
	{
		if(!LG_CheckFlag(CLMMgr::MM_NOD3D, nFlags))
		{
			pOut->Mesh.MakeD3DReady(pOut->Mesh.D3DR_FLAG_LOAD_DEF_TEX);
		}
	}
	else
	{
		m_stkMeshes.Push(pOut);
		pOut=LG_NULL;
	}
	return pOut;
}

void CLMeshMgr::DoDestroy(MeshItem* pItem)
{
	pItem->Mesh.Unload();
	m_stkMeshes.Push(pItem);
}

CLMeshLG* CLMeshMgr::GetInterface(hm_item mesh)
{
	if(mesh==0 || mesh==CLHashMgr::HM_DEFAULT_ITEM)
	{
		return LG_NULL;
	}
	else
	{
		return &m_pItemList[mesh-1].pItem->Mesh;
	}
}

void CLMeshMgr::Validate()
{
	//We start at 1 because the default mesh, is
	//not loaded.
	for(lg_dword i=1; i<m_nMaxItems; i++)
	{
		if(m_pItemList[i].nHashCode!=CLHashMgr::HM_EMPTY_NODE)
		{
			m_pItemList[i].pItem->Mesh.Validate();
		}
	}
}

void CLMeshMgr::Invalidate()
{
	//We start at 1 because the default mesh, is
	//not loaded.
	for(lg_dword i=1; i<m_nMaxItems; i++)
	{
		if(m_pItemList[i].nHashCode!=CLHashMgr::HM_EMPTY_NODE)
		{
			m_pItemList[i].pItem->Mesh.Invalidate();
		}
	}
}


/*******************************
*** Skeleton Manager Methods ***
*******************************/


CLSkelMgr::CLSkelMgr(lg_dword nMaxSkels)
	: CLHashMgr(nMaxSkels, "SkelMgr")
	, m_pSkelMem(LG_NULL)
	, m_stkSkels()
{	
	//Create and initialize the skel stack.
	//A list of skels is stored in a stack, that
	//way when we need a new skel we can retrive one from
	//the stack without creating a new instance.
	m_pSkelMem=new SkelItem[m_nMaxItems];
	if(!m_pSkelMem)
		throw LG_ERROR(LG_ERR_OUTOFMEMORY, LG_NULL);
	for(lg_dword i=0; i<m_nMaxItems; i++)
	{
		m_stkSkels.Push(&m_pSkelMem[i]);
	}
	
	
	//The default item is simply nothing for the skel manager.
	//m_pDefItem=LG_NULL;
	
	Err_Printf(LG_TEXT("SkelMgr: Initialized with %d skels available."), m_nMaxItems);
}

CLSkelMgr::~CLSkelMgr()
{
	CLHashMgr::UnloadItems();
	//m_pDefItem=LG_NULL;
	m_stkSkels.Clear();
	LG_SafeDeleteArray(m_pSkelMem);
}

SkelItem* CLSkelMgr::DoLoad(lg_path szFilename, lg_dword nFlags)
{
	SkelItem* pOut = (SkelItem*)m_stkSkels.Pop();
	if(!pOut)
		return LG_NULL;
	
	if(!pOut->Skel.Load(szFilename))
	{
		m_stkSkels.Push(pOut);
		pOut=LG_NULL;
	}
	
	return pOut;
}

void CLSkelMgr::DoDestroy(SkelItem* pItem)
{
	if(!pItem)
		return;
		
	pItem->Skel.Unload();
	m_stkSkels.Push(pItem);
}

CLSkelLG* CLSkelMgr::GetInterface(hm_item skel)
{
	if(skel==0 || skel==CLHashMgr::HM_DEFAULT_ITEM)
	{
		return LG_NULL;
	}
	else
	{
		return &m_pItemList[skel-1].pItem->Skel;
	}
}
