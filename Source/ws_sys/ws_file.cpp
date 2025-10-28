#include "ws_file.h"
#include "lg_err.h"
#include <string.h>
#include <stdio.h>

#include <windows.h>

CWSFile::CWSFile():
	//Map Header.
	m_nVersion(0),
	m_nFileFlags(0),
	m_nNameCount(0),
	m_nNameOffset(0),
	m_nObjectCount(0),
	m_nObjectOffset(0),
	
	m_nMatCount(0),
	m_nLMCount(0),
	m_nLMMaxSize(0),
	m_nBrushCount(0),
	m_nFaceCount(0),
	m_nVisibleFaceCount(0),
	m_nVertexCount(0),
	m_nVisGroupCount(0),
	m_nGeoVertCount(0),
	
	m_szNames(0),
	m_pVertexes(0),
	m_pMaterials(0),
	m_pLMs(0),
	m_pVisGrps(0),
	m_pRasterFaces(0),
	m_pGeoBlocks(0),
	m_pGeoFaces(0),
	m_pLights(0)
{

}

CWSFile::~CWSFile()
{
	DeallocateMemory();
}

lg_bool CWSFile::Load(lg_byte* pData, lg_dword nSize)
{
	Err_Printf("Loading 3D World Studio file...");
	m_File.Open(pData, nSize);
	
	try
	{
	
	//First check the map header to make sure it is a valid map...
	if((m_File.Read(&m_nVersion, 2)!=2) || (m_nVersion!=14) )
	{
		throw("WSToLW ERROR: File is not a valid 3D World Studio Map.");
	}
	//Now read the rest of the header...
	m_File.Read(&m_nFileFlags, 1);
	m_File.Read(&m_nNameCount, 4);
	m_File.Read(&m_nNameOffset, 4);
	m_File.Read(&m_nObjectCount, 4);
	m_File.Read(&m_nObjectOffset, 4);
	
	Err_Printf("Flags: 0x%02X", m_nFileFlags);
	Err_Printf("%d names at %d", m_nNameCount, m_nNameOffset);
	Err_Printf("%d objects at %d", m_nObjectCount, m_nObjectOffset);
	
	GetNames();
	
	GetCounts();
	
	Err_Printf("Materials: %d", m_nMatCount);
	Err_Printf("Light Maps: %d", m_nLMCount);
	Err_Printf("Biggest LM: %d", m_nLMMaxSize);
	Err_Printf("Brushes: %d", m_nBrushCount);
	Err_Printf("Faces: %d", m_nFaceCount);
	Err_Printf("Visible Faces: %d", m_nVisibleFaceCount);
	Err_Printf("Vertices: %d", m_nVertexCount);
	Err_Printf("Polys: %d", m_nVertexCount/3);
	Err_Printf("Vis Groups: %d", m_nVisGroupCount);
	Err_Printf("Lights: %d", m_nLightCount);
	Err_Printf("Geo Verts: %d", m_nGeoVertCount);
	Err_Printf("Geo tris: %d", m_nGeoTriCount);
	
	AllocateMemory();
	
	GetObjects();
	
	ModifyRefsAndVertexOrder();
	SortAndCompactRasterObjects();
	SortAndCompactEntitys();
	SortAndCompactGeoObjects();
	
	throw("OK");
	}
	catch(lg_char* szMessage)
	{
		Err_Printf("3D World Studio load result: %s", szMessage);
		if(strcmp(szMessage, "OK")==0)
			return LG_TRUE;
	}
	return LG_FALSE;
}

void CWSFile::ModifyRefsAndVertexOrder()
{
	//Reverse the vertex orders
	for(lg_dword i=0; i<m_nVertexCount; i+=3)
	{
		WS_VERTEX vt=m_pVertexes[i];
		m_pVertexes[i]=m_pVertexes[i+2];
		m_pVertexes[i+2]=vt;
	}
	
	//Reverse geo vertex order
	for(lg_dword i=0; i<m_nGeoTriCount; i++)
	{
		ml_vec3 v3Temp;
		v3Temp=m_pGeoTris[i].v3Vert[0];
		m_pGeoTris[i].v3Vert[0]=m_pGeoTris[i].v3Vert[2];
		m_pGeoTris[i].v3Vert[2]=v3Temp;
	}
	
	//Change the geo block vis group references.
	for(lg_dword i=0; i<m_nBrushCount; i++)
	{
		for(lg_dword j=0; j<m_nVisGroupCount; j++)
		{
			if(m_pGeoBlocks[i].nVisGrp==m_pVisGrps[j].nRef)
			{
				m_pGeoBlocks[i].nVisGrp=j;
				break;
			}
		}
	}
	
	//Change references for material, lightmap, and visgroups.
	for(lg_dword i=0; i<m_nFaceCount; i++)
	{
		for(lg_dword j=0; j<m_nMatCount; j++)
		{
			if(m_pRasterFaces[i].nMatRef==m_pMaterials[j].nRef)
			{
				m_pRasterFaces[i].nMatRef=j;
				break;
			}
		}
		
		for(lg_dword j=0; j<m_nLMCount; j++)
		{
			if(m_pRasterFaces[i].nLMRef==m_pLMs[j].nRef)
			{
				m_pRasterFaces[i].nLMRef=j;
				break;
			}
		}
		
		for(lg_dword j=0; j<m_nVisGroupCount; j++)
		{
			if(m_pRasterFaces[i].nVisGrp==m_pVisGrps[j].nRef)
			{
				m_pRasterFaces[i].nVisGrp=j;
				break;
			}
		}
	}
	
	//Change the light references
	for(lg_dword i=0; i<m_nLightCount; i++)
	{
		for(lg_dword j=0; j<m_nVisGroupCount; j++)
		{
			if(m_pLights[i].nVisGrp==m_pVisGrps[j].nRef)
			{
				m_pLights[i].nVisGrp=j;
				break;
			}
		}
	}
	
	
	//Change the sign of the plane equation d value (nevermind)
	for(lg_dword i=0; i<m_nFaceCount; i++)
	{
		//m_pGeoFaces[i].plnEq.d=-m_pGeoFaces[i].plnEq.d;
	}
}

