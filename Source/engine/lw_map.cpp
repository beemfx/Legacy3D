//lw_sys.cpp - The Legacy World (Map) system.
//Copyright (c) 2007 Blaine Myers

#include "lw_map.h"
#include "lg_err.h"
#include "lg_err_ex.h"
#include "ML_lib.h"
#include "ld_sys.h"
#include "lg_tmgr.h"
#include "lg_func.h"

#include <stdio.h>


CLMap::CLMap():
	m_bLoaded(LG_FALSE),
	
	m_nID(0),
	m_nVersion(0),
	m_nVertexCount(0),
	m_nVertexOffset(0),
	m_nMaterialCount(0),
	m_nMaterialOffset(0),
	m_nLMCount(0),
	m_nLMOffset(0),
	m_nLMTotalSize(0),
	m_nSegmentCount(0),
	m_nSegmentOffset(0),
	m_nRegionCount(0),
	m_nRegionOffset(0),
	m_nGeoBlockCount(0),
	m_nGeoBlockOffset(0),
	m_nGeoFaceCount(0),
	m_nGeoFaceOffset(0),
	m_nLightCount(0),
	m_nLightOffset(0),
	m_nGeoVertOffset(0),
	m_nGeoVertCount(0),
	m_nGeoTriCount(0),
	m_nGeoTriOffset(0),
	
	m_pVertexes(LG_NULL),
	m_pLMData(LG_NULL),
	m_pLMs(LG_NULL),
	m_pSegments(LG_NULL),
	m_pRegions(LG_NULL),
	m_pGeoBlocks(LG_NULL),
	m_pGeoFaces(LG_NULL),
	m_pLights(LG_NULL),
	m_pGeoVerts(LG_NULL),
	m_pGeoTris(LG_NULL),
	m_pMem(LG_NULL)
{
	m_szMapPath[0]=0;
}

CLMap::~CLMap()
{
	Unload();
}
const lg_str CLMap::GetMapPath()
{
	return m_szMapPath;
}

lg_bool CLMap::IsLoaded()
{
	return m_bLoaded;
}

void CLMap::Unload()
{
	DeallocateMemory();
	
	m_nID=0;
	m_nVersion=0;
	m_nVertexCount=0;
	m_nVertexOffset=0;
	m_nMaterialCount=0;
	m_nMaterialOffset=0;
	m_nLMCount=0;
	m_nLMOffset=0;
	m_nLMTotalSize=0;
	m_nSegmentCount=0;
	m_nSegmentOffset=0;
	m_nRegionCount=0;
	m_nRegionOffset=0;
	m_nGeoBlockCount=0;
	m_nGeoBlockOffset=0;
	m_nGeoFaceCount=0;
	m_nGeoFaceOffset=0;
	m_nLightCount=0;
	m_nLightOffset=0;
	m_nGeoVertCount=0;
	m_nGeoVertOffset=0;
	m_nGeoTriCount=0;
	m_nGeoTriOffset=0;
	m_szMapPath[0]=0;
	m_bLoaded=LG_FALSE;
}


