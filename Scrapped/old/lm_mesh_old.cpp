
#if 0

#include "lm_mesh_lg.h"
#include "lm_skin.h"

class CLMeshNode
{
friend class CLMeshNode;
public:
	CLMeshLG*   m_pMesh;      //The mesh for this node
	CLSkelLG*   m_pDefSkel;   //The default skeleton for this mesh.
	CLSkin       m_Skin; //The skin for this node.
private:
	CLSkelLG*      m_pSkel;      //The current skel for this node
	lg_dword      m_nAnim;      //The animation to use in the current skel
	float        m_fTime;      //The time for the animation (0.0f to 100.0f).
	lg_long       m_nAnimSpeed; //How many milliseconds it takes to cycle through the animation
	lg_dword      m_nTime;      //The saved time for animation purposes
	lg_dword      m_nLastUpdate;//The last time the animation was updated
	//The following info is used for transition animations
	CLSkelLG*      m_pPrevSkel;  //The previous skeleton
	lg_dword      m_nPrevAnim;  //The previous animation within the previous skeleton
	float        m_fPrevTime;  //The previous time for the previous animation
	
	lg_dword      m_nTransTime; //How many milliseconds it should take to transition to the new animation
private:
	CLMeshNode*  m_pNext;        //The next node in the list.
	CLMeshNode*  m_pChildren;    //The children of this node
	CLMeshNode*  m_pParent;      //The parent of the current node (not used)
	lg_dword      m_nParentJoint; //The reference of the joint to which the child is attached.
public:
	CLMeshNode();
	~CLMeshNode();
	//Load a node.  Note that the nodes are only used for frame and animation purposes,
	//they don't allocate or deallocate any memory.  For this reason the load
	//method takes a preexisting mesh, the SetAnimation method takes a preexisting skeleton.
	//For this reason it is necessary that skeletons and meshes are not unloaded while
	//a mesh node still contains a reference to them.
	void Load(CLMeshLG* pMesh);
	void SetCompatibleWith(CLSkelLG* pSkel);
	void AddChild(CLMeshNode* pNode, lg_cstr szJoint);
	void Unload();
	//It is only necessary to call Render on the topmost parent node, all child nodes
	//will be render according to the necessary joint transform.
	void Render(const ML_MAT* matTrans);
	
	//UpdateTime should be called once a frame, this sets all the time parameters
	//necessary for setting up the animation.  Update time will update all child
	//nodes times as well so it is only necessary to call update time for the topmost
	//node in a set.
	void UpdateTime(lg_dword nTime);
	//SetAnimation changes the current animation to a new one, if the same animation
	//is specified the animation will not change.  It is only necessary to call this
	//method when changing to a different animation, however calling it every frame
	//with the same paramters will not change anything, but it will waste processor
	//time.
	void SetAnimation(CLSkelLG* pSkel, lg_dword nAnim, lg_long nSpeed, lg_dword nTransTime);
	
	const CLMeshNode* GetParent();
};



//We can only use the mesh loading functions if FS2 is being used,
//as fs2 is used for opening the xml file.
CLMeshNode* LM_LoadMeshNodes(lg_path szXMLScriptFile, lg_dword* nNumMeshes);
void LM_DeleteMeshNodes(CLMeshNode* pNodes);
#endif

typedef struct _LMESH_VERTEX{
	float x, y, z;    //Vertex Coordinates.
	float nx, ny, nz; //Vertex Normals.
	float tu, tv;     //Texture Coordinates.
}LMESH_VERTEX, *PLMESH_VERTEX;

#define LMESH_VERTEX_FORMAT (D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX1)


typedef struct _LMESH_SET{
	LMESH_NAME szName;         //Name of the mesh.
	lg_dword nFirstIndex;      //Index of the first vertex in the mesh.
	lg_dword nNumTriangles;    //Number of triangles in the mesh.
	lg_dword nMaterialIndex;   //Index of the mesh material.
}LMESH_SET, *PLMESH_SET;

typedef struct _LMESH_MATERIAL{
	LMESH_NAME szMaterialName;
   LMESH_PATH szTexture;
}LMESH_MATERIAL, *PLMESH_MATERIAL;

class CLMesh
{
friend class CLSkin;
#ifdef LM_USE_LF2
public:
	static lg_uint ReadLF2(void* buffer, lg_uint size, lg_uint count, void* stream);
	static lg_uint WriteLF2(void* buffer, lg_uint size, lg_uint count, void* stream);
#endif LM_USE_LF2
public:
	CLMesh();
	virtual ~CLMesh();

	lg_bool Load(void* file, LM_RW_FN read, char* szModelPath);
	lg_bool Save(void* file, LM_RW_FN write);
	#ifdef LM_USE_LF2
	lg_bool Load(lf_path szFilename);
	#endif LM_USE_LF2
	
	virtual lg_bool Unload();
	
	const ML_AABB* GetBoundingBox();
	
	lg_dword GetJointRef(lg_cstr szJoint);
		
	lg_bool IsLoaded();	
protected:
	lg_bool AllocateMemory();
	void DeallocateMemory();
	lg_bool Serialize(
		void* file, 
		LM_RW_FN read_or_write,
		lg_bool bLoading);
		
	void BuildFullTexPath(LMESH_PATH szOut, LMESH_PATH szTexPath);
	
		
protected:
	lg_dword m_ID;
	lg_dword m_nVersion;
	LMESH_NAME m_szMeshName;
	lg_dword m_nNumVertices;
	lg_dword m_nNumTriangles;
	lg_dword m_nNumMeshes;
	lg_dword m_nNumMaterials;
	lg_dword m_nNumBones;
	
	LMESH_VERTEX*    m_pVertices;
	lg_word*          m_pIndexes; //Should be sized 3*m_nNumTriangles*sizeof(lg_word) in size;
	lg_dword*         m_pVertexBoneList;
	LMESH_NAME*      m_pBoneNameList;
	LMESH_SET*       m_pMeshes;
	LMESH_MATERIAL*  m_pMaterials;
	ML_AABB          m_AABB; //The bounding box for the static mesh (note that skeletal bounding boxes are used for animated objects).
	
	LMESH_PATH       m_szModelPath; //Stores the model path information.
	
	lg_bool m_bLoaded;

};

//lm_mesh.cpp - Source Code for the Legacy Mesh (CLMesh).
//Note that the loading code is compatible with Legacy
//File System 2, but that loading code will only be available
//if LM_USE_LF2 is defined (this should be a project wide definition).
#include <memory.h>
#include <stdio.h>
#include "lm_sys.h"
#include "ML_lib.h"
#include "lg_func.h"

//////////////////////////////
///  The Legacy Mesh Class ///
//////////////////////////////

CLMesh::CLMesh():
	m_ID(0),
	m_nVersion(0),
	m_nNumVertices(0),
	m_nNumTriangles(0),
	m_nNumMeshes(0),
	m_nNumMaterials(0),
	m_nNumBones(0),

	m_pVertices(LG_NULL),
	m_pIndexes(LG_NULL),
	m_pVertexBoneList(LG_NULL),
	m_pBoneNameList(LG_NULL),
	m_pMeshes(LG_NULL),
	m_pMaterials(LG_NULL),
	
	m_bLoaded(LG_FALSE)
{
	m_szMeshName[0]=0;
	memset(&m_AABB, 0, sizeof(ML_AABB));
}