void CWSFile::SortAndCompactGeoObjects()
{
	lg_dword nBlock=0, nVisGrp=0;
	
	WS_GEO_BLOCK* pTempBlock=new WS_GEO_BLOCK[m_nBrushCount];
	if(!pTempBlock)
		throw("ERROR: Out of memory.");
		
	memcpy(pTempBlock, m_pGeoBlocks, sizeof(WS_GEO_BLOCK)*m_nBrushCount);
	
	for(nVisGrp=0; nVisGrp<m_nVisGroupCount; nVisGrp++)
	{
		m_pVisGrps[nVisGrp].nFirstGeoBlock=0;
		m_pVisGrps[nVisGrp].nGeoBlockCount=0;
	}
	
	nVisGrp=0;
	
	do
	{		
		for(lg_dword i=0; i<m_nBrushCount; i++)
		{
			if(nBlock>=m_nBrushCount)
				break;
				
			if(pTempBlock[i].nVisGrp!=nVisGrp)
				continue;
				
			
			//If all the criteria are met add the block to the list.
			m_pGeoBlocks[nBlock]=pTempBlock[i];
			nBlock++;
			m_pVisGrps[nVisGrp].nGeoBlockCount++;		
		}
		
		nVisGrp++;
		if(nVisGrp>=m_nVisGroupCount)
			break;
			
		m_pVisGrps[nVisGrp].nFirstGeoBlock=nBlock;
		m_pVisGrps[nVisGrp].nGeoBlockCount=0;
	}while(nBlock<m_nBrushCount);
		
	L_safe_delete_array(pTempBlock);
	
	//Calculate region bounds.
	for(nVisGrp=0; nVisGrp<m_nVisGroupCount; nVisGrp++)
	{
		ML_AABB aabbTemp;
		aabbTemp.v3Min=m_pVertexes[m_pVisGrps[nVisGrp].nFirstVertex].v3Pos;
		aabbTemp.v3Max=aabbTemp.v3Min;
		if(nVisGrp==0)
			m_aabbWorld=aabbTemp;
		
		for(lg_dword i=1; i<m_pVisGrps[nVisGrp].nVertexCount; i++)
		{
			ML_AABBAddPoint(&aabbTemp, &aabbTemp, &m_pVertexes[i+m_pVisGrps[nVisGrp].nFirstVertex].v3Pos);
			ML_AABBAddPoint(&m_aabbWorld, &m_aabbWorld, &m_pVertexes[i+m_pVisGrps[nVisGrp].nFirstVertex].v3Pos);
		}
		m_pVisGrps[nVisGrp].aabbVisBounds=aabbTemp;
	}
	#if 0
	for(lg_dword i=0; i<m_nVisGroupCount; i++)
	{		
		lg_dword nStart=m_pVisGrps[i].nFirstGeoBlock;
		lg_dword nCount=m_pVisGrps[i].nGeoBlockCount;
		lg_dword nEnd=nStart+nCount;
		
		Err_Printf("Vis Group %d: %s S: %d C: %d", i, m_pVisGrps[i].szName, nStart, nCount);
		
		for(lg_dword i=nStart; i<nEnd; i++)
		{
			lg_dword nStart=m_pGeoBlocks[i].nFirstGeoFace;
			lg_dword nCount=m_pGeoBlocks[i].nGeoFaceCount;
			lg_dword nEnd=nStart+nCount;
		
			Err_Printf("   Geo Block: %d: F: %d C: %d VisGrp: %d", 
				i, 
				nStart,
				nCount,
				m_pGeoBlocks[i].nVisGrp);
				
				for(lg_dword i=nStart; i<nEnd; i++)
				{
					Err_Printf("      Geo Face %d: (%f, %f, %f, %f)", 
						i, 
						m_pGeoFaces[i].v4PlaneEq.x,
						m_pGeoFaces[i].v4PlaneEq.y,
						m_pGeoFaces[i].v4PlaneEq.z,
						m_pGeoFaces[i].v4PlaneEq.w);
				}
		}
	}
	#endif
}

void CWSFile::SortAndCompactEntitys()
{
	lg_dword nVisGrp=0, nLight=0;
	
	//Sort the lights...
	WS_LIGHT* pTempLights=new WS_LIGHT[m_nLightCount];
	if(!pTempLights)
		throw("ERROR: Out of memory.");
		
	memcpy(pTempLights, m_pLights, sizeof(WS_LIGHT)*m_nLightCount);
	
	for(nVisGrp=0; nVisGrp<m_nVisGroupCount; nVisGrp++)
	{
		m_pVisGrps[nVisGrp].nFirstLight=0;
		m_pVisGrps[nVisGrp].nLightCount=0;
	}
	
	nVisGrp=0;
	nLight=0;
	do
	{	
		for(lg_dword i=0; i<m_nLightCount; i++)
		{
			if(pTempLights[i].nVisGrp!=nVisGrp)
				continue;
				
			if(nLight>=m_nLightCount)
				break;
				
			m_pLights[nLight++]=pTempLights[i];
			m_pVisGrps[nVisGrp].nLightCount++;
		}
		nVisGrp++;
		if(nVisGrp>=m_nVisGroupCount)
			break;
		m_pVisGrps[nVisGrp].nFirstLight=nLight;
		m_pVisGrps[nVisGrp].nLightCount=0;
	}while(nLight<m_nLightCount);
	
	#if 0
	for(lg_dword i=0; i<m_nLightCount; i++)
	{
		WS_LIGHT lt=m_pLights[i];
		Err_Printf("===LIGHT===");
		Err_Printf("Position: (%f, %f, %f)", lt.v3Pos.x, lt.v3Pos.y, lt.v3Pos.z);
		Err_Printf("Color: 0x%08X", lt.Color);
		Err_Printf("Intensity: %f", lt.fIntensity);
		Err_Printf("VisGrp: %u", lt.nVisGrp);
	}
	#endif
	
	L_safe_delete_array(pTempLights);
}