lg_bool CLMap::AllocateMemory()
{
	DeallocateMemory();
	
	lg_dword nOffset=0;
	
	lg_dword nVertexOffset=nOffset;
	nOffset+=m_nVertexCount*sizeof(LW_VERTEX);
	
	lg_dword nMaterialOffset=nOffset;
	nOffset+=m_nMaterialCount*sizeof(LW_MATERIAL);
	
	lg_dword nLMOffset=nOffset;
	nOffset+=m_nLMTotalSize;
	
	lg_dword nLMPointerOffset=nOffset;
	nOffset+=m_nLMCount*sizeof(lg_byte*);
	
	lg_dword nFaceOffset=nOffset;
	nOffset+=m_nSegmentCount*sizeof(LW_RASTER_SEGMENT);
	
	lg_dword nRegionOffset=nOffset;
	nOffset+=m_nRegionCount*sizeof(LW_REGION_EX);
	
	lg_dword nGeoBlockOffset=nOffset;
	nOffset+=m_nGeoBlockCount*sizeof(LW_GEO_BLOCK_EX);
	
	lg_dword nGeoFaceOffset=nOffset;
	nOffset+=m_nGeoFaceCount*sizeof(LW_GEO_FACE);
	
	lg_dword nLightOffset=nOffset;
	nOffset+=m_nLightCount*sizeof(LW_LIGHT);
	
	lg_dword nGeoVertOffset=nOffset;
	nOffset+=m_nGeoVertCount*sizeof(LW_GEO_VERTEX);
	
	lg_dword nGeoTriOffset=nOffset;
	nOffset+=m_nGeoTriCount*sizeof(LW_GEO_TRI);
	lg_dword nSize=nOffset;
	
	//For more efficient memory usage, we'll allocate one big
	//lump of memory and simply point to the data within that
	//memory, for most of the items.
	m_pMem=new lg_byte[nSize];
	if(!m_pMem)
	{
		DeallocateMemory();
		return LG_FALSE;
	}
	
	m_pVertexes=(LW_VERTEX*)(m_pMem+nVertexOffset);
	m_pMaterials=(LW_MATERIAL*)(m_pMem+nMaterialOffset);
	m_pLMData=(lg_byte*)(m_pMem+nLMOffset);
	m_pLMs=(lg_byte**)(m_pMem+nLMPointerOffset);
	m_pSegments=(LW_RASTER_SEGMENT*)(m_pMem+nFaceOffset);
	m_pRegions=(LW_REGION_EX*)(m_pMem+nRegionOffset);
	m_pGeoBlocks=(LW_GEO_BLOCK_EX*)(m_pMem+nGeoBlockOffset);
	m_pGeoFaces=(LW_GEO_FACE*)(m_pMem+nGeoFaceOffset);
	m_pLights=(LW_LIGHT*)(m_pMem+nLightOffset);
	m_pGeoVerts=(LW_GEO_VERTEX*)(m_pMem+nGeoVertOffset);
	m_pGeoTris=(LW_GEO_TRI*)(m_pMem+nGeoTriOffset);
	Err_Printf("   %u bytes allocated.", nSize);
	return LG_TRUE;
}
void CLMap::DeallocateMemory()
{
	//We only have to delete the lump of data,
	//then set everything else to null.
	LG_SafeDeleteArray(m_pMem);
	m_pVertexes=LG_NULL;
	m_pMaterials=LG_NULL;
	m_pLMData=LG_NULL;
	m_pLMs=LG_NULL;
	m_pSegments=LG_NULL;
	m_pRegions=LG_NULL;
	m_pGeoBlocks=LG_NULL;
	m_pGeoFaces=LG_NULL;
	m_pLights=LG_NULL;
	m_pGeoVerts=LG_NULL;
	m_pGeoTris=LG_NULL;
}