CLMesh::~CLMesh()
{
	this->Unload();
}

lg_bool CLMesh::AllocateMemory()
{
	//Before this function is called
	//m_nNumVertices, m_nNumMeshes, m_nNumMaterials should be set.
	
	//Call deallocate memory, just in case memory is already allocated:
	DeallocateMemory();
	
	m_pVertices=new LMESH_VERTEX[m_nNumVertices];
	m_pIndexes=new lg_word[m_nNumTriangles*3];
	m_pMeshes=new LMESH_SET[m_nNumMeshes];
	m_pMaterials=new LMESH_MATERIAL[m_nNumMaterials];
	m_pVertexBoneList=new lg_dword[m_nNumVertices];
	m_pBoneNameList=new LMESH_NAME[m_nNumBones];
	
	if(!m_pVertices || !m_pIndexes || !m_pMeshes || !m_pMaterials || !m_pVertexBoneList || !m_pBoneNameList)
	{
		//If all the memory necessary can't be allocated then...
		//Deallocate any memory that has been allocated...
		DeallocateMemory();
		//And return false for failure...
		return LG_FALSE;
	}
	return LG_TRUE;
}

void CLMesh::DeallocateMemory()
{
	L_safe_delete_array(m_pVertices);
	L_safe_delete_array(m_pIndexes);
	L_safe_delete_array(m_pMeshes);
	L_safe_delete_array(m_pMaterials);
	L_safe_delete_array(m_pVertexBoneList);
	L_safe_delete_array(m_pBoneNameList);
}

lg_dword CLMesh::GetJointRef(lg_cstr szJoint)
{
	for(lg_dword i=0; i<m_nNumBones; i++)
	{
		if(L_strnicmp(m_pBoneNameList[i], szJoint, 0))
			return i;
	}
	return 0xFFFFFFFF;
}

const ML_AABB* CLMesh::GetBoundingBox()
{
	return &m_AABB;
}

void CLMesh::BuildFullTexPath(LMESH_PATH szOut, LMESH_PATH szTexPath)
{
	lg_byte nStart=L_strnicmp(szTexPath, ".\\", 2) || L_strnicmp(szTexPath, "./", 2)?2:0;
	LG_strncpy(szOut, m_szModelPath, LMESH_MAX_PATH-1);
	L_strncat(szOut, &szTexPath[nStart], LMESH_MAX_PATH);
}
#ifdef LM_USE_LF2
//The read and write LF2 functions are used for the legacy mesh
//and legacy skel serialize methods when the LF2 file system is in
//use.
lg_uint CLMesh::ReadLF2(void* buffer, lg_uint size, lg_uint count, void* stream)
{
	return LF_Read((LF_FILE3)stream, buffer, size*count);
}

 lg_uint WriteLF2(void* buffer, lg_uint size, lg_uint count, void* stream)
{
	return LF_Write((LF_FILE3)stream, buffer, size*count);
}

lg_bool CLMesh::Load(lf_path szFilename)
{
	Unload();
	
	LF_FILE3 fIn=LF_Open(szFilename, LF_ACCESS_READ, LF_OPEN_EXISTING);
	
	if(!fIn)
		return LG_FALSE;
	
	lg_bool bResult=Serialize(fIn, ReadLF2, LG_TRUE);
	LF_Close(fIn);
	
	if(!bResult)
		return LG_FALSE;
	
	//Because the LF2 file system always specifies the
	//full path name to open a file, we only need to
	//use the GetDirFromPath function to get the path
	//to the model.  The path to the model is used
	//when loading textures.		
	LF_GetDirFromPath(m_szModelPath, szFilename);
	m_bLoaded=LG_TRUE;
	return LG_TRUE;
}

#endif

lg_bool CLMesh::Load(void* file, LM_RW_FN read, char* szModelPath)
{
	//First unload the file.
	Unload();
	
	if(!Serialize(file, read, LG_TRUE))
	{
		return LG_FALSE;
	}
	LG_strncpy(m_szModelPath, szModelPath, LMESH_MAX_PATH-1);

	m_bLoaded=LG_TRUE;
	return LG_TRUE;
}

lg_bool CLMesh::Unload()
{
	if(!m_bLoaded)
		return LG_TRUE;
	//Deallocate model's memory.
	DeallocateMemory();

	m_ID=0;
	m_nVersion=0;
	m_nNumVertices=0;
	m_nNumMeshes=0;
	m_nNumMaterials=0;

	m_pVertices=LG_NULL;
	m_pVertexBoneList=LG_NULL;
	m_pMeshes=LG_NULL;
	m_pMaterials=LG_NULL;


	m_bLoaded=LG_FALSE;
	
	return LG_TRUE;
}

lg_bool CLMesh::IsLoaded()
{
	return m_bLoaded;
}

lg_bool CLMesh::Serialize(
	void* file, 
	lg_uint (__cdecl * read_or_write)(void* buffer, lg_uint size, lg_uint count, void* stream), 
	lg_bool bLoading)
{
	//Read or Write the file header.
	read_or_write(&m_ID, 4, 1, file);
	read_or_write(&m_nVersion, 4, 1, file);
	read_or_write(m_szMeshName, LMESH_MAX_NAME, 1, file);
	read_or_write(&m_nNumVertices, 4, 1, file);
	read_or_write(&m_nNumTriangles, 4, 1, file);
	read_or_write(&m_nNumMeshes, 4, 1, file);
	read_or_write(&m_nNumMaterials, 4, 1, file);
	read_or_write(&m_nNumBones, 4, 1, file);
	
	if(m_ID!=LMESH_ID || m_nVersion!=LMESH_VERSION)
	{
		return LG_FALSE;
	}
	
	//Allocate memory if we are loading.
	if(bLoading)
		if(!AllocateMemory())return LG_FALSE;
	
	//Read or Write the vertexes.
	lg_dword i=0;
	for(i=0; i<m_nNumVertices; i++)
	{
		read_or_write(&this->m_pVertices[i], sizeof(LMESH_VERTEX), 1, file);
	}
	for(i=0; i<m_nNumTriangles; i++)
	{
		read_or_write(&this->m_pIndexes[i*3], sizeof(lg_word)*3, 1, file);
	}
	//Read or Write the bone indexes.
	read_or_write(this->m_pVertexBoneList, sizeof(lg_dword), m_nNumVertices, file);
	//Write the bone names:
	for(i=0; i<m_nNumBones; i++)
	{
		read_or_write(this->m_pBoneNameList[i], sizeof(LMESH_NAME), 1, file);
	}
	//Read or Write the meses.
	for(i=0; i<m_nNumMeshes; i++)
	{
		read_or_write(&this->m_pMeshes[i], sizeof(LMESH_SET), 1, file);
	}
	//Read or Write the materials.
	for(i=0; i<m_nNumMaterials; i++)
	{
		read_or_write(&this->m_pMaterials[i], sizeof(LMESH_MATERIAL), 1, file);
	}
	
	//Read or write the bounding box.
	read_or_write(&this->m_AABB, 24, 1, file);
	
	return LG_TRUE;
}

lg_bool CLMesh::Save(void* file, LM_RW_FN write)
{
	if(!m_bLoaded)
		return LG_FALSE;
		
	return Serialize(file, write, LG_FALSE);
}