void CWSFile::SortAndCompactRasterObjects()
{
	lg_dword nFace=0, nVisGrp=0, nMaterial=0, nLightmap=0;
	
	WS_RASTER_FACE* pTempFaces=new WS_RASTER_FACE[m_nFaceCount];
	if(!pTempFaces)
		throw("ERROR: Out of memory.");
	
	//Store all the faces in the temporary array...
	memcpy(pTempFaces, m_pRasterFaces, sizeof(WS_RASTER_FACE)*m_nFaceCount);
	//Sort the data into the original array..
	
	for(nVisGrp=0; nVisGrp<m_nVisGroupCount; nVisGrp++)
	{
		m_pVisGrps[nVisGrp].nFirstFace=0;
		m_pVisGrps[nVisGrp].nFaceCount=0;
		m_pVisGrps[nVisGrp].nVisibleFaceCount=0;
		m_pVisGrps[nVisGrp].nFirstVertex=0;
		m_pVisGrps[nVisGrp].nVertexCount=0;
		m_pVisGrps[nVisGrp].nFirstLight=0;
		m_pVisGrps[nVisGrp].nLightCount=0;
	}
	
	nVisGrp=0;
		
	do
	{		
		for(lg_dword i=0; i<m_nFaceCount; i++)
		{
			if(nFace>=m_nVisibleFaceCount)
				break;
			
			if(!pTempFaces[i].bVisible)
				continue;
				
			if(pTempFaces[i].nVisGrp!=nVisGrp)
				continue;
				
			if(pTempFaces[i].nMatRef!=nMaterial)
				continue;
			
			if(pTempFaces[i].nLMRef!=nLightmap)
				continue;
			
			//If all the criteria are met add the face to the list.
			m_pRasterFaces[nFace]=pTempFaces[i];
			nFace++;
			m_pVisGrps[nVisGrp].nFaceCount++;
			m_pVisGrps[nVisGrp].nVisibleFaceCount++;
				
		}
		
		if(nLightmap==0xFFFFFFFF)
		{
			nLightmap=0;
			nMaterial++;
			if(nMaterial>=m_nMatCount)
			{
				nMaterial=0;
				nVisGrp++;
				if(nVisGrp>=m_nVisGroupCount)
					break;
					
				m_pVisGrps[nVisGrp].nFirstFace=nFace;
				m_pVisGrps[nVisGrp].nFaceCount=0;
				m_pVisGrps[nVisGrp].nVisibleFaceCount=0;
			}
		}
		else
			nLightmap++;
		
		if((nLightmap>=m_nLMCount) && (nLightmap!=0xFFFFFFFF))
		{
			nLightmap=0xFFFFFFFF;
		}
		
	}while(nFace<m_nVisibleFaceCount);
	
	
	for(lg_dword i=nFace; i<m_nFaceCount; i++)
	{
		memset(&m_pRasterFaces[i], 0, sizeof(WS_RASTER_FACE));
	}

	//Now we will resort all the vertexes...
	//The vertexes are being sorted in the order of the faces,
	//that way faces that have the same texture and lightmap
	//can be catenated.
	WS_VERTEX* pTempVerts=new WS_VERTEX[m_nVertexCount];
	if(!pTempVerts)
	{
		L_safe_delete_array(pTempVerts);
		throw("ERROR: Out of memory.");
	}
		
	memcpy(pTempVerts, m_pVertexes, sizeof(WS_VERTEX)*m_nVertexCount);
	
	for(lg_dword nFace=0, nVertex=0; nFace<m_nVisibleFaceCount; nFace++)
	{
		//Simply copy the faces vertexes to the destination buffer,
		//then set the new first vertex, and advance to the next vertex
		//to be written to.
		memcpy(
			&m_pVertexes[nVertex], 
			&pTempVerts[m_pRasterFaces[nFace].nFirst], 
			sizeof(WS_VERTEX)*m_pRasterFaces[nFace].nNumTriangles*3);
			
		m_pRasterFaces[nFace].nFirst=nVertex;
		nVertex+=m_pRasterFaces[nFace].nNumTriangles*3;
	}
		
	L_safe_delete_array(pTempVerts);
	
	//We'll now reduce the number of faces into segments (each segment will
	//be composed of triangles that have the same texture and lightmap).
	for(lg_dword nVisGrp=0; nVisGrp<m_nVisGroupCount; nVisGrp++)
	{
		lg_dword nLastMat=0, nLastLM=0;
		WS_RASTER_FACE* pCurFace=LG_NULL;
		lg_dword nStart=m_pVisGrps[nVisGrp].nFirstFace;
		lg_dword nEnd=m_pVisGrps[nVisGrp].nFirstFace+m_pVisGrps[nVisGrp].nFaceCount;
		
		for(lg_dword i=nStart; i<nEnd; i++)
		{
			if(!pCurFace)
			{
				pCurFace=&m_pRasterFaces[i];
				nLastMat=pCurFace->nMatRef;
				nLastLM=pCurFace->nLMRef;
				continue;
			}
			
			if((nLastMat==m_pRasterFaces[i].nMatRef) && (nLastLM==m_pRasterFaces[i].nLMRef))
			{
				pCurFace->nNumTriangles+=m_pRasterFaces[i].nNumTriangles;
				m_pRasterFaces[i].nNumTriangles=0;
			}
			else
			{
				pCurFace=&m_pRasterFaces[i];
				nLastMat=pCurFace->nMatRef;
				nLastLM=pCurFace->nLMRef;
			}
		}
	}
	
	//We'll now eliminate all the empty faces...
	memcpy(pTempFaces, m_pRasterFaces, sizeof(WS_RASTER_FACE)*m_nVisibleFaceCount);
	
	nFace=0;
	for(lg_dword nVisGrp=0; nVisGrp<m_nVisGroupCount; nVisGrp++)
	{		
		lg_dword nStart=m_pVisGrps[nVisGrp].nFirstFace;
		lg_dword nEnd=m_pVisGrps[nVisGrp].nFirstFace+m_pVisGrps[nVisGrp].nFaceCount;
		
		m_pVisGrps[nVisGrp].nFirstFace=nFace;
		m_pVisGrps[nVisGrp].nFaceCount=0;
		for(lg_dword i=nStart; i<nEnd; i++)
		{
			if(pTempFaces[i].nNumTriangles>0)
			{
				m_pRasterFaces[nFace]=pTempFaces[i];
				m_pVisGrps[nVisGrp].nFaceCount++;
				nFace++;
			}
		}
	}
	
	Err_Printf("NOTICE: %d faces reduced to %d segments %0.2f%%.",  
		m_nVisibleFaceCount,
		nFace,
		(float)nFace/(float)m_nVisibleFaceCount*100.0f);
	
	m_nVisibleFaceCount=nFace;
	
	L_safe_delete_array(pTempFaces);
	
	//We'll now setup some more information about the vis groups
	for(lg_dword i=0; i<m_nVisGroupCount; i++)
	{
		m_pVisGrps[i].nFirstVertex=m_pRasterFaces[m_pVisGrps[i].nFirstFace].nFirst;
		
		lg_dword nCount=0;
		for(lg_dword j=0; j<m_pVisGrps[i].nFaceCount; j++)
		{
			nCount+=m_pRasterFaces[m_pVisGrps[i].nFirstFace+j].nNumTriangles*3;
		}
		m_pVisGrps[i].nVertexCount=nCount;
	}

	#if 0
	//For testing purposes print out the results
	for(lg_dword i=0; i<m_nVisGroupCount; i++)
	{
		Err_Printf(
			"Vis Group %d: %s (%d) FirstFace=%d FaceCount=%d VisibleFaces=%d First Vertex: %d Vertex Count: %d", 
			i, 
			m_pVisGrps[i].szName, 
			m_pVisGrps[i].nRef, 
			m_pVisGrps[i].nFirstFace, 
			m_pVisGrps[i].nFaceCount,
			m_pVisGrps[i].nVisibleFaceCount,
			m_pVisGrps[i].nFirstVertex,
			m_pVisGrps[i].nVertexCount);
			
		for(lg_dword j=0; j<m_pVisGrps[i].nFaceCount; j++)
		{
			lg_dword offset=j+m_pVisGrps[i].nFirstFace;
			
			Err_Printf("   Face %d: S: %d: C: %d: Mat: %d: LM: %d: Room: %d\n",
				offset,
				m_pRasterFaces[offset].nFirst,
				m_pRasterFaces[offset].nNumTriangles,
				m_pRasterFaces[offset].nMatRef,
				m_pRasterFaces[offset].nLMRef,
				m_pRasterFaces[offset].nVisGrp);
		}

	}
	#endif
}

