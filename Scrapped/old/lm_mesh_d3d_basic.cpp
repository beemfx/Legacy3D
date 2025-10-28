#include <stdio.h>
#include "lf_sys2.h"
#include "lm_mesh_d3d_basic.h"

IDirect3DDevice9* CLMeshD3DBasic::s_pDevice=LG_NULL;

CLMesh2::MeshVertex* CLMeshD3DBasic::LockTransfVB()
{
	return m_pTransfVB;
}

lg_void CLMeshD3DBasic::UnlockTransfVB()
{

}

lg_bool CLMeshD3DBasic::Load(LMPath szFile)
{
	Unload();
	
	LF_FILE3 fin=LF_Open(szFile, LF_ACCESS_READ, LF_OPEN_EXISTING);
	if(!fin)
		return LG_FALSE;
	
	lg_bool bResult=Serialize(fin, ReadFn, RW_READ);
	
	LF_Close(fin);
	
	m_nFlags=bResult?LM_FLAG_LOADED:0;
	
	if(bResult)
	{
		lf_path szTexDir;
		lf_path szTexPath;
		
		LF_GetDirFromPath(szTexDir, szFile);
		
		m_pTex=new tm_tex[m_nNumMtrs];
		for(lg_dword i=0; i<m_nNumMtrs; i++)
		{
			_snprintf(szTexPath, LF_MAX_PATH, "%s%s", szTexDir, m_pMtrs[i].szFile);
			szTexPath[LF_MAX_PATH]=0;
			m_pTex[i]=CLTMgr::TM_LoadTex(szTexPath, 0);
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
lg_bool CLMeshD3DBasic::Save(LMPath szFile)
{
	return LG_FALSE;
}

lg_void CLMeshD3DBasic::Unload()
{
	CLMesh2::Unload();
	if(m_pTex){delete [] m_pTex; m_pTex=LG_NULL;}
	if(m_pTransfVB){delete[]m_pTransfVB; m_pTransfVB=LG_NULL;}
	DeleteAnimData();
}

lg_void CLMeshD3DBasic::Render()
{
	if(!m_nFlags&LM_FLAG_LOADED)
		return;
		
	s_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);	
	s_pDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	s_pDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
	s_pDevice->SetRenderState(D3DRS_NORMALIZENORMALS, TRUE);
	s_pDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
	
	s_pDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
	s_pDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
	s_pDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_WRAP);
	s_pDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_WRAP);

	s_pDevice->SetFVF(D3DFVF_XYZ|D3DFVF_NORMAL|D3DFVF_TEX1);
	
	for(lg_dword i=0; i<m_nNumSubMesh; i++)
	{
		CLTMgr::TM_SetTexture(m_pTex[m_pSubMesh[i].nMtrIndex], 0);
		s_pDevice->DrawIndexedPrimitiveUP(
			D3DPT_TRIANGLELIST,
			0,
			m_nNumVerts,
			m_pSubMesh[i].nNumTri,
			&m_pIndexes[m_pSubMesh[i].nFirstIndex],
			D3DFMT_INDEX16,
			m_pTransfVB,
			sizeof(MeshVertex));
	}
	
}
	

lg_uint __cdecl CLMeshD3DBasic::ReadFn(lg_void* file, lg_void* buffer, lg_uint size)
{
	return LF_Read(file, buffer, size);
}