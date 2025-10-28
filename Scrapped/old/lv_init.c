/* lv_init.c - Video initialization functions for Legacy 3D
	Copyright (c) 2006, Blaine Myers */
#include "common.h"
#include <windowsx.h>
#include "lg_sys.h"
#include "lv_init.h"

L_dword LV_StringToD3DFMT(char* szfmt);
L_dword LV_D3DFMTToBitDepth(D3DFORMAT fmt);
char*  LV_D3DFMTToString(D3DFORMAT fmt);


/***************************************************************
	LV_Init()

	This initializes all of the basic Direct3D stuff used in the
	game.  It doens't load 3D models, or textures, but it sets
	the render mode, and it does call the function to set all of
	the requested render and sampler stetes.  It doesn't create
	any Direct3D objects except for the IDirect3D9 interface, and
	the IDirect3DDevice9 interface (except for the graphical
	console, and the reason this is initialized here is because
	it always exists for the duration of the game, and it is 
	destroyed in LV_Shutdown.  LV_Init uses the cvars to 
	initialize the game, so the cvars must be register and set
	to the desired values before calling this function.
****************************************************************/

L_result LV_Init(L3DGame* lpGame)
{
	D3DPRESENT_PARAMETERS pp;
	CVar* cvar=NULL;
	L_dword dwBehaviorFlags=0;
	L_dword dwVertexProcFlag=0;
	L_result nResult=0x00000000l;

	/* To initialize the video we need to create an IDirect3D interface, set the present 
	parameters, then create the device. */
	if(!lpGame)
		return -1;

	if(!lpGame->m_Console || !lpGame->m_cvars)
		return -1;

	/* First create the IDirect3D interface. */
	Err_Printf("Creating IDirect3D9 interface...");
	lpGame->v.m_lpD3D=Direct3DCreate9(D3D_SDK_VERSION);
	if(lpGame->v.m_lpD3D)
		Err_Printf("IDirect3D9 interface created at 0x%08X.", lpGame->v.m_lpD3D);
	else
	{
		Err_Printf("Failed to create IDirect3D9 interface.");
		MessageBox(
			lpGame->m_hwnd,
			"This application requires ""DirectX 9.0c", "Legacy3D",
			MB_OK|MB_ICONINFORMATION);
		return -1;
	}

	/* Get the Direct3D setup parameters from teh cvarlist. */
	cvar=CVar_GetCVar(lpGame->m_cvars, "v_AdapterID");
	lpGame->v.m_nAdapterID=cvar?(int)cvar->value:D3DADAPTER_DEFAULT;
	cvar=CVar_GetCVar(lpGame->m_cvars, "v_DeviceType");
	lpGame->v.m_nDeviceType=cvar?(int)cvar->value:D3DDEVTYPE_HAL;
	cvar=CVar_GetCVar(lpGame->m_cvars, "v_VertexProc");
	dwVertexProcFlag=cvar?(int)cvar->value:D3DCREATE_SOFTWARE_VERTEXPROCESSING;
	
	if(CVar_GetValue(lpGame->m_cvars, "v_FPU_Preserve", NULL))
		dwBehaviorFlags|=D3DCREATE_FPU_PRESERVE;
	if(CVar_GetValue(lpGame->m_cvars, "v_MultiThread", NULL))
		dwBehaviorFlags|=D3DCREATE_MULTITHREADED;
	if(CVar_GetValue(lpGame->m_cvars, "v_PureDevice", NULL))
		dwBehaviorFlags|=D3DCREATE_PUREDEVICE;
	if(CVar_GetValue(lpGame->m_cvars, "v_DisableDriverManagement", NULL))
		dwBehaviorFlags|=D3DCREATE_DISABLE_DRIVER_MANAGEMENT;
	if(CVar_GetValue(lpGame->m_cvars, "v_AdapterGroupDevice", NULL))
		dwBehaviorFlags|=D3DCREATE_ADAPTERGROUP_DEVICE;

	/* Make sure the adapter is valid. */
	if(lpGame->v.m_nAdapterID >= lpGame->v.m_lpD3D->lpVtbl->GetAdapterCount(lpGame->v.m_lpD3D))
	{
		Err_Printf( 
			"Adapter %i not available, using default adapter.", 
			lpGame->v.m_nAdapterID);

		CVar_Set(lpGame->m_cvars, "v_AdatperID", "ADAPTER_DEFAULT");

		lpGame->v.m_nAdapterID=0;
	}

	/* Show the adapter identifier information. */
	{
	D3DADAPTER_IDENTIFIER9 adi;
	memset(&adi, 0, sizeof(adi));
	lpGame->v.m_lpD3D->lpVtbl->GetAdapterIdentifier(
		lpGame->v.m_lpD3D,
		lpGame->v.m_nAdapterID,
		0,
		&adi);
	Err_Printf("Using \"%s\".", adi.Description);
	}

	/* We set the present parameters using the cvars. */
	memset(&pp, 0, sizeof(pp));
	Err_Printf("Initialzing present parameters from cvarlist...");
	if(!LV_SetPPFromCVars(lpGame, &pp))
	{
		Err_Printf("Could not initialize present parameters.");
		L_safe_release(lpGame->v.m_lpD3D);
		return -1;
	}
	Err_Printf("Present parameters initialization complete.");

	/* Check to see that the adapter type is available. */
	if(L_failed(nResult=
		lpGame->v.m_lpD3D->lpVtbl->CheckDeviceType(
		lpGame->v.m_lpD3D,
		lpGame->v.m_nAdapterID,
		lpGame->v.m_nDeviceType,
		pp.BackBufferFormat,
		pp.BackBufferFormat,
		pp.Windowed)))
	{
		Err_PrintDX("CheckDeviceType", nResult);
		Err_Printf("Cannot use selected device type, defaulting.");
		/* If the HAL device can't be used then CreateDevice will fail. */
		lpGame->v.m_nDeviceType=D3DDEVTYPE_HAL;
	}

	Err_Printf("Creating IDirect3DDevice9 interface...");
	nResult=lpGame->v.m_lpD3D->lpVtbl->CreateDevice(
		lpGame->v.m_lpD3D, 
		lpGame->v.m_nAdapterID,
		lpGame->v.m_nDeviceType,
		lpGame->m_hwnd,
		dwBehaviorFlags|dwVertexProcFlag,
		&pp,
		&lpGame->v.m_lpDevice);
	Err_PrintDX("IDirect3D9::CreateDevice", nResult);

	/* If we failed to create the device, we can try a some
		different vertex processing modes, if it still fails
		we exit. */
	while(L_failed(nResult))
	{
		if(dwVertexProcFlag==D3DCREATE_SOFTWARE_VERTEXPROCESSING)
			break;
		else if(dwVertexProcFlag==D3DCREATE_HARDWARE_VERTEXPROCESSING)
		{
			dwVertexProcFlag=D3DCREATE_MIXED_VERTEXPROCESSING;
			CVar_Set(lpGame->m_cvars, "v_VertexProc", "VERT_MIXED");
		}
		else
		{
			dwVertexProcFlag=D3DCREATE_SOFTWARE_VERTEXPROCESSING;
			CVar_Set(lpGame->m_cvars, "v_VertexProc", "VERT_SOFTWARE");
		}
		
		nResult=lpGame->v.m_lpD3D->lpVtbl->CreateDevice(
			lpGame->v.m_lpD3D, 
			lpGame->v.m_nAdapterID,
			lpGame->v.m_nDeviceType,
			lpGame->m_hwnd,
			dwBehaviorFlags|dwVertexProcFlag,
			&pp,
			&lpGame->v.m_lpDevice);
		
		Err_PrintDX("IDirect3D9::CreateDevice", nResult);
	}
	

	if(L_succeeded(nResult))
	{
		Err_Printf( 
			"IDirect3DDevice9 interface created at 0x%08X", 
			lpGame->v.m_lpDevice);

		Err_Printf("Using the following Present Parameters:");
		LV_PrintPP(&pp);
	}
	else
	{
		Err_Printf( 
			"Failed to create IDirect3DDevice9 interface.", 
			nResult);
		Err_Printf(
			"Try software vertex processing.");

		L_safe_release(lpGame->v.m_lpD3D);
		return -1;
	}
	
	/* Call present to change the resolution.*/
	/* Could load up a loading screen here!.*/
	IDirect3DDevice9_Clear(lpGame->v.m_lpDevice, 0,0,D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,0xFF5050FF,1.0f,0);
	IDirect3DDevice9_Present(lpGame->v.m_lpDevice, NULL, NULL, NULL, NULL);

	/* Display allowed formats, and also set a few cvars that control them. 
		The allowed formats function only deals with the basic formats, not
		any advanced or compressed texture formats.*/
	LV_SupportedTexFormats(
		lpGame->m_cvars, 
		lpGame->v.m_lpD3D, 
		lpGame->v.m_nAdapterID, 
		lpGame->v.m_nDeviceType,
		pp.BackBufferFormat);


	/* Set the default viewport, we don't actually need to do
		this because d3d will set the default viewport for us. */
	
	/*
	{
	D3DVIEWPORT9 ViewPort;
	ViewPort.X=0;
	ViewPort.Y=0;
	ViewPort.Width=(L_dword)CVar_GetValue(lpGame->m_cvars, "v_Width", L_null);
	ViewPort.Height=(L_dword)CVar_GetValue(lpGame->m_cvars, "v_Height", L_null);
	ViewPort.MinZ=0.0f;
	ViewPort.MaxZ=1.0f;
	lpGame->v.m_lpDevice->lpVtbl->SetViewport(lpGame->v.m_lpDevice, &ViewPort);
	}
	*/
	

	/* With everything initialized we can set the sampler and render states. */
	Err_Printf("Calling LV_SetStates...");
	if(!LV_SetStates(lpGame))
		Err_Printf("An error occured while setting the sampler and render states.");


	/* Once the video is initialized, we can initalize the graphical console. */
	Err_Printf("Initializing console graphical interface...");
	
	lpGame->v.m_lpVCon=VCon_Create(
		lpGame->m_Console, 
		lpGame->m_cvars,
		lpGame->v.m_lpDevice);
	/* If the creation of the graphical console fails, the game can still run,
		but any calls to VCon_ functions will fail because the lpVCon data item is null.*/
	if(!lpGame->v.m_lpVCon)
		Err_Printf("Graphical console creation failed, feature not available.");
	

	/* some test stuff. */
	lpGame->v.m_lpTestObj=LVT_CreateTest(lpGame->v.m_lpDevice);
	return 0x00000000l;
}