lg_bool CLMap::LoadReal(lf_path szFilename)
{
	Unload();
	
	Err_Printf("Loading \"%s\"...", szFilename);
	
	LF_FILE3 fin=LF_Open(szFilename, LF_ACCESS_READ, LF_OPEN_EXISTING);
	if(!fin)
	{
		Err_Printf("   CLMap::Load ERROR: Could not open \"%s\".", szFilename);
		return LG_FALSE;
	}
	
	//Read the header, and make sure the map is actually a legacy world.
	LF_Read(fin, &m_nID, 2);
	LF_Read(fin, &m_nVersion, 4);
	if((m_nID!=LW_ID) || (m_nVersion!=LW_VERSION))
	{
		Err_Printf("   CLMap::Load ERROR: \"%s\" is not a valid Legacy World.", szFilename);
		LF_Close(fin);
		return LG_FALSE;
	}
	
	//Read the rest of the header...
	LF_Read(fin, &m_nVertexCount, 4);
	LF_Read(fin, &m_nVertexOffset, 4);
	LF_Read(fin, &m_nMaterialCount, 4);
	LF_Read(fin, &m_nMaterialOffset, 4);
	LF_Read(fin, &m_nLMCount, 4);
	LF_Read(fin, &m_nLMOffset, 4);
	LF_Read(fin, &m_nLMTotalSize, 4);
	LF_Read(fin, &m_nSegmentCount, 4);
	LF_Read(fin, &m_nSegmentOffset, 4);
	LF_Read(fin, &m_nRegionCount, 4);
	LF_Read(fin, &m_nRegionOffset, 4);
	LF_Read(fin, &m_nGeoBlockCount, 4);
	LF_Read(fin, &m_nGeoBlockOffset, 4);
	LF_Read(fin, &m_nGeoFaceCount, 4);
	LF_Read(fin, &m_nGeoFaceOffset, 4);
	LF_Read(fin, &m_nLightCount, 4);
	LF_Read(fin, &m_nLightOffset, 4);
	LF_Read(fin, &m_nGeoVertCount, 4);
	LF_Read(fin, &m_nGeoVertOffset, 4);
	LF_Read(fin, &m_nGeoTriCount, 4);
	LF_Read(fin, &m_nGeoTriOffset, 4);
	LF_Read(fin, &m_aabbMapBounds, 24);
	
	Err_Printf("   %u vertexes (%u bytes).", m_nVertexCount, m_nVertexCount*sizeof(LW_VERTEX));
	Err_Printf("   %u materials.", m_nMaterialCount);
	Err_Printf("   %u lightmaps (%u bytes).", m_nLMCount, m_nLMTotalSize);
	Err_Printf("   %u segments.", m_nSegmentCount);
	Err_Printf("   %u regions.", m_nRegionCount);
	Err_Printf("   %u geo blocks.", m_nGeoBlockCount);
	Err_Printf("   %u geo faces.", m_nGeoFaceCount);
	Err_Printf("   %u geo vertexes.", m_nGeoVertCount);
	Err_Printf("   %u lights.", m_nLightCount);
	//Now to begin reading the file...
	//We need some memory...
	Err_Printf("   Allocating memory...");
	if(!AllocateMemory())
	{
		Err_Printf("   CLMap::Load ERROR: Cannot load \"%s\".  Out of memory.", szFilename);
		LF_Close(fin);
		return LG_FALSE;
	}
	Err_Printf("   Reading world data...");
	//Read the vertexes
	LF_Seek(fin, LF_SEEK_BEGIN, m_nVertexOffset);
	for(lg_dword i=0; i<m_nVertexCount; i++)
	{
		LF_Read(fin, &m_pVertexes[i], 28);
	}	
	//Read the materials
	LF_Seek(fin, LF_SEEK_BEGIN, m_nMaterialOffset);
	for(lg_dword i=0; i<m_nMaterialCount; i++)
	{
		LF_Read(fin, &m_pMaterials[i], 256);
	}
	//Read the lightmaps and setup the data
	LF_Seek(fin, LF_SEEK_BEGIN, m_nLMOffset);
	LF_Read(fin, m_pLMData, m_nLMTotalSize);
	
	lg_byte* pTemp=m_pLMData;
	for(lg_dword i=0; i<m_nLMCount; i++)
	{
		//The first dword at the beginning of
		//a light map entry is the size of the lightmap
		//then the data is that size.
		
		//First get the size of the lm (so we can jump to the next lm).
		lg_dword nSize=0;
		memcpy(&nSize, pTemp, 4);
		
		m_pLMs[i]=pTemp;
		//jump to the next lightmap which is the size of the
		//current lightmap + 4 (the size position);
		pTemp+=nSize+4;
	}
	
	//Read the segments...
	LF_Seek(fin, LF_SEEK_BEGIN, m_nSegmentOffset);
	for(lg_dword i=0; i<m_nSegmentCount; i++)
	{
		LF_Read(fin, &m_pSegments[i], 16);
	}
	
	//Read the regions
	LF_Seek(fin, LF_SEEK_BEGIN, m_nRegionOffset);
	for(lg_dword i=0; i<m_nRegionCount; i++)
	{
		LF_Read(fin, &m_pRegions[i], LW_MAX_NAME+56);
		m_pRegions[i].pGeoBlocks=&m_pGeoBlocks[m_pRegions[i].nFirstGeoBlock];
		m_pRegions[i].pRasterSegs=&m_pSegments[m_pRegions[i].nFirstRasterSeg];
		m_pRegions[i].pLights=&m_pLights[m_pRegions[i].nFirstLight];
		//Err_Printf("=== Region %u: %s ===", i, m_pRegions[i].szName);
		//Err_Printf("Light Count: %u.  First Light: %u.", m_pRegions[i].nLightcount, m_pRegions[i].nFirstLight);
	}
	
	//Read the geo blocks
	LF_Seek(fin, LF_SEEK_BEGIN, m_nGeoBlockOffset);
	for(lg_dword i=0; i<m_nGeoBlockCount; i++)
	{
		LF_Read(fin, &m_pGeoBlocks[i], 24+24);
		
		m_pGeoBlocks[i].pFaces=&m_pGeoFaces[m_pGeoBlocks[i].nFirstFace];
		m_pGeoBlocks[i].pVerts=&m_pGeoVerts[m_pGeoBlocks[i].nFirstVertex];
		m_pGeoBlocks[i].pTris=&m_pGeoTris[m_pGeoBlocks[i].nFirstTri];
	}
	
	//Read the geo faces...
	LF_Seek(fin, LF_SEEK_BEGIN, m_nGeoFaceOffset);
	for(lg_dword i=0; i<m_nGeoFaceCount; i++)
	{
		LF_Read(fin, &m_pGeoFaces[i], 16);
	}
	
	//Read the geo vertexes...
	LF_Seek(fin, LF_SEEK_BEGIN, m_nGeoVertOffset);
	for(lg_dword i=0; i<m_nGeoVertCount; i++)
	{
		LF_Read(fin, &m_pGeoVerts[i], 12);
	}
	
	//Read the lights...
	LF_Seek(fin, LF_SEEK_BEGIN, m_nLightOffset);
	for(lg_dword i=0; i<m_nLightCount; i++)
	{
		LF_Read(fin, &m_pLights[i], 28);
		/*
		Err_Printf("Light: (%f, %f, %f) Color: (%f, %f, %f)",
			m_pLights[i].v3Pos.x,
			m_pLights[i].v3Pos.y,
			m_pLights[i].v3Pos.z,
			m_pLights[i].Color.r,
			m_pLights[i].Color.g,
			m_pLights[i].Color.b);
		*/
	}
	
	LF_Seek(fin, LF_SEEK_BEGIN, m_nGeoTriOffset);
	for(lg_dword i=0; i<m_nGeoTriCount; i++)
	{
		LF_Read(fin, &m_pGeoTris[i], 12*3);
	}
	
	LF_Close(fin);
	
	strcpy(m_szMapPath, szFilename);
	m_bLoaded=LG_TRUE;
	
	Err_Printf("   Legacy World successfully loaded.");
	
	return LG_TRUE;
}

