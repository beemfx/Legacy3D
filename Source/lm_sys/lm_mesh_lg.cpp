#include "lm_mesh_lg.h"

#include <stdio.h>
#include "lg_func.h"
#include "lg_err.h"
#include "lf_sys2.h"

IDirect3DDevice9* CLMeshLG::s_pDevice=LG_NULL;

CLMeshLG::CLMeshLG()
	: CLMeshAnim()
	, m_pVB(LG_NULL)
	, m_pIB(LG_NULL)
	, m_pTex(LG_NULL)
{
	m_szMeshFilePath[0]=0;
}

CLMeshLG::~CLMeshLG()
{
	Unload();
}

lg_bool CLMeshLG::Load(LMPath szFile)
{
	//Unload in case a mesh was already loaded.
	Unload();
	
	//Open the file:
	LF_FILE3 fin=LF_Open(szFile, LF_ACCESS_READ, LF_OPEN_EXISTING);
	if(!fin)
		return LG_FALSE;
	
	//Serialize will take care of loading the model data.
	lg_bool bResult=Serialize(fin, ReadFn, RW_READ);
	
	//We're now done with the file.
	LF_Close(fin);
	
	//If we succeeded in loading the mesh we'll take aditional steps
	if(bResult)
	{
		//We'll set the flag to loaded (this will also clear any
		//other garbage that might have been left in the loaded flag.
		m_nFlags=LM_FLAG_LOADED;
		
		//We'll save the directory in which the mesh was located
		//so that we can get the default textures if we want.
		LF_GetDirFromPath(m_szMeshFilePath, szFile);
		
		m_pTex=new tm_tex[m_nNumMtrs];
		for(lg_dword i=0; i<m_nNumMtrs; i++)
		{
			m_pTex[i]=0;
		}
		
		/*
		//We'll make our mesh d3d ready
		MakeD3DReady();
		*/
		
		//We also want the animation data to be setup.
		InitAnimData();
	}
	else
	{
		//If we couldn't load, we'll set m_nFlags to zero so that
		//if the LM_FLAG_LOADED flag was set it will be clear.
		m_nFlags=0;
	}
	
	return bResult;
}

lg_bool CLMeshLG::Save(CLMBase::LMPath szFile)
{
	//No reason to be able to save meshes in the game.
	return LG_FALSE;
}

lg_void CLMeshLG::Unload()
{
	//We won't bother to make sure a mesh is loaded,
	//we'll just delete everything anyway, and everything
	//that is empty should be set to null anyway.
	
	//We need to get rid of the anim data:
	DeleteAnimData();
	
	//Get rid of all rasterization information:
	//We can just delete the texture array,
	//the texture manager will worry about getting
	//rid of the textures:
	LG_SafeDeleteArray(m_pTex);
	//Release Direct3D interfaces:
	LG_SafeRelease(m_pVB);
	LG_SafeRelease(m_pIB);
	
	//Call the base method unload:
	CLMeshAnim::Unload();
	
	//Clear the flags (the base method should have set it, but just in case:
	m_nFlags=0;
}

lg_void CLMeshLG::MakeD3DReady(lg_dword flags)
{
	//Check some flags to make sure we actually can or need to
	//creat the direct3d interfaces, if we don't have a mesh loaded
	//or the mesh is already D3D ready we're good, if there is no
	//device there is nothing we can do.
	if(!LG_CheckFlag(m_nFlags, LM_FLAG_LOADED) 
		|| LG_CheckFlag(m_nFlags, LM_FLAG_D3D_READY)
		|| !s_pDevice)
	{
		Err_Printf("CLMeshLG ERROR: Could not make mesh D3D Ready.");
		return;
	}
	
	//If we want to load the default textures we'll load them
	//from the file, 
	if(LG_CheckFlag(flags, D3DR_FLAG_LOAD_DEF_TEX))
	{
		
		lf_path szTexPath;
		for(lg_dword i=0; i<m_nNumMtrs; i++)
		{
			_snprintf(szTexPath, LF_MAX_PATH, "%s%s", m_szMeshFilePath, m_pMtrs[i].szFile);
			szTexPath[LF_MAX_PATH]=0;
			m_pTex[i]=CLTMgr::TM_LoadTex(szTexPath, 0);
		}
	}
	
	//We can now call Validate to get everything D3D ready,
	//we'll know if the LM_FLAG_D3D_VALID flag is set afterwards
	//that we suceeded, we have to set the LM_FLAG_D3D_READY before
	//hand or validate will fail.
	
	LG_SetFlag(m_nFlags, LM_FLAG_D3D_READY);
	Validate();
	
	//If we failed...
	if(!LG_CheckFlag(m_nFlags, LM_FLAG_D3D_VALID))
	{
		//Unset the D3D_READY flag, and print an error message:
		LG_UnsetFlag(m_nFlags, LM_FLAG_D3D_READY);
		Err_Printf("CLMeshLG ERROR: Could not validate mesh!");
	}
	
	//If we succeed we're good.
}