L_bool LV_SupportedTexFormats(HCVARLIST cvars, IDirect3D9* lpD3D, UINT nAdapterID, UINT nDeviceType, D3DFORMAT dfmt)
{
	L_result nResult=0;
	Err_Printf("Supported Texture Formats:");
	/* We check to see which texture formats are supported, we are only 
		checking the ones that we plan on using in this application.  We set
		appropriate cvars for the texture formats, so that when we call the
		texture loading funcitons, they can check the cvars to determine
		what format to generate the textrues.  This code is not bug proof
		as it doesn't set an cvars that note that D3DFMT_R5G6B5 textures are
		supported, it just assumes they are supported (pretty much all video
		cards I've encountered support this format that is why I assume it
		will work.*/
	#define TEX_CHECK(fmt) IDirect3D9_CheckDeviceFormat(lpD3D,nAdapterID,nDeviceType,dfmt,0,D3DRTYPE_TEXTURE, fmt);
	nResult=TEX_CHECK(D3DFMT_A8R8G8B8);
	if(L_failed(nResult))CVar_Set(cvars, "v_32BitTextureAlpha", "FALSE");
	else Err_Printf("   %s", LV_D3DFMTToString(D3DFMT_A8R8G8B8));

	nResult=TEX_CHECK(D3DFMT_X8R8G8B8)
	if(L_failed(nResult))CVar_Set(cvars, "v_Force16BitTextures", "TRUE");
	else Err_Printf("   %s", LV_D3DFMTToString(D3DFMT_X8R8G8B8));

	nResult=TEX_CHECK(D3DFMT_A1R5G5B5)
	if(L_failed(nResult))CVar_Set(cvars, "v_16BitTextureAlpha", "FALSE");
	else Err_Printf("   %s", LV_D3DFMTToString(D3DFMT_A1R5G5B5));

	nResult=TEX_CHECK(D3DFMT_X1R5G5B5)
	if(L_succeeded(nResult))Err_Printf("   %s", LV_D3DFMTToString(D3DFMT_X1R5G5B5));

	nResult=TEX_CHECK(D3DFMT_R5G6B5)
	if(L_succeeded(nResult))Err_Printf("   %s", LV_D3DFMTToString(D3DFMT_R5G6B5));

	return L_true;
}

