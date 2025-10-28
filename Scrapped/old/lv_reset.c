#include "common.h"
#include "lv_reset.h"
#include "lv_init.h"
#include "lg_sys.h"

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
L_result LV_ValidateGraphics(L3DGame* lpGame)
{
	VCon_Validate(lpGame->v.m_lpVCon, lpGame->v.m_lpDevice);
	LVT_ValidateInvalidate(lpGame->v.m_lpTestObj, lpGame->v.m_lpDevice, L_true);
	/* Reset the states. */
	Err_Printf("Calling LV_SetStates...");
	if(!LV_SetStates(lpGame))
		Err_Printf("An error occured while setting sampler and render states.");
	return LG_OK;
}

L_result LV_InvalidateGraphics(L3DGame* lpGame)
{
	VCon_Invalidate(lpGame->v.m_lpVCon);
	LVT_ValidateInvalidate(lpGame->v.m_lpTestObj, L_null, L_false);
	return LG_OK;
}


/******************************************************************* 
	LV_Restart()

	Resets the video with the present parameters located in the cvar
	list.  This is called from LV_ValidateDevice() when the device
	gets reset, or when the user sends the VRESTART command to the
	console.
********************************************************************/

L_result LV_Restart(L3DGame* lpGame)
{
	D3DPRESENT_PARAMETERS pp;

	L_bool bBackSurface=L_false;
	L_result nResult=0;

	if(!lpGame)
		return -1;

	/* Invalidate all graphics. */
	LV_InvalidateGraphics(lpGame);
	/* Reset the device. */

	/* If there was a back surface invalidate it. */
	if(lpGame->v.m_lpBackSurface)
	{
		lpGame->v.m_lpBackSurface->lpVtbl->Release(lpGame->v.m_lpBackSurface);
		lpGame->v.m_lpBackSurface=L_null;
		bBackSurface=L_true;
	}

	/* Reset the device. */
	memset(&pp, 0, sizeof(pp));
	LV_SetPPFromCVars(lpGame, &pp);

	/* Call reset. */
	nResult=lpGame->v.m_lpDevice->lpVtbl->Reset(lpGame->v.m_lpDevice, &pp);
	Err_PrintDX("IDirect3DDevice9::Reset", nResult);
	if(L_failed(nResult))
	{
		return LVERR_CANTRECOVER;
	}
	else
	{
		Err_Printf("Reset Device With the following Present Parameters:");
		LV_PrintPP(&pp);
		if(bBackSurface)
		{
			nResult=lpGame->v.m_lpDevice->lpVtbl->GetBackBuffer(
				lpGame->v.m_lpDevice,
				0,
				0,
				D3DBACKBUFFER_TYPE_MONO,
				&(lpGame->v.m_lpBackSurface));
			Err_PrintDX("IDirect3DDevice9::GetBackBuffer", nResult);
		}
		/* Finally validate the graphics. */
		LV_ValidateGraphics(lpGame);
	}
	return LG_OK;

}

/******************************************************************************* 
	LV_ValidateDevice()

	Called to make sure that the device is valid for rendering.  This is called
	every frame before anything is rendered.  If this funciton fails it means
	the device can't render right now, and in some cases it means the app
	should shut down.  If it succeeds it means it is okay for the device
	to draw.
********************************************************************************/

L_result LV_ValidateDevice(L3DGame* lpGame)
{
	L_result nResult=0;
	L_bool bGetBackSurface=L_false;
	if(!lpGame || !lpGame->v.m_lpDevice)
	{
		Err_Printf("No device to check validation on.");
		return LVERR_NODEVICE;
	}
	if(L_succeeded((nResult=lpGame->v.m_lpDevice->lpVtbl->TestCooperativeLevel(lpGame->v.m_lpDevice))))
		return LG_OK;



	switch(nResult)
	{
	case D3DERR_DEVICELOST:
		/* We can't do anything if the device is lost, we
			have to wait for it to recover. */
		return LVERR_DEVICELOST;
	case D3DERR_DEVICENOTRESET:
	{
		D3DPRESENT_PARAMETERS pp;
		memset(&pp, 0, sizeof(pp));
		/* If the device is not reset it means that windows is
			giving control of the application back to us and we
			need to reset it. */
		if(nResult=L_failed(LV_Restart(lpGame)))
			return nResult;
		return LVERR_DEVICERESET;
	}
	default:
		break;
	}

	return LVERR_DEVICELOST;	
}