void CWSFile::ScaleMap(lg_float fScale)
{
	ML_MAT matTrans;
	ML_MatScaling(&matTrans, fScale, fScale, fScale);
	
	ML_Vec3TransformCoord(
		&m_aabbWorld.v3Min,
		&m_aabbWorld.v3Min,
		&matTrans);
		
	ML_Vec3TransformCoord(
		&m_aabbWorld.v3Max,
		&m_aabbWorld.v3Max,
		&matTrans);
	
	ML_Vec3TransformCoordArray(
		(ML_VEC3*)m_pVertexes, 
		sizeof(WS_VERTEX), 
		(ML_VEC3*)m_pVertexes, 
		sizeof(WS_VERTEX), 
		&matTrans, 
		m_nVertexCount);
	
	ML_Vec3TransformCoordArray(
		(ML_VEC3*)&m_pGeoBlocks[0].aabbBlock.v3Max,
		sizeof(WS_GEO_BLOCK),
		(ML_VEC3*)&m_pGeoBlocks[0].aabbBlock.v3Max,
		sizeof(WS_GEO_BLOCK),
		&matTrans,
		m_nBrushCount);
		
	ML_Vec3TransformCoordArray(
		(ML_VEC3*)&m_pGeoBlocks[0].aabbBlock.v3Min,
		sizeof(WS_GEO_BLOCK),
		(ML_VEC3*)&m_pGeoBlocks[0].aabbBlock.v3Min,
		sizeof(WS_GEO_BLOCK),
		&matTrans,
		m_nBrushCount);
		
	ML_Vec3TransformCoordArray(
		(ML_VEC3*)&m_pVisGrps[0].aabbVisBounds.v3Min,
		sizeof(WS_VISGROUP),
		(ML_VEC3*)&m_pVisGrps[0].aabbVisBounds.v3Min,
		sizeof(WS_VISGROUP),
		&matTrans,
		m_nVisGroupCount);
		
	ML_Vec3TransformCoordArray(
		(ML_VEC3*)&m_pVisGrps[0].aabbVisBounds.v3Max,
		sizeof(WS_VISGROUP),
		(ML_VEC3*)&m_pVisGrps[0].aabbVisBounds.v3Max,
		sizeof(WS_VISGROUP),
		&matTrans,
		m_nVisGroupCount);
		
	ML_Vec3TransformCoordArray(
		(ML_VEC3*)&m_pLights[0].v3Pos,
		sizeof(WS_LIGHT),
		(ML_VEC3*)&m_pLights[0].v3Pos,
		sizeof(WS_LIGHT),
		&matTrans,
		m_nLightCount);
		
	ML_Vec3TransformCoordArray(
		&m_pGeoVerts[0].v3Pos,
		sizeof(WS_GEO_VERT),
		&m_pGeoVerts[0].v3Pos,
		sizeof(WS_GEO_VERT),
		&matTrans,
		m_nGeoVertCount);
		
	ML_Vec3TransformCoordArray(
		&m_pGeoTris[0].v3Vert[0],
		sizeof(ML_VEC3),
		&m_pGeoTris[0].v3Vert[0],
		sizeof(ML_VEC3),
		&matTrans,
		m_nGeoTriCount*3);
	
	for(lg_dword i=0; i<m_nFaceCount; i++)
	{
		m_pGeoFaces[i].plnEq.d*=fScale;
	}
}

WS_COLOR CWSFile::StringToColor(lg_str szColor)
{
	WS_COLOR c;
	c.r=atoi(szColor);
	for(lg_dword i=0; i<strlen(szColor); i++)
	{
		if(szColor[i]==' ')
		{
			szColor=&szColor[i+1];
			break;
		}
	}
	c.g=atoi(szColor);
	for(lg_dword i=0; i<strlen(szColor); i++)
	{
		if(szColor[i]==' ')
		{
			szColor=&szColor[i+1];
			break;
		}
	}
	c.b=atoi(szColor);
	return c;
}

lg_bool CWSFile::GetLight(WS_LIGHT* lt)
{
	memset(lt, 0, sizeof(WS_LIGHT));
	lg_bool bIsLight=LG_FALSE;
	lg_byte Flags;
	m_File.Read(&Flags, 1);
	m_File.Read(&lt->v3Pos, 12);
	lg_long nKeys;
	m_File.Read(&nKeys, 4);
	for(lg_long i=0; i<nKeys; i++)
	{
		lg_dword nKeyName;
		lg_dword nKeyValue;
		
		m_File.Read(&nKeyName, 4);
		m_File.Read(&nKeyValue, 4);
		
		if(strcmp(m_szNames[nKeyName], "classname")==0)
		{
			if(
				(strcmp(m_szNames[nKeyValue], "light")!=0) &&
				(strcmp(m_szNames[nKeyValue], "spotlight")!=0) &&
				(strcmp(m_szNames[nKeyValue], "sunlight")!=0)
			)
			{
				return LG_FALSE;
			}
			else
				bIsLight=LG_TRUE;
		}
		else if(strcmp(m_szNames[nKeyName], "color")==0)
		{
			lt->Color=StringToColor(m_szNames[nKeyValue]);
			
		}
		else if(strcmp(m_szNames[nKeyName], "intensity")==0)
		{
			lt->fIntensity=(lg_float)atof(m_szNames[nKeyValue]);
		}
		/*
		else if(strcmp(m_szNames[nKeyName], "range")==0)
		{
			lt->nRange=atoi(m_szNames[nKeyValue]);
		}
		*/
	}
	if(!bIsLight)
		return LG_FALSE;
		
	//Skip the group;
	m_File.Seek(CWSMemFile::MEM_SEEK_CUR, 4);
	m_File.Read(&lt->nVisGrp, 4);
	
	return LG_TRUE;
}

