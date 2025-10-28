#include <memory.h>
#include <stdio.h>
#include "lm_sys.h"
#include "lg_err.h"

extern "C" L_int Err_Printf(const char* format, ...);


D3DXMATRIX* LM_EulerToMatrix(D3DXMATRIX* pOut, float* pEuler)
{
	D3DXMATRIX MX, MY, MZ;
	D3DXMatrixRotationX(&MX, -pEuler[0]);
	D3DXMatrixRotationY(&MY, -pEuler[1]);
	D3DXMatrixRotationZ(&MZ, -pEuler[2]);
	*pOut=MX*MY*MZ;
	return pOut;
}

//////////////////////////////
///  The Legacy Mesh Class ///
//////////////////////////////

CLegacyMesh::CLegacyMesh()
{
	m_ID=0;
	m_nVersion=0;
	m_nNumVertices=0;
	m_nNumMeshes=0;
	m_nNumMaterials=0;

	m_pVertices=L_null;
	m_pVertexBoneList=L_null;
	m_pMeshes=L_null;
	m_pMaterials=L_null;


	m_bLoaded=L_false;
}

CLegacyMesh::~CLegacyMesh()
{
	this->Unload();
}

L_dword CLegacyMesh::GetNumFrames()
{
	return m_cSkel.GetNumFrames();
}


L_bool CLegacyMesh::Load(void* file, LMESH_CALLBACKS *pcb, char* szModelPath)
{
	L_dword i=0, j=0;
	LMESH_CALLBACKS cb;

	cb.close=(int (__cdecl*)(void*))fclose;
	cb.read=(unsigned int (__cdecl *)(void *,unsigned int,unsigned int,void *))fread;
	cb.seek=(int (__cdecl *)(void *,long,int))fseek;
	cb.tell=(int (__cdecl *)(void *))ftell;

	if(!pcb)
	{
		pcb=&cb;
	}

	Unload();

	//Read the header.
	pcb->read(&m_ID, 4, 1, file);
	pcb->read(&m_nVersion, 4, 1, file);
	pcb->read(&m_nNumVertices, 4, 1, file);
	pcb->read(&m_nNumMeshes, 4, 1, file);
	pcb->read(&m_nNumMaterials, 4, 1, file);
	

	if(m_ID!=LMESH_ID || m_nVersion!=LMESH_VERSION)
	{
		pcb->close(file);
		return L_false;
	}

	m_pVertices=new LMESH_VERTEX[m_nNumVertices];
	m_pMeshes=new LMESH_SET[m_nNumMeshes];
	m_pMaterials=new LMESH_MATERIAL[m_nNumMaterials];
	m_pVertexBoneList=new L_dword[m_nNumVertices];
	if(!m_pVertices || !m_pMeshes || !m_pMaterials || !m_pVertexBoneList)
	{
		L_safe_delete_array(m_pVertexBoneList);
		L_safe_delete_array(m_pVertices);
		L_safe_delete_array(m_pMeshes);
		L_safe_delete_array(m_pMaterials);
		pcb->close(file);
		return L_false;
	}
	
	//Read the vertexes.
	for(i=0; i<m_nNumVertices; i++)
	{
		pcb->read(&m_pVertices[i], sizeof(LMESH_VERTEX), 1, file);
	}
	//The bone list needs to be done a little differently,
	//the bone list needs to point to some names of joints.
	//So that the vertex knows whre the bone is based off of
	//its name, that way it can be compatible with different skeletal
	//animations.
	pcb->read(m_pVertexBoneList, sizeof(L_dword), m_nNumVertices, file);
	//Read the mesh information.
	for(i=0; i<m_nNumMeshes; i++)
	{
		pcb->read(&m_pMeshes[i], sizeof(LMESH_SET), 1, file);
	}
	//Read the material information.
	for(i=0; i<m_nNumMaterials; i++)
	{
		pcb->read(&m_pMaterials[i], 1, sizeof(LMESH_MATERIAL), file);
		//We wamt to convert the texture name to something that
		//the legacy engine can actuall find.  Right now what we do
		//is get the text path, and if it is ".\" then we know that
		//the texture is in the same directory as the model.  So we
		//add the model's path onto that to get what we want.
		char szTemp[LMESH_MAX_PATH];
		L_getfilepath(szTemp, m_pMaterials[i].szTexture);
		if(L_strnicmp(szTemp, ".\\", 2) || L_strnicmp(szTemp, "./", 2))
		{
			L_strncpy(szTemp, szModelPath, LMESH_MAX_PATH);
			L_strncat(szTemp, &m_pMaterials[i].szTexture[2], LMESH_MAX_PATH);
			L_strncpy(m_pMaterials[i].szTexture, szTemp, LMESH_MAX_PATH);
		}
	}

	//CLegacySkeleton::Load will call pcb->close();
	m_cSkel.Load(file, pcb);
	//pcb->close(file);

	m_bLoaded=L_true;
	return L_true;
}

L_bool CLegacyMesh::Unload()
{
	L_dword i=0;
	if(!m_bLoaded)
		return L_true;
	//Deallocate model's memory.
	L_safe_delete_array(this->m_pVertexBoneList);
	L_safe_delete_array(this->m_pMaterials);
	L_safe_delete_array(this->m_pMeshes);
	L_safe_delete_array(this->m_pVertices);
	
	//Unload the skeleton.
	m_cSkel.Unload();

	m_ID=0;
	m_nVersion=0;
	m_nNumVertices=0;
	m_nNumMeshes=0;
	m_nNumMaterials=0;

	m_pVertices=L_null;
	m_pVertexBoneList=L_null;
	m_pMeshes=L_null;
	m_pMaterials=L_null;


	m_bLoaded=L_false;
	
	return L_true;
}

L_bool CLegacyMesh::Save(char* szFilename)
{
	FILE* fout=L_null;
	fout=fopen(szFilename, "wb");
	if(!fout)
		return L_false;

	//Write the file header.
	fwrite(&m_ID, 4, 1, fout);
	fwrite(&m_nVersion, 4, 1, fout);
	fwrite(&m_nNumVertices, 4, 1, fout);
	fwrite(&m_nNumMeshes, 4, 1, fout);
	fwrite(&m_nNumMaterials, 4, 1, fout);
	
	//Write the vertexes.
	L_dword i=0;
	for(i=0; i<m_nNumVertices; i++)
	{
		fwrite(&this->m_pVertices[i], sizeof(LMESH_VERTEX), 1, fout);
	}
	//Write the bone indexes.
	fwrite(this->m_pVertexBoneList, sizeof(L_dword), m_nNumVertices, fout);
	//Write the meses.
	for(i=0; i<m_nNumMeshes; i++)
	{
		fwrite(&this->m_pMeshes[i], sizeof(LMESH_SET), 1, fout);
	}
	//Write the materials.
	for(i=0; i<m_nNumMaterials; i++)
	{
		fwrite(&this->m_pMaterials[i], sizeof(LMESH_MATERIAL), 1, fout);
	}

	//Write the base skeleton.
	//Actually need to save a base skeleton with 
	//the append option.

	fclose(fout);

	return L_true;
}