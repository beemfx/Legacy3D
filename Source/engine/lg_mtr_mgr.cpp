#include "lg_mtr_mgr.h"
#include "lg_err.h"
#include "lg_err_ex.h"
#include "lx_sys.h"
#include "lg_tmgr.h"
#include "lg_fxmgr.h"

CLMtrMgr* CLMtrMgr::s_pMtrMgr=LG_NULL;

CLMtrMgr::CLMtrMgr(lg_dword nMaxMtr)
	: CLHashMgr(nMaxMtr, "MatMgr")
	, m_pMtrMem(LG_NULL)
{
	s_pMtrMgr=this;
	
	//Prepare the list stack.
	m_pMtrMem=new MtrItem[m_nMaxItems];
	if(!m_pMtrMem)
		throw LG_ERROR(LG_ERR_OUTOFMEMORY, LG_NULL);
		
	for(lg_dword i=0; i<m_nMaxItems; i++)
	{
		m_stkMtr.Push(&m_pMtrMem[i]);
	}
	
	//Setup the default material, we simply
	//use the default texture and effect for this.
	m_DefMtr.Texture=CLHashMgr::HM_DEFAULT_ITEM;
	m_DefMtr.Effect=CLHashMgr::HM_DEFAULT_ITEM;
	//Setup the default itme
	m_pItemList[0].pItem=&m_DefMtr;
	//m_pDefItem=&m_DefMtr;
}

CLMtrMgr::~CLMtrMgr()
{
	CLHashMgr::UnloadItems();
	m_stkMtr.Clear();
	LG_SafeDeleteArray(m_pMtrMem);
}

MtrItem* CLMtrMgr::DoLoad(lg_path szFilename, lg_dword nFlags)
{
	MtrItem* pOut = (MtrItem*)m_stkMtr.Pop();
	if(!pOut)
		return LG_NULL;
		
	lx_mtr mtr;
		
	if(!LX_LoadMtr(szFilename, &mtr))
		pOut=LG_NULL;
	else
	{
		//If we loaded then let's load the texture,
		//and effect, note that sometimes a texture
		//or effect is not specified for a material,
		//in this case we set them to 0 and the game
		//can do whatever it wants (whether that be to
		//not use an effect, or to use the default
		//effect or texure.
		pOut->Texture=mtr.szTexture[0]?CLTMgr::TM_LoadTex(mtr.szTexture, 0):0;
		pOut->Effect=mtr.szFx[0]?CLFxMgr::FXM_LoadFx(mtr.szFx, 0):0;
	}
		
	return pOut;
}

void CLMtrMgr::DoDestroy(MtrItem* pItem)
{
	if(!pItem)
		return;
	//All we have to do is push the item back on the stack
	//no unloading is necessary since the material is a
	//fixed size.
	m_stkMtr.Push(pItem);
}

void CLMtrMgr::RenderVB(lg_material mtr, IDirect3DDevice9* pDevice, IDirect3DVertexBuffer9* pVB)
{
	//Nothing yet.
}

void CLMtrMgr::GetInterfaces(lg_material mtr, tm_tex* pTex, ID3DXEffect** pFX)
{
	mtr+=mtr?0:1;
	#if 0
	if(mtr==0 || mtr>=this->m_nMaxItems)
	{
		Err_Printf("Tried to get %d", mtr);
		return;
	}
	#endif
		
	*pTex=m_pItemList[mtr-1].pItem->Texture;
	*pFX=CLFxMgr::FXM_GetInterface(m_pItemList[mtr-1].pItem->Effect);
}

lg_material CLMtrMgr::MTR_Load(lg_path szFilename, lg_dword nFlags)
{
	return s_pMtrMgr->Load(szFilename, nFlags);
}

void CLMtrMgr::MTR_RenderVB(lg_material mtr, IDirect3DDevice9 *pDevice, IDirect3DVertexBuffer9 *pVB)
{
	s_pMtrMgr->RenderVB(mtr, pDevice, pVB);
}

void CLMtrMgr::MTR_GetInterfaces(lg_material mtr, tm_tex *pTex, ID3DXEffect **pFX)
{
	s_pMtrMgr->GetInterfaces(mtr, pTex, pFX);
}