void CWSFile::GetObjects()
{	
	// Get the materials and count the number of vertices
	m_File.Seek(CWSMemFile::MEM_SEEK_SET, m_nObjectOffset);
	lg_dword nMat=0, nVert=0, nBrush=0, nLM=0, nFace=0, nVisGrp=0;
	lg_dword nGeoBlock=0, nGeoFace=0, nLight=0, nGeoVert=0, nGeoTri=0;
	
	for(lg_long nObj=1; nObj<=m_nObjectCount; nObj++)
	{
		lg_dword nName;
		lg_long nSize;
		m_File.Read(&nName, 4);
		m_File.Read(&nSize, 4);
		
		lg_dword nBegin=m_File.Tell();
		
		if(strcmp(m_szNames[nName], "material")==0)
		{
			if(nMat>=m_nMatCount)
			{
				m_File.Seek(CWSMemFile::MEM_SEEK_CUR, nSize);
				continue;
			}
			lg_byte nFlags;
			lg_dword nGroup;
			lg_dword nObject;
			lg_dword nExt;
			
			m_File.Read(&nFlags, 1);
			m_File.Read(&nGroup, 4);
			m_File.Read(&nObject, 4);
			if(nFlags&0x02)
				m_File.Read(&nExt, 4);
			const lg_char* szExt=(nFlags&0x02)?m_szNames[nExt]:"tga";
			
			if(strcmp(m_szNames[nGroup], "textures")==0)
				_snprintf(m_pMaterials[nMat].szMaterial, 255, "%s/%s.%s", m_szNames[nGroup], m_szNames[nObject], szExt);
			else	
				_snprintf(m_pMaterials[nMat].szMaterial, 255, "textures/%s/%s.%s", m_szNames[nGroup], m_szNames[nObject], szExt);
			
			m_pMaterials[nMat].nRef=nObj;
			
			//Err_Printf("Material %d: %s", nMat, m_pMaterials[nMat].szMaterial);
			nMat++;
		}
		else if(strcmp(m_szNames[nName], "entity")==0)
		{
			WS_LIGHT lt;
			lg_dword nStart=m_File.Tell();
			if(GetLight(&lt) && (nLight<m_nLightCount))
			{
				if(nLight>=m_nLightCount)
					break;
				//Err_Printf("===LIGHT===");
				//Err_Printf("Position: (%f, %f, %f)", lt.v3Pos.x, lt.v3Pos.y, lt.v3Pos.z);
				//Err_Printf("Color: 0x%08X", lt.Color);
				//Err_Printf("Intensity: %f", lt.fIntensity);
				//Err_Printf("VisGrp: %u", lt.nVisGrp);
				
				m_pLights[nLight++]=lt;
			}
			/*else if(GetAIThingy())
			{
			}
			*/
			m_File.Seek(CWSMemFile::MEM_SEEK_SET, nStart+nSize);
		}
		else if(strcmp(m_szNames[nName], "visgroup")==0)
		{
			if(nVisGrp>=m_nVisGroupCount)
			{
				m_File.Seek(CWSMemFile::MEM_SEEK_CUR, nSize);
				continue;
			}
			lg_dword nName;
			//Skip the flags
			m_File.Seek(CWSMemFile::MEM_SEEK_CUR, 1);
			m_File.Read(&nName, 4);
			//Skip the color
			m_File.Seek(CWSMemFile::MEM_SEEK_CUR, 3);
			
			m_pVisGrps[nVisGrp].nRef=nObj;
			strncpy(m_pVisGrps[nVisGrp].szName, m_szNames[nName], WS_MAX_NAME-1);
			m_pVisGrps[nVisGrp].szName[WS_MAX_NAME-1]=0;
			//Err_Printf("Vis Group %d: %s", nVisGrp, m_pVisGrps[nVisGrp].szName);
			nVisGrp++;
			
		}
		else if(strcmp(m_szNames[nName], "lightmap")==0)
		{
			//We'll just read the lightmap data, and do the conversion
			//to a tga when we write the lw file.
			if(nLM>=m_nLMCount)
			{
				m_File.Seek(CWSMemFile::MEM_SEEK_CUR, nSize);
				continue;
			}
			m_pLMs[nLM].nRef=nObj;
			m_pLMs[nLM].nSize=nSize;
			m_pLMs[nLM].pLM=m_File.GetPointer(m_File.Tell());
			nLM++;
		}
		else if(strcmp(m_szNames[nName], "brush")==0)
		{
			if(nVert>=m_nVertexCount)
			{
				m_File.Seek(CWSMemFile::MEM_SEEK_CUR, nSize);
				continue;
			}
			lg_byte nFlags;
			lg_long nKeys;
			lg_long nGrp;
			lg_long nVisGrp;
			
			lg_byte nVertexCount;
			lg_byte nFaceCount;
			
			m_File.Read(&nFlags, 1);
			m_File.Read(&nKeys, 4);
			m_File.Seek(CWSMemFile::MEM_SEEK_CUR, nKeys*8);
			m_File.Read(&nGrp, 4);
			m_File.Read(&nVisGrp, 4);
			//Skip the color
			m_File.Seek(CWSMemFile::MEM_SEEK_CUR, 3);
			
			m_File.Read(&nVertexCount, 1);
			ML_VEC3* pVerts=new ML_VEC3[nVertexCount];
			if(!pVerts)
				throw("WSToLW ERROR: Out of memory.");
				
			for(lg_byte i=0; i<nVertexCount; i++)
			{
				m_File.Read(&pVerts[i], 12);
				memcpy(&m_pGeoVerts[nGeoVert+i], &pVerts[i], 12);
			}
			m_File.Read(&nFaceCount, 1);
			
			//Set the geo information...
			m_pGeoBlocks[nGeoBlock].nFirstVertex=nGeoVert;
			m_pGeoBlocks[nGeoBlock].nVertexCount=nVertexCount;
			m_pGeoBlocks[nGeoBlock].nFirstGeoFace=nGeoFace;
			m_pGeoBlocks[nGeoBlock].nGeoFaceCount=0;
			m_pGeoBlocks[nGeoBlock].nVisGrp=nVisGrp;
			m_pGeoBlocks[nGeoBlock].nFirstTri=nGeoTri;
			m_pGeoBlocks[nGeoBlock].nNumTris=0;
			
			ML_AABBFromVec3s(&m_pGeoBlocks[nGeoBlock].aabbBlock, pVerts, nVertexCount);
			nGeoBlock++;
			nGeoVert+=nVertexCount;
				
			for(lg_byte i=0; i<nFaceCount; i++)
			{
				lg_byte nFaceFlags;
				ML_PLANE v4PlaneEq;
				lg_long nMaterial;
				lg_long nLightMap;
				
				m_File.Read(&nFaceFlags, 1);
				m_File.Read(&v4PlaneEq, 16);
				m_File.Seek(CWSMemFile::MEM_SEEK_CUR, 64);
				m_File.Read(&nMaterial, 4);
				
				//Set the geo face information (and update the current geo block).
				m_pGeoFaces[nGeoFace].plnEq=v4PlaneEq;
				nGeoFace++;
				m_pGeoBlocks[nGeoBlock-1].nGeoFaceCount++;
				
				if(nFaceFlags&16)
					m_File.Read(&nLightMap, 4);
				else
					nLightMap=0xFFFFFFFF;
					
				lg_byte nIndexCount;
				m_File.Read(&nIndexCount, 1);
				
				m_pGeoBlocks[nGeoBlock-1].nNumTris+=(nIndexCount-2);
				
				m_pRasterFaces[nFace].bVisible=!(nFaceFlags&0x04);
				m_pRasterFaces[nFace].nFirst=nVert;
				m_pRasterFaces[nFace].nMatRef=nMaterial;
				m_pRasterFaces[nFace].nLMRef=nLightMap;
				//Note need to change this...
				m_pRasterFaces[nFace].nVisGrp=nVisGrp;
				m_pRasterFaces[nFace].nNumTriangles=(nIndexCount-2);
				//m_pRasterFaces[nFace].v4PlaneEq=v4PlaneEq;
				//m_pRasterFaces[nFace].nOpFlags=0;
				nFace++;

				for(lg_byte i=0; i<nIndexCount; i++)
				{
					lg_byte nV;
					ML_VEC2 v2Tex, v2LM;
					
					m_File.Read(&nV, 1);
					m_pVertexes[nVert].v3Pos=pVerts[nV];
					m_File.Read(&v2Tex, 8);
					if(nFaceFlags&16)
					{
						m_File.Read(&v2LM, 8);
					}
					else
					{
						v2LM.x=0;
						v2LM.y=0;
					}
					
					//Save triangles for raster data.
					if(i<3)
					{
						m_pGeoTris[nGeoTri].v3Vert[i]=pVerts[nV];
						if(i==2)
							nGeoTri++;
					}
					else
					{
						m_pGeoTris[nGeoTri].v3Vert[0]=m_pGeoTris[nGeoTri-1].v3Vert[0];
						m_pGeoTris[nGeoTri].v3Vert[1]=m_pGeoTris[nGeoTri-1].v3Vert[2];
						m_pGeoTris[nGeoTri].v3Vert[2]=pVerts[nV];
						nGeoTri++;
					}
					
					//If the face isn't visible we don't
					//need to save it to the raster data.
					if(L_CHECK_FLAG(nFaceFlags, 0x04) || (nVert>=m_nVertexCount))
						continue;
						
					//The first three vertexes in a face are
					//the first three vertexes read, after that the
					//vertexes are in a fan, and therefore it is
					//necessary to set the first vertex to the first
					//vertex of the last triangle, and the second vertex
					//to the last vertex of the previous triangle, and
					//the new vertex as the new vertex.  In this way a
					//triangle list is created.
					if(i<3)
					{
						m_pVertexes[nVert].v3Pos=pVerts[nV];
						m_pVertexes[nVert].v2Tex=v2Tex;
						m_pVertexes[nVert].v2LM=v2LM;

					}
					else
					{
						m_pVertexes[nVert]=m_pVertexes[nVert-3];
						m_pVertexes[nVert+1]=m_pVertexes[nVert-1];
						nVert+=2;
						
						m_pVertexes[nVert].v3Pos=pVerts[nV];
						m_pVertexes[nVert].v2Tex=v2Tex;
						m_pVertexes[nVert].v2LM=v2LM;
						
					}
					nVert++;
				}
			}
			L_safe_delete_array(pVerts);
		}
		m_File.Seek(CWSMemFile::MEM_SEEK_SET, nBegin+nSize);
	}
	
	Err_Printf("Got %d geo tris", nGeoTri);
}