//Load is only temporary till the final map format
//is established, but for now it will load a
//3dws map.
lg_bool CLMap::Load(lf_path szFilename)
{
	Unload();
	return LoadFromWS(szFilename);
}

#include "../ws_sys/ws_file.h"


lg_bool CLMap::LoadFromWS(lf_path szFilename)
{	
	LF_FILE3 fin=LF_Open(szFilename, LF_ACCESS_MEMORY|LF_ACCESS_READ, LF_OPEN_EXISTING);
	if(!fin)
	{
		Err_Printf("CLWorld::Load ERROR: Could not open \"%s\".", szFilename);
		return LG_FALSE;
	}
	
	CWSFile wsFile;	
	if(!wsFile.Load((lf_byte*)LF_GetMemPointer(fin), LF_GetSize(fin)))
	{
		LF_Close(fin);
		Err_Printf("CLWorld::Load ERROR could not convert 3DW file.");
		return LG_FALSE;
	}
	wsFile.ScaleMap(0.01f);
	wsFile.SaveAsLW("/dbase/maps/temp.lw");
	LF_Close(fin);

	return LoadReal("/dbase/maps/temp.lw");
}

lg_dword CLMap::CheckRegions(const ML_AABB* pAABB, lg_dword* pRegions, lg_dword nMaxRegions)
{
	//CheckRegions checks to see which regions the entity is currently occupying.
	//it checks the catenated aabb against each regions aabb, so this method is meant
	//to be called after ProcessAI, but before world and entity collision testing.
	lg_dword nNumRegions=0;

	//If no map is set then we aren't occupying any rooms.	
	if(!m_bLoaded)
		return 0;
	
	//TODO: First should check the regions that the object was previously
	//occupying, then check adjacent regions, in that way we don't have to
	//check every region every time.	
	for(lg_dword i=0; (i<m_nRegionCount) && (nNumRegions<nMaxRegions); i++)
	{
		if(ML_AABBIntersect(pAABB, &m_pRegions[i].aabbAreaBounds, LG_NULL))
		{
			pRegions[nNumRegions++]=i;
		}
	}
	return nNumRegions;
}


