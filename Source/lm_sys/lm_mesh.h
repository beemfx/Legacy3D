#ifndef __LM_MESH_H__
#define __LM_MESH_H__

#include "lm_base.h"

class CLMesh2: public CLMBase
{
friend class CLSkin;
//Internally used constants:
public:
	static const lg_dword LM_MESH_ID=0x48534D4C;//(*(lg_dword*)"LMSH");
	static const lg_dword LM_MESH_VER=(122);//Final version will be 200.

//Internally used classes and structs:
public:
	//MeshVertex is the type of vertexes that make up
	//the mesh.  It is designed to be direct3D compatible
	//but should be OpenGL compatible as well.
	struct MeshVertex{
		lg_float x, y, z;    //Vertex Coordinates.
		lg_float nx, ny, nz; //Vertex Normals.
		lg_float tu, tv;     //Texture Coordinates.
	};
	
	//SubMesh is a smaller mesh within the entire mesh,
	//submeshes are used for different parts of the mesh
	//that use different materials, typically you want as
	//few as possible, because they are rendered separately.
	struct SubMesh{
		LMName szName;        //Name of the submesh.
		lg_dword nFirstIndex; //Index of the first vertex in the mesh.
		lg_dword nNumTri;     //Number of triangles in the mesh.
		lg_dword nMtrIndex;   //Index of the mesh's material.
	};
	
	//MeshMtr is the structure that stores information about
	//the texture, not that this is only default information
	//as typically a mesh gets it's texture from a skin.
	struct MeshMtr{
		LMName szName; //Name of the material.
		LMPath szFile; //Filename of material (relative to the mesh);
	};
	
//The actual model format:
//The model format in the file is pretty much exactly
//as it is seen here, the header is first, followed by
//the data (the size of which can be calculated using
//the information in the header).
protected:
	//File Format Start:
	lg_dword m_nID;
	lg_dword m_nVer;
	LMName   m_szMeshName;
	lg_dword m_nNumVerts;
	lg_dword m_nNumTris;
	lg_dword m_nNumSubMesh;
	lg_dword m_nNumMtrs;
	lg_dword m_nNumBones;
	
	MeshVertex* m_pVerts;
	lg_word*    m_pIndexes; //Should be sized 3*m_nNumTriangles*sizeof(lg_word) in size;
	lg_dword*   m_pVertBoneList;
	LMName*     m_pBoneNameList;
	SubMesh*    m_pSubMesh;
	MeshMtr*    m_pMtrs;
	ml_aabb     m_AABB; //The bounding box for the static mesh (note that skeletal bounding boxes are used for animated objects).
	//File Format End.
	
	//Additional information that is stored:
	lg_byte* m_pMem;      //The memory allocation chunk.
	LMPath m_szModelPath; //Stores the model path information.

public:
	CLMesh2();
	~CLMesh2();
		
public:
	virtual lg_bool Load(LMPath szFile)=0;
	virtual lg_bool Save(LMPath szFile)=0;
	
	virtual lg_void Unload();
	
protected:
	lg_bool AllocateMemory();
	lg_void DeallocateMemory();
	
	virtual lg_bool Serialize(
		lg_void* file, 
		ReadWriteFn read_or_write,
		RW_MODE mode);	
};

#endif __LM_MESH_H__