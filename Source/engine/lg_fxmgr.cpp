#include <d3dx9.h>
#include "lg_fxmgr.h"
#include "lg_func.h"
#include "lg_err.h"
#include "lg_err_ex.h"
#include "lf_sys2.h"
#include "lc_sys2.h"
#include "lg_cvars.h"

CLFxMgr* CLFxMgr::s_pFxMgr=LG_NULL;

CLFxMgr::CLFxMgr(IDirect3DDevice9* pDevice, lg_dword nMaxFx):
	CLHashMgr(nMaxFx, "FxMgr"),
	m_pDevice(LG_NULL),
	m_nPool(D3DPOOL_MANAGED)
{
	//If the effect manager already exists then we made an error somewhere.
	if(s_pFxMgr)
	{
		throw new CLError(
			LG_ERR_DEFAULT, __FILE__, __LINE__, 
			LG_TEXT("The Effect Manager can only be initialized once."));
	}
	
	//Store the global manager.
	s_pFxMgr=this;
	
	//Save a copy of the device.
	m_pDevice=pDevice;
	m_pDevice->AddRef();
	
	//Load the default effect:
	m_pItemList[0].pItem=DoLoad(CV_Get(CVAR_fxm_DefaultFx)->szValue, 0);
	LG_strncpy(m_pItemList[0].szName, CV_Get(CVAR_fxm_DefaultFx)->szValue, LG_MAX_PATH);
	//m_pDefItem=this->DoLoad(CV_Get(CVAR_fxm_DefaultFx)->szValue, 0);
	
	if(m_pItemList[0].pItem)
		Err_Printf("FxMgr: Default effect is \"%s\".", CV_Get(CVAR_fxm_DefaultFx)->szValue);
	else
		Err_Printf("FxMgr: Could not load default effect.");
	
	Err_Printf("FxMgr: Initialized with %u effects available.", m_nMaxItems);
}

CLFxMgr::~CLFxMgr()
{
	//Clear global manager.
	s_pFxMgr=LG_NULL;
	//Release all effects.
	UnloadItems();
	DoDestroy(m_pItemList[0].pItem);
	//Release Direct3D
	LG_SafeRelease(m_pDevice);	
	//Base class deallocates memory.
}

void CLFxMgr::Invalidate()
{
	//Starting at 0 will clear invalidate the default
	for(lg_dword i=0; i<m_nMaxItems; i++)
	{
		if(m_pItemList[i].nHashCode!=CLHashMgr::HM_EMPTY_NODE){
			DoDestroy(m_pItemList[i].pItem);
		}
	}
}

void CLFxMgr::Validate()
{
	//Load all other effects, note that the default effect
	//is included because it is at 0.
	for(lg_dword i=0; i<m_nMaxItems; i++)
	{
		if(m_pItemList[i].nHashCode!=CLHashMgr::HM_EMPTY_NODE)
			m_pItemList[i].pItem=DoLoad(m_pItemList[i].szName, m_pItemList[i].nFlags);
	}
	
	if(m_pItemList[0].pItem)
		Err_Printf("FxMgr: Default effect is \"%s\".", CV_Get(CVAR_fxm_DefaultFx)->szValue);
	else
		Err_Printf("FxMgr: Could not load default effect.");
}

ID3DXEffect* CLFxMgr::DoLoad(lg_path szFilename, lg_dword nFlags)
{
	//Open the file, we open with memory access.
	LF_FILE3 fileFx=LF_Open(szFilename, LF_ACCESS_MEMORY|LF_ACCESS_READ, LF_OPEN_EXISTING);
	if(!fileFx)
		return LG_NULL;
		
	ID3DXEffect* pOut=LG_NULL;
	
	ID3DXBuffer* pErrMsg=LG_NULL;
	
	HRESULT hRes=D3DXCreateEffect(
		m_pDevice,
		LF_GetMemPointer(fileFx),
		LF_GetSize(fileFx),
		LG_NULL,
		LG_NULL,
		0,
		LG_NULL,
		&pOut,
		&pErrMsg);

	LF_Close(fileFx);
	
	if(LG_FAILED(hRes))
	{
		pOut=LG_NULL;
		Err_Printf("FxMgr::FxLoad ERROR: \"%s\"", (const char*)pErrMsg->GetBufferPointer());
		pErrMsg->Release();
		pOut=LG_NULL;
	}
	else
	{
		D3DXHANDLE hBT;
		pOut->FindNextValidTechnique(NULL, &hBT);
		//Err_Printf("FxMgr: Found technique \"%s\".", hBT);
	}
	
	return pOut;
}

void CLFxMgr::DoDestroy(ID3DXEffect* pItem)
{
	LG_SafeRelease(pItem);
}

void CLFxMgr::SetEffect(fxm_fx effect)
{
	if(effect==CLHashMgr::HM_DEFAULT_ITEM)
		;//m_pDevice->SetEffect(nStage, m_pDefFx);
	else if(effect)
		;//m_pDevice->SetEffect(nStage, m_pItemList[effect-1].pFx);
	else
		;//m_pDevice->SetEffect(nStage, 0);
}

//Temp method only:
ID3DXEffect* CLFxMgr::GetInterface(fxm_fx effect)
{
	return effect?m_pItemList[effect-1].pItem:m_pItemList[0].pItem;
	/*
	if(effect==CLHashMgr::HM_DEFAULT_ITEM)
		return m_pDefItem;//m_pDevice->SetEffect(nStage, m_pDefFx);
	else if(effect)
		return m_pItemList[effect-1].pItem;//m_pDevice->SetEffect(nStage, m_pItemList[effect-1].pFx);
	else
		return m_pDefItem;//m_pDevice->SetEffect(nStage, 0);
	*/
}


fxm_fx CLFxMgr::FXM_LoadFx(lg_path szFilename, lg_dword nFlags)
{
	return s_pFxMgr->Load(szFilename, nFlags);
}

void CLFxMgr::FXM_SetEffect(fxm_fx effect)
{
	return s_pFxMgr->SetEffect(effect);
}

ID3DXEffect* CLFxMgr::FXM_GetInterface(fxm_fx effect)
{
	return s_pFxMgr->GetInterface(effect);
}