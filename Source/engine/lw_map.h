//lw_sys.h - The Legacy World (map) system.
//Copyright (c) 2007 Blaine Myers
#ifndef __LW_MAP_H__
#define __LW_MAP_H__

#include <d3d9.h>
#include "lf_sys2.h"
#include "ML_lib.h"
#include "lg_tmgr.h"
#include "common.h"

#define LW_ID      (*(lg_word*)"LW")
#define LW_VERSION (1)


#define LW_MAX_NAME (32)
#define LW_MAX_PATH (256)

typedef lg_char LW_MATERIAL[LW_MAX_PATH];
typedef lg_char LW_NAME[LW_MAX_NAME];

typedef struct _LW_VERTEX{
	ML_VEC3  v3Pos;
	ML_VEC2  v2Tex;
	ML_VEC2  v2LM;
}LW_VERTEX;

#define LW_VERTEX_TYPE (D3DFVF_XYZ|D3DFVF_TEX2)

typedef struct _LW_RASTER_SEGMENT{
	lg_dword nFirst;
	lg_dword nTriCount;
	lg_dword nMaterialRef;
	lg_dword nLMRef;
}LW_RASTER_SEGMENT;

typedef ML_PLANE LW_GEO_FACE;
typedef ML_VEC3  LW_GEO_VERTEX;
typedef ML_VEC3  LW_GEO_TRI[3];

typedef struct _LW_GEO_BLOCK{
	lg_dword nFirstVertex;
	lg_dword nVertexCount;
	lg_dword nFirstTri;
	lg_dword nTriCount;
	lg_dword nFirstFace;
	lg_dword nFaceCount;
	ML_AABB  aabbBlock;
}LW_GEO_BLOCK;

typedef struct _LW_GEO_BLOCK_EX: public _LW_GEO_BLOCK{
	LW_GEO_FACE*   pFaces;
	LW_GEO_VERTEX* pVerts;
	LW_GEO_TRI*    pTris;
}LW_GEO_BLOCK_EX;

typedef struct _LW_REGION{
	LW_NAME  szName;
	lg_dword nFirstRasterSeg;
	lg_dword nRasterSegCount;
	lg_dword nFirstVertex;
	lg_dword nVertexCount;
	lg_dword nFirstGeoBlock;
	lg_dword nGeoBlockCount;
	lg_dword nFirstLight;
	lg_dword nLightcount;
	ML_AABB  aabbAreaBounds;
}LW_REGION;

typedef struct _LW_COLORVALUE{
	lg_float r, g, b, a;
}LW_COLORVALUE;


typedef struct _LW_LIGHT{
	ML_VEC3       v3Pos;
	LW_COLORVALUE Color;
}LW_LIGHT;

typedef struct _LW_REGION_EX: public _LW_REGION{
	LW_RASTER_SEGMENT* pRasterSegs;
	LW_GEO_BLOCK_EX*   pGeoBlocks;
	LW_LIGHT*          pLights;
}LW_REGION_EX;

class CLMap{
public:
	lg_bool m_bLoaded;
	lf_path m_szMapPath;
	
	lg_word  m_nID;
	lg_dword m_nVersion;
	lg_dword m_nVertexCount;
	lg_dword m_nVertexOffset;
	lg_dword m_nMaterialCount;
	lg_dword m_nMaterialOffset;
	lg_dword m_nLMCount;
	lg_dword m_nLMOffset;
	lg_dword m_nLMTotalSize;
	lg_dword m_nSegmentCount;
	lg_dword m_nSegmentOffset;
	lg_dword m_nRegionCount;
	lg_dword m_nRegionOffset;
	lg_dword m_nGeoBlockCount;
	lg_dword m_nGeoBlockOffset;
	lg_dword m_nGeoFaceCount;
	lg_dword m_nGeoFaceOffset;
	lg_dword m_nLightCount;
	lg_dword m_nLightOffset;
	lg_dword m_nGeoVertCount;
	lg_dword m_nGeoVertOffset;
	lg_dword m_nGeoTriCount;
	lg_dword m_nGeoTriOffset;
	
	//Map data
	ML_AABB            m_aabbMapBounds;
	LW_VERTEX*         m_pVertexes;
	LW_MATERIAL*       m_pMaterials;
	lg_byte*           m_pLMData;
	lg_byte**          m_pLMs;
	LW_RASTER_SEGMENT* m_pSegments;
	LW_REGION_EX*      m_pRegions;
	LW_GEO_BLOCK_EX*   m_pGeoBlocks;
	LW_GEO_FACE*       m_pGeoFaces;
	LW_LIGHT*          m_pLights;
	LW_GEO_VERTEX*     m_pGeoVerts;
	LW_GEO_TRI*        m_pGeoTris;
	
	//The memory lump
	lg_byte* m_pMem;
private:
	lg_bool AllocateMemory();
	void DeallocateMemory();
public:
	CLMap();
	~CLMap();
	
	lg_bool LoadFromWS(lf_path szFilename);
	lg_bool Load(lf_path szFilename);
	lg_bool LoadReal(lf_path szFilename);
	void Unload();
	
	lg_bool IsLoaded();
	const lg_str GetMapPath();
	lg_dword CheckRegions(const ML_AABB* pAABB, lg_dword* pRegions, lg_dword nMaxRegions);
};

class CLMapD3D
{
public:
	static const lg_dword  MAPDEBUG_HULLAABB=0x00000001;
	static const lg_dword  MAPDEBUG_HULLTRI=0x00000002;
	static const lg_dword  MAPDEBUG_WORLDAABB=0x00000004;
private:
	//The actual map.
	CLMap* m_pMap;
	//Direct3D interfaces.
	IDirect3DDevice9*       m_pDevice;
	tm_tex*     m_pTextures;
	tm_tex*     m_pLMTextures;
	IDirect3DVertexBuffer9* m_pVB;
	
	//Data lump.
	lg_byte* m_pMem;
	
	lg_bool m_bIsValid;

public:
	CLMapD3D();
	~CLMapD3D();
	
	void Init(IDirect3DDevice9* pDevice, CLMap* pMap);
	void UnInit();
	
	void Render();
	void RenderAABBs(lg_dword nFlags);
	void Validate();
	void Invalidate();
};

#if 0

class CLWorldMap: public CLMap
{
friend class CLEntity;
friend class CLWorld;
friend class CLJack;
friend class CLPhysBody;
friend class CElementNewton;
friend class CElementPhysX;

private:
	lg_bool m_bD3DValid;
	
	//Direct3D Interfaces
	IDirect3DDevice9*       m_pDevice;
	
	tm_tex*     m_pTextures;
	tm_tex*     m_pLMTextures;
	
	IDirect3DVertexBuffer9* m_pVB;
	
public:
	CLWorldMap();
	~CLWorldMap();
	void Render();
	lg_bool Validate();
	void Invalidate();
	lg_bool LoadFromWS(lf_path szFilename, IDirect3DDevice9* pDevice);
	lg_bool Load(lf_path szFilename, IDirect3DDevice9* pDevice);
	void Unload();
};

#endif

#endif __LW_MAP_H__