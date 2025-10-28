#include "lw_sys.h"
#include "lg_err.h"
#include <stdio.h>

typedef struct _WSToLW_COUNTS{
	lg_dword nMaterialCount;
	lg_dword nLightMapCount;
	lg_dword nLMSize;
	lg_dword nBrushCount;
	lg_dword nFaceCount;
	lg_dword nVisibleFaceCount;
	lg_dword nTotalVertexCount;
	lg_dword nVisGroupCount;
}WSToLW_COUNTS;

typedef struct _WS_MATERIAL{
	LW_MATERIAL  szMaterial;
	lg_dword     nReference;
}WS_MATERIAL;

typedef struct _WS_VISGROUP{
	LW_NAME  szName;
	lg_dword nRef;
	
	lg_dword nFirstFace;
	lg_dword nFaceCount;
	lg_dword nVisibleFaceCount;
	
	lg_dword nFirstVertex;
	lg_dword nVertexCount;
}WS_VISGROUP;


typedef struct _WS_RASTER_FACE{
	lg_bool  bVisible;
	lg_dword nFirst;
	lg_dword nNumTriangles;
	
	lg_dword nMatRef;
	lg_dword nLMRef;
	
	lg_dword nRoom;
	
	ML_VEC4  v4PlaneEq;
	lg_dword nOpFlags;
}WS_RASTER_FACE;

#define WS_RASTER_FACE_COPIED 0x00000001

void WSToLW_SortData(WS_RASTER_FACE* pFaces, WS_VISGROUP* pVisGrp, LW_VERTEX* pVerts, WSToLW_COUNTS* pCounts);

void WSToLW_GetNameTable(lg_char** szNames, lg_byte* pData, lg_dword nSize, lg_long nNumNames);
void WSToLW_GetCounts(LF_FILE3 file, WSToLW_COUNTS* pCounts, lg_char** szNames, lg_long nObjectCount);

