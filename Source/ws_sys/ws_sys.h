#ifndef __WS_SYS_H__
#define __WS_SYS_H__
//I don't want to use common.h in this file,
//but for now it is used for file IO.
#include "common.h"

#include <pshpack1.h>

#define WS_MAX_STRING_LENGTH 512
#define WS_TRUE  1
#define WS_FALSE 0
#define WS_NULL  0

#define WS_MAP_VERSION 14

typedef char wsChar;
typedef unsigned char wsByte;
typedef unsigned short wsWord;
typedef unsigned long wsDword;
typedef signed short wsShort;
typedef signed long wsLong;
typedef float wsFloat;
typedef char* wspString;
typedef unsigned long wsName;
typedef int wsBool;
typedef unsigned char wsString[WS_MAX_STRING_LENGTH];

#define WS_SAFE_DELETE_ARRAY(p) {if(p){delete[]p;p=WS_NULL;}}


typedef struct _wsColor3
{
	wsByte r, g, b;
}wsColor3;

typedef struct _wsVec2
{
	wsFloat x, y;
}wsVec2;

typedef struct _wsVec3
{
	wsFloat x, y, z;
}wsVec3;

typedef struct _wsVec4
{
	wsFloat x, y, z, w;
}wsVec4;

//Object Types:
typedef struct _wsGroup{
	wsByte Flags;
	wsLong Index;
}wsGroup;

typedef struct _wsVisGroup{
	wsByte Flags;
	wsName Name;
	wsColor3 Color;
}wsVisGroup;

#define WS_MATERIAL_EXT_NAME 0x02
typedef struct _wsMaterial{
	wsByte Flags;
	wsName GroupName;
	wsName ObjectName;
	
	wsName ExtensionName; //Only if Flags&2
}wsMaterial;

typedef struct _wsMeshReference{
	wsByte Flags;
	wsName GroupName;
	wsName ObjectName;
	wsByte LimbCount;
}wsMeshReference;

typedef struct _wsLightMap{
	wsByte Flags;
	wsByte Resolution; //Actually 2^Resolution (always square).
	wsLong Format;
	wsColor3* Pixels;
}wsLightMap;

typedef struct _wsKey
{
	wsName  Name;
	wsName Value;
}wsKey;

#define WS_FACE_LIGHTMAP 0x10
typedef struct _wsIndex{
	wsByte Vertex;
	wsVec2 TexCoords;
	
	wsVec2 LightMapTexCoords; //If 16 & Flags in wsFace structure
}wsIndex;

typedef struct _wsFace{
	wsByte Flags;
	wsVec4 FacePlaneEquation;
	wsVec2 TexturePosition;
	wsVec2 TextureScale;
	wsVec2 TextureRotation;
	wsVec4 UTexMappingPlane;
	wsVec4 VTexMappingPlane;
	wsFloat LuxelSize;
	wsLong SmoothGroupIndex;
	wsLong MaterialIndex;
	
	wsLong LightMapIndex; //If Flags&0x10
	wsByte IndexCount;
	
}wsFace;

#define WS_BRUSH_HIDDEN 0x04
typedef struct _wsBrush
{
	wsByte Flags;
	wsLong NumKeys;
	wsKey* Keys;
	wsLong GroupIndex;
	wsLong VisGroupIndex;
	wsColor3 BrushColor;
	wsByte VertexCount;
	wsVec3* VertexPositions;
	wsByte FaceCount;
	wsFace* Faces;
}wsBrush;

#define WS_MESH_LIT 0x10
typedef struct _wsMeshShade{
	wsLong MaterialIndex;
	
	//If(16&Flags) from wsMesh
	wsShort VertexCount;
	wsColor3* VertexColor;
}wsMeshShade;

#define WS_MESH_NO_SCALE 0x01
typedef struct _wsMesh{
	wsByte Flags;
	wsLong NumKeys;
	wsKey* Keys;
	wsLong Group;
	wsLong VisGroup;
	wsColor3 MeshColor;
	wsLong MeshRefIndex;
	wsVec3 Position;
	wsVec3 Rotation;
	
	wsVec3 Scale; //If !(Flags&1)
	
	wsMeshShade* LimbShader;
}wsMesh;

typedef struct _wsEntity{
	wsByte Flags;
	wsVec3 Position;
	wsLong NumKeys;
	wsKey* Keys;
	wsLong Group;
	wsLong VisGroup;
}wsEntity;
#include <poppack.h>

class CWSFile
{
private:
	//Header Data:
	wsWord m_nMapVersion; //Should be 14
	wsByte m_nMapFlags;
	wsLong m_nNameCount;
	wsLong m_nNameOffset;
	wsLong m_nObjectCount;
	wsLong m_nObjectOffset;
	
	wsString* m_pNameTable;
	
	wsBool m_bOpen;
private:
	static wsDword ReadString(void* fin, fio_callbacks* pCB, wsString szOut);
public:
	CWSFile();
	~CWSFile();
	
	wsBool TempOpen(wspString szFilename);
	void Close();
		
};

#endif __WS_SYS_H__