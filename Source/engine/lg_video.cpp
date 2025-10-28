#include "common.h"
#include "lg_sys.h"
#include "lg_err.h"
#include "lg_cvars.h"
#include "lg_err_ex.h"

/****************************************************************
	LV_ValidateGraphics() & LV_InvalidateGraphics ()

	These functions are called whenever the graphics in the game
	have to be reset, so any graphics that need to be loaded
	should be validated and invalidated here.  Invalidating means
	removing the object as a Direct3D resource.  D3DPOOL_MANAGED
	resources usually don't need to be invalidated or revalidated.
	Any object invalidated in LV_InvalidateGraphics, should be
	revalidated in LV_ValidateGraphics.
*****************************************************************/
void CLGame::LV_ValidateGraphics()
{
	Err_Printf("Validating Direct3D objects...");
	//This will set the texture load modes...
	//LV_InitTexLoad();
	
	Err_Printf("Validating graphical console...");
	m_VCon.Validate();
	
	#ifdef OLD_WORLD
	Err_Printf("Validating meshes...");
	m_pMeshMgr->ValidateMeshes();
	
	Err_Printf("Validating world...");
	
	m_pWorld->Validate();
	#endif
	
	ValidateMgrs();
	//m_pMMgr->ValidateMeshes();
	
	m_WorldCln.Validate();
	/* Reset the states. */
	Err_Printf("Calling SetStates...");
	if(!m_Video.SetStates())
		Err_Printf("An error occured while setting sampler and render states.");
		
	//m_pTMgr->Validate();
	//m_pFxMgr->Validate();
}

void CLGame::LV_InvalidateGraphics()
{
	Err_Printf("Invalidating Direct3D objects...");
	Err_Printf("Invalidating World...");
	#ifdef OLD_WORLD
	m_pWorld->Invalidate();
	#endif
	Err_Printf("Invalidating graphical console...");
	m_VCon.Invalidate();
	//m_pMMgr->InvalidateMeshes();
	#ifdef OLD_WORLD
	Err_Printf("Invalidating meshes...");
	m_pMeshMgr->InvalidateMeshes();
	#endif OLD_WORLD
	m_WorldCln.Invalidate();
	Err_Printf("Purging textures...");
	//m_pTMgr->Invalidate();
	//m_pFxMgr->Invalidate();
	InvalidateMgrs();
}

#if 0
void CLGame::LV_InitTexLoad()
{
	Err_Printf("Initializing texture management...");
	lg_dword TexLdFlags=0;
	if(CV_Get(CVAR_v_UseMipMaps)->nValue)
		TexLdFlags|=TEXLOAD_GENMIPMAP;
	if(CV_Get(CVAR_v_HWMipMaps)->nValue)
		TexLdFlags|=TEXLOAD_HWMIPMAP;
	if(CV_Get(CVAR_v_MipGenFilter)->nValue>=FILTER_LINEAR)
		TexLdFlags|=TEXLOAD_LINEARMIPFILTER;
	if(CV_Get(CVAR_v_Force16BitTextures)->nValue)
		TexLdFlags|=TEXLOAD_FORCE16BIT;
	if(CV_Get(CVAR_v_16BitTextureAlpha)->nValue)
		TexLdFlags|=TEXLOAD_16BITALPHA;
			
	lg_uint nSizeLimit=(lg_uint)CV_Get(CVAR_v_TextureSizeLimit)->nValue;
	//CLTexMgr::Init calls Tex_SetLoadOptionsFlag.
	#if 0
	m_cTexMgr.Init(
		s_pDevice, 
		CV_Get(CVAR_v_szDefaultTexture)->szValue, 
		TexLdFlags, 
		nSizeLimit);
	#endif

	/*
	Tex_SetLoadOptions(
		(lg_bool)CVar_GetValue(s_cvars, CVAR_v_UseMipMaps, LG_NULL),
		(lg_bool)CVar_GetValue(s_cvars, CVAR_v_HWMipMaps, LG_NULL),
		(TEXFILTER_MODE)(lg_int)CVar_GetValue(s_cvars, CVAR_v_MipGenFilter, LG_NULL),
		(lg_uint)CVar_GetValue(s_cvars, CVAR_v_TextureSizeLimit, LG_NULL),
		(lg_bool)CVar_GetValue(s_cvars, CVAR_v_Force16BitTextures, LG_NULL),
		(lg_bool)CVar_GetValue(s_cvars, CVAR_v_16BitTextureAlpha, LG_NULL));
	*/	
}
#endif

/******************************************************************* 
	LV_Restart()

	Resets the video with the present parameters located in the cvar
	list.  This is called from LV_ValidateDevice() when the device
	gets reset, or when the user sends the VRESTART command to the
	console.
********************************************************************/

lg_result CLGame::LV_Restart()
{
	/* Invalidate all graphics. */
	LV_InvalidateGraphics();
	/* Reset the device. */

	if(m_Video.Restart())
	{
		LV_ValidateGraphics();
		return LG_OK;
	}
	else
		return LVERR_CANTRECOVER;
}

/******************************************************************************* 
	LV_ValidateDevice()

	Called to make sure that the device is valid for rendering.  This is called
	every frame before anything is rendered.  If this funciton fails it means
	the device can't render right now, and in some cases it means the app
	should shut down.  If it succeeds it means it is okay for the device
	to draw.  Should only be called if TestCooperativeLevel fails in Render
	methods.
********************************************************************************/

lg_result CLGame::LV_ValidateDevice()
{
	lg_result nResult=0;
	lg_bool bGetBackSurface=LG_FALSE;
	if(!s_pDevice)
	{
		Err_Printf("No device to check validation on.");
		//CVar_Set(s_cvars, CVAR_lg_ShouldQuit, DEF_TRUE);
		CV_Set(CV_Get(CVAR_lg_ShouldQuit), DEF_TRUE);
		return LG_FAIL;
	}
	
	if(LG_SUCCEEDED(nResult=s_pDevice->TestCooperativeLevel()))
		return LG_OK;



	switch(nResult)
	{
	case D3DERR_DEVICELOST:
		/* We can't do anything if the device is lost, we
			have to wait for it to recover. */
		return LG_V_OR_S_DISABLED;
	case D3DERR_DEVICENOTRESET:
	{
		D3DPRESENT_PARAMETERS pp;
		memset(&pp, 0, sizeof(pp));
		/* If the device is not reset it means that windows is
			giving control of the application back to us and we
			need to reset it. */
		if(LG_FAILED(LV_Restart()))
		{
			Err_Printf("Could not restart device.");
			return LG_V_OR_S_DISABLED;
		}
		else
			return LG_OK;
		
	}
	default:
		return LG_SHUTDOWN;
	}

	return LG_SHUTDOWN;	
}