/*********************************************************
	LV_Shutdown()

	This uninitialized everything that was initialized in
	LV_Init(), it doesn't necessarly shut down other items
	that were initialized by Direct3D such as textures and
	vertex buffers used by  in the game.  This function 
	should only be called after all Direct3D pool objects 
	have been released.
***********************************************************/

L_result LV_Shutdown(L3DGame* lpGame)
{
	L_ulong nNumLeft=0;

	if(!lpGame)
		return -1;

	if(!(lpGame->v.m_lpDevice))
	{
		Err_Printf("Device not available for game shutdown.");
		return L_false;
	}

	/* Some test stuff. */
	LVT_Delete(lpGame->v.m_lpTestObj);

	/* Destroy the graphical console, then shutdown the video. */
	Err_Printf("Destroying gconsole graphical interface.");
	VCon_Delete(lpGame->v.m_lpVCon);
	lpGame->v.m_lpVCon=L_null;

	Err_Printf("Releasing IDirect3DDevice9 interface...");
	nNumLeft=lpGame->v.m_lpDevice->lpVtbl->Release(lpGame->v.m_lpDevice);
	Err_Printf( 
		"Released IDirect3DDevice9 interface at 0x%08X with %i references left.", 
		lpGame->v.m_lpDevice, 
		nNumLeft);
	lpGame->v.m_lpDevice=NULL;
	
	Err_Printf("Releasing IDirect3D9 interface...");
	if(lpGame->v.m_lpD3D)
	{
		nNumLeft=lpGame->v.m_lpD3D->lpVtbl->Release(lpGame->v.m_lpD3D);
		Err_Printf( 
			"Released IDirect3D9 interface at 0x%08X with %i references left.", 
			lpGame->v.m_lpD3D, 
			nNumLeft);
	}
	return 0x00000000l;
}

/***********************************************
	LV_CorrectWindowSize()

	Adjusts the window size so that the client
	area is the specified width and height.
	Called from LV_SetPPFromCVars.
************************************************/
L_bool LV_CorrectWindowSize(
	HWND hWnd, 
	DWORD nWidth, 
	DWORD nHeight)
{
	RECT rc;
	SetRect(&rc, 0, 0, nWidth, nHeight);
	AdjustWindowRectEx(
		&rc, 
		GetWindowStyle(hWnd), 
		GetMenu(hWnd)!=NULL,
		GetWindowExStyle(hWnd));				
	SetWindowPos(
		hWnd, 
		NULL, 
		0, 
		0, 
		rc.right-rc.left, 
		rc.bottom-rc.top,
		SWP_NOMOVE|SWP_NOZORDER|SWP_NOACTIVATE);
	
	return L_true;
}


L_bool LV_PrintPP(D3DPRESENT_PARAMETERS* pp)
{
	Err_Printf("   Resolution: %iX%iX%i(%s) at %iHz, %s", 
		pp->BackBufferWidth, 
		pp->BackBufferHeight, 
		LV_D3DFMTToBitDepth(pp->BackBufferFormat), 
		LV_D3DFMTToString(pp->BackBufferFormat),
		pp->FullScreen_RefreshRateInHz,
		pp->Windowed?"WINDOWED":"FULL SCREEN");
	Err_Printf("   FSAA %s (%i)", pp->MultiSampleType==D3DMULTISAMPLE_NONE?"Disabled":"Enabled", pp->MultiSampleQuality);
	Err_Printf("   Back Buffers: %i, Swap Effect: %s", 
		pp->BackBufferCount, 
		pp->SwapEffect==D3DSWAPEFFECT_DISCARD?"DISCARD":pp->SwapEffect==D3DSWAPEFFECT_FLIP?"FLIP":"COPY");
	Err_Printf("   Flags: %s%s%s%s",
		L_CHECK_FLAG(pp->Flags, D3DPRESENTFLAG_DEVICECLIP)?"DEVICECLIP ":"",
		L_CHECK_FLAG(pp->Flags, D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL)?"DISCARD_DEPTHSTRENCIL ":"",
		L_CHECK_FLAG(pp->Flags, D3DPRESENTFLAG_LOCKABLE_BACKBUFFER)?"LOCKABLE_BACKBUFFER ":"",
		L_CHECK_FLAG(pp->Flags, D3DPRESENTFLAG_VIDEO)?"VIDEO ":"");
	Err_Printf("   Device Window At 0x%08X", pp->hDeviceWindow);
	Err_Printf("   Auto Depth Stencil %s (%s)", pp->EnableAutoDepthStencil?"Enabled":"Disabled", LV_D3DFMTToString(pp->AutoDepthStencilFormat));
	Err_Printf("   Presentation Interval: %i", pp->PresentationInterval);

	return L_true;
}