/***************************************
	CLMapD3D - 3D Renderable Map Stuff
***************************************/
#if 0
class CLMapD3D
{
public:
	CLMapD3D();
	~CLMapD3D();
	
	void Render();
	void Validate();
	void Invalidate();
	
	void Init(CLMap* pMap);
	void UnInit();
};
#endif

CLMapD3D::CLMapD3D():
	m_pMap(LG_NULL),
	m_pDevice(LG_NULL),
	m_pTextures(LG_NULL),
	m_pLMTextures(LG_NULL),
	m_pVB(LG_NULL),
	m_pMem(LG_NULL),
	m_bIsValid(LG_NULL)
{

}

CLMapD3D::~CLMapD3D()
{
	UnInit();
}

void CLMapD3D::Init(IDirect3DDevice9 *pDevice, CLMap *pMap)
{
	UnInit();
	m_pDevice=pDevice;
	if(m_pDevice)
		m_pDevice->AddRef();
		
	m_pMap=pMap;
	
	//Calculate the total size of memory needed.
	lg_dword nOffset=0;
	lg_dword nTexOffset=nOffset;
	nOffset += m_pMap->m_nMaterialCount * sizeof(tm_tex);
	lg_dword nLMOffset=nOffset;
	nOffset += m_pMap->m_nLMCount * sizeof(tm_tex);
	lg_dword nSize=nOffset;
	
	m_pMem = new lg_byte[nSize];
	
	m_pTextures = (tm_tex*)(m_pMem+nTexOffset);
	m_pLMTextures = (tm_tex*)(m_pMem+nLMOffset);
	
	//If there is no map we are done.
	if(!m_pMap || !m_pDevice || !m_pMap->IsLoaded())
		return;
	
	//Load the materials and light maps.
	
	for(lg_dword i=0; i<m_pMap->m_nMaterialCount; i++)
	{
		//Technically the entire pathname should be saved
		//in the map file (/dbase/... or /gbase/...) but
		//as of now the map format is incomplete so we'll
		//make a substitution.
		lf_path szFullPath;
		sprintf(szFullPath, "/dbase/%s", m_pMap->m_pMaterials[i]);
		m_pTextures[i]=CLTMgr::TM_LoadTex(szFullPath, 0);
		//If we couldn't load the texture then the default
		//texture is returned, so we can still use the texture.
	}
	
	
	for(lg_dword i=0; i<m_pMap->m_nLMCount; i++)
	{
		lg_dword nSize;
		memcpy(&nSize, m_pMap->m_pLMs[i], 4);
		m_pLMTextures[i]=CLTMgr::TM_LoadTexMem(m_pMap->m_pLMs[i]+4, nSize, 0);
	}
	
	Validate();
}

void CLMapD3D::UnInit()
{
	Invalidate();
	LG_SafeDeleteArray(m_pMem);
	LG_SafeRelease(m_pDevice);
	m_pMap=LG_NULL;
}