class CLMeshD3D: public CLMesh
{
friend class CLMeshNode;
public:
	CLMeshD3D();
	~CLMeshD3D();
	//lg_bool Create(char* szFilename, IDirect3DDevice9* lpDevice);
	lg_bool CreateD3DComponents();//IDirect3DDevice9* lpDevice);
	lg_bool Validate();
	lg_bool Invalidate();
	lg_bool Render(CLSkin* pSkin=LG_NULL);
	lg_bool RenderNormals();
	void SetRenderTexture(lg_bool b);
	void SetCompatibleWithSkel(CLSkel* pSkel);
	lg_bool PrepareFrame(lg_dword dwFist, lg_dword dwSecond, float fTime, CLSkel* pSkel, lg_bool bSetupVerts=LG_TRUE);
	lg_bool PrepareFrame(lg_dword nAnim, float fTime, CLSkel* pSkel, lg_bool bSetupVerts=LG_TRUE);
	void PrepareDefaultFrame();
	lg_bool TransitionTo(lg_dword nAnim, float fTime, CLSkel* pSkel);
	
	lg_bool IsD3DReady();
	virtual lg_bool Unload();
	const ML_MAT* GetJointTransform(lg_dword nRef);
	const ML_MAT* GetJointAttachTransform(lg_dword nRef);
	
private:
	//IDirect3DDevice9*       m_lpDevice;
	IDirect3DVertexBuffer9* m_lpVB;
	IDirect3DIndexBuffer9*  m_lpIB;
	tm_tex*                 m_lppTexs;
	lg_bool                  m_bD3DValid;
	lg_bool                  m_bD3DReady;
	lg_bool                  m_bRenderTexture;
	//Some structures to hold the stuff that is used every
	//frame but is only temporary.
	ML_MAT*   m_pAnimMats;
	ML_MAT*   m_pAttachMats;
	
	//The joint reference coordination array.
	lg_dword* m_pBoneRef;
	
	void TransformVerts();
	
public:
	static void SetRenderStates();
	static void SetSkyboxRenderStates();
	static lg_bool InitMeshDevice(IDirect3DDevice9* pDevice);
	static void UnInitMeshDevice();
private:
	static IDirect3DDevice9* s_pDevice;
};


IDirect3DDevice9* CLMeshLG::s_pDevice=LG_NULL;

lg_bool CLMeshLG::InitMeshDevice(IDirect3DDevice9* pDevice)
{
	s_pDevice=pDevice;
	if(!s_pDevice)
		return LG_FALSE;	
	s_pDevice->AddRef();
	return LG_TRUE;
}
void CLMeshLG::UnInitMeshDevice()
{
	if(s_pDevice)
	{
		lg_ulong nRef=s_pDevice->Release();
		s_pDevice=nRef?s_pDevice:LG_NULL;
	}
}

void CLMeshLG::SetRenderStates()
{
	if(!s_pDevice)
		return;
		
	s_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);	
	s_pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	s_pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	s_pDevice->SetRenderState(D3DRS_NORMALIZENORMALS, TRUE);
	s_pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	
	s_pDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
	s_pDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
	s_pDevice->SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
	s_pDevice->SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
}

void CLMeshLG::SetSkyboxRenderStates()
{
	ML_MAT m2;
	ML_MatIdentity((ML_MAT*)&m2);
	s_pDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX*)&m2);
	SetRenderStates();
	s_pDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
	s_pDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
	s_pDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
	
	s_pDevice->SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	s_pDevice->SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
}


CLMeshLG::CLMeshLG():
CLMesh()
{
	//m_lpDevice=LG_NULL;
	m_lpVB=LG_NULL;
	m_lpIB=LG_NULL;
	m_lppTexs=LG_NULL;
	m_bD3DValid=LG_FALSE;
	m_pAnimMats=LG_NULL;
	m_bRenderTexture=LG_TRUE;
	m_pBoneRef=LG_NULL;
	m_bD3DReady=LG_FALSE;
}

CLMeshLG::~CLMeshLG()
{
	Unload();
}

lg_bool CLMeshLG::CreateD3DComponents()//IDirect3DDevice9* lpDevice)
{
	if(!m_bLoaded)
		return LG_FALSE;
		
	if(!s_pDevice)
		return LG_FALSE;
		
	if(m_bD3DReady)
		return LG_TRUE;
	//Allocate memory for textures.
	m_lppTexs=new tm_tex[m_nNumMaterials];
	//Prepare the joints for rendering.
	m_pAnimMats=new ML_MAT[m_nNumBones*2];
	m_pAttachMats=&m_pAnimMats[m_nNumBones];
	//Set up the bone references
	m_pBoneRef=new lg_dword[m_nNumBones];
	for(lg_dword i=0; i<m_nNumBones; i++)
		m_pBoneRef[i]=i;
	
	lg_dword i=0;
	for(i=0; i<m_nNumBones; i++)
	{
		ML_MatIdentity(&m_pAnimMats[i]);
	}
	
	//Load the textures.
	for(i=0; i<m_nNumMaterials; i++)
	{
		LMESH_PATH szFullTexPath;
		BuildFullTexPath(szFullTexPath, m_pMaterials[i].szTexture);
		m_lppTexs[i]=CLTMgr::TM_LoadTex(szFullTexPath, 0);//Tex_Load2(szFullTexPath);
	}
	
	if(!Validate())
	{
		LG_SafeDeleteArray(m_lppTexs);
		LG_SafeDeleteArray(m_pAnimMats);
		return LG_FALSE;
	}
	m_bD3DReady=LG_TRUE;
	return LG_TRUE;
}


lg_bool CLMeshLG::Unload()
{
	if(!m_bLoaded)
		return LG_TRUE;
	lg_dword i=0;
	Invalidate();
	L_safe_delete_array(m_lppTexs);
	L_safe_delete_array(m_pAnimMats);
	L_safe_delete_array(m_pBoneRef);
	this->CLMesh::Unload();
	
	m_lpVB=LG_NULL;
	m_lpIB=LG_NULL;
	m_lppTexs=LG_NULL;
	m_bD3DValid=LG_FALSE;

	return LG_TRUE;
}

lg_bool CLMeshLG::IsD3DReady()
{
	return m_bD3DReady;
}

