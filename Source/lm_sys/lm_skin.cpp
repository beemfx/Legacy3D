#include "lm_skin.h"
#include "lx_sys.h"
#include "lg_malloc.h"
#include "lg_err.h"

CLSkin::CLSkin()
	: CLMBase()
	, m_nMtrCount(0)
	, m_nGrpCount(0)
	, m_pMaterials(LG_NULL)
	, m_pGrps(LG_NULL)
	, m_pMem(LG_NULL)
{
}

CLSkin::~CLSkin()
{
	Unload();
}

lg_bool CLSkin::Load(lg_path szFilename)
{
	//Make sure the skin is loaded
	Unload();
	lg_bool bLoaded=LX_LoadSkin(szFilename, this);
	m_nFlags=bLoaded?LM_FLAG_LOADED:0;
	#if 0
	Err_Printf("SknMgr: Loaded \"%s\".", szFilename);
	Err_Printf("   Materials: %d", m_nMtrCount);
	for(lg_dword i=0; i<m_nMtrCount; i++)
	{
		Err_Printf("   Material[%d]: %d", i+1, m_pMaterials[i]);
	}
	Err_Printf("   Groups: %d", m_nGrpCount);
	for(lg_dword i=0; i<m_nGrpCount; i++)
	{
		Err_Printf("   Group[%d]: %s, %d", i, this->m_pGrps[i].szName, m_pGrps[i].nMtr);
	}
	#endif
	return bLoaded;
}

void CLSkin::Unload()
{
	//We've got to zero out everything.
	m_nMtrCount=0;
	m_nGrpCount=0;
	m_nFlags=0;
	m_pMaterials=LG_NULL;
	m_pGrps=LG_NULL;
	//Memory was allocated using LG_Malloc not new.
	LG_SafeFree(m_pMem);
}

void CLSkin::MakeCompatibleWithMesh(CLMesh2* pMesh)
{
	//If the files aren't loaded we can't do anything.
	if(!pMesh || !pMesh->IsLoaded() || !LG_CheckFlag(m_nFlags, LM_FLAG_LOADED))
		return;
		
	lg_dword nLoopCount=LG_Min(m_nGrpCount, pMesh->m_nNumSubMesh);
	
	//Loop through each of the groups (meshes in the mesh file)
	//and set up the references.
	for(lg_dword i=0; i<nLoopCount; i++)
	{
		//We'll set it to 0 by default (we should probably set it to
		//something that represents null), that way if we don't
		//find anything, it will at least match something.
		
		//Loop through each of the meshes to find a matching one.
		for(lg_dword j=0; j<nLoopCount; j++)
		{
			//If we found a matching group we set the compatible array.
			if(stricmp(m_pGrps[i].szName, pMesh->m_pSubMesh[j].szName)==0)
			{
				m_pCmpRefs[j]=m_pMaterials[m_pGrps[i].nMtr-1];
				//Err_Printf("%s is %d=%d", pMesh->m_pMeshes[j].szName, m_pCmpRefs[j], m_pGrps[i].nMtr-1);
				
				//Set j high to break out of the loop
				j=pMesh->m_nNumSubMesh+1;
			}
		}
	}
	
	#if 0
	for(lg_dword i=0; i<pMesh->m_nNumMeshes; i++)
	{
		if(m_pCmpRefs[i]==0)
			Err_Printf("%s IS NOT COMPATIBLE.", pMesh->m_pMeshes[i].szName);
	}
	#endif
}

lg_bool CLSkin::Serialize(lg_void *file, ReadWriteFn read_or_write, RW_MODE mode)
{
	return LG_FALSE;
}
