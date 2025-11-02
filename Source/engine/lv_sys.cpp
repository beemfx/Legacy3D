//lv_sys.cpp Copyright (c) 2007 Blaine Myers
//This is the legacy video system, the methods here
//are used to initialize CElementD3D elements for use
//throughout the game.

#include "lv_sys.h"
#include "lg_err.h"
#include "lg_cvars.h"
#include "lg_func.h"

#include "../lc_sys2/lc_sys2.h"

CLVideo::CLVideo()//:
	//s_cvars(LG_NULL)
{

}

CLVideo::~CLVideo()
{
}


lg_bool CLVideo::SetStates()
{
	/* If there is no device, there is no point
		in setting any sampler states. */
	if(!s_pDevice)
		return LG_FALSE;

	/*********************** 
		Set the filter mode. 
	************************/
	SetFilterMode((lg_int)CV_Get(CVAR_v_TextureFilter)->nValue, 0);
	
	if(CV_Get(CVAR_v_DebugWireframe)->nValue)
	{
		IDirect3DDevice9_SetRenderState(s_pDevice, D3DRS_FILLMODE, D3DFILL_WIREFRAME);
	}
	else
	{
		IDirect3DDevice9_SetRenderState(s_pDevice, D3DRS_FILLMODE, D3DFILL_SOLID);
	}
	return LG_TRUE;
}