lg_bool CLMeshLG::Validate()
{
	//If the device has not been set, then we can't validate.
	if(!s_pDevice)
		return LG_FALSE;
	//If we are already valid we don't need to revalidate the object.
	if(m_bD3DValid)
		return LG_TRUE;
	//Create the vertex buffer and fill it with the default information.
	lg_result nResult=0;
	nResult=s_pDevice->CreateVertexBuffer(
		m_nNumVertices*sizeof(LMESH_VERTEX),
		D3DUSAGE_WRITEONLY,
		LMESH_VERTEX_FORMAT,
		D3DPOOL_DEFAULT,
		&m_lpVB,
		LG_NULL);

	if(LG_FAILED(nResult))
	{
		return LG_FALSE;
	}
	
	nResult=s_pDevice->CreateIndexBuffer(
		m_nNumTriangles*3*sizeof(*m_pIndexes),
		D3DUSAGE_WRITEONLY,
		D3DFMT_INDEX16,
		D3DPOOL_DEFAULT,
		&m_lpIB,
		LG_NULL);
	
	if(LG_FAILED(nResult))
	{
		m_lpVB->Release();
		return LG_FALSE;
	}
	// Now fill the vertex buffer with the default vertex data.
	// Note that animation data will be created and filled into
	// the vertex buffer on the fly.
	void* lpBuffer=LG_NULL;

	//Fill in the vertex buffer (note that this is the only time
	//that texture coords are written, but the vertex and normal
	//data gets rewritten when PrepareFrame is called.
	nResult=m_lpVB->Lock(0, m_nNumVertices*sizeof(LMESH_VERTEX), &lpBuffer, 0);
	if(LG_FAILED(nResult))
	{
		L_safe_release(m_lpVB);
		L_safe_release(m_lpIB);
		return LG_FALSE;
	}

	memcpy(lpBuffer, m_pVertices, m_nNumVertices*sizeof(LMESH_VERTEX));

	m_lpVB->Unlock();
	
	//Now fill the index buffer:
	nResult=m_lpIB->Lock(0, m_nNumTriangles*3*sizeof(*m_pIndexes), &lpBuffer, 0);
	if(LG_FAILED(nResult))
	{
		L_safe_release(m_lpVB);
		L_safe_release(m_lpIB);
		return LG_FALSE;
	}
	memcpy(lpBuffer, m_pIndexes, m_nNumTriangles*3*sizeof(*m_pIndexes));
	m_lpIB->Unlock();

	m_bD3DValid=LG_TRUE;
	return LG_TRUE;
}

lg_bool CLMeshLG::Invalidate()
{
	if(!m_bD3DValid)
		return LG_FALSE;

	lg_dword i=0;
	L_safe_release(m_lpVB);
	L_safe_release(m_lpIB);
	
	m_bD3DValid=LG_FALSE;
	return LG_TRUE;
}

void CLMeshLG::PrepareDefaultFrame()
{
	for(lg_dword i=0; i<m_nNumBones*2; i++)
		ML_MatIdentity(&m_pAnimMats[i]);
		
	TransformVerts();
}


/**********************************************************************
*** Technically the prepare frame functions should
*** get a compatible skeleton, not all skeleton's are compatible
*** with every model so in the future there will be a method to
*** check compatibility, but this should not be called every frame
*** as it would only use unecessary processor power, instead
*** it is up to the the programmer to make sure a skeleton is
*** compatible before using it for rendering, as the function stands
*** right now the PrepareFrame functions simply try to prepare a
*** frame based off joint references
***********************************************************************/
lg_bool CLMeshLG::PrepareFrame(lg_dword dwFrame1, lg_dword dwFrame2, float t, CLSkel* pSkel, lg_bool bSetupVerts)
{
	if(!pSkel || !pSkel->IsLoaded())
	{
		PrepareDefaultFrame();
		return LG_FALSE;
	}
	
	ML_MAT* pJointTrans=(ML_MAT*)m_pAnimMats;
	//Prepare each joints final transformation matrix.
	for(lg_dword i=0; i<m_nNumBones; i++)
	{
		//To calculate the interpolated transform matrix
		//we need only call MCSkel's GenerateJointTransform method.
		pSkel->GenerateJointTransform(&pJointTrans[i], m_pBoneRef[i], dwFrame1, dwFrame2, t);
		//We don't necessarily need to setup attachment matrices for every bone (only for the
		//ones that actually get objects attached to them), but for now we set them up for
		//all bone anyway.
		ML_MatMultiply(&m_pAttachMats[i], pSkel->GetBaseTransform(m_pBoneRef[i]), &pJointTrans[i]);
	}
	//Transform all the vertices.
	if(bSetupVerts)
		TransformVerts();
	
	return LG_TRUE;
}

//TransformVerts is called to actually transform
//all the vertices in the mesh based upon the joint transforms.
void CLMeshLG::TransformVerts()
{
	if(!m_bD3DValid)
		return;
		
	//Lock the vertex buffer to prepare to write the data.
	LMESH_VERTEX* pVerts=LG_NULL;
	if(LG_FAILED(m_lpVB->Lock(0, m_nNumVertices*sizeof(LMESH_VERTEX), (void**)&pVerts, 0)))
		return;

	//Now that the transformations are all set up,
	//all we have to do is multiply each vertex by the 
	//transformation matrix for it's joint.  Note that
	//the texture coordinates have already been established
	//so they are not recopyed here.
	for(lg_dword i=0; i<m_nNumVertices; i++)
	{
		//Copy the vertex we want.
		//memcpy(&pVerts[i], &m_pVertices[i], sizeof(LMESH_VERTEX));
		
		//If the vertex's bone is -1 it means that it is
		//not attached to a joint so we don't need to transform it.
		if(m_pVertexBoneList[i]!=-1)
		{
			//Multiply each vertex by it's joints transform.
			ML_Vec3TransformCoord((ML_VEC3*)&pVerts[i].x, (ML_VEC3*)&m_pVertices[i].x, &m_pAnimMats[m_pVertexBoneList[i]]);	
			//Do the same for each vertex's normal, but we don't transorm by the
			//x,y,z.. so we call Vec3TransformNormal instead of Coord.
			ML_Vec3TransformNormal((ML_VEC3*)&pVerts[i].nx, (ML_VEC3*)&m_pVertices[i].nx, &m_pAnimMats[m_pVertexBoneList[i]]);
		}
	}
	
	//We've written all the vertices so lets unlock the VB.
	m_lpVB->Unlock();
	return;
}

//TransitionTo should be called after PrepareFrame, it will generate
//transforms to transform the current frame to the new animation.
lg_bool CLMeshLG::TransitionTo(lg_dword nAnim, float fTime, CLSkel* pSkel)
{
	const LSKEL_ANIM* pAnim=pSkel->GetAnim(nAnim);
	for(lg_dword i=0; i<m_nNumBones; i++)
	{
		ML_MAT matTemp;
		ML_MAT matTemp2;
		pSkel->GenerateJointTransform(&matTemp, m_pBoneRef[i], pAnim->m_nFirstFrame, pAnim->m_nFirstFrame, 0.0f);
		ML_MatMultiply(&matTemp2, pSkel->GetBaseTransform(m_pBoneRef[i]), &matTemp);
		ML_MatSlerp(&m_pAnimMats[i], &m_pAnimMats[i], &matTemp, fTime);
		ML_MatSlerp(&m_pAttachMats[i], &m_pAttachMats[i], &matTemp2, fTime);
	}
	TransformVerts();
	return LG_TRUE;
}

const ML_MAT* CLMeshLG::GetJointTransform(lg_dword nRef)
{
	if(nRef<m_nNumBones)
		return &m_pAnimMats[nRef];
	else
		return LG_NULL;
}

const ML_MAT* CLMeshLG::GetJointAttachTransform(lg_dword nRef)
{
	if(nRef<m_nNumBones)
		return &m_pAnimMats[nRef+m_nNumBones];
	else
		return LG_NULL;
}

//This verison of prepare frames takes an animation and
//it wants a value from 0.0f to 100.0f.  0.0f would be
//the first frame of the animation, and 100.0f would also
//be the first frame of the animation.
lg_bool CLMeshLG::PrepareFrame(lg_dword nAnim, float fTime, CLSkel* pSkel, lg_bool bSetupVerts)
{
	lg_dword nFrame1, nFrame2;
	nFrame1=pSkel->GetFrameFromTime(nAnim, fTime, &fTime, &nFrame2);
	return PrepareFrame(nFrame1, nFrame2, fTime, pSkel, bSetupVerts);
}