lg_void CLMeshLG::Validate()
{
	const D3DPOOL nPool=D3DPOOL_MANAGED;
	
	//If the D3D_READY flag is set we know we are
	//good to validate, if the D3D_VALID flag is set
	//then we are already valid:
	if(LG_CheckFlag(m_nFlags, LM_FLAG_D3D_VALID) || !LG_CheckFlag(m_nFlags, LM_FLAG_D3D_READY))
		return;
		
	//We don't need to worry about validating textures because
	//the texture manager does that.
	
	//We do need a vertex buffer, though:
	lg_result nResult;
	
	nResult=s_pDevice->CreateVertexBuffer(
		m_nNumVerts*sizeof(MeshVertex),
		D3DUSAGE_WRITEONLY,
		LM_VERTEX_FORMAT,
		nPool,
		&m_pVB,
		LG_NULL);
		
	if(LG_FAILED(nResult))
		return;
		
	//We also need an index buffer:
	nResult=s_pDevice->CreateIndexBuffer(
		m_nNumTris*3*sizeof(*m_pIndexes),
		D3DUSAGE_WRITEONLY,
		D3DFMT_INDEX16,
		nPool,
		&m_pIB,
		LG_NULL);
		
	if(LG_FAILED(nResult))
	{
		LG_SafeRelease(m_pVB);
		m_pIB=LG_NULL;
		return;
	}
	
	//Now we need to fill in the buffer with default data:
	lg_void* pBuffer=LG_NULL;
	nResult=m_pVB->Lock(0, m_nNumVerts*sizeof(MeshVertex), (void**)&pBuffer, 0);
	//If we couldn't lock we got serious issues:
	if(LG_FAILED(nResult))
	{
		LG_SafeRelease(m_pVB);
		LG_SafeRelease(m_pIB);
		return;
	}
	
	//Just copy over the buffer:
	memcpy(pBuffer, m_pVerts, m_nNumVerts*sizeof(MeshVertex));
	
	//And unlock the buffer:
	m_pVB->Unlock();
	
	//Now fill the index buffer:
	nResult=m_pIB->Lock(0, m_nNumTris*3*sizeof(*m_pIndexes), &pBuffer, 0);
	//If we couldn't lock we got serious issues:
	if(LG_FAILED(nResult))
	{
		LG_SafeRelease(m_pVB);
		LG_SafeRelease(m_pIB);
		return;
	}
	
	memcpy(pBuffer, m_pIndexes, m_nNumTris*3*sizeof(*m_pIndexes));
	m_pIB->Unlock();
	
	LG_SetFlag(m_nFlags, LM_FLAG_D3D_VALID);
}

lg_void CLMeshLG::Invalidate()
{
	//Because the index and vertex buffers are managed
	//we don't need to do anything here.  If they
	//weren't managed the code would look as follows.
	/*
	LG_SafeRelease(m_pVB);
	LG_SafeRelease(m_pIB)
	LG_UnsetFlag(m_nFlags, LM_FLAG_D3D_VALID);
	*/
}


lg_bool CLMeshLG::IsD3DReady()
{
	return LG_CheckFlag(m_nFlags, LM_FLAG_D3D_READY);
}

CLMesh2::MeshVertex* CLMeshLG::LockTransfVB()
{
	if(!LG_CheckFlag(m_nFlags, LM_FLAG_D3D_VALID))
		return LG_NULL;
		
	lg_result nResult;
	MeshVertex* pBuffer;
	nResult=m_pVB->Lock(0, m_nNumVerts*sizeof(MeshVertex), (lg_void**)&pBuffer, 0);
	if(LG_FAILED(nResult))
		return LG_NULL;
		
	return pBuffer;
}
void CLMeshLG::UnlockTransfVB()
{
	m_pVB->Unlock();
}