lg_bool WSToLW(lf_path szFilenameIn, lf_path szFilenameOut)
{
	//Map Header.
	lg_word nVersion;
	lg_byte nFlags;
	lg_long nNameCount;
	lg_long nNameOffset;
	lg_long nObjectCount;
	lg_long nObjectOffset;
	
	LF_FILE3 fileMap=LG_NULL;
	lg_char** szNames=LG_NULL;
	WS_MATERIAL* pMaterials=LG_NULL;
	LW_VERTEX*   pVertexes=LG_NULL;
	lg_byte*     pLMData=LG_NULL;
	WS_RASTER_FACE*     pFaces=LG_NULL;
	WS_VISGROUP* pVisGroups=LG_NULL;
	
	lg_byte* pData;
	
	try
	{
	fileMap=LF_Open(szFilenameIn, LF_ACCESS_READ|LF_ACCESS_MEMORY, LF_OPEN_EXISTING);
	if(!fileMap)
	{
		throw("WSToLW ERROR: Could not open file.");
	}
	
	//First check the map header to make sure it is a valid map...
	if((LF_Read(fileMap, &nVersion, 2)!=2) || (nVersion!=14) )
	{
		throw("WSToLW ERROR: File is not a valid 3D World Studio Map.");
	}
	
	pData=(lg_byte*)LF_GetMemPointer(fileMap);
	
	//Now read the rest of the header...
	LF_Read(fileMap, &nFlags, 1);
	LF_Read(fileMap, &nNameCount, 4);
	LF_Read(fileMap, &nNameOffset, 4);
	LF_Read(fileMap, &nObjectCount, 4);
	LF_Read(fileMap, &nObjectOffset, 4);
	
	//Now read the name table...
	szNames=new lg_char*[nNameCount+1];
	if(!szNames)
	{
		throw("WSToLW ERROR: Out of memory.");
	}
	
	WSToLW_GetNameTable(szNames, &pData[nNameOffset], LF_GetSize(fileMap)-nNameOffset, nNameCount);
	
	/*
	for(lg_dword i=0; i<=nNameCount; i++)
	{
		Err_Printf("Name %d: %s", i, szNames[i]);
	}
	*/
	
	//Get all the object counts...
	WSToLW_COUNTS counts;
	LF_Seek(fileMap, LF_SEEK_BEGIN, nObjectOffset);
	WSToLW_GetCounts(fileMap, &counts, szNames, nObjectCount);
	
	#if 1
	Err_Printf("Materials: %d", counts.nMaterialCount);
	Err_Printf("Light Maps: %d", counts.nLightMapCount);
	Err_Printf("Biggest LM: %d", counts.nLMSize);
	Err_Printf("Brushes: %d", counts.nBrushCount);
	Err_Printf("Faces: %d", counts.nFaceCount);
	Err_Printf("Visible Faces: %d", counts.nVisibleFaceCount);
	Err_Printf("Vertices: %d", counts.nTotalVertexCount);
	Err_Printf("Polys: %d", counts.nTotalVertexCount/3);
	Err_Printf("Vis Groups: %d", counts.nVisGroupCount);
	#endif
	//Get all the objects.
	pMaterials=new WS_MATERIAL[counts.nMaterialCount];
	pVertexes=new LW_VERTEX[counts.nTotalVertexCount+3]; //Add room for 3 more vertexes for processing.
	pLMData=new lg_byte[counts.nLightMapCount*(counts.nLMSize+4)];
	pVisGroups=new WS_VISGROUP[counts.nVisGroupCount];
	pFaces=new WS_RASTER_FACE[counts.nFaceCount];
	
	if(!pMaterials || !pVertexes || !pLMData || !pFaces)
		throw("WSToLW ERROR: Out of memory.");
		
	// Get the materials and count the number of vertices
	LF_Seek(fileMap, LF_SEEK_BEGIN, nObjectOffset);
	for(lg_dword i=1, nMat=0, nVert=0, nBrush=0, nLM=0, nFace=0, nVisGrp=0; i<=nObjectCount; i++)
	{
		lg_dword nName;
		lg_long nSize;
		LF_Read(fileMap, &nName, 4);
		LF_Read(fileMap, &nSize, 4);
		
		if(strcmp(szNames[nName], "material")==0)
		{
			if(nMat>=counts.nMaterialCount)
			{
				LF_Seek(fileMap, LF_SEEK_CURRENT, nSize);
				continue;
			}
			lg_byte nFlags;
			lg_dword nGroup;
			lg_dword nObject;
			lg_dword nExt;
			
			LF_Read(fileMap, &nFlags, 1);
			LF_Read(fileMap, &nGroup, 4);
			LF_Read(fileMap, &nObject, 4);
			if(nFlags&0x02)
				LF_Read(fileMap, &nExt, 4);
			if(strcmp(szNames[nGroup], "textures")==0)
				_snprintf(pMaterials[nMat].szMaterial, 255, "%s/%s.%s", szNames[nGroup], szNames[nObject], (nFlags&0x02)?szNames[nExt]:"tga");
			else	
				_snprintf(pMaterials[nMat].szMaterial, 255, "textures/%s/%s.%s", szNames[nGroup], szNames[nObject], (nFlags&0x02)?szNames[nExt]:"tga");
			pMaterials[nMat].nReference=i;
			nMat++;
		}
		else if(strcmp(szNames[nName], "visgroup")==0)
		{
			if(nVisGrp>=counts.nVisGroupCount)
			{
				LF_Seek(fileMap, LF_SEEK_CURRENT, nSize);
				continue;
			}
			lg_dword nName;
			//Skip the flags
			LF_Seek(fileMap, LF_SEEK_CURRENT, 1);
			LF_Read(fileMap, &nName, 4);
			//Skip the color
			LF_Seek(fileMap, LF_SEEK_CURRENT, 3);
			
			pVisGroups[nVisGrp].nRef=i;
			strncpy(pVisGroups[nVisGrp].szName, szNames[nName], LW_MAX_NAME-1);
			pVisGroups[nVisGrp].szName[LW_MAX_NAME-1]=0;
			nVisGrp++;
			
		}
		else if(strcmp(szNames[nName], "lightmap")==0)
		{
			//We'll just read the lightmap data, and do the conversion
			//to a tga when we write the lw file.
			lg_dword* pRef=(lg_dword*)&pLMData[4*nLM];
			lg_byte* pLM=&pLMData[4*counts.nLightMapCount+nLM*counts.nLMSize];
			
			if(nLM>=counts.nLightMapCount)
			{
				LF_Seek(fileMap, LF_SEEK_CURRENT, nSize);
				continue;
			}
			*pRef=i;
			LF_Read(fileMap, pLM, nSize);
			nLM++;
		}
		else if(strcmp(szNames[nName], "brush")==0)
		{
			if(nVert>=counts.nTotalVertexCount)
			{
				LF_Seek(fileMap, LF_SEEK_CURRENT, nSize);
				continue;
			}
			lg_dword nBegin=LF_Tell(fileMap);
			lg_byte nFlags;
			lg_long nKeys;
			lg_long nGrp;
			lg_long nVisGrp;
			
			lg_byte nVertexCount;
			lg_byte nFaceCount;
			
			LF_Read(fileMap, &nFlags, 1);
			LF_Read(fileMap, &nKeys, 4);
			LF_Seek(fileMap, LF_SEEK_CURRENT, nKeys*8);
			LF_Read(fileMap, &nGrp, 4);
			LF_Read(fileMap, &nVisGrp, 4);
			//Skip the color
			LF_Seek(fileMap, LF_SEEK_CURRENT, 3);
			
			LF_Read(fileMap, &nVertexCount, 1);
			ML_VEC3* pVerts=new ML_VEC3[nVertexCount];
			if(!pVerts)
				throw("WSToLW ERROR: Out of memory.");
			for(lg_byte i=0; i<nVertexCount; i++)
			{
				LF_Read(fileMap, &pVerts[i], 12);
			}
			
			LF_Read(fileMap, &nFaceCount, 1);	
			for(lg_byte i=0; i<nFaceCount; i++)
			{
				lf_byte nFaceFlags;
				ML_VEC4 v4PlaneEq;
				lg_long nMaterial;
				lg_long nLightMap;
				
				LF_Read(fileMap, &nFaceFlags, 1);
				LF_Read(fileMap, &v4PlaneEq, 16);
				LF_Seek(fileMap, LF_SEEK_CURRENT, 64);
				LF_Read(fileMap, &nMaterial, 4);
				
				if(nFaceFlags&16)
					LF_Read(fileMap, &nLightMap, 4);
				else
					nLightMap=0xFFFFFFFF;
					
				lf_byte nIndexCount;
				LF_Read(fileMap, &nIndexCount, 1);
				
				pFaces[nFace].bVisible=!(nFaceFlags&0x04);
				pFaces[nFace].nFirst=nVert;
				pFaces[nFace].nMatRef=nMaterial;
				pFaces[nFace].nLMRef=nLightMap;
				//Note need to change this...
				pFaces[nFace].nRoom=nVisGrp;
				pFaces[nFace].nNumTriangles=(nIndexCount-2);
				pFaces[nFace].v4PlaneEq=v4PlaneEq;
				pFaces[nFace].nOpFlags=0;
				nFace++;
				
				for(lg_byte i=0; i<nIndexCount; i++)
				{
					lg_byte nV;
					ML_VEC2 v2Tex, v2LM;
					
					LF_Read(fileMap, &nV, 1);
					pVertexes[nVert].v3Pos=pVerts[nV];
					LF_Read(fileMap, &v2Tex, 8);
					if(nFaceFlags&16)
					{
						LF_Read(fileMap, &v2LM, 8);
					}
					else
					{
						v2LM.x=0;
						v2LM.y=0;
					}
					
					//If the face isn't visible we don't
					//need to save it to the raster data.
					if(L_CHECK_FLAG(nFaceFlags, 0x04) || (nVert>=counts.nTotalVertexCount))
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
						pVertexes[nVert].v3Pos=pVerts[nV];
						pVertexes[nVert].v2Tex=v2Tex;
						pVertexes[nVert].v2LM=v2LM;

					}
					else
					{
						pVertexes[nVert]=pVertexes[nVert-3];
						pVertexes[nVert+1]=pVertexes[nVert-1];
						nVert+=2;
						
						pVertexes[nVert].v3Pos=pVerts[nV];
						pVertexes[nVert].v2Tex=v2Tex;
						pVertexes[nVert].v2LM=v2LM;
						
					}
					nVert++;
				}
			}
			
			L_safe_delete_array(pVerts);
			
			LF_Seek(fileMap, LF_SEEK_BEGIN, nBegin+nSize);
		}
		else
		{
			LF_Seek(fileMap, LF_SEEK_CURRENT, nSize);
		}
	}
	
	//Reverse the vertex orders
	for(lg_dword i=0; i<counts.nTotalVertexCount; i+=3)
	{
		LW_VERTEX vt=pVertexes[i];
		pVertexes[i]=pVertexes[i+2];
		pVertexes[i+2]=vt;
	}
	
	//Change references for material, lightmap, and visgroups.
	for(lg_dword i=0; i<counts.nFaceCount; i++)
	{
		for(lg_dword j=0; j<counts.nMaterialCount; j++)
		{
			if(pFaces[i].nMatRef==pMaterials[j].nReference)
			{
				pFaces[i].nMatRef=j;
				break;
			}
		}
		
		for(lg_dword j=0; j<counts.nLightMapCount; j++)
		{
			lg_dword* pLMRef=(lg_dword*)&pLMData[j*4];
			if(pFaces[i].nLMRef==*pLMRef)
			{
				pFaces[i].nLMRef=j;
				break;
			}
		}
		
		for(lg_dword j=0; j<counts.nVisGroupCount; j++)
		{
			if(pFaces[i].nRoom==pVisGroups[j].nRef)
			{
				pFaces[i].nRoom=j;
				break;
			}
		}
	}
	
	//We are going to restructure the vertexes, so that all
	//faces that share the same texture in a room will be part
	//of the same segment.
	WSToLW_SortData(pFaces, pVisGroups, pVertexes, &counts);
		
	//Now write the map...
	lg_word nID=LW_ID;
	lg_dword nVersion=LW_VERSION;
	lg_dword nVertexCount=counts.nTotalVertexCount;
	lg_dword nVertexOffset=0;
	lg_dword nMaterialCount=counts.nMaterialCount;
	lg_dword nMaterialOffset=0;
	lg_dword nLMCount=counts.nLightMapCount;
	lg_dword nLMOffset=0;
	lg_dword nLMTotalSize=0;
	//This is just temporary, later faces will be organized according to room.
	lg_dword nFaceCount=counts.nVisibleFaceCount;
	lg_dword nFaceOffset=0;
	lg_dword nRegionCount=counts.nVisGroupCount;
	lg_dword nRegionOffset=0;
	
	LF_FILE3 fout=LF_Open(szFilenameOut, LF_ACCESS_WRITE, LF_CREATE_ALWAYS);
	if(!fout)
		throw("WSToLW ERROR: Could not open output file.");
	
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
	
	
	//We can simply write the vertexes
	nVertexOffset=LF_Tell(fout);
	for(lg_dword i=0; i<nVertexCount; i++)
	{
		LF_Write(fout, &pVertexes[i], 28);
	}
	//we can do the same for the materials
	nMaterialOffset=LF_Tell(fout);
	for(lg_dword i=0; i<nMaterialCount; i++)
	{
		LF_Write(fout, &pMaterials[i].szMaterial, 256);
	}
	//Light maps are a little more complicate, we need to convert each lightmap
	//to a tga file, and then write the tga file.
	nLMOffset=LF_Tell(fout);
	//Write each of the light maps
	for(lg_dword i=0; i<nLMCount; i++)
	{
		lg_byte* pLM=&pLMData[4*counts.nLightMapCount+i*counts.nLMSize];
		lg_word nRes=ML_Pow2(pLM[1]);
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
		LF_Write(fout, &pFaces[i].nFirst, 4);
		LF_Write(fout, &pFaces[i].nNumTriangles, 4);
		LF_Write(fout, &pFaces[i].nMatRef, 4);
		LF_Write(fout, &pFaces[i].nLMRef, 4);
	}
	
	//Write the regions...
	nRegionOffset=LF_Tell(fout);
	for(lg_dword i=0; i<nRegionCount; i++)
	{
		LF_Write(fout, &pVisGroups[i].szName, 32);
		LF_Write(fout, &pVisGroups[i].nFirstFace, 4);
		LF_Write(fout, &pVisGroups[i].nFaceCount, 4);
		LF_Write(fout, &pVisGroups[i].nFirstVertex, 4);
		LF_Write(fout, &pVisGroups[i].nVertexCount, 4);
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
	LF_Close(fout);
	
	
	throw("OK");
	} //try
	catch(lg_cstr szError)
	{
		Err_Printf("Map Conversion Result: %s", szError);
		
		if(fileMap)LF_Close(fileMap);
		L_safe_delete_array(szNames);
		L_safe_delete_array(pMaterials);
		L_safe_delete_array(pVertexes);
		L_safe_delete_array(pLMData);
		L_safe_delete_array(pFaces);
		L_safe_delete_array(pVisGroups);
		
		
		if(strcmp(szError, "OK")==0)
			return LG_TRUE;
		else
			return LG_FALSE;
	}
}