void CLMeshLG::SetCompatibleWithSkel(CLSkel* pSkel)
{
	if(!pSkel || !pSkel->IsLoaded())
		return;
		
	for(lg_dword i=0; i<m_nNumBones; i++)
	{
		for(lg_dword j=0; j<pSkel->m_nNumJoints; j++)
		{
			if(L_strnicmp(this->m_pBoneNameList[i], pSkel->m_pBaseSkeleton[j].szName, 0))
			{
				m_pBoneRef[i]=j;
				j=pSkel->m_nNumJoints+1;
			}
		}
	}
}

void CLMeshLG::SetRenderTexture(lg_bool b)
{
	m_bRenderTexture=b;
}

/* Render()
	Alright this methods is setup to render with a skin.
	Note however, that this is not optimized at all this
	is just a basic rendering method, while we work on
	other parts of the engine.  This has quite a few branches,
	and in general is not optimized.  It is set up for pixel
	and vertex shading.
*/
lg_bool CLMeshLG::Render(CLSkin* pSkin)
{
	//Test render the model.
	if(!m_bD3DValid)
		return LG_FALSE;

	//PrepareFrame should not be called here, but should be called
	//by the application prior to the call to Render(). 
	//PrepareFrame(0, 1, 0.0f);

	s_pDevice->SetFVF(LMESH_VERTEX_FORMAT);
	s_pDevice->SetStreamSource(0, m_lpVB, 0, sizeof(LMESH_VERTEX));
	s_pDevice->SetIndices(m_lpIB);
	lg_dword i=0;
	lg_bool bTextured=m_bRenderTexture;
	
	//Set the material for the mesh, in the future this might
	//be controlled by model properties, for now it is 
	//just a default material.
	D3DMATERIAL9 Material={
	{1.0f, 1.0f, 1.0f, 1.0f},//Diffuse
	{1.0f, 1.0f, 1.0f, 1.0f},//Ambient
	{1.0f, 1.0f, 1.0f, 1.0f},//Specular
	{0.0f, 0.0f, 0.0f, 0.0f},//Emissive
	0.0f}; //Power
	
	s_pDevice->SetMaterial(&Material);
	
	if(bTextured)
	{
		for(i=0; i<m_nNumMeshes; i++)
		{
			if(pSkin && LG_CheckFlag(pSkin->m_nFlags, CLSkin::LMS_LOADED))
			{
				lg_material mtr=pSkin->m_pCmpRefs[i];
				ID3DXEffect* pFx=LG_NULL;
				tm_tex txtr=0;
				//pFx=CLFxMgr::FXM_GetInterface(1);
				CLMtrMgr::MTR_GetInterfaces(mtr, &txtr, &pFx);
				
				//if(!pFx)
				//	pFx=CLFxMgr::FXM_GetInterface(1);
				
				UINT nPasses=0;
				
				D3DXMATRIX matWorld, matView, matProj;
				
				CLTMgr::TM_SetTexture(txtr, 0);
				
				
				s_pDevice->GetTransform(D3DTS_PROJECTION, &matProj);
				s_pDevice->GetTransform(D3DTS_VIEW, &matView);
				s_pDevice->GetTransform(D3DTS_WORLD, &matWorld);

				matWorld=matWorld*matView*matProj;
				
				pFx->SetMatrix("matWVP", &matWorld);
				//pFx->SetInt("nTime", timeGetTime());
			
				pFx->Begin(&nPasses, 0);
				for(lg_dword j=0; j<nPasses; j++)
				{
					pFx->BeginPass(j);
					
					s_pDevice->DrawIndexedPrimitive(
						D3DPT_TRIANGLELIST,
						0,
						0,
						m_nNumVertices,
						m_pMeshes[i].nFirstIndex,
						m_pMeshes[i].nNumTriangles);
						
					pFx->EndPass();
				}
					
				pFx->End();
			}
			else
			{
				#if 1
				if(m_pMeshes[i].nMaterialIndex!=-1)
				{
					CLTMgr::TM_SetTexture(m_lppTexs[m_pMeshes[i].nMaterialIndex], 0);//s_pDevice->SetTexture(0, m_lppTexs[m_pMeshes[i].nMaterialIndex]);
				}
				else
				{
					CLTMgr::TM_SetTexture(0, 0);//s_pDevice->SetTexture(0, LG_NULL);
				}

				s_pDevice->DrawIndexedPrimitive(
					D3DPT_TRIANGLELIST, 
					0, 
					0, 
					m_nNumVertices, 
					m_pMeshes[i].nFirstIndex, 
					m_pMeshes[i].nNumTriangles);	
				#endif
			}
		}
	}
	else
	{
		//we can render the whole model at once if there is no texture.
		s_pDevice->SetTexture(0, LG_NULL);
		s_pDevice->DrawIndexedPrimitive(
				D3DPT_TRIANGLELIST, 
				0, 
				0, 
				m_nNumVertices, 
				0, 
				m_nNumTriangles);
	}
	
	return LG_TRUE;
}

//Slow function for debugging only.
lg_bool CLMeshLG::RenderNormals()
{
	if(!m_bD3DValid)
		return LG_FALSE;
		
	s_pDevice->SetTexture(0, LG_NULL);
	s_pDevice->SetFVF(BONEVERTEX_FVF);
	BONEVERTEX Normal[2];
	
	
	lg_dword i=0;
	LMESH_VERTEX* pVertices;
	m_lpVB->Lock(0, m_nNumVertices*sizeof(LMESH_VERTEX), (void**)&pVertices, 0);
	for(i=0; i<this->m_nNumVertices; i++)
	{
		Normal[0].x=pVertices[i].x;
		Normal[0].y=pVertices[i].y;
		Normal[0].z=pVertices[i].z;
		Normal[0].color=0xFFFFFFFF;
		Normal[1].x=pVertices[i].nx;
		Normal[1].y=pVertices[i].ny;
		Normal[1].z=pVertices[i].nz;
		Normal[1].color=0xFFFFFFFF;
		s_pDevice->DrawPrimitiveUP(D3DPT_LINELIST, 1, &Normal, sizeof(BONEVERTEX));
	}
	m_lpVB->Unlock();
	return LG_TRUE;
}

#include <stdio.h>
#ifdef LM_USE_LF2
#include "lf_sys2.h"
#endif LM_USE_LF2
#include "lm_sys.h"
#include "ML_lib.h"


//Alright here is the skeleton class.
//Further this needs to support some kind of
//mechanism for pointing out different animations
//based on a range of animations.

//Also need to change the way the model gets rendered.
//Either CLegacyMesh needs to have a RenderWithSkel funciton.
//Or CLSkel needs to have a RenderModel functions.


CLSkel::CLSkel():
m_nID(0),
m_nVersion(0),
m_nNumJoints(0),
m_nNumKeyFrames(0),
m_pBaseSkeleton(LG_NULL),
m_bLoaded(LG_FALSE),
m_pFrames(LG_NULL),
m_nNumAnims(0),
m_pAnims(LG_NULL)
{
	m_szSkelName[0]=0;
}

CLSkel::~CLSkel()
{
	Unload();
}

lg_dword CLSkel::GetNumAnims()
{
	return m_nNumAnims;
}