void CWSFile::AllocateMemory()
{
	//Allocate everything but the name table,
	//because the names have already been allocated.
	m_pVertexes=new WS_VERTEX[m_nVertexCount+3];
	m_pMaterials=new WS_MATERIAL[m_nMatCount];
	m_pLMs=new WS_LIGHTMAP[m_nLMCount];
	m_pVisGrps=new WS_VISGROUP[m_nVisGroupCount];
	m_pRasterFaces=new WS_RASTER_FACE[m_nFaceCount];
	m_pGeoBlocks=new WS_GEO_BLOCK[m_nBrushCount];
	m_pGeoFaces=new WS_GEO_FACE[m_nFaceCount];
	m_pLights=new WS_LIGHT[m_nLightCount];
	m_pGeoVerts=new WS_GEO_VERT[m_nGeoVertCount];
	m_pGeoTris=new WS_GEO_TRI[m_nGeoTriCount];
	
	if(!m_pGeoTris || !m_pGeoVerts || !m_pVertexes || !m_pMaterials || !m_pLMs || !m_pVisGrps || !m_pRasterFaces  || !m_pGeoFaces || !m_pGeoBlocks || !m_pLights)
	{
		DeallocateMemory();
		throw("CWSFile ERROR: Out of memory.");
	}
	
}
void CWSFile::DeallocateMemory()
{
	L_safe_delete_array(m_szNames);
	L_safe_delete_array(m_pVertexes);
	L_safe_delete_array(m_pMaterials);
	L_safe_delete_array(m_pLMs);
	L_safe_delete_array(m_pVisGrps);
	L_safe_delete_array(m_pRasterFaces);
	L_safe_delete_array(m_pGeoFaces);
	L_safe_delete_array(m_pGeoBlocks);
	L_safe_delete_array(m_pLights);
	L_safe_delete_array(m_pGeoVerts);
	L_safe_delete_array(m_pGeoTris);
}