void WSToLW_SortData(WS_RASTER_FACE* pFaces, WS_VISGROUP* pVisGrps, LW_VERTEX* pVerts, WSToLW_COUNTS* pCounts)
{
	lg_dword nFace=0, nVisGrp=0, nMaterial=0, nLightmap=0;
	
	WS_RASTER_FACE* pTempFaces=new WS_RASTER_FACE[pCounts->nFaceCount];
	if(!pTempFaces)
		return;
	
	//Store all the faces in the temporary array...
	memcpy(pTempFaces, pFaces, sizeof(WS_RASTER_FACE)*pCounts->nFaceCount);
	//Sort the data into the original array..
	
	for(nVisGrp=0; nVisGrp<pCounts->nVisGroupCount; nVisGrp++)
	{
		pVisGrps[nVisGrp].nFirstFace=nFace;
		pVisGrps[nVisGrp].nFaceCount=0;
		pVisGrps[nVisGrp].nVisibleFaceCount=0;
		pVisGrps[nVisGrp].nFirstVertex=0;
		pVisGrps[nVisGrp].nVertexCount=0;
	}
	
	nVisGrp=0;
		
	do
	{		
		for(lg_dword i=0; i<pCounts->nFaceCount; i++)
		{
			if(nFace>=pCounts->nVisibleFaceCount)
				break;
			
			if(!pTempFaces[i].bVisible)
				continue;
				
			if(pTempFaces[i].nRoom!=nVisGrp)
				continue;
				
			if(pTempFaces[i].nMatRef!=nMaterial)
				continue;
			
			if(pTempFaces[i].nLMRef!=nLightmap)
				continue;
			
			//If all the criteria are met add the face to the list.
			pFaces[nFace]=pTempFaces[i];
			nFace++;
			pVisGrps[nVisGrp].nFaceCount++;
			pVisGrps[nVisGrp].nVisibleFaceCount++;
				
		}
		
		if(nLightmap==0xFFFFFFFF)
		{
			nLightmap=0;
			nMaterial++;
			if(nMaterial>=pCounts->nMaterialCount)
			{
				nMaterial=0;
				nVisGrp++;
				if(nVisGrp>=pCounts->nVisGroupCount)
					break;
					
				pVisGrps[nVisGrp].nFirstFace=nFace;
				pVisGrps[nVisGrp].nFaceCount=0;
				pVisGrps[nVisGrp].nVisibleFaceCount=0;
			}
		}
		else
			nLightmap++;
		
		if((nLightmap>=pCounts->nLightMapCount) && (nLightmap!=0xFFFFFFFF))
		{
			nLightmap=0xFFFFFFFF;
		}
		
	}while(nFace<pCounts->nVisibleFaceCount);
	
	
	for(lg_dword i=nFace; i<pCounts->nFaceCount; i++)
	{
		memset(&pFaces[i], 0, sizeof(WS_RASTER_FACE));
	}

	//Now we will resort all the vertexes...
	//The vertexes are being sorted in the order of the faces,
	//that way faces that have the same texture and lightmap
	//can be catenated.
	LW_VERTEX* pTempVerts=new LW_VERTEX[pCounts->nTotalVertexCount];
	if(!pTempVerts)
		return;
		
	memcpy(pTempVerts, pVerts, sizeof(LW_VERTEX)*pCounts->nTotalVertexCount);
	
	for(lg_dword nFace=0, nVertex=0; nFace<pCounts->nVisibleFaceCount; nFace++)
	{
		//Simply copy the faces vertexes to the destination buffer,
		//then set the new first vertex, and advance to the next vertex
		//to be written to.
		memcpy(
			&pVerts[nVertex], 
			&pTempVerts[pFaces[nFace].nFirst], 
			sizeof(LW_VERTEX)*pFaces[nFace].nNumTriangles*3);
			
		pFaces[nFace].nFirst=nVertex;
		nVertex+=pFaces[nFace].nNumTriangles*3;
	}
		
	L_safe_delete_array(pTempVerts);
	
	//We'll now reduce the number of faces into segments (each segment will
	//be composed of triangles that have the same texture and lightmap).
	for(lg_dword nVisGrp=0; nVisGrp<pCounts->nVisGroupCount; nVisGrp++)
	{
		lg_dword nLastMat=0, nLastLM=0;
		WS_RASTER_FACE* pCurFace=LG_NULL;
		lg_dword nStart=pVisGrps[nVisGrp].nFirstFace;
		lg_dword nEnd=pVisGrps[nVisGrp].nFirstFace+pVisGrps[nVisGrp].nFaceCount;
		
		for(lg_dword i=nStart; i<nEnd; i++)
		{
			if(!pCurFace)
			{
				pCurFace=&pFaces[i];
				nLastMat=pCurFace->nMatRef;
				nLastLM=pCurFace->nLMRef;
				continue;
			}
			
			if((nLastMat==pFaces[i].nMatRef) && (nLastLM==pFaces[i].nLMRef))
			{
				pCurFace->nNumTriangles+=pFaces[i].nNumTriangles;
				pFaces[i].nNumTriangles=0;
			}
			else
			{
				pCurFace=&pFaces[i];
				nLastMat=pCurFace->nMatRef;
				nLastLM=pCurFace->nLMRef;
			}
		}
	}
	
	//We'll now eliminate all the empty faces...
	memcpy(pTempFaces, pFaces, sizeof(WS_RASTER_FACE)*pCounts->nVisibleFaceCount);
	
	nFace=0;
	for(lg_dword nVisGrp=0; nVisGrp<pCounts->nVisGroupCount; nVisGrp++)
	{		
		lg_dword nStart=pVisGrps[nVisGrp].nFirstFace;
		lg_dword nEnd=pVisGrps[nVisGrp].nFirstFace+pVisGrps[nVisGrp].nFaceCount;
		
		pVisGrps[nVisGrp].nFirstFace=nFace;
		pVisGrps[nVisGrp].nFaceCount=0;
		for(lg_dword i=nStart; i<nEnd; i++)
		{
			if(pTempFaces[i].nNumTriangles>0)
			{
				pFaces[nFace]=pTempFaces[i];
				pVisGrps[nVisGrp].nFaceCount++;
				nFace++;
			}
		}
	}
	
	Err_Printf("NOTICE: %d faces reduced to %d segments %0.2f%%.",  
		pCounts->nVisibleFaceCount,
		nFace,
		(float)nFace/(float)pCounts->nVisibleFaceCount*100.0f);
	
	pCounts->nVisibleFaceCount=nFace;
	
	L_safe_delete_array(pTempFaces);
	
	//We'll now setup some more information about the vis groups
	for(lg_dword i=0; i<pCounts->nVisGroupCount; i++)
	{
		pVisGrps[i].nFirstVertex=pFaces[pVisGrps[i].nFirstFace].nFirst;
		
		lg_dword nCount=0;
		for(lg_dword j=0; j<pVisGrps[i].nFaceCount; j++)
		{
			nCount+=pFaces[pVisGrps[i].nFirstFace+j].nNumTriangles*3;
		}
		pVisGrps[i].nVertexCount=nCount;
	}

	//For testing purposes print out the results
	for(lg_dword i=0; i<pCounts->nVisGroupCount; i++)
	{
		Err_Printf(
			"Vis Group %d: %s (%d) FirstFace=%d FaceCount=%d VisibleFaces=%d First Vertex: %d Vertex Count: %d", 
			i, 
			pVisGrps[i].szName, 
			pVisGrps[i].nRef, 
			pVisGrps[i].nFirstFace, 
			pVisGrps[i].nFaceCount,
			pVisGrps[i].nVisibleFaceCount,
			pVisGrps[i].nFirstVertex,
			pVisGrps[i].nVertexCount);
			
		for(lg_dword j=0; j<pVisGrps[i].nFaceCount; j++)
		{
			lg_dword offset=j+pVisGrps[i].nFirstFace;
			
			Err_Printf("   Face %d: S: %d: C: %d: Mat: %d: LM: %d: Room: %d\n",
				offset,
				pFaces[offset].nFirst,
				pFaces[offset].nNumTriangles,
				pFaces[offset].nMatRef,
				pFaces[offset].nLMRef,
				pFaces[offset].nRoom);
		}

	}
}