lg_dword CLSkel::GetNumJoints(){return m_nNumJoints;}
lg_dword CLSkel::GetNumKeyFrames(){return m_nNumKeyFrames;}
lg_bool CLSkel::IsLoaded(){return m_bLoaded;}
lg_dword CLSkel::GetParentBoneRef(lg_dword nBone){return m_pBaseSkeleton[nBone].nParentBone;}

lg_bool CLSkel::Unload()
{
	this->DeallocateMemory();

	m_nID=0;
	m_nVersion=0;
	m_nNumJoints=0;
	m_nNumKeyFrames=0;
	//m_nTotalSize=0;
	m_bLoaded=LG_FALSE;
	m_nNumAnims=0;
	return LG_TRUE;
}
#ifdef LM_USE_LF2
//The read and write LF2 functions are used for the legacy mesh
//and legacy skel serialize methods when the LF2 file system is in
//use.
lg_uint CLSkel::ReadLF2(void* buffer, lg_uint size, lg_uint count, void* stream)
{
	return LF_Read((LF_FILE3)stream, buffer, size*count);
}

lg_bool CLSkel::Load(lf_path szFilename)
{
	Unload();
	
	LF_FILE3 fIn=LF_Open(szFilename, LF_ACCESS_READ, LF_OPEN_EXISTING);
	
	if(!fIn)
		return LG_FALSE;
	
	lg_bool bResult=Serialize(fIn, ReadLF2, LG_TRUE);
		
	if(!bResult)
		Unload();
	
	m_bLoaded=bResult;
	//We will now get all the calculations for the extra data (the local
	//and final matrices for the base skeleton and keyframes).
	//this->CalcExData();
	if(m_bLoaded)
	{
		CalcExData();
	}
	LF_Close(fIn);
	return m_bLoaded;
}
#endif
lg_bool CLSkel::Load(void* file, LM_RW_FN read)
{	
	//Unload in case a skeleton is already loaded.
	Unload();
	
	lg_bool bResult=Serialize(
		file,
		read,
		LG_TRUE);
		
	if(!bResult)
		Unload();
		
	//pcb->close(file);
	
	m_bLoaded=bResult;
	//We will now get all the calculations for the extra data (the local
	//and final matrices for the base skeleton and keyframes).
	//this->CalcExData();
	if(m_bLoaded)
	{
		CalcExData();
	}
	return m_bLoaded;
}

lg_bool CLSkel::Save(void* file, LM_RW_FN write)
{
	if(!m_bLoaded)
		return LG_FALSE;
		
	if(!m_pBaseSkeleton || !m_pFrames)
		return LG_FALSE;
	
	if(!file)
		return LG_FALSE;
		
	return Serialize(
		file,
		write,
		LG_FALSE);
}

lg_bool CLSkel::Serialize(
	void* file, 
	lg_uint (__cdecl * read_or_write)(void* buffer, lg_uint size, lg_uint count, void* stream), 
	lg_bool bLoading)
{
	lg_dword i=0, j=0;
	
	//Read or Write the file header.
	read_or_write(&m_nID, 4, 1, file);
	read_or_write(&m_nVersion, 4, 1, file);
	read_or_write(m_szSkelName, LMESH_MAX_NAME, 1, file);
	read_or_write(&m_nNumJoints, 4, 1, file);
	read_or_write(&m_nNumKeyFrames, 4, 1, file);
	read_or_write(&m_nNumAnims, 4, 1, file);
	
	if((m_nID != LSKEL_ID) || (m_nVersion != LSKEL_VERSION))
		return LG_FALSE;
		
	//Allocate memory if we are loading.
	if(bLoading)
		if(!AllocateMemory())return LG_FALSE;
		
	//Read or write the base skeleton.
	for(i=0; i<m_nNumJoints; i++)
	{
		read_or_write(&m_pBaseSkeleton[i], sizeof(LSKEL_JOINT), 1, file);
	}
	
	//Read or write the key frames.
	for(i=0; i<m_nNumKeyFrames; i++)
	{
		for(j=0; j<m_nNumJoints; j++)
		{
			read_or_write(&m_pFrames[i].LocalPos[j], sizeof(LSKEL_JOINTPOS), 1, file);
		}
		//Write the bounding box.
		read_or_write(&m_pFrames[i].aabbBox, sizeof(ML_AABB), 1, file);
	}
	//Read or write the animation information.
	for(i=0; i<m_nNumAnims; i++)
	{
		read_or_write(&m_pAnims[i], sizeof(LSKEL_ANIM), 1, file);
	}
	return LG_TRUE;
}

lg_bool CLSkel::GetMinsAndMaxes(
		float* fXMin,
		float* fXMax,
		float* fYMin,
		float* fYMax,
		float* fZMin,
		float* fZMax)
{
	if(!m_bLoaded)
		return LG_FALSE;
	//This only gets the mins and the maxes for the default frame, it
	//does not get mins and maxes for keyframes.
	lg_dword i=0;
	
	if(fXMin)*fXMin=m_pBaseSkeleton[0].Final._41;
	if(fXMax)*fXMax=m_pBaseSkeleton[0].Final._41;
	if(fYMin)*fYMin=m_pBaseSkeleton[0].Final._42;
	if(fYMax)*fYMax=m_pBaseSkeleton[0].Final._42;
	if(fZMin)*fZMin=m_pBaseSkeleton[0].Final._43;
	if(fZMax)*fZMax=m_pBaseSkeleton[0].Final._43;
	
	for(i=1; i<m_nNumJoints; i++)
	{
		if(fXMin)
			*fXMin=LG_Min(*fXMin, m_pBaseSkeleton[i].Final._41);
		if(fXMax)
			*fXMax=LG_Max(*fXMax, m_pBaseSkeleton[i].Final._41);
		if(fYMin)
			*fYMin=LG_Min(*fYMin, m_pBaseSkeleton[i].Final._42);
		if(fYMax)
			*fYMax=LG_Max(*fYMax, m_pBaseSkeleton[i].Final._42);
		if(fZMin)
			*fZMin=LG_Min(*fZMin, m_pBaseSkeleton[i].Final._43);
		if(fZMax)
			*fZMax=LG_Max(*fZMax, m_pBaseSkeleton[i].Final._43);
	}
	
	return LG_TRUE;
}

const LSKEL_ANIM* CLSkel::GetAnim(lg_dword n)
{
	if(n>=m_nNumAnims)
		return LG_NULL;
		
	return &m_pAnims[n];
}

lg_dword CLSkel::GetFrameFromTime(lg_dword nAnim, float fTime, float* pFrameTime, lg_dword* pFrame2)
{
	#define PREP_MAX_RANGE 100.0f
	if(!m_nNumAnims)
		return 0;
	
	nAnim=LG_Clamp(nAnim, 0, m_nNumAnims-1);
	fTime=LG_Clamp(fTime, 0.0f, PREP_MAX_RANGE);
	
	float fFrame;
	lg_dword nFrame1, nFrame2;
	
	if(L_CHECK_FLAG(m_pAnims[nAnim].m_nFlags, LSKEL_ANIM_LOOPBACK))
	{
		if(fTime>=50.0f)
			fTime=100.0f-fTime;
		
		fFrame=m_pAnims[nAnim].m_nFirstFrame + 
			((float)m_pAnims[nAnim].m_nNumFrames-1-0.000001f) * 
			fTime/(PREP_MAX_RANGE*0.5f);
	}
	else
	{
		fFrame=m_pAnims[nAnim].m_nFirstFrame + 
			((float)m_pAnims[nAnim].m_nNumFrames-0.000001f) * 
			fTime/(PREP_MAX_RANGE);
	}
	
	nFrame1=(lg_dword)fFrame;
	nFrame2=nFrame1>=(m_pAnims[nAnim].m_nFirstFrame+m_pAnims[nAnim].m_nNumFrames-1)?m_pAnims[nAnim].m_nFirstFrame:nFrame1+1;
	
	if(pFrameTime)
		*pFrameTime=fFrame-nFrame1;
	if(pFrame2)
		*pFrame2=nFrame2;
		
	return nFrame1;
}

