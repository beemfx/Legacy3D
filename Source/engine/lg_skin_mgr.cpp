#include "lg_skin_mgr.h"

CLSkinMgr* CLSkinMgr::s_pSkinMgr=LG_NULL;

CLSkinMgr::CLSkinMgr(lg_dword nMaxSkin)
	: CLHashMgr(nMaxSkin, "SkinMgr")
	, m_pSkinMem(LG_NULL)
{
	s_pSkinMgr=this;
	
	//Prepare the list stack.
	m_pSkinMem=new SkinItem[m_nMaxItems];
	if(!m_pSkinMem)
		throw LG_ERROR(LG_ERR_OUTOFMEMORY, LG_NULL);
	
	m_stkSkin.Init(m_pSkinMem, m_nMaxItems, sizeof(SkinItem));
	//Setup the default skin
	m_pItemList[0].pItem=&m_DefSkin;
}

CLSkinMgr::~CLSkinMgr()
{
	CLHashMgr::UnloadItems();
	m_stkSkin.Clear();
	LG_SafeDeleteArray(m_pSkinMem);
}

SkinItem* CLSkinMgr::DoLoad(lg_path szFilename, lg_dword nFlags)
{
	SkinItem* pOut = (SkinItem*)m_stkSkin.Pop();
	if(!pOut)
		return LG_NULL;

	if(!pOut->Skin.Load(szFilename))
	{
		m_stkSkin.Push(pOut);
		pOut=LG_NULL;
	}
	return pOut;
}

void CLSkinMgr::DoDestroy(SkinItem* pItem)
{
	if(!pItem)
		return;
	//All we have to do is push the item back on the stack
	//no unloading is necessary since the material is a
	//fixed size.
	m_stkSkin.Push(pItem);
}

CLSkin* CLSkinMgr::GetInterface(sm_item skin)
{
	return skin?&m_pItemList[skin-1].pItem->Skin:LG_NULL;
}

CLSkin* CLSkinMgr::SM_Load(lg_path szFilename, lg_dword nFlags)
{
	sm_item skin=s_pSkinMgr->Load(szFilename, nFlags);
	return s_pSkinMgr->GetInterface(skin);
}
