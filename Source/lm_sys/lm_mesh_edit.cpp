#include "stdafx.h"
#include "lm_mesh_edit.h"
#include "lg_func.h"

IDirect3DDevice9* CLMeshEdit::s_pDevice=LG_NULL;

CLMesh2::MeshVertex* CLMeshEdit::LockTransfVB()
{
	return m_pTransfVB;
}

lg_void CLMeshEdit::UnlockTransfVB()
{

}

lg_void CLMeshEdit::SetRenderTexture(lg_bool bRenderTexture)
{
	if(bRenderTexture)
		LG_SetFlag(m_nFlags, LM_FLAG_RENDER_TEX);
	else
		LG_UnsetFlag(m_nFlags, LM_FLAG_RENDER_TEX);
}

const ml_aabb* CLMeshEdit::GetBoundingBox()
{
	return &m_AABB;
}

lg_bool CLMeshEdit::Load(LMPath szFile)
{
	Unload();
	CFile fin;
	
	if(!fin.Open(szFile, fin.modeRead))
		return LG_FALSE;
	
	lg_bool bResult=Serialize((lg_void*)&fin, ReadFn, RW_READ);
	
	fin.Close();
	
	m_nFlags=bResult?LM_FLAG_LOADED:0;
	
	if(bResult)
	{
		lg_path szTexDir;
		lg_path szTexPath;
		
		GetDirFromPath(szTexDir, szFile);
		
		m_ppTex=new IDirect3DTexture9*[m_nNumMtrs];
		for(lg_dword i=0; i<m_nNumMtrs; i++)
		{
			_snprintf(szTexPath, LG_MAX_PATH, "%s%s", szTexDir, m_pMtrs[i].szFile);
			szTexPath[LG_MAX_PATH]=0;
			
			HRESULT nRes=D3DXCreateTextureFromFileEx(
				s_pDevice,
				szTexPath,
				D3DX_DEFAULT,
				D3DX_DEFAULT,
				D3DX_DEFAULT,
				0,
				D3DFMT_UNKNOWN,
				D3DPOOL_MANAGED,
				D3DX_DEFAULT,
				D3DX_DEFAULT,
				0,
				LG_NULL,
				LG_NULL,
				&m_ppTex[i]);
				
			if(FAILED(nRes))
			{
				m_ppTex[i]=LG_NULL;
			}
		}
		
		m_pTransfVB=new MeshVertex[m_nNumVerts];
		for(lg_dword i=0; i<m_nNumVerts; i++)
		{
			m_pTransfVB[i]=m_pVerts[i];
		}
		
		InitAnimData();
	}
	
	return bResult;
}
lg_bool CLMeshEdit::Save(LMPath szFile)
{
	if(!IsLoaded())
		return LG_FALSE;
	
	CFile fin;
	
	if(!fin.Open(szFile, fin.modeWrite|fin.modeCreate))
		return LG_FALSE;
	
	lg_bool bResult=Serialize((lg_void*)&fin, WriteFn, RW_WRITE);
	
	fin.Close();
	
	return bResult;
}

lg_void CLMeshEdit::Unload()
{
	CLMesh2::Unload();
	if(m_ppTex)
	{
		for(lg_dword i=0; i<this->m_nNumMtrs; i++)
		{
			LG_SafeRelease(m_ppTex[i]);
		}
		LG_SafeDeleteArray(m_ppTex);
	}
	LG_SafeDeleteArray(m_pTransfVB);
	DeleteAnimData();
}

lg_str CLMeshEdit::GetDirFromPath(lg_str szDir, lg_cstr szFullPath)
{
	lg_dword dwLen=strlen(szFullPath);
	lg_long i=0;
	for(i=(lg_long)dwLen-1; i>=0; i--)
	{
		if(szFullPath[i]=='\\' || szFullPath[i]=='/')
			break;
	}
	strncpy(szDir, szFullPath, i+1);
	szDir[i+1]=0;
	return szDir;
}

lg_void CLMeshEdit::Render()
{
	if(!LG_CheckFlag(m_nFlags, LM_FLAG_LOADED))
		return;

	/*	
	s_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);	
	s_pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	s_pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	s_pDevice->SetRenderState(D3DRS_NORMALIZENORMALS, TRUE);
	s_pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	
	s_pDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
	s_pDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
	s_pDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
	s_pDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);
	*/
	s_pDevice->SetFVF(D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX1);
	
	//Set the material for the mesh, in the future this might
	//be controlled by model properties, for now it is 
	//just a default material.
	static const D3DMATERIAL9 d3d9mtr={
		{1.0f, 1.0f, 1.0f, 1.0f},//Diffuse
		{1.0f, 1.0f, 1.0f, 1.0f},//Ambient
		{1.0f, 1.0f, 1.0f, 1.0f},//Specular
		{0.0f, 0.0f, 0.0f, 1.0f},//Emissive
		0.0f}; //Power
		
	s_pDevice->SetMaterial(&d3d9mtr);
	
	for(lg_dword i=0; i<m_nNumSubMesh; i++)
	{
		s_pDevice->SetTexture(0, LG_CheckFlag(m_nFlags, LM_FLAG_RENDER_TEX)?m_ppTex[m_pSubMesh[i].nMtrIndex]:LG_NULL);
			
		HRESULT Res = s_pDevice->DrawIndexedPrimitiveUP(
			D3DPT_TRIANGLELIST,
			0,
			m_nNumVerts,
			m_pSubMesh[i].nNumTri,
			&m_pIndexes[m_pSubMesh[i].nFirstIndex],
			D3DFMT_INDEX16,
			m_pTransfVB,
			sizeof(MeshVertex));

		if( FAILED( Res ) )
		{
			__debugbreak();
		}
	}
	
}
	

lg_uint __cdecl CLMeshEdit::ReadFn(lg_void* file, lg_void* buffer, lg_uint size)
{
	CFile* pFile=(CFile*)file;
	
	return pFile->Read(buffer, size);//LF_Read(file, buffer, size);
}

lg_uint __cdecl CLMeshEdit::WriteFn(lg_void* file, lg_void* buffer, lg_uint size)
{
	CFile* pFile=(CFile*)file;
	
	pFile->Write(buffer, size);//LF_Read(file, buffer, size);
	return size;
}