ML_AABB* CLSkel::GenerateAABB(ML_AABB* pOut, lg_dword nFrame1, lg_dword nFrame2, float t)
{
	if(!pOut)
		return pOut;
		
	nFrame1=LG_Clamp(nFrame1, 1, m_nNumKeyFrames);
	nFrame2=LG_Clamp(nFrame2, 1, m_nNumKeyFrames);
	t=LG_Clamp(t, 0.0f, 1.0f);
	//We should probably have an AABB for frame 0 (the default frame).
	ML_AABB *a, *b;
	a=&m_pFrames[nFrame1-1].aabbBox;
	b=&m_pFrames[nFrame2-1].aabbBox;
	pOut->v3Min.x=a->v3Min.x+(b->v3Min.x-a->v3Min.x)*t;
	pOut->v3Min.y=a->v3Min.y+(b->v3Min.y-a->v3Min.y)*t;
	pOut->v3Min.z=a->v3Min.z+(b->v3Min.z-a->v3Min.z)*t;
	
	pOut->v3Max.x=a->v3Max.x+(b->v3Max.x-a->v3Max.x)*t;
	pOut->v3Max.y=a->v3Max.y+(b->v3Max.y-a->v3Max.y)*t;
	pOut->v3Max.z=a->v3Max.z+(b->v3Max.z-a->v3Max.z)*t;
	return pOut;
}
ML_MAT* CLSkel::GenerateJointTransform(ML_MAT* pOut, lg_dword nJoint, lg_dword nFrame1, lg_dword nFrame2, float t)
{
	if(!pOut)
		return LG_NULL;
	nJoint=LG_Clamp(nJoint, 0, m_nNumJoints-1);
	nFrame1=LG_Clamp(nFrame1, 0, m_nNumKeyFrames);
	nFrame2=LG_Clamp(nFrame2, 0, m_nNumKeyFrames);
	t=LG_Clamp(t, 0.0f, 1.0f);
	
	
	const ML_MAT *pM1, *pM2;
	ML_MAT MI;
	ML_MatIdentity(&MI);
	
	pM1=nFrame1>0?m_pFrames[nFrame1-1].GetFinalMat(nJoint):&MI;
	pM2=nFrame2>0?m_pFrames[nFrame2-1].GetFinalMat(nJoint):&MI;
	
	return ML_MatSlerp(
		pOut,
		(ML_MAT*)pM1,
		(ML_MAT*)pM2,
		t);
}

//This second version of GenerateJointTransform uses a second skeleton to generate
//the transform.  Note that both skeletons should be compatible with each other and
//have the same base skeleton, or the transformation may not be pretty.
ML_MAT* CLSkel::GenerateJointTransform(ML_MAT* pOut, lg_dword nJoint, lg_dword nFrame1, lg_dword nFrame2, float t, CLSkel* pSkel2)
{
	if(!pOut)
		return LG_NULL;
	nJoint=LG_Clamp(nJoint, 0, m_nNumJoints-1);
	nFrame1=LG_Clamp(nFrame1, 0, m_nNumKeyFrames);
	nFrame2=LG_Clamp(nFrame2, 0, pSkel2->m_nNumKeyFrames);
	t=LG_Clamp(t, 0.0f, 1.0f);
	
	
	const ML_MAT *pM1, *pM2;
	ML_MAT MI;
	ML_MatIdentity(&MI);
	
	pM1=nFrame1>0?m_pFrames[nFrame1-1].GetFinalMat(nJoint):&MI;
	pM2=nFrame2>0?pSkel2->m_pFrames[nFrame2-1].GetFinalMat(nJoint):&MI;
	
	return ML_MatSlerp(
		pOut,
		(ML_MAT*)pM1,
		(ML_MAT*)pM2,
		t);
}

const ML_MAT* CLSkel::GetBaseTransform(lg_dword nJoint)
{
	nJoint=LG_Clamp(nJoint, 0, m_nNumJoints-1);
	
	return &m_pBaseSkeleton[nJoint].Final;
}

lg_bool CLSkel::AllocateMemory()
{
	lg_dword i=0, j=0;
	//Deallocate memory first just in case,
	//note that because the poitners are set to null in
	//the constructor it is okay to call this before they
	//are initialized.
	DeallocateMemory();
	//Allocate memory for the frames and base skeleton.
	//m_pKeyFrames=new LMESH_KEYFRAME[m_nNumKeyFrames];
	
	m_pFrames=new CLFrame[m_nNumKeyFrames];
	if(!m_pFrames)
		return LG_FALSE;
		
	for(i=0; i<m_nNumKeyFrames;i++)
	{
		if(!m_pFrames[i].Initialize(m_nNumJoints))
		{
			L_safe_delete_array(m_pFrames);
			return LG_FALSE;
		}
	}

	m_pBaseSkeleton=new LSKEL_JOINTEX[m_nNumJoints];
	if(!m_pBaseSkeleton)
	{
		L_safe_delete_array(m_pFrames);
		return LG_FALSE;
	}
	
	m_pAnims=new LSKEL_ANIM[m_nNumAnims];
	if(!m_pAnims)
	{
		L_safe_delete_array(m_pFrames);
		L_safe_delete_array(m_pBaseSkeleton);
		return LG_FALSE;
	}
	return LG_TRUE;
}

void CLSkel::DeallocateMemory()
{
	L_safe_delete_array(this->m_pBaseSkeleton);
	L_safe_delete_array(m_pFrames);
	L_safe_delete_array(m_pAnims);
}


lg_dword CLSkel::GetNumFrames()
{
	return m_nNumKeyFrames;
}