lg_void CLMeshLG::Render()
{
	//This is the basic render method:
	
	//Can't render if we're not valid:
	if(!LG_CheckFlag(m_nFlags, LM_FLAG_D3D_VALID))
		return;
		
	s_pDevice->SetFVF(LM_VERTEX_FORMAT);
	s_pDevice->SetStreamSource(0, m_pVB, 0, sizeof(MeshVertex));
	s_pDevice->SetIndices(m_pIB);
	
	//Set the material for the mesh, in the future this might
	//be controlled by model properties, for now it is 
	//just a default material.
	static D3DMATERIAL9 d3d9mtr={
		{1.0f, 1.0f, 1.0f, 1.0f},//Diffuse
		{1.0f, 1.0f, 1.0f, 1.0f},//Ambient
		{1.0f, 1.0f, 1.0f, 1.0f},//Specular
		{0.0f, 0.0f, 0.0f, 0.0f},//Emissive
		0.0f}; //Power
		
	s_pDevice->SetMaterial(&d3d9mtr);
	
	//The basic render method simply renders all the SubMeshes
	//with the default textures
	for(lg_dword i=0; i<m_nNumSubMesh; i++)
	{
		CLTMgr::TM_SetTexture(m_pSubMesh[i].nMtrIndex!=-1?m_pTex[m_pSubMesh[i].nMtrIndex]:0, 0);
		
		s_pDevice->DrawIndexedPrimitive(
			D3DPT_TRIANGLELIST, 
			0, 
			0, 
			m_nNumVerts, 
			m_pSubMesh[i].nFirstIndex, 
			m_pSubMesh[i].nNumTri);	
	}
}

lg_void CLMeshLG::Render(const CLSkin *pSkin)
{
	//The skinned render method:
	//This is the basic render method:
	if(!pSkin || !LG_CheckFlag(pSkin->m_nFlags, pSkin->LM_FLAG_LOADED))
	{
		Render();
		return;
	}
	
	//Can't render if we're not valid:
	if(!LG_CheckFlag(m_nFlags, LM_FLAG_D3D_VALID))
		return;
		
	s_pDevice->SetFVF(LM_VERTEX_FORMAT);
	s_pDevice->SetStreamSource(0, m_pVB, 0, sizeof(MeshVertex));
	s_pDevice->SetIndices(m_pIB);
	
	//Set the material for the mesh, in the future this might
	//be controlled by model properties, for now it is 
	//just a default material.
	static D3DMATERIAL9 d3d9mtr={
		{1.0f, 1.0f, 1.0f, 1.0f},//Diffuse
		{1.0f, 1.0f, 1.0f, 1.0f},//Ambient
		{1.0f, 1.0f, 1.0f, 1.0f},//Specular
		{0.0f, 0.0f, 0.0f, 0.0f},//Emissive
		0.0f}; //Power
		
	s_pDevice->SetMaterial(&d3d9mtr);
	
	static D3DXMATRIX matTrans, matView, matProj;
	s_pDevice->GetTransform(D3DTS_PROJECTION, &matProj);
	s_pDevice->GetTransform(D3DTS_VIEW, &matView);
	s_pDevice->GetTransform(D3DTS_WORLD, &matTrans);
	matTrans=matTrans*matView*matProj;
	
	//The basic render method simply renders all the SubMeshes
	//with the default textures
	for(lg_dword i=0; i<m_nNumSubMesh; i++)
	{
		CLTMgr::TM_SetTexture(m_pSubMesh[i].nMtrIndex!=-1?m_pTex[m_pSubMesh[i].nMtrIndex]:0, 0);
		ID3DXEffect* pFx=LG_NULL;
		tm_tex txtr=0;
		
		CLMtrMgr::MTR_GetInterfaces(pSkin->m_pCmpRefs[i], &txtr, &pFx);
		
		UINT nPasses=0;
		
		
		CLTMgr::TM_SetTexture(txtr, 0);
		pFx->SetMatrix("matWVP", &matTrans);
		//pFx->SetInt("nTime", timeGetTime());
		
		pFx->Begin(&nPasses, 0);
		for(lg_dword j=0; j<nPasses; j++)
		{
			pFx->BeginPass(j);
			
			s_pDevice->DrawIndexedPrimitive(
				D3DPT_TRIANGLELIST, 
				0, 
				0, 
				m_nNumVerts, 
				m_pSubMesh[i].nFirstIndex, 
				m_pSubMesh[i].nNumTri);	
				
			pFx->EndPass();
		}
		
		pFx->End();
	}
}

lg_void CLMeshLG::LM_Init(IDirect3DDevice9* pDevice)
{
	LM_Shutdown();
	
	s_pDevice=pDevice;
	if(s_pDevice)s_pDevice->AddRef();
}
lg_void CLMeshLG::LM_Shutdown()
{
	LG_SafeRelease(s_pDevice);
}

lg_void CLMeshLG::LM_SetRenderStates()
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
lg_void CLMeshLG::LM_SetSkyboxRenderStates()
{
	if(!s_pDevice)
		return;
		
	D3DMATRIX m;
	
	s_pDevice->SetTransform(D3DTS_WORLD, D3DXMatrixIdentity((D3DXMATRIX*)&m));
	LM_SetRenderStates();
	s_pDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
	s_pDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);
	s_pDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
	
	s_pDevice->SetSamplerState( 0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	s_pDevice->SetSamplerState( 0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
}

lg_uint __cdecl CLMeshLG::ReadFn(lg_void* file, lg_void* buffer, lg_uint size)
{
	return LF_Read(file, buffer, size);
}