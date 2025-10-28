#include <memory.h>
#include "lm_mesh.h"

CLMesh2::CLMesh2()
	: CLMBase()
	, m_nID(0)
	, m_nVer(0)
	, m_nNumVerts(0)
	, m_nNumTris(0)
	, m_nNumSubMesh(0)
	, m_nNumMtrs(0)
	, m_nNumBones(0)
	, m_pVerts(LG_NULL)
	, m_pIndexes(LG_NULL)
	, m_pVertBoneList(LG_NULL)
	, m_pBoneNameList(LG_NULL)
	, m_pSubMesh(LG_NULL)
	, m_pMtrs(LG_NULL)
	, m_pMem(LG_NULL)
{
	m_szMeshName[0]=0;
	memset(&m_AABB, 0, sizeof(ML_AABB));
}

CLMesh2::~CLMesh2()
{
	Unload();
}

lg_bool CLMesh2::AllocateMemory()
{
	DeallocateMemory();
	
	lg_dword nSize=0;
	
	lg_dword nVertOffset=nSize;
	nSize+=sizeof(MeshVertex)*m_nNumVerts;
	
	lg_dword nIndexOffset=nSize;
	nSize+=sizeof(lg_word)*m_nNumTris*3;
	
	lg_dword nMtrOffset=nSize;
	nSize+=sizeof(MeshMtr)*m_nNumMtrs;
	
	lg_dword nVertBoneListOffset=nSize;
	nSize+=sizeof(lg_dword)*m_nNumVerts;
	
	lg_dword nBoneNameListOffset=nSize;
	nSize+=sizeof(LMName)*m_nNumBones;
	
	lg_dword nSubMeshOffset=nSize;
	nSize+=sizeof(SubMesh)*m_nNumSubMesh;
	
	m_pMem = new lg_byte[nSize];
	
	if(!m_pMem)
	{
		DeallocateMemory();
		return LG_FALSE;
	}

	memset( m_pMem , 0 , nSize );
	
	m_pVerts=       (MeshVertex*)&m_pMem[nVertOffset];
	m_pIndexes=     (lg_word*)   &m_pMem[nIndexOffset];
	m_pMtrs=        (MeshMtr*)   &m_pMem[nMtrOffset];
	m_pVertBoneList=(lg_dword*)  &m_pMem[nVertBoneListOffset];
	m_pBoneNameList=(LMName*)    &m_pMem[nBoneNameListOffset];
	m_pSubMesh=     (SubMesh*)   &m_pMem[nSubMeshOffset];
	
	
	return LG_TRUE;
}

lg_void CLMesh2::DeallocateMemory()
{
	if(m_pMem)
	{
		delete [] m_pMem;
	}
	
	m_pMem=LG_NULL;
	m_pVerts=LG_NULL;
	m_pIndexes=LG_NULL;
	m_pSubMesh=LG_NULL;
	m_pMtrs=LG_NULL;
	m_pVertBoneList=LG_NULL;
	m_pBoneNameList=LG_NULL;
}
	
lg_bool CLMesh2::Serialize(lg_void* file, ReadWriteFn read_or_write, RW_MODE mode)
{
	//We'll start by reading or writing the header:
	read_or_write(file, &m_nID, 4);
	read_or_write(file, &m_nVer, 4);
	read_or_write(file, m_szMeshName, 32);
	read_or_write(file, &m_nNumVerts, 4);
	read_or_write(file, &m_nNumTris, 4);
	read_or_write(file, &m_nNumSubMesh, 4);
	read_or_write(file, &m_nNumMtrs, 4);
	read_or_write(file, &m_nNumBones, 4);
	
	//We have two possible exits from the serialize
	//method, one if the version or ID wasn't correct
	//and the other if we couldn't allocate memory on
	//a read.
	
	if(m_nID!=LM_MESH_ID || m_nVer!=LM_MESH_VER)
		return LG_FALSE;
		
	//Now if we are loading we need to allocate memory:
	if(mode==RW_READ)
	{
		if(!AllocateMemory())
			return LG_FALSE;
	}
	
	
	//Now we must read or write the rest of the data:
	//Vertexes:
	for(lg_dword i=0; i<m_nNumVerts; i++)
	{
		read_or_write(file, &m_pVerts[i], sizeof(MeshVertex));
	}
	//Triangles:
	for(lg_dword i=0; i<m_nNumTris; i++)
	{
		read_or_write(file, &m_pIndexes[i*3], sizeof(lg_word)*3);
	}
	
	//Bone indexes:
	read_or_write(file, m_pVertBoneList, sizeof(lg_dword)*m_nNumVerts);
	
	
	//Bones:
	for(lg_dword i=0; i<m_nNumBones; i++)
	{
		read_or_write(file, m_pBoneNameList[i], 32);
	}
	//Sub Meshes:
	for(lg_dword i=0; i<m_nNumSubMesh; i++)
	{
		read_or_write(file, m_pSubMesh[i].szName, 32);
		read_or_write(file, &m_pSubMesh[i].nFirstIndex, 3*sizeof(lg_dword));
	}
	//Materials:
	for(lg_dword i=0; i<m_nNumMtrs; i++)
	{
		read_or_write(file, m_pMtrs[i].szName, 32);
		read_or_write(file, m_pMtrs[i].szFile, 260);
	}
	
	//Bounding box:
	read_or_write(file, &m_AABB, 24);

	return LG_TRUE;
}

lg_void CLMesh2::Unload()
{
	//To unload we'll just deallocate memory,
	//and set the ID and version to 0 so that
	//if we load again we won't accidently keep
	//the information and think that the mesh was
	//read properly.
	DeallocateMemory();
	m_nID=0;
	m_nVer=0;
	
	//We'll also set the flags to 0 to clear the
	//LM_FLAG_LOADED setting.
	m_nFlags=0;
}