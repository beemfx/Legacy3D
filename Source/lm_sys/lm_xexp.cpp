#include <d3dx9.h>
#include <rmxftmpl.h>
#include <rmxfguid.h>
#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "dxguid.lib")
#include "lm_xexp.h"
#include "lg_malloc.h"


lg_bool CLMeshXExp::SaveAsX(char* szFilename)
{	
	lg_result nResult=0;
	ID3DXFile* pFile=NULL;
	ID3DXFileSaveObject* pFileSave=NULL;
	ID3DXFileSaveData *pFileSaveRoot=NULL;
	try
	{	
		if(LG_FAILED(D3DXFileCreate(&pFile)))
			throw "Could not create ID3DXFile interface.";
			
		nResult=pFile->RegisterTemplates((void*)D3DRM_XTEMPLATES, D3DRM_XTEMPLATE_BYTES);
		if(LG_FAILED(nResult))
			throw "Could not register templates I.";
			
		nResult=pFile->RegisterTemplates((void*)XSKINEXP_TEMPLATES, strlen(XSKINEXP_TEMPLATES));
		if(LG_FAILED(nResult))
			throw "Could not register templates II.";
		
		nResult=pFile->CreateSaveObject(szFilename, D3DXF_FILESAVE_TOFILE, D3DXF_FILEFORMAT_TEXT,&pFileSave);
		if(LG_FAILED(nResult))
			throw "Could not create save object.";
			
		nResult = pFileSave->AddDataObject(TID_D3DRMFrame, "Scene_Root", NULL, 0, NULL, &pFileSaveRoot);
		if(LG_FAILED(nResult))
			throw "Could not add the root object.";
			
		if(!AddMesh(pFileSaveRoot))
			throw "Could not add the mesh.";
		
		pFileSave->Save();
		
		L_safe_release(pFileSaveRoot);
		L_safe_release(pFileSave);
		L_safe_release(pFile);
		return LG_TRUE;
	}
	catch(lg_pstr szError)
	{
		MessageBox(NULL, szError, "lmsh X file exporter", MB_ICONERROR);
		L_safe_release(pFileSaveRoot);
		L_safe_release(pFileSave);
		L_safe_release(pFile);
		return LG_FALSE;
	}
}

lg_bool CLMeshXExp::AddMesh(void* SaveRoot)
{
	ID3DXFileSaveData* pFileSaveRoot=(ID3DXFileSaveData*)SaveRoot;
	
	ID3DXFileSaveData* pMesh=NULL;
	ID3DXFileSaveData* pTempXData=NULL;
	lg_byte* pData=NULL;
	//The following pointers are not allocated
	//they point to areas of pData
	ML_VEC3* pVerts=NULL;
	XMESH_INDEX* pIndex=NULL;
	ML_VEC2* pTU=NULL;
	
	try
	{
		lg_result nResult=0;
		//The biggest data structure we will need will be the size of
		//the actuall mesh, so only that much memory must be allocated.
			
		//The first thing we need is to create the base of the mesh.
		lg_dword nVertOffset=4;
		lg_dword nTriCountOffset=nVertOffset+m_nNumVertices*sizeof(ML_VEC3);
		lg_dword nIndexOffset=nTriCountOffset+4;
		lg_dword nSize=nIndexOffset+m_nNumTriangles*sizeof(XMESH_INDEX);
		pData=(lg_byte*)malloc(nSize);
		if(!pData)
			throw "Could not allocate data for exporting.";
				
		ML_VEC3* pVerts=(ML_VEC3*)(pData+nVertOffset);
		XMESH_INDEX* pIndex=(XMESH_INDEX*)(pData+nIndexOffset);
		
		//Copy the vertices
		lg_dword i=0;
		memcpy(pData, &m_nNumVertices, 4);
		for(i=0; i<m_nNumVertices; i++)
		{
			pVerts[i].x=m_pVertices[i].x;
			pVerts[i].y=m_pVertices[i].y;
			pVerts[i].z=m_pVertices[i].z;
		}
		//Copy the indices
		lg_dword j=0;
		memcpy(pData+nTriCountOffset, &m_nNumTriangles, 4);
		for(i=0, j=0; i<m_nNumTriangles; i++, j+=3)
		{
			pIndex[i].nNumVert=3;
			pIndex[i].v[0]=m_pIndexes[j];
			pIndex[i].v[1]=m_pIndexes[j+1];
			pIndex[i].v[2]=m_pIndexes[j+2];
		}
		//Add the base mesh.
		nResult=pFileSaveRoot->AddDataObject(TID_D3DRMMesh, m_szMeshName, NULL, nSize, pData, &pMesh);
		if(LG_FAILED(nResult))
			throw "Could not add base mesh data.";
		//Since the normal structure is basically the same
		//we only need to change pVerts to the normals.
		for(i=0; i<m_nNumVertices; i++)
		{
			pVerts[i].x=m_pVertices[i].nx;
			pVerts[i].y=m_pVertices[i].ny;
			pVerts[i].z=m_pVertices[i].nz;
		}
		nResult=pMesh->AddDataObject(TID_D3DRMMeshNormals, NULL, NULL, nSize, pData, &pTempXData);
		if(LG_FAILED(nResult))
			throw "Could add mesh normals.";
			
		L_safe_release(pTempXData);
		
		//Now we need to copy the tex coord data...
		pTU=(ML_VEC2*)(pData+4);
		for(i=0; i<m_nNumVertices; i++)
		{
			pTU[i].x=m_pVertices[i].tu;
			pTU[i].y=m_pVertices[i].tv;
		}
		//We need to adjust nSize for this one.
		nSize=4+m_nNumVertices*sizeof(ML_VEC2);
		nResult=pMesh->AddDataObject(TID_D3DRMMeshTextureCoords, NULL, NULL, nSize, pData, &pTempXData);
		if(LG_FAILED(nResult))
			throw "Could not add texture coordinates.";
		L_safe_release(pTempXData);
		
		
		L_safe_release(pMesh);
		L_safe_free(pData);
		return LG_TRUE;
	}
	catch (lg_pstr szError)
	{
		MessageBox(NULL, szError, "lmsh X file exporter", MB_ICONERROR);
		L_safe_release(pTempXData);
		L_safe_release(pMesh);	
		L_safe_free(pData);
		return LG_FALSE;
	}
}