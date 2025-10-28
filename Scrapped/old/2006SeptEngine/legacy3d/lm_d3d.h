#include <d3dx9.h>
#include <d3d9.h>
#include "lm_sys.h"

//Some temporary bone rendering structures
//and definitions.
typedef struct _JOINTVERTEX{
	float x, y, z;
	float psize;
	L_dword color;
}JOINTVERTEX;
#define JOINTVERTEX_FVF (D3DFVF_XYZ|D3DFVF_PSIZE|D3DFVF_DIFFUSE)

typedef struct _BONEVERTEX{
	float x, y, z;
	L_dword color;
}BONEVERTEX;
#define BONEVERTEX_FVF (D3DFVF_XYZ|D3DFVF_DIFFUSE)


//void CreateBoneMatrices2(D3DXMATRIX* pMList, LMESH_BONE* pBoneList, L_dword nNumBones);

class CLegacyMeshD3D: public CLegacyMesh
{
public:
	CLegacyMeshD3D();
	L_bool Create(char* szFilename, IDirect3DDevice9* lpDevice);
	L_bool CreateD3DComponents(IDirect3DDevice9* lpDevice);
	L_bool Validate();
	L_bool Invalidate();
	L_bool Render();
	L_bool RenderSkeleton();
	L_bool SetupSkeleton(L_dword nFrame1, L_dword nFrame2, float fTime);
	L_bool PrepareFrame(L_dword dwFist, L_dword dwSecond, float fTime);
	virtual L_bool Unload();
private:
	IDirect3DDevice9*       m_lpDevice;
	IDirect3DVertexBuffer9* m_lpVB;
	IDirect3DTexture9**     m_lppTexs;
	L_bool                  m_bD3DValid;

	//Some structures to hold the stuff that is used every
	//frame but is only temporary.
	D3DXMATRIX*   m_pAnimMats;

	//Some temp skeleton rendering stuff.
	JOINTVERTEX*            m_pSkel;
};