lg_bool CLVideo::SetFilterMode(lg_int nFilterMode, lg_dword dwStage)
{
	switch(nFilterMode)
	{
	default:
	case FILTER_UNKNOWN:
		CV_Set(CV_Get(CVAR_v_TextureFilter), DEF_POINT);
		/* Fall through and set filter mode to point. */
	case FILTER_POINT: /* 1 is point filtering, see LG_RegisterCVars(). */
		IDirect3DDevice9_SetSamplerState(s_pDevice, dwStage, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
		IDirect3DDevice9_SetSamplerState(s_pDevice, dwStage, D3DSAMP_MINFILTER, D3DTEXF_POINT);
		IDirect3DDevice9_SetSamplerState(s_pDevice, dwStage, D3DSAMP_MIPFILTER, D3DTEXF_POINT);
		break;
	case FILTER_BILINEAR: /* 2 is bilinear filtering. */
		IDirect3DDevice9_SetSamplerState(s_pDevice, dwStage, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
		IDirect3DDevice9_SetSamplerState(s_pDevice, dwStage, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
		IDirect3DDevice9_SetSamplerState(s_pDevice, dwStage, D3DSAMP_MIPFILTER, D3DTEXF_POINT);
		break;
	case FILTER_TRILINEAR: /* 3 is trilinear filtering. */
		IDirect3DDevice9_SetSamplerState(s_pDevice, dwStage, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
		IDirect3DDevice9_SetSamplerState(s_pDevice, dwStage, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
		IDirect3DDevice9_SetSamplerState(s_pDevice, dwStage, D3DSAMP_MIPFILTER, D3DTEXF_LINEAR);
		break;
	case FILTER_ANISOTROPIC: /* 4 is anisotropic filtering. */
	{
		lg_dword nMaxAnisotropy=0;
		D3DCAPS9 d3dcaps;

		memset(&d3dcaps, 0, sizeof(D3DCAPS9));
		IDirect3DDevice9_GetDeviceCaps(s_pDevice, &d3dcaps);

		IDirect3DDevice9_SetSamplerState(s_pDevice, dwStage, D3DSAMP_MAGFILTER, D3DTEXF_ANISOTROPIC);
		IDirect3DDevice9_SetSamplerState(s_pDevice, dwStage, D3DSAMP_MINFILTER, D3DTEXF_ANISOTROPIC);
		IDirect3DDevice9_SetSamplerState(s_pDevice, dwStage, D3DSAMP_MIPFILTER, D3DTEXF_ANISOTROPIC);
		nMaxAnisotropy=(lg_dword)CV_Get(CVAR_v_MaxAnisotropy)->nValue;

		if((nMaxAnisotropy<1) || (nMaxAnisotropy>d3dcaps.MaxAnisotropy))
		{
			nMaxAnisotropy=d3dcaps.MaxAnisotropy;
			CV_Set_l(CV_Get(CVAR_v_MaxAnisotropy), nMaxAnisotropy);
		}
		break;
	}
	}
	Err_Printf("Texture filter mode: %s", CV_Get(CVAR_v_TextureFilter)->szValue);
	
	return LG_TRUE;
}

/******************************************************************* 
	LV_Restart()

	Resets the video with the present parameters located in the cvar
	list.  This is called from LV_ValidateDevice() when the device
	gets reset, or when the user sends the VRESTART command to the
	console.
********************************************************************/

lg_bool CLVideo::Restart()
{
	//All POOL_DEFAULT Direct3D items should be released
	//before calling Restart...
	D3DPRESENT_PARAMETERS pp;

	lg_bool bBackSurface=LG_FALSE;
	lg_result nResult=0;
	/* Reset the device. */

	/* If there was a back surface invalidate it. */
	if(s_pBackSurface)
	{
		s_pBackSurface->Release();
		s_pBackSurface=LG_NULL;
		bBackSurface=LG_TRUE;
	}

	/* Reset the device. */
	memset(&pp, 0, sizeof(pp));
	SetPPFromCVars(&pp);

	/* Call reset. */
	nResult=s_pDevice->Reset(&pp);
	Err_PrintDX("IDirect3DDevice9::Reset", nResult);
	if(LG_FAILED(nResult))
	{
		return LG_FALSE;
	}
	else
	{
		Err_Printf("Reset Device With the following Present Parameters:");
		PrintPP(&pp);
		
		if(bBackSurface)
		{
			nResult=s_pDevice->GetBackBuffer(
				0,
				0,
				D3DBACKBUFFER_TYPE_MONO,
				&(s_pBackSurface));
			Err_PrintDX("IDirect3DDevice9::GetBackBuffer", nResult);
		}
	}
	s_nDisplayWidth=pp.BackBufferWidth;
	s_nDisplayHeight=pp.BackBufferHeight;
	return LG_TRUE;
}	


//Direct3D Display Initialization:
lg_bool CLVideo::Init(HWND hwnd)
{
	m_hwnd=hwnd;
	
	if(s_pD3D)
	{
		Err_Printf("Video Init WARNING: IDirect3D interface exists at 0x%08X.", s_pD3D);
		Err_Printf("   Direct3D may have already been initialized, exiting LV_Init()...");
		return LG_FALSE;
	}
	D3DPRESENT_PARAMETERS pp;
	
	lg_dword dwBehaviorFlags=0;
	lg_dword dwVertexProcFlag=0;
	lg_result nResult=0x00000000l;

	/* To initialize the video we need to create an IDirect3D interface, set the present 
	parameters, then create the device. */
	
	/* First create the IDirect3D interface. */
	Err_Printf("Creating IDirect3D9 interface...");
	s_pD3D=Direct3DCreate9(D3D_SDK_VERSION);
	if(s_pD3D)
	{
		Err_Printf("IDirect3D9 interface created at 0x%08X.", s_pD3D);
	}
	else
	{
		Err_Printf("Failed to create IDirect3D9 interface.");
		MessageBox(
			m_hwnd,
			"This application requires ""DirectX 9.0c", "Legacy",
			MB_OK|MB_ICONINFORMATION);
		return LG_FALSE;
	}

	/* Get the Direct3D setup parameters from the cvarlist. */
	s_nAdapterID=(UINT)CV_Get(CVAR_v_AdapterID)->nValue;
	s_nDeviceType=(D3DDEVTYPE)(lg_int)CV_Get(CVAR_v_DeviceType)->nValue;
	dwVertexProcFlag=(lg_dword)CV_Get(CVAR_v_VertexProc)->nValue;
	
	dwBehaviorFlags=0;
	if(CV_Get(CVAR_v_FPU_Preserve)->nValue)
		dwBehaviorFlags|=D3DCREATE_FPU_PRESERVE;
		
	if(CV_Get(CVAR_v_MultiThread)->nValue)
		dwBehaviorFlags|=D3DCREATE_MULTITHREADED;
	
	if(CV_Get(CVAR_v_PureDevice)->nValue)
		dwBehaviorFlags|=D3DCREATE_PUREDEVICE;
		
	if(CV_Get(CVAR_v_DisableDriverManagement)->nValue)
		dwBehaviorFlags|=D3DCREATE_DISABLE_DRIVER_MANAGEMENT;
	
		
	if(CV_Get(CVAR_v_AdapterGroupDevice)->nValue)
		dwBehaviorFlags|=D3DCREATE_ADAPTERGROUP_DEVICE;
			
	//dwBehaviorFlags=0;
	Err_Printf("Flags 0x%08X", dwBehaviorFlags);

	/* Make sure the adapter is valid. */
	if(s_nAdapterID >= s_pD3D->GetAdapterCount())
	{
		Err_Printf( "Adapter %i not available, using default adapter.", s_nAdapterID);
		CV_Set(CV_Get(CVAR_v_AdapterID), DEF_ADAPTER_DEFAULT);
		s_nAdapterID=0;
	}

	/* Show the adapter identifier information. */
	D3DADAPTER_IDENTIFIER9 adi;
	memset(&adi, 0, sizeof(adi));
	s_pD3D->GetAdapterIdentifier(s_nAdapterID,0,&adi);
	Err_Printf("Using \"%s\".", adi.Description);

	/* We set the present parameters using the cvars. */
	memset(&pp, 0, sizeof(pp));
	Err_Printf("Initialzing present parameters from cvarlist...");
	if(!SetPPFromCVars(&pp))
	{
		Err_Printf("Could not initialize present parameters.");
		LG_SafeRelease(s_pD3D);
		return LG_FALSE;
	}
	Err_Printf("Present parameters initialization complete.");

	/* Check to see that the adapter type is available. */
	
	nResult=s_pD3D->CheckDeviceType(
		s_nAdapterID,
		s_nDeviceType,
		pp.BackBufferFormat,
		pp.BackBufferFormat,
		pp.Windowed);
		
	if(LG_FAILED(nResult))
	{
		Err_PrintDX("CheckDeviceType", nResult);
		Err_Printf("Cannot use selected device type, defaulting.");
		/* If the HAL device can't be used then CreateDevice will fail. */
		s_nDeviceType=D3DDEVTYPE_HAL;
	}

	Err_Printf("Creating IDirect3DDevice9 interface...");
	nResult=s_pD3D->CreateDevice(
		s_nAdapterID,
		s_nDeviceType,
		m_hwnd,
		dwBehaviorFlags|dwVertexProcFlag,
		&pp,
		&s_pDevice);
		
	Err_PrintDX("IDirect3D9::CreateDevice", nResult);
	/* If we failed to create the device, we can try a some
		different vertex processing modes, if it still fails
		we exit. */
	while(LG_FAILED(nResult))
	{
		if(dwVertexProcFlag==D3DCREATE_SOFTWARE_VERTEXPROCESSING)
			break;
		else if(dwVertexProcFlag==D3DCREATE_HARDWARE_VERTEXPROCESSING)
		{
			dwVertexProcFlag=D3DCREATE_MIXED_VERTEXPROCESSING;
			CV_Set(CV_Get(CVAR_v_VertexProc), DEF_MIXEDVP);
		}
		else
		{
			dwVertexProcFlag=D3DCREATE_SOFTWARE_VERTEXPROCESSING;
			CV_Set(CV_Get(CVAR_v_VertexProc), DEF_SWVP);
		}
		
		nResult=s_pD3D->CreateDevice(
			s_nAdapterID,
			s_nDeviceType,
			m_hwnd,
			dwBehaviorFlags|dwVertexProcFlag,
			&pp,
			&s_pDevice);
		
		Err_PrintDX("IDirect3D9::CreateDevice", nResult);
	}
	

	if(LG_SUCCEEDED(nResult))
	{
		Err_Printf("IDirect3DDevice9 interface created at 0x%08X", s_pDevice);
		Err_Printf("Using the following Present Parameters:");
		PrintPP(&pp);
	}
	else
	{
		Err_Printf("Failed to create IDirect3DDevice9 interface.", nResult);
		Err_Printf("Try software vertex processing.");

		LG_SafeRelease(s_pD3D);;
		return LG_FALSE;
	}
	s_nDisplayWidth=pp.BackBufferWidth;
	s_nDisplayHeight=pp.BackBufferHeight;

	Err_Printf("Calling SetStates...");
	if(!SetStates())
		Err_Printf("An error occured while setting the sampler and render states.");
	
	/* Call present to change the resolution.*/
	/* Could load up a loading screen here!.*/
	IDirect3DDevice9_Clear(s_pDevice, 0,0,D3DCLEAR_TARGET|D3DCLEAR_ZBUFFER,0xFF5050FF,1.0f,0);
	IDirect3DDevice9_Present(s_pDevice, NULL, NULL, NULL, NULL);

	/* Display allowed formats, and also set a few cvars that control them. 
	The allowed formats function only deals with the basic formats, not
	any advanced or compressed texture formats.*/	
	//SupportedTexFormats();
	#if 0
	LV_InitTexLoad();
	#endif
	
	return LG_TRUE;
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

void CLVideo::Shutdown()
{
	lg_ulong nNumLeft=0;

	Err_Printf("Releasing IDirect3DDevice9 interface...");
	if(s_pDevice)
	{
		nNumLeft=s_pDevice->Release();
		Err_Printf( 
			"Released IDirect3DDevice9 interface at 0x%08X with %i references left.", 
			s_pDevice, 
			nNumLeft);
		s_pDevice=LG_NULL;
	}
	Err_Printf("Releasing IDirect3D9 interface...");
	if(s_pD3D)
	{
		nNumLeft=s_pD3D->Release();
		Err_Printf( 
			"Released IDirect3D9 interface at 0x%08X with %i references left.", 
			s_pD3D, 
			nNumLeft);
			
		s_pD3D=LG_NULL;
	}
}

#if 0
lg_bool CLVideo::SupportedTexFormats()
{
	lg_result nResult=0;
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
	#define TEX_CHECK(fmt) IDirect3D9_CheckDeviceFormat(s_pD3D,s_nAdapterID,s_nDeviceType,s_nBackBufferFormat,0,D3DRTYPE_TEXTURE, fmt);
	nResult=TEX_CHECK(D3DFMT_A8R8G8B8);
	if(LG_FAILED(nResult))CV_Set(CV_Get(CVAR_tm_32BitTextureAlpha), DEF_FALSE);
	else Err_Printf("   %s", CLVideo::D3DFMTToString(D3DFMT_A8R8G8B8));

	nResult=TEX_CHECK(D3DFMT_X8R8G8B8)
	if(LG_FAILED(nResult))CV_Set(CV_Get(CVAR_tm_Force16BitTextures), DEF_TRUE);
	else Err_Printf("   %s", CLVideo::D3DFMTToString(D3DFMT_X8R8G8B8));

	nResult=TEX_CHECK(D3DFMT_A1R5G5B5)
	if(LG_FAILED(nResult))CV_Set(CV_Get(CVAR_tm_16BitTextureAlpha), DEF_FALSE);
	else Err_Printf("   %s", CLVideo::D3DFMTToString(D3DFMT_A1R5G5B5));

	nResult=TEX_CHECK(D3DFMT_X1R5G5B5)
	if(LG_SUCCEEDED(nResult))Err_Printf("   %s", CLVideo::D3DFMTToString(D3DFMT_X1R5G5B5));

	nResult=TEX_CHECK(D3DFMT_R5G6B5)
	if(LG_SUCCEEDED(nResult))Err_Printf("   %s", CLVideo::D3DFMTToString(D3DFMT_R5G6B5));

	return LG_TRUE;
}
#endif

lg_bool CLVideo::SetPPFromCVars(D3DPRESENT_PARAMETERS* pp)
{
	//CVar* cvar=NULL;
	DWORD dwFlags=0;
	lg_long nBackBufferFmt=0;
	lg_dword nBitDepth=0;
	lg_result nResult=0;
	lg_dword nQualityLevels=0;

	if(!pp)
		return LG_FALSE;

	/* This function will attemp to get the appropriate values for the present
		parameters from the cvars, if it can't get the cvar it will SET_PP_FROM_CVAR an 
		appropriate default value, that should work on most systems. */		
	#define SET_PP_FROM_CVAR(prpr, cvarname, defvalue, type) {cvar=CV_Get(cvarname);\
		pp->prpr=cvar?(type)cvar->nValue:defvalue;}

	lg_cvar* cvar;
	SET_PP_FROM_CVAR(BackBufferWidth, CVAR_v_Width, 640, UINT);
	SET_PP_FROM_CVAR(BackBufferHeight, CVAR_v_Height, 480, UINT);
	/* Create the back buffer format. */

	cvar=CV_Get(CVAR_v_BitDepth);
	if(cvar)nBitDepth=(int)cvar->nValue;
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
		CV_Set_l(CV_Get(CVAR_v_BitDepth), 16);
	}

	SET_PP_FROM_CVAR(BackBufferCount, CVAR_v_ScreenBuffers, 1, UINT);
	SET_PP_FROM_CVAR(SwapEffect, CVAR_v_SwapEffect, D3DSWAPEFFECT_DISCARD, D3DSWAPEFFECT);
	pp->hDeviceWindow=m_hwnd;
	nQualityLevels=(int)CV_Get(CVAR_v_FSAAQuality)->nValue;
	SET_PP_FROM_CVAR(Windowed, CVAR_v_Windowed, LG_FALSE, BOOL);
	SET_PP_FROM_CVAR(EnableAutoDepthStencil, CVAR_v_EnableAutoDepthStencil, TRUE, BOOL);
	SET_PP_FROM_CVAR(AutoDepthStencilFormat, CVAR_v_AutoDepthStencilFormat, D3DFMT_D16, D3DFORMAT);
	
	/* Set the flags. */
	cvar=CV_Get(CVAR_v_LockableBackBuffer);
	if(cvar)
		if(cvar->nValue)
			dwFlags|=D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;
	cvar=CV_Get(CVAR_v_DiscardDepthStencil);
	if(cvar)
		if(cvar->nValue)
			dwFlags|=D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL;
	cvar=CV_Get(CVAR_v_DeviceClip);
	if(cvar)
		if(cvar->nValue)
			dwFlags|=D3DPRESENTFLAG_DEVICECLIP;
	cvar=CV_Get(CVAR_v_VideoHint);
	if(cvar)
		if(cvar->nValue)
			dwFlags|=D3DPRESENTFLAG_VIDEO;
	pp->Flags=dwFlags;

	SET_PP_FROM_CVAR(FullScreen_RefreshRateInHz, CVAR_v_RefreshRate, D3DPRESENT_RATE_DEFAULT, UINT);
	pp->PresentationInterval =
		CV_Get(CVAR_v_EnableVSync)->nValue?
			D3DPRESENT_INTERVAL_ONE:
			D3DPRESENT_INTERVAL_IMMEDIATE;

#if _DEBUG
	pp->Windowed = LG_TRUE;
#endif

	/* Now we do need to make some adjustment depending on some of the cvars. */
	/* If we are windowed we need to set certain presentation parameters. */
	if(pp->Windowed)
	{
		D3DDISPLAYMODE dmode;
		memset(&dmode, 0, sizeof(dmode));

		//pp->PresentationInterval=0;

		CorrectWindowSize(
			m_hwnd,
			pp->BackBufferWidth,
			pp->BackBufferHeight);

		s_pD3D->GetAdapterDisplayMode(
			s_nAdapterID,
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
		s_pD3D->CheckDeviceMultiSampleType(
			s_nAdapterID,
			s_nDeviceType,
			pp->BackBufferFormat,
			pp->Windowed,
			pp->MultiSampleType,
			&nQualityLevels);

		if(LG_FAILED(nResult))
		{
			Err_Printf("FSAA mode not allowed, using default.");
			pp->MultiSampleType=D3DMULTISAMPLE_NONE;
			pp->MultiSampleQuality=0;
			CV_Set_l(CV_Get(CVAR_v_FSAAQuality), 0);
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
			Err_Printf("FSAA not allowed unless swap effect is DISCARD.");
			pp->MultiSampleType=D3DMULTISAMPLE_NONE;
			pp->MultiSampleQuality=0;
		}
		if((pp->MultiSampleType>0) && (LG_CheckFlag(dwFlags, D3DPRESENTFLAG_LOCKABLE_BACKBUFFER)))
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
		s_nBackBufferFormat=pp->BackBufferFormat;
	}
	
	/* Make sure the format is valid. */
	if(!pp->Windowed)
	{
		lg_uint nCount=0;
		lg_uint i=0;
		lg_bool bDisplayAllowed=LG_FALSE;
		D3DDISPLAYMODE dmode;
		memset(&dmode, 0, sizeof(dmode));

		if(s_pD3D->GetAdapterModeCount(
			s_nAdapterID,
			pp->BackBufferFormat)<1)
		{
			/* We are assuming that the device supports the
				R5G6B5 format, if not then IDirect3D::CreateDevice will
				fail and we can't do anything. */
			pp->BackBufferFormat=D3DFMT_R5G6B5;
			CV_Set_l(CV_Get(CVAR_v_BitDepth), 16);
			Err_Printf("Could not use selected bit depth, defaulting.");
		}
		nCount=s_pD3D->GetAdapterModeCount(
			s_nAdapterID,
			pp->BackBufferFormat);
		if(nCount<1)
		{
			Err_Printf("Cannot find a suitable display mode.");
			return 0;
		}
		
		for(i=0, bDisplayAllowed=LG_FALSE; i<nCount; i++)
		{
			s_pD3D->EnumAdapterModes(
				s_nAdapterID,
				pp->BackBufferFormat,
				i,
				&dmode);
				
			if((dmode.Width==pp->BackBufferWidth) && (dmode.Height==pp->BackBufferHeight))
			{
				/* The display mode is acceptable. */
				bDisplayAllowed=LG_TRUE;
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

			CV_Set_l(CV_Get(CVAR_v_Width), 640);
			CV_Set_l(CV_Get(CVAR_v_Height), 480);
		}

		s_nBackBufferFormat=pp->BackBufferFormat;
	}
	return LG_TRUE;
}

void CLVideo::PrintPP(D3DPRESENT_PARAMETERS* pp)
{
	Err_Printf("   Resolution: %iX%iX%i(%s) at %iHz, %s", 
		pp->BackBufferWidth, 
		pp->BackBufferHeight, 
		D3DFMTToBitDepth(pp->BackBufferFormat), 
		D3DFMTToString(pp->BackBufferFormat),
		pp->FullScreen_RefreshRateInHz,
		pp->Windowed?"WINDOWED":"FULL SCREEN");
	Err_Printf("   FSAA %s (%i)", pp->MultiSampleType==D3DMULTISAMPLE_NONE?"Disabled":"Enabled", pp->MultiSampleQuality);
	Err_Printf("   Back Buffers: %i, Swap Effect: %s", 
		pp->BackBufferCount, 
		pp->SwapEffect==D3DSWAPEFFECT_DISCARD?"DISCARD":pp->SwapEffect==D3DSWAPEFFECT_FLIP?"FLIP":"COPY");
	Err_Printf("   Flags: %s%s%s%s",
		LG_CheckFlag(pp->Flags, D3DPRESENTFLAG_DEVICECLIP)?"DEVICECLIP ":"",
		LG_CheckFlag(pp->Flags, D3DPRESENTFLAG_DISCARD_DEPTHSTENCIL)?"DISCARD_DEPTHSTRENCIL ":"",
		LG_CheckFlag(pp->Flags, D3DPRESENTFLAG_LOCKABLE_BACKBUFFER)?"LOCKABLE_BACKBUFFER ":"",
		LG_CheckFlag(pp->Flags, D3DPRESENTFLAG_VIDEO)?"VIDEO ":"");
	Err_Printf("   Device Window At 0x%08X", pp->hDeviceWindow);
	Err_Printf("   Auto Depth Stencil %s (%s)", pp->EnableAutoDepthStencil?"Enabled":"Disabled", D3DFMTToString(pp->AutoDepthStencilFormat));
	if(pp->PresentationInterval==0x80000000)
		Err_Printf("   Presentation Interval: IMMEDIATE");
	else
		Err_Printf("   Presentation Interval: %d", pp->PresentationInterval);
}


lg_dword CLVideo::D3DFMTToBitDepth(D3DFORMAT fmt)
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



lg_dword CLVideo::StringToD3DFMT(lg_cstr szfmt)
{
	#define STRING_TO_FMT(fmt) if(strnicmp((char*)szfmt, #fmt, 0)==0)return fmt;
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
lg_cstr CLVideo::D3DFMTToString(D3DFORMAT fmt)
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


#include <windowsx.h>

lg_bool CLVideo::CorrectWindowSize(
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
	
	return LG_TRUE;
}