/**********************************************************
	LV_SetPPFromCVars()

	This sets the D3DPRESENT_PARAMETERS structure based on
	the cvars.  It also does some checking to make sure
	that the settings are valid for the device, and chenges
	them as necesary. This function is called in LV_Init, 
	and LV_Restart.
***********************************************************/
int LV_SetPPFromCVars(
	L3DGame* lpGame, 
	D3DPRESENT_PARAMETERS* pp)
{
	CVar* cvar=NULL;
	DWORD dwFlags=0;
	L_long nBackBufferFmt=0;
	L_dword nBitDepth=0;
	L_bool bCheck=0;
	HCVARLIST cvarlist;
	HLCONSOLE hConsole;
	L_result nResult=0;
	L_dword nQualityLevels=0;

	if(!pp || !lpGame->m_cvars)
		return 0;

	cvarlist=lpGame->m_cvars;
	hConsole=lpGame->m_Console;

	/* This function will attemp to get the appropriate values for the present
		parameters from the cvars, if it can't get the cvar it will SET_PP_FROM_CVAR an 
		appropriate default value, that should work on most systems. */

	#define SET_PP_FROM_CVAR(prpr, cvarname, defvalue) {cvar=CVar_GetCVar(cvarlist, cvarname);\
		pp->prpr=cvar?(int)cvar->value:defvalue;\
		if(!cvar)\
			Err_Printf("Could not get \"%s\" for video initialization.", cvarname);}

	SET_PP_FROM_CVAR(BackBufferWidth, "v_Width", 640);
	SET_PP_FROM_CVAR(BackBufferHeight, "v_Height", 480);

	/* Create the back buffer format. */

	cvar=CVar_GetCVar(cvarlist, "v_BitDepth");
	if(cvar)nBitDepth=(int)cvar->value;
	/* Transform the bit depth to a D3DFORMAT, not if the bit depth is not valid, we
		just go for D3DFMT_R5G6B5.*/
	if(nBitDepth==32)
		pp->BackBufferFormat=D3DFMT_X8R8G8B8;
	else if(nBitDepth==16)
		pp->BackBufferFormat=D3DFMT_R5G6B5;
	else if(nBitDepth==15)
		pp->BackBufferFormat=D3DFMT_X1R5G5B5;
	else
	{
		nBitDepth=16;
		pp->BackBufferFormat=D3DFMT_R5G6B5;
		CVar_Set(cvarlist, "v_BitDepth", "16");
	}

	SET_PP_FROM_CVAR(BackBufferCount, "v_ScreenBuffers", 1);
	SET_PP_FROM_CVAR(SwapEffect, "v_SwapEffect", D3DSWAPEFFECT_DISCARD);
	pp->hDeviceWindow=lpGame->m_hwnd;
	nQualityLevels=(int)CVar_GetValue(lpGame->m_cvars, "v_FSAAQuality", &bCheck);
	if(!bCheck)
	{
		Err_Printf("Could not get \"v_FSAAQuality\" for video initialization.");
		nQualityLevels=0;
	}
	SET_PP_FROM_CVAR(Windowed, "v_Windowed", L_false);
	SET_PP_FROM_CVAR(EnableAutoDepthStencil, "v_EnableAutoDepthStencil", TRUE);
	SET_PP_FROM_CVAR(AutoDepthStencilFormat, "v_AutoDepthStencilFormat", D3DFMT_D16);
	/* Set the flags. */
	cvar=CVar_GetCVar(cvarlist, "v_LockableBackBuffer");
	if(cvar)
		if(cvar->value)
			dwFlags|=D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
	cvar=CVar_GetCVar(cvarlist, "v_DiscardDepthStencil");
	if(cvar)
		if(cvar->value)
			dwFlags|=D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL;
	cvar=CVar_GetCVar(cvarlist, "v_DeviceClip");
	if(cvar)
		if(cvar->value)
			dwFlags|=D3DPRESENTFLAG_DEVICECLIP;
	cvar=CVar_GetCVar(cvarlist, "v_VideoHint");
	if(cvar)
		if(cvar->value)
			dwFlags|=D3DPRESENTFLAG_VIDEO;
	pp->Flags=dwFlags;

	SET_PP_FROM_CVAR(FullScreen_RefreshRateInHz, "v_RefreshRate", D3DPRESENT_RATE_DEFAULT);
	cvar=CVar_GetCVar(cvarlist, "v_EnableVSync");
	if(cvar)
		bCheck=(int)cvar->value;
	else
	{
		Err_Printf("Could not get desired vsync from cvarlist, defaulting.");
		bCheck=L_false;
	}

	if(bCheck)
	{
		pp->PresentationInterval=D3DPRESENT_INTERVAL_ONE;
	}
	else
		pp->PresentationInterval=D3DPRESENT_INTERVAL_IMMEDIATE;

	/* Now we do need to make some adjustment depending on some of the cvars. */
	/* If we are windowed we need to set certain presentation parameters. */

	if(pp->Windowed)
	{
		D3DDISPLAYMODE dmode;
		memset(&dmode, 0, sizeof(dmode));

		//pp->PresentationInterval=0;

		LV_CorrectWindowSize(
			lpGame->m_hwnd,
			pp->BackBufferWidth,
			pp->BackBufferHeight);

		lpGame->v.m_lpD3D->lpVtbl->GetAdapterDisplayMode(
			lpGame->v.m_lpD3D,
			lpGame->v.m_nAdapterID,
			&dmode);

		pp->BackBufferFormat=dmode.Format;
	}	

	if(!nQualityLevels)
	{
		pp->MultiSampleType=D3DMULTISAMPLE_NONE;
		pp->MultiSampleQuality=0;
	}
	else
	{
		pp->MultiSampleType=D3DMULTISAMPLE_NONMASKABLE;
		pp->MultiSampleQuality=nQualityLevels-1;
		nQualityLevels=0;
		/* Check if multisample type is available. */
		nResult=lpGame->v.m_lpD3D->lpVtbl->CheckDeviceMultiSampleType(
			lpGame->v.m_lpD3D,
			lpGame->v.m_nAdapterID,
			lpGame->v.m_nDeviceType,
			pp->BackBufferFormat,
			pp->Windowed,
			pp->MultiSampleType,
			&nQualityLevels);

		if(L_failed(nResult))
		{
			Err_Printf("FSAA mode not allowed, using default.");
			pp->MultiSampleType=D3DMULTISAMPLE_NONE;
			pp->MultiSampleQuality=0;
			CVar_Set(lpGame->m_cvars, "v_FSAAQuality", "0");
		}
		else
		{
			if(pp->MultiSampleQuality>=nQualityLevels)
			{
				pp->MultiSampleQuality=nQualityLevels-1;
			}
			//Err_Printf("FSAA Sample Quality set to: %i", pp->MultiSampleQuality+1);
		}

		if((pp->MultiSampleType>0) && (pp->SwapEffect!=D3DSWAPEFFECT_DISCARD))
		{
			Con_SendMessage(hConsole, "FSAA not allowed unless swap effect is DISCARD.");
			pp->MultiSampleType=D3DMULTISAMPLE_NONE;
			pp->MultiSampleQuality=0;
		}
		if((pp->MultiSampleType>0) && (L_CHECK_FLAG(dwFlags, D3DPRESENTFLAG_LOCKABLE_BACKBUFFER)))
		{
			Err_Printf("FSAA not allowed with Lockable Back Buffer.");
			pp->MultiSampleType=D3DMULTISAMPLE_NONE;
			pp->MultiSampleQuality=0;
		}
	}

	/* Check to see if the display format is supported. */
		/* Make sure the display mode is valid. */

	/* If we are windowed we need to set certain values. */
	if(pp->Windowed)
	{
		pp->FullScreen_RefreshRateInHz=0;
		lpGame->v.m_nBackBufferFormat=pp->BackBufferFormat;
	}
	
	/* Make sure the format is valid. */
	if(!pp->Windowed)
	{
		L_uint nCount=0;
		L_uint i=0;
		L_bool bDisplayAllowed=L_false;
		D3DDISPLAYMODE dmode;
		memset(&dmode, 0, sizeof(dmode));

		if(lpGame->v.m_lpD3D->lpVtbl->GetAdapterModeCount(
			lpGame->v.m_lpD3D, 
			lpGame->v.m_nAdapterID, 
			pp->BackBufferFormat)<1)
		{
			/* We are assuming that the device supports the
				R5G6B5 format, if not then IDirect3D::CreateDevice will
				fail and we can't do anything. */
			pp->BackBufferFormat=D3DFMT_R5G6B5;
			CVar_Set(lpGame->m_cvars, "v_BitDepth", "16");
			Err_Printf("Could not use selected bit depth, defaulting.");
		}
		nCount=lpGame->v.m_lpD3D->lpVtbl->GetAdapterModeCount(
			lpGame->v.m_lpD3D,
			lpGame->v.m_nAdapterID,
			pp->BackBufferFormat);

		if(nCount<1)
		{
			Err_Printf("Cannot find a suitable display mode.");
			return 0;
		}
		
		for(i=0; i<nCount; i++)
		{
			lpGame->v.m_lpD3D->lpVtbl->EnumAdapterModes(
				lpGame->v.m_lpD3D,
				lpGame->v.m_nAdapterID,
				pp->BackBufferFormat,
				i,
				&dmode);

			if((dmode.Width==pp->BackBufferWidth) && (dmode.Height==pp->BackBufferHeight))
			{
				/* The display mode is acceptable. */
				bDisplayAllowed=L_true;
				break;
			}
		}

		if(!bDisplayAllowed)
		{
			Err_Printf(
				"The display mode of %ix%i is not allowed, defaulting.", 
				pp->BackBufferWidth, 
				pp->BackBufferHeight);

			/* We are assuming 640x480 will work, if not, then
			IDirect3D9::CreateDevice will fail anyway. */
			pp->BackBufferWidth=640;
			pp->BackBufferHeight=480;

			CVar_Set(lpGame->m_cvars, "v_Width", "640");
			CVar_Set(lpGame->m_cvars, "v_Height", "480");
		}

		lpGame->v.m_nBackBufferFormat=pp->BackBufferFormat;
	}
	return 1;
}





