/* 
	lg_cvars.h - The cvars used in the legacy game.
	This file is where all the cvars used in the game
	should be declared, though they are registered in
	the CLGame::LG_RegisterCvars method (lg_sys_init.h).
	
	All cvars and defintions should be declared here.
	the DECLARE_DEF/CVAR macros will create statements
	similar to the following:
	const char DEF/CVAR_NAME = "NAME"
	
	From that point on the cvar can be refered to by
	CVAR_NAME so it isn't necessary to enter the string
	every time and hence it is unlikely to make a mistake
	by putting the wrong string in the code and not knowing
	why it isn't working right.
*/
#ifndef __LG_CVARS_H__
#define __LG_CVARS_H__

#include "../lc_sys2/lc_sys2.h"

//Definition declarations.
#ifdef DECLARING_CVARS
#define DECLARE_DEF(name) extern const char DEF_##name[] = #name
#define DECLARE_CVAR(name) extern const char CVAR_##name[] = #name
#else DECLARING_CVARS
#define DECLARE_DEF(name) extern const char DEF_##name[]
#define DECLARE_CVAR(name) extern const char CVAR_##name[]
#endif DECLARING_CVARS

//Boolean definitions
DECLARE_DEF(TRUE);
DECLARE_DEF(FALSE);
//Adapter ID definitions
DECLARE_DEF(ADAPTER_DEFAULT);
//Defice type definitions
DECLARE_DEF(HAL);
DECLARE_DEF(REF);
DECLARE_DEF(SW);
//Vertex processing definitions
DECLARE_DEF(SWVP);
DECLARE_DEF(HWVP);
DECLARE_DEF(MIXEDVP);
//Texture format declarations
//Back Buffer formats
DECLARE_DEF(FMT_UNKNOWN);
DECLARE_DEF(FMT_A8R8G8B8);
DECLARE_DEF(FMT_X8R8G8B8);
DECLARE_DEF(FMT_R5G6B5);
DECLARE_DEF(FMT_X1R5G5B5);
DECLARE_DEF(FMT_A1R5G5B5);
DECLARE_DEF(FMT_A2R10G10B10);
//Depth stencil formats
DECLARE_DEF(FMT_D16_LOCKABLE);
DECLARE_DEF(FMT_D32);
DECLARE_DEF(FMT_D15S1);
DECLARE_DEF(FMT_D24S8);
DECLARE_DEF(FMT_D24X8);
DECLARE_DEF(FMT_D24X4S4);
DECLARE_DEF(FMT_D16);
DECLARE_DEF(FMT_D32F_LOCKABLE);
DECLARE_DEF(FMT_D24FS8);
//Multi sample types
/*
DECLARE_DEF(D3DMULTISAMPLE_NONE);
DECLARE_DEF(D3DMULTISAMPLE_NONMASKABLE);
DECLARE_DEF(D3DMULTISAMPLE_2_SAMPLES);
DECLARE_DEF(D3DMULTISAMPLE_3_SAMPLES);
DECLARE_DEF(D3DMULTISAMPLE_4_SAMPLES);
DECLARE_DEF(D3DMULTISAMPLE_5_SAMPLES);
DECLARE_DEF(D3DMULTISAMPLE_6_SAMPLES);
DECLARE_DEF(D3DMULTISAMPLE_7_SAMPLES);
DECLARE_DEF(D3DMULTISAMPLE_8_SAMPLES);
DECLARE_DEF(D3DMULTISAMPLE_9_SAMPLES);
DECLARE_DEF(D3DMULTISAMPLE_10_SAMPLES);
DECLARE_DEF(D3DMULTISAMPLE_11_SAMPLES);
DECLARE_DEF(D3DMULTISAMPLE_12_SAMPLES);
DECLARE_DEF(D3DMULTISAMPLE_13_SAMPLES);
DECLARE_DEF(D3DMULTISAMPLE_14_SAMPLES);
DECLARE_DEF(D3DMULTISAMPLE_15_SAMPLES);
DECLARE_DEF(D3DMULTISAMPLE_16_SAMPLES);
*/
//Swap effect
DECLARE_DEF(SWAP_DISCARD);
DECLARE_DEF(SWAP_FLIP);
DECLARE_DEF(SWAP_COPY);
//Presentation intervals
/*
DECLARE_DEF(D3DPRESENT_INTERVAL_DEFAULT);
DECLARE_DEF(D3DPRESENT_INTERVAL_ONE);
DECLARE_DEF(D3DPRESENT_INTERVAL_TWO);
DECLARE_DEF(D3DPRESENT_INTERVAL_THREE);
DECLARE_DEF(D3DPRESENT_INTERVAL_FOUR);
DECLARE_DEF(D3DPRESENT_INTERVAL_IMMEDIATE);
*/
//Filter types
DECLARE_DEF(POINT);
DECLARE_DEF(LINEAR);
DECLARE_DEF(BILINEAR);
DECLARE_DEF(TRILINEAR);
DECLARE_DEF(ANISOTROPIC);