void CLMapD3D::Validate()
{
	//If we're already valid we're done, note that when the client
	//updates the map '''the client''' needs to call invalidate
	//first.
	if(m_bIsValid)
		return;
		
	
	//Create the vertex buffer...
	lg_result nResult=m_pDevice->CreateVertexBuffer(
		sizeof(LW_VERTEX)*m_pMap->m_nVertexCount,
		0,
		LW_VERTEX_TYPE,
		D3DPOOL_DEFAULT,
		&m_pVB,
		LG_NULL);
		
	if(LG_FAILED(nResult))
	{
		Err_Printf("CLMapD3D ERROR: Could not create vertex buffer.");
		Err_PrintDX("IDirect3DDevice9::CreateVertexBuffer", nResult);
		m_pVB=LG_NULL;
		Invalidate();
		return;
	}
	
	LW_VERTEX* pLockedVerts=LG_NULL;
	nResult=m_pVB->Lock(0, sizeof(LW_VERTEX)*m_pMap->m_nVertexCount, (void**)&pLockedVerts, 0);
	
	if(LG_FAILED(nResult))
	{
		Err_Printf("CLWorldMap ERROR: Could not lock vertex buffer.");
		Err_PrintDX("IDirect3DVertexBuffer9::Lock", nResult);
		Invalidate();
		return;
	}
	
	memcpy(pLockedVerts, m_pMap->m_pVertexes, sizeof(LW_VERTEX)*m_pMap->m_nVertexCount);

	m_pVB->Unlock();
	m_bIsValid=LG_TRUE;
}

void CLMapD3D::Invalidate()
{
	//If we are already invalid, or the map is
	//not set, then we won't do anything.
	if(!m_bIsValid || !m_pMap)
		return;
	
	LG_SafeRelease(m_pVB);
	m_bIsValid=LG_FALSE;
}

void CLMapD3D::RenderAABBs(lg_dword nFlags)
{
	if(!m_bIsValid)
		return;
		
	/*
		static const lg_dword  MAPDEBUG_HULLAABB=0x00000001;
	static const lg_dword  MAPDEBUG_HULLTRI=0x00000002;
	static const lg_dword  MAPDEBUG_WORLDAABB=0x00000004;
	*/
		
	if(LG_CheckFlag(nFlags, MAPDEBUG_HULLAABB))
	{
		for(lg_dword i=0; i<m_pMap->m_nGeoBlockCount; i++)
		{
			LD_DrawAABB(m_pDevice, &m_pMap->m_pGeoBlocks[i].aabbBlock, 0xFFFF0000);
		}
	}
	
	if(LG_CheckFlag(nFlags, MAPDEBUG_WORLDAABB))
		LD_DrawAABB(m_pDevice, &m_pMap->m_aabbMapBounds, 0xFF0000FF);
	
	
	if(LG_CheckFlag(nFlags, MAPDEBUG_HULLTRI))
	{
		for(lg_dword i=0; i<m_pMap->m_nGeoBlockCount; i++)
		{
			LD_DrawTris(
				m_pDevice, 
				(ml_vec3*)&m_pMap->m_pGeoBlocks[i].pTris[0], 
				m_pMap->m_pGeoBlocks[i].nTriCount);	
		}
	}
	
}

void CLMapD3D::Render()
{
	if(!m_bIsValid)
		return;
	
	m_pDevice->SetFVF(LW_VERTEX_TYPE);	
	m_pDevice->SetStreamSource(0, m_pVB, 0, sizeof(LW_VERTEX));
	
	
	m_pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	m_pDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
	m_pDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
	
	m_pDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
	m_pDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
	m_pDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
	
	//First pass (texture)
	m_pDevice->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
	m_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	
	for(lg_dword nRegion=0; nRegion<m_pMap->m_nRegionCount; nRegion++)
	{
		for(lg_dword j=0; j<m_pMap->m_pRegions[nRegion].nRasterSegCount; j++)
		{
			lg_dword nFace=m_pMap->m_pRegions[nRegion].nFirstRasterSeg+j;
			
			//m_pDevice->SetTexture(0, m_pTextures[m_pMap->m_pSegments[nFace].nMaterialRef]);
			CLTMgr::TM_SetTexture(m_pTextures[m_pMap->m_pSegments[nFace].nMaterialRef], 0);
			m_pDevice->DrawPrimitive(
				D3DPT_TRIANGLELIST,
				m_pMap->m_pSegments[nFace].nFirst,
				m_pMap->m_pSegments[nFace].nTriCount);
		}
	}
	
	//Second pass (lightmap)
	m_pDevice->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 1);
	m_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	m_pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO);
	m_pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR);
	m_pDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	m_pDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
	
	for(lg_dword nRegion=0; nRegion<m_pMap->m_nRegionCount; nRegion++)
	{
		for(lg_dword j=0; j<m_pMap->m_pRegions[nRegion].nRasterSegCount; j++)
		{
			lg_dword nFace=m_pMap->m_pRegions[nRegion].nFirstRasterSeg+j;
			if(m_pMap->m_pSegments[nFace].nLMRef==0xFFFFFFFF)
				continue;
				
			//m_pDevice->SetTexture(0, m_pLMTextures[m_pMap->m_pSegments[nFace].nLMRef]);
			CLTMgr::TM_SetTexture(m_pLMTextures[m_pMap->m_pSegments[nFace].nLMRef], 0);
			m_pDevice->DrawPrimitive(
				D3DPT_TRIANGLELIST,
				m_pMap->m_pSegments[nFace].nFirst,
				m_pMap->m_pSegments[nFace].nTriCount);
		}
	}
	
	m_pDevice->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
	
}