void CWSFile::GetCounts()
{
	m_nMatCount=0;
	m_nLMCount=0;
	m_nLMMaxSize=0;
	m_nBrushCount=0;
	m_nFaceCount=0;
	m_nVisibleFaceCount=0;
	m_nVertexCount=0;
	m_nVisGroupCount=0;
	m_nLightCount=0;
	m_nGeoVertCount=0;
	m_nGeoTriCount=0;
	
	//Count all the objects
	for(lg_long i=1; i<=m_nObjectCount; i++)
	{		
		lg_dword nName;
		lg_long nSize;
		m_File.Read(&nName, 4);
		m_File.Read(&nSize, 4);
		
		lg_dword nBegin=m_File.Tell();
		
		//Err_Printf("Ojbect %d (%d): %s", i, nSize, m_szNames[nName]);
		if(strcmp(m_szNames[nName], "material")==0)
			m_nMatCount++;
		if(strcmp(m_szNames[nName], "visgroup")==0)
			m_nVisGroupCount++;
		if(strcmp(m_szNames[nName], "entity")==0)
		{
			//Err_Printf("===ENTITY===");
			lg_byte Flags;
			m_File.Read(&Flags, 1);
			//Err_Printf("Flags: 0x%02X", Flags);
			//ML_VEC3 v3Pos;
			//m_File.Read(&v3Pos, 12);
			//Err_Printf("Position: (%f, %f, %f)", v3Pos.x, v3Pos.y, v3Pos.z);
			m_File.Seek(CWSMemFile::MEM_SEEK_CUR, 12);
			lg_long nKeys;
			m_File.Read(&nKeys, 4);
			for(lg_long i=0; i<nKeys; i++)
			{
				lg_dword nKeyName, nKeyValue;
				m_File.Read(&nKeyName, 4);
				m_File.Read(&nKeyValue, 4);
				if(strcmp(m_szNames[nKeyName], "classname")==0)
				{
					if( 
						(strcmp(m_szNames[nKeyValue], "light")==0) ||
						(strcmp(m_szNames[nKeyValue], "spotlight")==0) ||
						(strcmp(m_szNames[nKeyValue], "sunlight")==0)
					)
					{
						m_nLightCount++;
					}
				}
				//Err_Printf("%s: \"%s\"", m_szNames[nKeyName], m_szNames[nKeyValue]);
			}
			//Err_Printf("============");	
		}
		if(strcmp(m_szNames[nName], "lightmap")==0)
		{
			m_nLMCount++;
			m_nLMMaxSize=LG_Max(m_nLMMaxSize, (lg_dword)nSize);
		}
		if(strcmp(m_szNames[nName], "brush")==0)
		{
			m_nBrushCount++;
			
			lg_byte nFlags;
			lg_long nKeys;
			
			lg_byte nVertexCount;
			lg_byte nFaceCount;
			
			m_File.Read(&nFlags, 1);
			m_File.Read(&nKeys, 4);
			m_File.Seek(CWSMemFile::MEM_SEEK_CUR, nKeys*8);
			m_File.Seek(CWSMemFile::MEM_SEEK_CUR, 4+4+3);
			
			m_File.Read(&nVertexCount, 1);
			m_nGeoVertCount+=nVertexCount;
			m_File.Seek(CWSMemFile::MEM_SEEK_CUR, 12*nVertexCount);
			m_File.Read(&nFaceCount, 1);
			m_nFaceCount+=nFaceCount;
			//m_nGeoTriCount+=nFaceCount;
			for(lg_byte i=0; i<nFaceCount; i++)
			{
				lg_byte nFaceFlags;
				
				m_File.Read(&nFaceFlags, 1);
				m_File.Seek(CWSMemFile::MEM_SEEK_CUR, 84);
				if(nFaceFlags&16)
					m_File.Seek(CWSMemFile::MEM_SEEK_CUR, 4);
					
				lg_byte nIndexCount;
				m_File.Read(&nIndexCount, 1);
				
				m_nGeoTriCount+=(nIndexCount-2);	
				
				if(!(nFaceFlags&0x04))
				{
					m_nVertexCount+=3+(nIndexCount-3)*3;
					m_nVisibleFaceCount++;
				}
				
				if(nFaceFlags&16)
					m_File.Seek(CWSMemFile::MEM_SEEK_CUR, nIndexCount*17);
				else
					m_File.Seek(CWSMemFile::MEM_SEEK_CUR, nIndexCount*9);
			}
		}
		
		m_File.Seek(CWSMemFile::MEM_SEEK_SET, nBegin+nSize);
	}
}


void CWSFile::GetNames()
{
	m_szNames=new lg_char*[this->m_nNameCount+1];
	if(!m_szNames)
		throw("CWSFile ERROR: Out of memory.");
		
	m_szNames[0]=LG_NULL;
	
	lg_long nNamesFound=1;
	lg_byte* pData=m_File.GetPointer(m_nNameOffset);
	
	m_szNames[1]=(lg_char*)&pData[0];
	for(lg_dword i=0; i<m_File.GetSize()-m_nNameOffset; i++)
	{
		if(pData[i]==0)
		{
			nNamesFound++;
			m_szNames[nNamesFound]=(lg_char*)&pData[i+1];
		}
		
		if(nNamesFound>=m_nNameCount)
			return;
	}
}

#include "lf_sys2.h"
#include "lw_map.h"

