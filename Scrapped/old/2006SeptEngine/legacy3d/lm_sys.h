#ifndef __LM_SYS_H__
#define __LM_SYS_H__

#include <common.h>
#include <d3dx9.h>


#define LMESH_ID      (*(L_dword*)"L3DM")
#define LMESH_VERSION (116) //Final version will be 200.


#define LMESH_MAX_NAME_LENGTH (32)
#define LMESH_MAX_PATH        (64)

//The Structures:
#pragma pack(1)

typedef struct _LMESH_VERTEX{
	float x, y, z;    //Vertex Coordinates.
	float nx, ny, nz; //Vertex Normals.
	float tu, tv;     //Texture Coordinates.
}LMESH_VERTEX, *PLMESH_VERTEX;

#define LMESH_VERTEX_FORMAT (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX1)


typedef struct _LMESH_SET{
	char    szName[LMESH_MAX_NAME_LENGTH];   //Name of the mesh.
	L_dword nFirstVertex;    //Index of the first vertex in the mesh.
	L_dword nNumTriangles;    //Number of triangles in the mesh.
	L_dword nMaterialIndex;        //Index of the mesh material.
}LMESH_SET, *PLMESH_SET;

typedef struct _LMESH_MATERIAL{
	char szMaterialName[LMESH_MAX_NAME_LENGTH];
   char szTexture[LMESH_MAX_PATH];
}LMESH_MATERIAL, *PLMESH_MATERIAL;

typedef struct _LMESH_BONE_LOCATION{
	float xr, yr, zr; //x, y, and z rotation.
	float xp, yp, zp; //x, y, and z position.
}LMESH_BONE_LOCATION;

typedef struct _LMESH_JOINT{
	char szName[LMESH_MAX_NAME_LENGTH];
	char szParentBone[LMESH_MAX_NAME_LENGTH];
	L_dword nParentBone;
	float position[3];
	float rotation[3];
}LMESH_JOINT;

typedef struct _LMESH_JOINTEX: public LMESH_JOINT{
	D3DXMATRIX Local;
	D3DXMATRIX Final;
	L_dword    nJointRef;
}LMESH_JOINTEX;

typedef struct _LMESH_JOINTPOS{
	float position[3];
	float rotation[3];
}LMESH_JOINTPOS;

typedef struct _LMESH_JOINTPOSEX: public LMESH_JOINTPOS{
	D3DXMATRIX Local;
	D3DXMATRIX Final;
}LMESH_JOINTPOSEX;

typedef struct _LMESH_KEYFRAME{
	LMESH_JOINTPOSEX* pJointPos;
}LMESH_KEYFRAME;


//The Format:
typedef struct _LMESH_CALLBACKS{
	int (__cdecl *close)(void* stream);
	int (__cdecl *seek)(void* stream, long offset, int origin);
	int (__cdecl *tell)(void* stream);
	unsigned int (__cdecl *read)(void* buffer, unsigned int size, unsigned int count, void* stream);
}LMESH_CALLBACKS, *PLMESH_CALLBACKS;

#pragma pack()


D3DXMATRIX* LM_EulerToMatrix(D3DXMATRIX* pOut, float* pEuler);


#define LSKEL_ID      (*(L_dword*)"LSKL")
#define LSKEL_VERSION 101

class CLegacyMesh;

class CLegacySkeleton{
public:
	friend class CLegacyMeshD3D;
	CLegacySkeleton();
	~CLegacySkeleton();

	L_bool Load(void* file, LMESH_CALLBACKS* pcb);
	L_bool CalcExData();
	L_bool Save(char* szFilename, L_bool bAppend);
	L_bool Unload();
	L_dword GetNumFrames();
protected:
	L_bool AllocateMemory();
	void DeallocateMemory();
protected:
	L_dword m_nID;
	L_dword m_nVersion;
	L_dword m_nNumJoints;
	L_dword m_nNumKeyFrames;
	LMESH_JOINTEX*  m_pBaseSkeleton;
	LMESH_KEYFRAME* m_pKeyFrames;
	L_dword m_nTotalSize;

	L_bool m_bLoaded;
};

class CLegacyMesh{
public:
	CLegacyMesh();
	virtual ~CLegacyMesh();

	L_bool         Load(void* file, LMESH_CALLBACKS* pcb, char* szModelPath);
	L_bool         Save(L_lpstr szFilename);
	virtual L_bool Unload();
	L_dword        GetNumFrames();
protected:

protected:
	L_dword m_ID;
	L_dword m_nVersion;
	L_dword m_nNumVertices;
	L_dword m_nNumMeshes;
	L_dword m_nNumMaterials;

	LMESH_VERTEX*   m_pVertices;
	L_dword*        m_pVertexBoneList;
	LMESH_SET*      m_pMeshes;
	LMESH_MATERIAL* m_pMaterials;
	CLegacySkeleton m_cSkel;


	L_bool m_bLoaded;

private:

};

#endif //__SM_SYS_H__