/*****************************************/
#if 0
CLWorldMap::CLWorldMap():
	CLMap(),
	m_pVB(LG_NULL),
	m_pTextures(LG_NULL),
	m_pLMTextures(LG_NULL),
	m_bD3DValid(LG_FALSE),
	m_pDevice(LG_NULL)
{

}

CLWorldMap::~CLWorldMap()
{
	Unload();
}

#if 0
void CLWorldMap::Render()
{
	if(!m_bD3DValid)
		return;
	
	m_pDevice->SetFVF(LW_VERTEX_TYPE);	
	m_pDevice->SetStreamSource(0, m_pVB, 0, sizeof(LW_VERTEX));
	
	
	m_pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	m_pDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
	m_pDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
	
	//First pass (texture)
	m_pDevice->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
	m_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
	
	for(lg_dword nRegion=0; nRegion<m_nRegionCount; nRegion++)
	{
		for(lg_dword j=0; j<m_pRegions[nRegion].nRasterSegCount; j++)
		{
			lg_dword nFace=m_pRegions[nRegion].nFirstRasterSeg+j;
			
			//m_pDevice->SetTexture(0, m_pTextures[m_pSegments[nFace].nMaterialRef]);
			CLTMgr::TM_SetTexture(m_pTextures[m_pSegments[nFace].nMaterialRef], 0);
			m_pDevice->DrawPrimitive(
				D3DPT_TRIANGLELIST,
				m_pSegments[nFace].nFirst,
				m_pSegments[nFace].nTriCount);
		}
	}
	
	//Second pass (lightmap)
	m_pDevice->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 1);
	m_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
	m_pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO);
	m_pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR);
	m_pDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	m_pDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
	
	for(lg_dword nRegion=0; nRegion<m_nRegionCount; nRegion++)
	{
		for(lg_dword j=0; j<m_pRegions[nRegion].nRasterSegCount; j++)
		{
			lg_dword nFace=m_pRegions[nRegion].nFirstRasterSeg+j;
			if(m_pSegments[nFace].nLMRef==0xFFFFFFFF)
				continue;
				
			//m_pDevice->SetTexture(0, m_pLMTextures[m_pSegments[nFace].nLMRef]);
			CLTMgr::TM_SetTexture(m_pLMTextures[m_pSegments[nFace].nLMRef], 0);
			m_pDevice->DrawPrimitive(
				D3DPT_TRIANGLELIST,
				m_pSegments[nFace].nFirst,
				m_pSegments[nFace].nTriCount);
		}
	}
	
	m_pDevice->SetTextureStageState(0, D3DTSS_TEXCOORDINDEX, 0);
	
}
#endif

void CLWorldMap::Unload()
{
	Invalidate();
	L_safe_release(m_pDevice);
	CLMap::Unload();
}