//Physics Engine Types:
DECLARE_DEF(PHYS_LEGACY);
DECLARE_DEF(PHYS_NEWTON);
DECLARE_DEF(PHYS_PHYSX);

//CVar declarations
//D3D Initialization
DECLARE_CVAR(v_AdapterID);
DECLARE_CVAR(v_DeviceType);
DECLARE_CVAR(v_VertexProc);
DECLARE_CVAR(v_FPU_Preserve);
DECLARE_CVAR(v_MultiThread);
DECLARE_CVAR(v_PureDevice);
DECLARE_CVAR(v_DisableDriverManagement);
DECLARE_CVAR(v_AdapterGroupDevice);
DECLARE_CVAR(v_Managed);
//D3DPRESENT_PARAMETERS
DECLARE_CVAR(v_Width);
DECLARE_CVAR(v_Height);
DECLARE_CVAR(v_BitDepth);
DECLARE_CVAR(v_ScreenBuffers);
DECLARE_CVAR(v_FSAAQuality);
DECLARE_CVAR(v_SwapEffect);
DECLARE_CVAR(v_Windowed);
DECLARE_CVAR(v_EnableAutoDepthStencil);
DECLARE_CVAR(v_AutoDepthStencilFormat);
//D3DPP_Flags
DECLARE_CVAR(v_LockableBackBuffer);
DECLARE_CVAR(v_DiscardDepthStencil);
DECLARE_CVAR(v_DeviceClip);
DECLARE_CVAR(v_VideoHint);
//More D3DPP
DECLARE_CVAR(v_RefreshRate);
DECLARE_CVAR(v_EnableVSync);
//Texture filter cvars
//FILTER_MODE          "v_TextureFilter"
//FILTER_MAXANISOTROPY "v_MaxAnisotropy"
DECLARE_CVAR(v_TextureFilter);
DECLARE_CVAR(v_MaxAnisotropy);

//Texture loading cvars
DECLARE_CVAR(tm_UseMipMaps);
//DECLARE_CVAR(tm_HWMipMaps);
//DECLARE_CVAR(tm_MipGenFilter);
DECLARE_CVAR(tm_TextureSizeLimit);
DECLARE_CVAR(tm_Force16BitTextures);
//DECLARE_CVAR(tm_32BitTextureAlpha);
//DECLARE_CVAR(tm_16BitTextureAlpha);

DECLARE_CVAR(tm_DefaultTexture);
//Debug cvars
DECLARE_CVAR(v_DebugWireframe);
//Sound cvars
DECLARE_CVAR(s_Channels);
DECLARE_CVAR(s_Frequency);
DECLARE_CVAR(s_BitsPerSample);

DECLARE_CVAR(s_MusicVolume);
//Console cvars
DECLARE_CVAR(lc_UseLegacyFont);
DECLARE_CVAR(lc_BG);
DECLARE_CVAR(lc_Font);
DECLARE_CVAR(lc_FontColor);
DECLARE_CVAR(lc_LegacyFont);
DECLARE_CVAR(lc_LegacyFontSizeInFile);
DECLARE_CVAR(lc_FontSize);
//game cvars
DECLARE_CVAR(lg_ShouldQuit);
DECLARE_CVAR(lg_AverageFPS);
//Debug Dump?
DECLARE_CVAR(lg_DumpDebugData);
//Input cvars
DECLARE_CVAR(li_ExclusiveKB);
DECLARE_CVAR(li_ExclusiveMouse);
DECLARE_CVAR(li_DisableWinKey);
DECLARE_CVAR(li_MouseSmoothing);

//Server cvars
DECLARE_CVAR(srv_MaxEntities);
DECLARE_CVAR(srv_Name);
DECLARE_CVAR(srv_PhysEngine);

//Additional game cvars
DECLARE_CVAR(lg_MaxMeshes);
DECLARE_CVAR(lg_MaxSkels);
DECLARE_CVAR(lg_MaxTex);
DECLARE_CVAR(lg_MaxFx);
DECLARE_CVAR(lg_MaxMtr);
DECLARE_CVAR(lg_MaxSkin);

DECLARE_CVAR(lg_GameLib);

//Effect manager cvars
DECLARE_CVAR(fxm_DefaultFx);

#endif __LG_CVARS_H__