void WSToLW_GetCounts(LF_FILE3 file, WSToLW_COUNTS* pCounts, lg_char** szNames, lg_long nObjectCount)
{
	WSToLW_COUNTS counts;
	
	memset(&counts, 0, sizeof(counts));
	
	//Count all the objects
	for(lg_dword i=1; i<=nObjectCount; i++)
	{
		lg_dword nName;
		lg_long nSize;
		LF_Read(file, &nName, 4);
		LF_Read(file, &nSize, 4);
		
		//Err_Printf("Ojbect %d (%d): %s", i, nSize, szNames[nName]);
		if(strcmp(szNames[nName], "material")==0)
			counts.nMaterialCount++;
		if(strcmp(szNames[nName], "visgroup")==0)
			counts.nVisGroupCount++;
		if(strcmp(szNames[nName], "lightmap")==0)
		{
			counts.nLightMapCount++;
			counts.nLMSize=LG_Max(counts.nLMSize, nSize);
		}
		if(strcmp(szNames[nName], "brush")==0)
		{
			counts.nBrushCount++;
			
			lg_dword nBegin=LF_Tell(file);
			lg_byte nFlags;
			lg_long nKeys;
			
			lg_byte nVertexCount;
			lg_byte nFaceCount;
			
			LF_Read(file, &nFlags, 1);
			LF_Read(file, &nKeys, 4);
			LF_Seek(file, LF_SEEK_CURRENT, nKeys*8);
			LF_Seek(file, LF_SEEK_CURRENT, 4+4+3);
			
			LF_Read(file, &nVertexCount, 1);
			LF_Seek(file, LF_SEEK_CURRENT, 12*nVertexCount);
			LF_Read(file, &nFaceCount, 1);
			counts.nFaceCount+=nFaceCount;
			for(lg_byte i=0; i<nFaceCount; i++)
			{
				lf_byte nFaceFlags;
				
				LF_Read(file, &nFaceFlags, 1);
				LF_Seek(file, LF_SEEK_CURRENT, 84);
				if(nFaceFlags&16)
					LF_Seek(file, LF_SEEK_CURRENT, 4);
					
				lf_byte nIndexCount;
				LF_Read(file, &nIndexCount, 1);
				
				if(!(nFaceFlags&0x04))
				{
					counts.nTotalVertexCount+=3+(nIndexCount-3)*3;
					counts.nVisibleFaceCount++;
				}
					
				if(nFaceFlags&16)
					LF_Seek(file, LF_SEEK_CURRENT, nIndexCount*17);
				else
					LF_Seek(file, LF_SEEK_CURRENT, nIndexCount*9);
			}
			
			LF_Seek(file, LF_SEEK_BEGIN, nBegin);
		}
		LF_Seek(file, LF_SEEK_CURRENT, nSize);
	}
	
	*pCounts=counts;
}

void WSToLW_GetNameTable(lg_char** szNames, lg_byte* pData, lg_dword nSize, lg_long nNumNames)
{
	lg_long nNamesFound=1;
	szNames[0]=LG_NULL;
	szNames[1]=(lg_char*)&pData[0];
	for(lg_dword i=0; i<nSize; i++)
	{
		if(pData[i]==0)
		{
			nNamesFound++;
			szNames[nNamesFound]=(lg_char*)&pData[i+1];
		}
		
		if(nNamesFound>=nNumNames)
			return;
	}
}