/*****************************************************
	LV_SetStates()

	Goes through each of the sampler and render states
	that the game uses, and according the the cvars it
	sets them.  This function is called when a smapler
	or render state cvar changes, when LV_Init is
	called, and when LV_ValidateGraphics is called.
******************************************************/
L_bool LV_SetStates(L3DGame* lpGame)
{
	/* If there is no device, there is no point
		in setting any sampler states. */
	if(!lpGame->v.m_lpDevice)
		return L_false;

	/*********************** 
		Set the filter mode. 
	************************/
	LV_SetFilterMode(lpGame, (L_int)CVar_GetValue(lpGame->m_cvars, FILTER_MODE, L_null), 0);
	if(CVar_GetValue(lpGame->m_cvars, "v_DebugWireframe", L_null))
	{
		IDirect3DDevice9_SetRenderState(lpGame->v.m_lpDevice, D3DRS_FILLMODE, D3DFILL_WIREFRAME);
	}
	else
	{
		IDirect3DDevice9_SetRenderState(lpGame->v.m_lpDevice, D3DRS_FILLMODE, D3DFILL_SOLID);
	}
	

	return L_true;
}

/*********************************************************
	LV_SetFilterMode()

	Sets the texture filter mode for a particular texture
	stage.  Probably should impliment error checking to
	make sure the selected filter mode is valid on the
	device.  Called by LV_SetStates().
*********************************************************/
L_bool LV_SetFilterMode(L3DGame* lpGame, L_int nFilterMode, L_dword dwStage)
{
	switch(nFilterMode)
	{
	default:
	case FILTER_UNKNOWN:
		CVar_Set(lpGame->m_cvars, FILTER_MODE, "POINT");
		/* Fall through and set filter mode to point. */
	case FILTER_POINT: /* 1 is point filtering, see LG_RegisterCVars(). */
		IDirect3DDevice9_SetSamplerState(lpGame->v.m_lpDevice, dwStage, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
		IDirect3DDevice9_SetSamplerState(lpGame->v.m_lpDevice, dwStage, D3DSAMP_MINFILTER, D3DTEXF_POINT);
		IDirect3DDevice9_SetSamplerState(lpGame->v.m_lpDevice, dwStage, D3DSAMP_MIPFILTER, D3DTEXF_POINT);
		break;
	case FILTER_BILINEAR: /* 2 is bilinear filtering. */
		IDirect3DDevice9_SetSamplerState(lpGame->v.m_lpDevice, dwStage, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
		IDirect3DDevice9_SetSamplerState(lpGame->v.m_lpDevice, dwStage, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
		IDirect3DDevice9_SetSamplerState(lpGame->v.m_lpDevice, dwStage, D3DSAMP_MIPFILTER, D3DTEXF_POINT);
		break;
	case FILTER_TRILINEAR: /* 3 is trilinear filtering. */
		IDirect3DDevice9_SetSamplerState(lpGame->v.m_lpDevice, dwStage, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
		IDirect3DDevice9_SetSamplerState(lpGame->v.m_lpDevice, dwStage, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
		IDirect3DDevice9_SetSamplerState(lpGame->v.m_lpDevice, dwStage, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
		break;
	case FILTER_ANISOTROPIC: /* 4 is anisotropic filtering. */
	{
		L_dword nMaxAnisotropy=0;
		D3DCAPS9 d3dcaps;

		memset(&d3dcaps, 0, sizeof(D3DCAPS9));
		IDirect3DDevice9_GetDeviceCaps(lpGame->v.m_lpDevice, &d3dcaps);

		IDirect3DDevice9_SetSamplerState(lpGame->v.m_lpDevice, dwStage, D3DSAMP_MAGFILTER, D3DTEXF_ANISOTROPIC);
		IDirect3DDevice9_SetSamplerState(lpGame->v.m_lpDevice, dwStage, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC);
		IDirect3DDevice9_SetSamplerState(lpGame->v.m_lpDevice, dwStage, D3DSAMP_MIPFILTER, D3DTEXF_ANISOTROPIC);
		nMaxAnisotropy=(L_dword)CVar_GetValue(lpGame->m_cvars, FILTER_MAXANISOTROPY, L_null);

		if((nMaxAnisotropy<1) || (nMaxAnisotropy>d3dcaps.MaxAnisotropy))
		{
			nMaxAnisotropy=d3dcaps.MaxAnisotropy;
			CVar_SetValue(lpGame->m_cvars, FILTER_MAXANISOTROPY, (float)nMaxAnisotropy);
		}
		break;
	}
	}
	Err_Printf("Texture filter mode: %s", CVar_Get(lpGame->m_cvars, FILTER_MODE, L_null));
	
	return L_true;
}


L_dword LV_D3DFMTToBitDepth(D3DFORMAT fmt)
{
	#define FMT_TO_BITDEPTH(f, b) if(fmt==f)return b;
	FMT_TO_BITDEPTH(D3DFMT_UNKNOWN, 0)
	FMT_TO_BITDEPTH(D3DFMT_R8G8B8, 24)
	FMT_TO_BITDEPTH(D3DFMT_A8R8G8B8, 32)
	FMT_TO_BITDEPTH(D3DFMT_X8R8G8B8, 32)
	FMT_TO_BITDEPTH(D3DFMT_R5G6B5, 16)
	FMT_TO_BITDEPTH(D3DFMT_X1R5G5B5, 16)
	FMT_TO_BITDEPTH(D3DFMT_A1R5G5B5, 16)
	FMT_TO_BITDEPTH(D3DFMT_A4R4G4B4, 16)
	FMT_TO_BITDEPTH(D3DFMT_R3G3B2, 8)
	FMT_TO_BITDEPTH(D3DFMT_A8, 8)
	FMT_TO_BITDEPTH(D3DFMT_A8R3G3B2, 16)
	FMT_TO_BITDEPTH(D3DFMT_X4R4G4B4, 16)
	FMT_TO_BITDEPTH(D3DFMT_A2B10G10R10, 32)
	FMT_TO_BITDEPTH(D3DFMT_A8B8G8R8, 32)
	FMT_TO_BITDEPTH(D3DFMT_X8B8G8R8, 32)
	FMT_TO_BITDEPTH(D3DFMT_G16R16, 32)
	FMT_TO_BITDEPTH(D3DFMT_A2R10G10B10, 32)
	FMT_TO_BITDEPTH(D3DFMT_A16B16G16R16, 64)
	FMT_TO_BITDEPTH(D3DFMT_A8P8, 16)
	FMT_TO_BITDEPTH(D3DFMT_P8, 8)
	FMT_TO_BITDEPTH(D3DFMT_L8, 8)
	FMT_TO_BITDEPTH(D3DFMT_A8L8, 16)
	FMT_TO_BITDEPTH(D3DFMT_A4L4, 8)
	FMT_TO_BITDEPTH(D3DFMT_V8U8, 16)
	FMT_TO_BITDEPTH(D3DFMT_L6V5U5, 16)
	FMT_TO_BITDEPTH(D3DFMT_X8L8V8U8, 32)
	FMT_TO_BITDEPTH(D3DFMT_Q8W8V8U8, 32)
	FMT_TO_BITDEPTH(D3DFMT_V16U16, 32)
	FMT_TO_BITDEPTH(D3DFMT_A2W10V10U10, 32)
	FMT_TO_BITDEPTH(D3DFMT_UYVY, 0)
	FMT_TO_BITDEPTH(D3DFMT_R8G8_B8G8, 16)
	FMT_TO_BITDEPTH(D3DFMT_YUY2, 0)
	FMT_TO_BITDEPTH(D3DFMT_G8R8_G8B8, 16)
	FMT_TO_BITDEPTH(D3DFMT_DXT1, 32)
	FMT_TO_BITDEPTH(D3DFMT_DXT2, 32)
	FMT_TO_BITDEPTH(D3DFMT_DXT3, 32)
	FMT_TO_BITDEPTH(D3DFMT_DXT4, 32)
	FMT_TO_BITDEPTH(D3DFMT_DXT5, 32)
	FMT_TO_BITDEPTH(D3DFMT_D16_LOCKABLE, 16)
	FMT_TO_BITDEPTH(D3DFMT_D32, 32)
	FMT_TO_BITDEPTH(D3DFMT_D15S1, 16)
	FMT_TO_BITDEPTH(D3DFMT_D24S8, 32)
	FMT_TO_BITDEPTH(D3DFMT_D24X8, 32)
	FMT_TO_BITDEPTH(D3DFMT_D24X4S4, 32)
	FMT_TO_BITDEPTH(D3DFMT_D16, 16)
	FMT_TO_BITDEPTH(D3DFMT_D32F_LOCKABLE, 32)
	FMT_TO_BITDEPTH(D3DFMT_D24FS8, 32)
	FMT_TO_BITDEPTH(D3DFMT_L16, 16)
	FMT_TO_BITDEPTH(D3DFMT_VERTEXDATA, 0)
	FMT_TO_BITDEPTH(D3DFMT_INDEX16, 16)
	FMT_TO_BITDEPTH(D3DFMT_INDEX32, 32)
	FMT_TO_BITDEPTH(D3DFMT_Q16W16V16U16, 64)
	FMT_TO_BITDEPTH(D3DFMT_MULTI2_ARGB8, 32)
	FMT_TO_BITDEPTH(D3DFMT_R16F, 16)
	FMT_TO_BITDEPTH(D3DFMT_G16R16F, 32)
	FMT_TO_BITDEPTH(D3DFMT_A16B16G16R16F, 64)
	FMT_TO_BITDEPTH(D3DFMT_R32F, 32)
	FMT_TO_BITDEPTH(D3DFMT_G32R32F, 64)
	FMT_TO_BITDEPTH(D3DFMT_A32B32G32R32F, 128)
	FMT_TO_BITDEPTH(D3DFMT_CxV8U8, 16)
	return 0;
}



L_dword LV_StringToD3DFMT(char* szfmt)
{
	#define STRING_TO_FMT(fmt) if(L_strnicmp(szfmt, #fmt, 0))return fmt;
	STRING_TO_FMT(D3DFMT_UNKNOWN)
	STRING_TO_FMT(D3DFMT_R8G8B8)
	STRING_TO_FMT(D3DFMT_A8R8G8B8)
	STRING_TO_FMT(D3DFMT_X8R8G8B8)
	STRING_TO_FMT(D3DFMT_R5G6B5)
	STRING_TO_FMT(D3DFMT_X1R5G5B5)
	STRING_TO_FMT(D3DFMT_A1R5G5B5)
	STRING_TO_FMT(D3DFMT_A4R4G4B4)
	STRING_TO_FMT(D3DFMT_R3G3B2)
	STRING_TO_FMT(D3DFMT_A8)
	STRING_TO_FMT(D3DFMT_A8R3G3B2)
	STRING_TO_FMT(D3DFMT_X4R4G4B4)
	STRING_TO_FMT(D3DFMT_A2B10G10R10)
	STRING_TO_FMT(D3DFMT_A8B8G8R8)
	STRING_TO_FMT(D3DFMT_X8B8G8R8)
	STRING_TO_FMT(D3DFMT_G16R16)
	STRING_TO_FMT(D3DFMT_A2R10G10B10)
	STRING_TO_FMT(D3DFMT_A16B16G16R16)
	STRING_TO_FMT(D3DFMT_A8P8)
	STRING_TO_FMT(D3DFMT_P8)
	STRING_TO_FMT(D3DFMT_L8)
	STRING_TO_FMT(D3DFMT_A8L8)
	STRING_TO_FMT(D3DFMT_A4L4)
	STRING_TO_FMT(D3DFMT_V8U8)
	STRING_TO_FMT(D3DFMT_L6V5U5)
	STRING_TO_FMT(D3DFMT_X8L8V8U8)
	STRING_TO_FMT(D3DFMT_Q8W8V8U8)
	STRING_TO_FMT(D3DFMT_V16U16)
	STRING_TO_FMT(D3DFMT_A2W10V10U10)
	STRING_TO_FMT(D3DFMT_UYVY)
	STRING_TO_FMT(D3DFMT_R8G8_B8G8)
	STRING_TO_FMT(D3DFMT_YUY2)
	STRING_TO_FMT(D3DFMT_G8R8_G8B8)
	STRING_TO_FMT(D3DFMT_DXT1)
	STRING_TO_FMT(D3DFMT_DXT2)
	STRING_TO_FMT(D3DFMT_DXT3)
	STRING_TO_FMT(D3DFMT_DXT4)
	STRING_TO_FMT(D3DFMT_DXT5)
	STRING_TO_FMT(D3DFMT_D16_LOCKABLE)
	STRING_TO_FMT(D3DFMT_D32)
	STRING_TO_FMT(D3DFMT_D15S1)
	STRING_TO_FMT(D3DFMT_D24S8)
	STRING_TO_FMT(D3DFMT_D24X8)
	STRING_TO_FMT(D3DFMT_D24X4S4)
	STRING_TO_FMT(D3DFMT_D16)
	STRING_TO_FMT(D3DFMT_D32F_LOCKABLE)
	STRING_TO_FMT(D3DFMT_D24FS8)
	STRING_TO_FMT(D3DFMT_L16)
	STRING_TO_FMT(D3DFMT_VERTEXDATA)
	STRING_TO_FMT(D3DFMT_INDEX16)
	STRING_TO_FMT(D3DFMT_INDEX32)
	STRING_TO_FMT(D3DFMT_Q16W16V16U16)
	STRING_TO_FMT(D3DFMT_MULTI2_ARGB8)
	STRING_TO_FMT(D3DFMT_R16F)
	STRING_TO_FMT(D3DFMT_G16R16F)
	STRING_TO_FMT(D3DFMT_A16B16G16R16F)
	STRING_TO_FMT(D3DFMT_R32F)
	STRING_TO_FMT(D3DFMT_G32R32F)
	STRING_TO_FMT(D3DFMT_A32B32G32R32F)
	STRING_TO_FMT(D3DFMT_CxV8U8)
	STRING_TO_FMT(D3DFMT_FORCE_DWORD)
	return D3DFMT_UNKNOWN;
}
char*  LV_D3DFMTToString(D3DFORMAT fmt)
{
	#define FMT_TO_STRING(f) if(f==fmt)return #f;
	FMT_TO_STRING(D3DFMT_UNKNOWN)
	FMT_TO_STRING(D3DFMT_R8G8B8)
	FMT_TO_STRING(D3DFMT_A8R8G8B8)
	FMT_TO_STRING(D3DFMT_X8R8G8B8)
	FMT_TO_STRING(D3DFMT_R5G6B5)
	FMT_TO_STRING(D3DFMT_X1R5G5B5)
	FMT_TO_STRING(D3DFMT_A1R5G5B5)
	FMT_TO_STRING(D3DFMT_A4R4G4B4)
	FMT_TO_STRING(D3DFMT_R3G3B2)
	FMT_TO_STRING(D3DFMT_A8)
	FMT_TO_STRING(D3DFMT_A8R3G3B2)
	FMT_TO_STRING(D3DFMT_X4R4G4B4)
	FMT_TO_STRING(D3DFMT_A2B10G10R10)
	FMT_TO_STRING(D3DFMT_A8B8G8R8)
	FMT_TO_STRING(D3DFMT_X8B8G8R8)
	FMT_TO_STRING(D3DFMT_G16R16)
	FMT_TO_STRING(D3DFMT_A2R10G10B10)
	FMT_TO_STRING(D3DFMT_A16B16G16R16)
	FMT_TO_STRING(D3DFMT_A8P8)
	FMT_TO_STRING(D3DFMT_P8)
	FMT_TO_STRING(D3DFMT_L8)
	FMT_TO_STRING(D3DFMT_A8L8)
	FMT_TO_STRING(D3DFMT_A4L4)
	FMT_TO_STRING(D3DFMT_V8U8)
	FMT_TO_STRING(D3DFMT_L6V5U5)
	FMT_TO_STRING(D3DFMT_X8L8V8U8)
	FMT_TO_STRING(D3DFMT_Q8W8V8U8)
	FMT_TO_STRING(D3DFMT_V16U16)
	FMT_TO_STRING(D3DFMT_A2W10V10U10)
	FMT_TO_STRING(D3DFMT_UYVY)
	FMT_TO_STRING(D3DFMT_R8G8_B8G8)
	FMT_TO_STRING(D3DFMT_YUY2)
	FMT_TO_STRING(D3DFMT_G8R8_G8B8)
	FMT_TO_STRING(D3DFMT_DXT1)
	FMT_TO_STRING(D3DFMT_DXT2)
	FMT_TO_STRING(D3DFMT_DXT3)
	FMT_TO_STRING(D3DFMT_DXT4)
	FMT_TO_STRING(D3DFMT_DXT5)
	FMT_TO_STRING(D3DFMT_D16_LOCKABLE)
	FMT_TO_STRING(D3DFMT_D32)
	FMT_TO_STRING(D3DFMT_D15S1)
	FMT_TO_STRING(D3DFMT_D24S8)
	FMT_TO_STRING(D3DFMT_D24X8)
	FMT_TO_STRING(D3DFMT_D24X4S4)
	FMT_TO_STRING(D3DFMT_D16)
	FMT_TO_STRING(D3DFMT_D32F_LOCKABLE)
	FMT_TO_STRING(D3DFMT_D24FS8)
	FMT_TO_STRING(D3DFMT_L16)
	FMT_TO_STRING(D3DFMT_VERTEXDATA)
	FMT_TO_STRING(D3DFMT_INDEX16)
	FMT_TO_STRING(D3DFMT_INDEX32)
	FMT_TO_STRING(D3DFMT_Q16W16V16U16)
	FMT_TO_STRING(D3DFMT_MULTI2_ARGB8)
	FMT_TO_STRING(D3DFMT_R16F)
	FMT_TO_STRING(D3DFMT_G16R16F)
	FMT_TO_STRING(D3DFMT_A16B16G16R16F)
	FMT_TO_STRING(D3DFMT_R32F)
	FMT_TO_STRING(D3DFMT_G32R32F)
	FMT_TO_STRING(D3DFMT_A32B32G32R32F)
	FMT_TO_STRING(D3DFMT_CxV8U8)
	FMT_TO_STRING(D3DFMT_FORCE_DWORD)
	return "D3DFMT_UNKNOWN";
}