lg_bool CWSFile::SaveAsLW(lg_char* szFilename)
{
	LF_FILE3 fout=LF_Open(szFilename, LF_ACCESS_WRITE, LF_CREATE_ALWAYS);
	if(!fout)
		return LG_FALSE;
		
	//Calculate the whole world AABB
		
		//Now write the map...
	lg_word nID=LW_ID;
	lg_dword nVersion=LW_VERSION;
	lg_dword nVertexCount=m_nVertexCount;
	lg_dword nVertexOffset=0;
	lg_dword nMaterialCount=m_nMatCount;
	lg_dword nMaterialOffset=0;
	lg_dword nLMCount=m_nLMCount;
	lg_dword nLMOffset=0;
	lg_dword nLMTotalSize=0;
	lg_dword nFaceCount=m_nVisibleFaceCount;
	lg_dword nFaceOffset=0;
	lg_dword nRegionCount=m_nVisGroupCount;
	lg_dword nRegionOffset=0;
	lg_dword nGeoBlockCount=m_nBrushCount;
	lg_dword nGeoBlockOffset=0;
	lg_dword nGeoFaceCount=m_nFaceCount;
	lg_dword nGeoFaceOffset=0;
	lg_dword nLightCount=m_nLightCount;
	lg_dword nLightOffset=0;
	lg_dword nGeoVertOffset=0;
	lg_dword nGeoVertCount=m_nGeoVertCount;
	lg_dword nGeoTriOffset=0;
	lg_dword nGeoTriCount=m_nGeoTriCount;
	
	
	LF_Write(fout, &nID, 2);
	LF_Write(fout, &nVersion, 4);
	LF_Write(fout, &nVertexCount, 4);
	LF_Write(fout, &nVertexOffset, 4);
	LF_Write(fout, &nMaterialCount, 4);
	LF_Write(fout, &nMaterialOffset, 4);
	LF_Write(fout, &nLMCount, 4);
	LF_Write(fout, &nLMOffset, 4);
	LF_Write(fout, &nLMTotalSize, 4);
	LF_Write(fout, &nFaceCount, 4);
	LF_Write(fout, &nFaceOffset, 4);
	LF_Write(fout, &nRegionCount, 4);
	LF_Write(fout, &nRegionOffset, 4);
	LF_Write(fout, &nGeoBlockCount, 4);
	LF_Write(fout, &nGeoBlockOffset, 4);
	LF_Write(fout, &nGeoFaceCount, 4);
	LF_Write(fout, &nGeoFaceOffset, 4);
	LF_Write(fout, &nLightCount, 4);
	LF_Write(fout, &nLightOffset, 4);
	LF_Write(fout, &nGeoVertCount, 4);
	LF_Write(fout, &nGeoVertOffset, 4);
	LF_Write(fout, &nGeoTriCount, 4);
	LF_Write(fout, &nGeoTriOffset, 4);
	LF_Write(fout, &m_aabbWorld, 24);
	
	
	//We can simply write the vertexes
	nVertexOffset=LF_Tell(fout);
	for(lg_dword i=0; i<nVertexCount; i++)
	{
		LF_Write(fout, &m_pVertexes[i], 28);
	}
	//we can do the same for the materials
	nMaterialOffset=LF_Tell(fout);
	for(lg_dword i=0; i<nMaterialCount; i++)
	{
		LF_Write(fout, &m_pMaterials[i].szMaterial, WS_MAX_PATH);
	}
	//Light maps are a little more complicate, we need to convert each lightmap
	//to a tga file, and then write the tga file.
	nLMOffset=LF_Tell(fout);
	//Write each of the light maps
	for(lg_dword i=0; i<nLMCount; i++)
	{
		lg_byte* pLM=m_pLMs[i].pLM;//&pLMData[4*counts.nLightMapCount+i*counts.nLMSize];
		lg_word nRes=(lg_word)ML_Pow2(pLM[1]);
		lg_byte nHeader[18];
		lg_dword nImgDataSize=nRes*nRes*3;
		
		lg_dword nSize=18+nImgDataSize;
		//Start off by writing the size, then write the image data...
		LF_Write(fout, &nSize, 4);
		nLMTotalSize+=4;
		
		memset(nHeader, 0, 18);
		//Setup the tga header...
		nHeader[2]=2;
		*(lg_word*)(&nHeader[12])=nRes;
		*(lg_word*)(&nHeader[14])=nRes;
		nHeader[16]=24;
		nHeader[17]=0x20;
		//Write the header...
		LF_Write(fout, nHeader, 18);
		nLMTotalSize+=18;
		//Write the image data...
		LF_Write(fout, &pLM[6], nImgDataSize);
		nLMTotalSize+=nImgDataSize;
	}
	
	//We'll write only visible faces...
	nFaceOffset=LF_Tell(fout);
	for(lg_dword i=0; i<nFaceCount; i++)
	{
		LF_Write(fout, &m_pRasterFaces[i].nFirst, 4);
		LF_Write(fout, &m_pRasterFaces[i].nNumTriangles, 4);
		LF_Write(fout, &m_pRasterFaces[i].nMatRef, 4);
		LF_Write(fout, &m_pRasterFaces[i].nLMRef, 4);
	}
	
	//Write the regions...
	nRegionOffset=LF_Tell(fout);
	for(lg_dword i=0; i<nRegionCount; i++)
	{
		LF_Write(fout, &m_pVisGrps[i].szName, 32);
		LF_Write(fout, &m_pVisGrps[i].nFirstFace, 4);
		LF_Write(fout, &m_pVisGrps[i].nFaceCount, 4);
		LF_Write(fout, &m_pVisGrps[i].nFirstVertex, 4);
		LF_Write(fout, &m_pVisGrps[i].nVertexCount, 4);
		LF_Write(fout, &m_pVisGrps[i].nFirstGeoBlock, 4);
		LF_Write(fout, &m_pVisGrps[i].nGeoBlockCount, 4);
		LF_Write(fout, &m_pVisGrps[i].nFirstLight, 4);
		LF_Write(fout, &m_pVisGrps[i].nLightCount, 4);
		LF_Write(fout, &m_pVisGrps[i].aabbVisBounds, 24);
	}
	
	//Write the geo blocks...
	nGeoBlockOffset=LF_Tell(fout);
	for(lg_dword i=0; i<nGeoBlockCount; i++)
	{
		LF_Write(fout, &m_pGeoBlocks[i].nFirstVertex, 4);
		LF_Write(fout, &m_pGeoBlocks[i].nVertexCount, 4);
		LF_Write(fout, &m_pGeoBlocks[i].nFirstTri, 4);
		LF_Write(fout, &m_pGeoBlocks[i].nNumTris, 4);
		LF_Write(fout, &m_pGeoBlocks[i].nFirstGeoFace, 4);
		LF_Write(fout, &m_pGeoBlocks[i].nGeoFaceCount, 4);
		LF_Write(fout, &m_pGeoBlocks[i].aabbBlock, 24);
	}
	
	//Write the geo faces...
	nGeoFaceOffset=LF_Tell(fout);
	for(lg_dword i=0; i<nGeoFaceCount; i++)
	{
		LF_Write(fout, &m_pGeoFaces[i].plnEq, 16);
	}
	
	//Write the geo vertexes...
	nGeoVertOffset=LF_Tell(fout);
	for(lg_dword i=0; i<nGeoVertCount; i++)
	{
		LF_Write(fout, &m_pGeoVerts[i].v3Pos, 12);
	}
	
	//Write the lights
	nLightOffset=LF_Tell(fout);
	for(lg_dword i=0; i<nLightCount; i++)
	{
		LF_Write(fout, &m_pLights[i].v3Pos, 12);
		//Write the color
		lg_float c=m_pLights[i].Color.r/255.0f;
		LF_Write(fout, &c, 4);
		c=m_pLights[i].Color.g/255.0f;
		LF_Write(fout, &c, 4);
		c=m_pLights[i].Color.b/255.0f;
		LF_Write(fout, &c, 4);
		c=1.0f;
		LF_Write(fout, &c, 4);
	}
	
	nGeoTriOffset=LF_Tell(fout);
	for(lg_dword i=0; i<nGeoTriCount; i++)
	{
		LF_Write(fout, &m_pGeoTris[i], 12*3);
	}
	
	//Err_Printf("Light Map Size: %d", nLMTotalSize);
	LF_Seek(fout, LF_SEEK_BEGIN, 10);
	LF_Write(fout, &nVertexOffset, 4);
	LF_Seek(fout, LF_SEEK_CURRENT, 4);
	LF_Write(fout, &nMaterialOffset, 4);
	LF_Seek(fout, LF_SEEK_CURRENT, 4);
	LF_Write(fout, &nLMOffset, 4);
	LF_Write(fout, &nLMTotalSize, 4);
	LF_Seek(fout, LF_SEEK_CURRENT, 4);
	LF_Write(fout, &nFaceOffset, 4);
	LF_Seek(fout, LF_SEEK_CURRENT, 4);
	LF_Write(fout, &nRegionOffset, 4);
	LF_Seek(fout, LF_SEEK_CURRENT, 4);
	LF_Write(fout, &nGeoBlockOffset, 4);
	LF_Seek(fout, LF_SEEK_CURRENT, 4);
	LF_Write(fout, &nGeoFaceOffset, 4);
	LF_Seek(fout, LF_SEEK_CURRENT, 4);
	LF_Write(fout, &nLightOffset, 4);
	LF_Seek(fout, LF_SEEK_CURRENT, 4);
	LF_Write(fout, &nGeoVertOffset, 4);
	LF_Seek(fout, LF_SEEK_CURRENT, 4);
	LF_Write(fout, &nGeoTriOffset, 4);
	LF_Close(fout);
	
	return LG_TRUE;
}