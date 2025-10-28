#ifndef __WS_FILE_H__
#define __WS_FILE_H__

#include "common.h"
#include "ML_lib.h"
#include <memory.h>


class CWSMemFile
{
private:
	lg_byte* m_pData;
	lg_dword m_nSize;
	lg_dword m_nPosition;
public:
	CWSMemFile():m_pData(LG_NULL),m_nSize(0),m_nPosition(0){}
	~CWSMemFile(){}
	
	void Open(lg_byte* pData, lg_dword nSize)
	{
		m_pData=pData; m_nSize=nSize; m_nPosition=0;
	}
	void Seek(lg_dword nMode, lg_long nOffset)
	{
		if(nMode==MEM_SEEK_SET)
		{
			m_nPosition=nOffset;
		}
		else if(nMode==MEM_SEEK_CUR)
		{
			m_nPosition+=nOffset;
		}
		else if(nMode==MEM_SEEK_END)
		{
			m_nPosition=m_nSize+nOffset;
		}
		
		if(m_nPosition>m_nSize)
				m_nPosition=nOffset>=0?m_nSize:0;
	}
	lg_dword Tell()
	{
		return m_nPosition;
	}
	
	lg_dword Read(lg_void* pOut, lg_dword nSize)
	{
		if((m_nPosition+nSize)>m_nSize)
		{
			nSize=m_nSize-m_nPosition;
		}
		
		memcpy(pOut, &m_pData[m_nPosition], nSize);
		m_nPosition+=nSize;
		return nSize;
	}
	
	lg_byte* GetPointer(lg_dword nOffset)
	{
		if(nOffset>=m_nSize)
			return LG_NULL;
		
		return &m_pData[nOffset];
	}
	
	lg_dword GetSize(){return m_nSize;}
	
	static const lg_dword MEM_SEEK_SET=0;
	static const lg_dword MEM_SEEK_CUR=1;
	static const lg_dword MEM_SEEK_END=2;
};

#define WS_MAX_NAME 32
#define WS_MAX_PATH 256

typedef lg_char WS_PATH[WS_MAX_PATH];
typedef lg_char WS_NAME[WS_MAX_NAME];

typedef struct _WS_VERTEX{
	ML_VEC3  v3Pos;
	ML_VEC2  v2Tex;
	ML_VEC2  v2LM;
}WS_VERTEX;

typedef struct _WS_MATERIAL{
	WS_PATH  szMaterial;
	lg_dword nRef;
}WS_MATERIAL;

typedef struct _WS_LIGHTMAP{
	lg_byte* pLM;
	lg_dword nSize;
	lg_dword nRef;
}WS_LIGHTMAP;

typedef struct _WS_VISGROUP{
	WS_NAME  szName;
	lg_dword nRef;
	
	lg_dword nFirstFace;
	lg_dword nFaceCount;
	lg_dword nVisibleFaceCount;
	
	lg_dword nFirstVertex;
	lg_dword nVertexCount;
	
	lg_dword nFirstGeoBlock;
	lg_dword nGeoBlockCount;
	
	lg_dword nFirstLight;
	lg_dword nLightCount;
	
	ML_AABB  aabbVisBounds;
}WS_VISGROUP;


typedef struct _WS_RASTER_FACE{
	lg_bool  bVisible;
	lg_dword nFirst;
	lg_dword nNumTriangles;
	
	lg_dword nMatRef;
	lg_dword nLMRef;
	
	lg_dword nVisGrp;
}WS_RASTER_FACE;

typedef struct _WS_GEO_BLOCK{
	lg_dword nFirstVertex;
	lg_dword nVertexCount;
	lg_dword nFirstTri;
	lg_dword nNumTris;
	lg_dword nFirstGeoFace;
	lg_dword nGeoFaceCount;
	ML_AABB  aabbBlock;
	lg_dword nVisGrp;
}WS_GEO_BLOCK;

typedef struct _WS_GEO_FACE{
	ML_PLANE plnEq;
}WS_GEO_FACE;

typedef struct _WS_COLOR{
	lg_byte r, g, b;
}WS_COLOR;

typedef struct _WS_LIGHT{
	ML_VEC3  v3Pos;
	WS_COLOR Color;
	lg_float fIntensity;
	lg_dword nVisGrp;
	//lg_long  nRange;
	//lg_bool linearfalloff...
}WS_LIGHT;

typedef struct _WS_GEO_VERT{
	ML_VEC3 v3Pos;
}WS_GEO_VERT;

typedef struct _WS_GEO_TRIANGLE{
	ML_VEC3 v3Vert[3];
}WS_GEO_TRI;


class CWSFile
{
private:
	//Map Header.
	lg_word m_nVersion;
	lg_byte m_nFileFlags;
	lg_long m_nNameCount;
	lg_long m_nNameOffset;
	lg_long m_nObjectCount;
	lg_long m_nObjectOffset;
	
	lg_dword m_nMatCount;
	lg_dword m_nLMCount;
	lg_dword m_nLMMaxSize;
	lg_dword m_nBrushCount;
	lg_dword m_nFaceCount;
	lg_dword m_nVisibleFaceCount;
	lg_dword m_nVertexCount;
	lg_dword m_nVisGroupCount;
	lg_dword m_nLightCount;
	lg_dword m_nGeoVertCount;
	lg_dword m_nGeoTriCount;
	
	CWSMemFile m_File;
	
	ML_AABB         m_aabbWorld;
	
	lg_char**       m_szNames;
	WS_VERTEX*      m_pVertexes;
	WS_MATERIAL*    m_pMaterials;
	WS_LIGHTMAP*    m_pLMs;
	WS_VISGROUP*    m_pVisGrps;
	WS_RASTER_FACE* m_pRasterFaces;
	WS_GEO_FACE*    m_pGeoFaces;
	WS_GEO_BLOCK*   m_pGeoBlocks;
	WS_LIGHT*       m_pLights;
	WS_GEO_VERT*    m_pGeoVerts;
	WS_GEO_TRI*     m_pGeoTris;
	
	void GetNames();
	void GetCounts();
	void GetObjects();
	void ModifyRefsAndVertexOrder();
	void SortAndCompactRasterObjects();
	void SortAndCompactEntitys();
	void SortAndCompactGeoObjects();
	void AllocateMemory();
	void DeallocateMemory();
	
	lg_bool GetLight(WS_LIGHT* lt);
	WS_COLOR StringToColor(lg_str szColor);
public:
	CWSFile();
	~CWSFile();
	
	lg_bool Load(lg_byte* pData, lg_dword nSize);
	lg_bool SaveAsLW(lg_char* szFilename);
	
	void ScaleMap(lg_float fScale);
};

#endif __WS_FILE_H__