#if 0
lg_bool CLWorldMap::Validate()
{
	if(m_bD3DValid)
		return LG_TRUE;
		
	if(!m_bLoaded)
		return LG_FALSE;
		
	//m_pTextures=new tm_tex[m_nMaterialCount];
	//m_pLMTextures=new tm_tex[m_nLMCount];
		
	//Load up the materials and lightmaps...
	for(lg_dword i=0; i<m_nMaterialCount; i++)
	{
		lf_path szFullMatName;
		sprintf(szFullMatName, "/dbase/%s", m_pMaterials[i]);
		m_pTextures[i]=CLTMgr::TM_LoadTex(szFullMatName, 0);//Tex_Load2(szFullMatName, LG_FALSE);
	}
	
	for(lg_dword i=0; i<m_nLMCount; i++)
	{
		lg_dword nSize;
		memcpy(&nSize, m_pLMs[i], 4);
		m_pLMTextures[i]=CLTMgr::TM_LoadTexMem(m_pLMs[i]+4, nSize, 0);
	}
	
	//Create the vertex buffer...
	lg_result nResult=m_pDevice->CreateVertexBuffer(
		sizeof(LW_VERTEX)*m_nVertexCount,
		0,
		LW_VERTEX_TYPE,
		D3DPOOL_DEFAULT,
		&m_pVB,
		LG_NULL);
		
	if(LG_FAILED(nResult))
	{
		Err_Printf("CLWorldMap ERROR: Could not create vertex buffer.");
		Err_PrintDX("IDirect3DDevice9::CreateVertexBuffer", nResult);
		m_pVB=LG_NULL;
		Invalidate();
		return LG_FALSE;
	}
	
	LW_VERTEX* pLockedVerts=LG_NULL;
	nResult=m_pVB->Lock(0, sizeof(LW_VERTEX)*m_nVertexCount, (void**)&pLockedVerts, 0);
	
	if(LG_FAILED(nResult))
	{
		Err_Printf("CLWorldMap ERROR: Could not lock vertex buffer.");
		Err_PrintDX("IDirect3DVertexBuffer9::Lock", nResult);
		Invalidate();
		return LG_FALSE;
	}
	
	memcpy(pLockedVerts, m_pVertexes, sizeof(LW_VERTEX)*m_nVertexCount);

	m_pVB->Unlock();
	m_bD3DValid=LG_TRUE;
	return LG_TRUE;
}

void CLWorldMap::Invalidate()
{
	#if 0
	for(lg_dword i=0; i<m_nMaterialCount; i++)
	{
		L_safe_release(m_pTextures[i]);
	}
	for(lg_dword i=0; i<m_nLMCount; i++)
	{
		m_pLMTextures[i]=0;//L_safe_release(m_pLMTextures[i]);
	}
	#endif
	L_safe_release(m_pVB);
	
	L_safe_delete_array(m_pTextures);
	L_safe_delete_array(m_pLMTextures);
	
	m_bD3DValid=LG_FALSE;
}
#endif


lg_bool CLWorldMap::Load(lf_path szFilename, IDirect3DDevice9* pDevice)
{
	Unload();
	
	CLMap::Load(szFilename);
		
	Err_Printf("   Creating Direct3D resources...");
	m_pDevice=pDevice;
	m_pDevice->AddRef();
	
	if(Validate())
		Err_Printf("   Legacy World successfully loaded.");
	else
		Err_Printf("   CLWorldMap::Load ERROR: Could not create Direct3D resources.");
	
	return LG_TRUE;
}


#include "../ws_sys/ws_file.h"

lg_bool CLWorldMap::LoadFromWS(lf_path szFilename, IDirect3DDevice9* pDevice)
{	
	LF_FILE3 fin=LF_Open(szFilename, LF_ACCESS_MEMORY|LF_ACCESS_READ, LF_OPEN_EXISTING);
	if(!fin)
	{
		Err_Printf("CLWorld::Load ERROR: Could not open \"%s\".", szFilename);
		return LG_FALSE;
	}
	
	CWSFile wsFile;	
	if(!wsFile.Load((lf_byte*)LF_GetMemPointer(fin), LF_GetSize(fin)))
	{
		LF_Close(fin);
		Err_Printf("CLWorld::Load ERROR could not convert 3DW file.");
		return LG_FALSE;
	}
	wsFile.ScaleMap(0.01f);
	wsFile.SaveAsLW("/dbase/maps/temp.lw");
	LF_Close(fin);

	Err_Printf("HERE");
	return Load("/dbase/maps/temp.lw", pDevice);
}
#endif