lg_bool CLSkel::CalcExData()
{
	lg_dword i=0, j=0;

	if(!m_bLoaded)
		return LG_FALSE;

	//Firstly we must convert Euler angles for the base skeleton and
	//keyframes to matrices.
	//Read the base skeleton.
	for(i=0; i<m_nNumJoints; i++)
	{

		//Now create the rotation matrixes (in the final format the rotation
		//matrices will probably be stored instead of the euler angles.
		EulerToMatrix((ML_MAT*)&m_pBaseSkeleton[i].Local, (float*)&m_pBaseSkeleton[i].rotation);
		//Now stick the translation into the matrix.
		m_pBaseSkeleton[i].Local._41=m_pBaseSkeleton[i].position[0];
		m_pBaseSkeleton[i].Local._42=m_pBaseSkeleton[i].position[1];
		m_pBaseSkeleton[i].Local._43=m_pBaseSkeleton[i].position[2];

		m_pBaseSkeleton[i].nJointRef=i;
	}

	//Read the key frames.
	
	for(i=0; i<m_nNumKeyFrames; i++)
	{
		for(j=0; j<m_nNumJoints; j++)
		{
			EulerToMatrix(&m_pFrames[i].Local[j], (float*)&m_pFrames[i].LocalPos[j].rotation);
			m_pFrames[i].Local[j]._41=m_pFrames[i].LocalPos[j].position[0];
			m_pFrames[i].Local[j]._42=m_pFrames[i].LocalPos[j].position[1];
			m_pFrames[i].Local[j]._43=m_pFrames[i].LocalPos[j].position[2];
		}
	}

	//Calculate the final matrices for the base skeleton.
	//This is simply a matter of multiplying each joint by
	//all of it's parent's matrices.
	for(i=0; i<m_nNumJoints; i++)
	{
		LSKEL_JOINTEX* pTemp=&m_pBaseSkeleton[i];
		m_pBaseSkeleton[i].Final=pTemp->Local;
		while(pTemp->nParentBone)
		{
			pTemp=&m_pBaseSkeleton[pTemp->nParentBone-1];
			ML_MatMultiply((ML_MAT*)&m_pBaseSkeleton[i].Final, (ML_MAT*)&m_pBaseSkeleton[i].Final, (ML_MAT*)&m_pBaseSkeleton[pTemp->nJointRef].Local);
		}
	}

	//We calculate the final transformation matrix for each joint for each
	//frame now, that way we don't have to calculate them on the fly.  It
	//takes more memory this way, but the real time rendering is faster because
	//it isn't necessary to calculate each frames matrix every frame, it is only
	//necessary to interploate the joint matrixes for the given frame.
	
	//For each joint...
	for(i=0; i<m_nNumJoints; i++)
	{
		//For each frame for each joint...
		for(j=0; j<m_nNumKeyFrames; j++)
		{
			//1. Obtain the base joint for that joint.
			LSKEL_JOINTEX* pTemp=&m_pBaseSkeleton[i];
			//2. Start out by making the final translation for the frame the local frame joint
			//location multiplyed the by the local base joint.
			
			//ML_MatMultiply((ML_MAT*)&m_pKeyFrames[j].pJointPos[i].Final, (ML_MAT*)&m_pKeyFrames[j].pJointPos[i].Local, (ML_MAT*)&pTemp->Local);
			ML_MatMultiply((ML_MAT*)&m_pFrames[j].Final[i], &m_pFrames[j].Local[i], &pTemp->Local);
			//3. Then if the joint has a parent...
			while(pTemp->nParentBone)
			{
				//3 (cont'd). It is necessary to multiply the final frame matrix
				//by the same calculation in step 2 (frame pos for the frame * local pos for the frame).
				pTemp=&m_pBaseSkeleton[pTemp->nParentBone-1];
				ML_MAT MT;
				ML_MatMultiply(
					(ML_MAT*)&m_pFrames[j].Final[i],
					(ML_MAT*)&m_pFrames[j].Final[i],
					ML_MatMultiply(&MT, (ML_MAT*)&m_pFrames[j].Local[pTemp->nJointRef], (ML_MAT*)&pTemp->Local));
				
			}

			//4. Fianlly the final position needs to be multiplied by the
			//final base position's inverse so that the transformation is
			//relative to the joint's location and not to 0,0,0.
			ML_MAT MI;
			ML_MatInverse(&MI, LG_NULL, &m_pBaseSkeleton[i].Final);
			ML_MatMultiply((ML_MAT*)&m_pFrames[j].Final[i], &MI, (ML_MAT*)&m_pFrames[j].Final[i]);
		
		}
		
	}
	return LG_TRUE;
}


///////////////////////////////
/// CLFrame Member Methods  ///
///////////////////////////////

CLFrame::CLFrame():
	Local(LG_NULL),
	Final(LG_NULL),
	m_nNumJoints(0),
	LocalPos(LG_NULL)
{
	
}

CLFrame::CLFrame(lg_dword nNumJoints):
	Local(LG_NULL),
	Final(LG_NULL),
	m_nNumJoints(0),
	LocalPos(LG_NULL)
{
	Initialize(nNumJoints);
}

CLFrame::~CLFrame()
{
	L_safe_delete_array(Local);
	L_safe_delete_array(Final);
	L_safe_delete_array(LocalPos);
}

lg_bool CLFrame::Initialize(lg_dword nNumJoints)
{
	L_safe_delete_array(Local);
	L_safe_delete_array(Final);
	L_safe_delete_array(LocalPos);
	
	m_nNumJoints=nNumJoints;
	Local=new ML_MAT[nNumJoints];
	Final=new ML_MAT[nNumJoints];
	LocalPos=new LSKEL_JOINTPOS[nNumJoints];
	
	if(!Local || !Final || !LocalPos)
		return LG_FALSE;
		
	float f[3]={0.0f, 0.0f, 0.0f};
	for(lg_dword i=0; i<nNumJoints; i++)
		SetLocalMat(i, f, f);
	return LG_TRUE;
}

lg_dword CLFrame::GetNumJoints()
{
	return m_nNumJoints;
}

const ML_MAT* CLFrame::GetFinalMat(lg_dword nJoint)
{
	nJoint=LG_Clamp(nJoint, 0, m_nNumJoints-1);
	return &Final[nJoint];
}

const ML_MAT* CLFrame::GetLocalMat(lg_dword nJoint)
{
	nJoint=LG_Clamp(nJoint, 0, m_nNumJoints-1);
	return &Local[nJoint];
}

lg_bool CLFrame::SetFinalMat(lg_dword nJoint, ML_MAT* pM)
{
	if(nJoint>=m_nNumJoints)
		return LG_FALSE;
		
	Final[nJoint]=*pM;
	return LG_TRUE;
}

lg_bool CLFrame::SetLocalMat(lg_dword nJoint, float* position, float* rotation)
{
	if(nJoint>=m_nNumJoints)
		return LG_FALSE;
		
	LocalPos[nJoint].position[0]=position[0];
	LocalPos[nJoint].position[1]=position[1];
	LocalPos[nJoint].position[2]=position[2];
	
	LocalPos[nJoint].rotation[0]=rotation[0];
	LocalPos[nJoint].rotation[1]=rotation[1];
	LocalPos[nJoint].rotation[2]=rotation[2];
	
	CLSkel::EulerToMatrix(&Local[nJoint], rotation);
	
	Local[nJoint]._41=position[0];
	Local[nJoint]._42=position[1];
	Local[nJoint]._43=position[2];
	return LG_TRUE;
}

lg_bool CLFrame::SetLocalMat(lg_dword nJoint, ML_MAT* pM)
{
	if(nJoint>=m_nNumJoints)
		return LG_FALSE;
	
	Local[nJoint]=*pM;
	return LG_TRUE;
}

ML_MAT* CLSkel::EulerToMatrix(ML_MAT* pOut, float* pEuler)
{
	ML_MAT MX, MY, MZ;
	ML_MatRotationX(&MX, -pEuler[0]);
	ML_MatRotationY(&MY, -pEuler[1]);
	ML_MatRotationZ(&MZ, -pEuler[2]);
		
	return ML_MatMultiply(
		pOut,
		&MX,
		ML_MatMultiply(&MY, &MY, &MZ));
}
