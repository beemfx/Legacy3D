/* lg_cvars.cpp has just one purpose, when it includes lg_cvars.h it actually
	declares and defines the cvars, in that way every file that uses the cvar list doesn't
	have to have memory allocated for the entire list. */
	
#define DECLARING_CVARS
#include "lg_cvars.h"
#include "lg_sys.h"
#include "lp_sys2.h"

#include "../lc_sys2/lc_sys2.h"


/***************************************************************************************
	LG_RegisterCVars()

	Register the cvars that the game will use, we set some default values for all
	of them. The cvarlist should report any errors in registering to the console, so
	we don't test it out ourselves.  If we need to save a cvar we can. All cvars
	should be registered in this function.  The user can register cvars on the fly
	using the regcvar console command, but this is discouraged.
***************************************************************************************/
void CLGame::LG_RegisterCVars()
{
	/* We'll register the built in definitions.  The user can define definitions on the
		fly with define, which is dangerous because the define function will redefine
		values. */
	#define CVAR_DEF(a, b) CV_Define_f(DEF_##a, (float)b)
	//Definitions, users should not change these.
	CVAR_DEF(TRUE, 1);
	CVAR_DEF(FALSE, 0);

	CVAR_DEF(ADAPTER_DEFAULT, D3DADAPTER_DEFAULT);

	//typedef enum _D3DDEVTYPE
	CVAR_DEF(HAL, D3DDEVTYPE_HAL);
	CVAR_DEF(REF, D3DDEVTYPE_REF);
	CVAR_DEF(SW, D3DDEVTYPE_SW);

	//Vertex processing methods.
	CVAR_DEF(SWVP, D3DCREATE_SOFTWARE_VERTEXPROCESSING);
	CVAR_DEF(HWVP, D3DCREATE_HARDWARE_VERTEXPROCESSING);
	CVAR_DEF(MIXEDVP, D3DCREATE_MIXED_VERTEXPROCESSING);

	//typdef enum _D3DFORMAT
	//Back buffer formats.
	CVAR_DEF(FMT_UNKNOWN, D3DFMT_UNKNOWN);
	CVAR_DEF(FMT_A8R8G8B8, D3DFMT_A8R8G8B8);
	CVAR_DEF(FMT_X8R8G8B8, D3DFMT_X8R8G8B8);
	CVAR_DEF(FMT_R5G6B5, D3DFMT_R5G6B5);
	CVAR_DEF(FMT_X1R5G5B5, D3DFMT_X1R5G5B5);
	CVAR_DEF(FMT_A1R5G5B5, D3DFMT_A1R5G5B5);
	CVAR_DEF(FMT_A2R10G10B10, D3DFMT_A2R10G10B10);

	//Depth buffer formats.
	CVAR_DEF(FMT_D16_LOCKABLE, D3DFMT_D16_LOCKABLE);
	CVAR_DEF(FMT_D32, D3DFMT_D32);
	CVAR_DEF(FMT_D15S1, D3DFMT_D15S1);
	CVAR_DEF(FMT_D24S8, D3DFMT_D24S8);
	CVAR_DEF(FMT_D24X8, D3DFMT_D24X8);
	CVAR_DEF(FMT_D24X4S4, D3DFMT_D24X4S4);
	CVAR_DEF(FMT_D16, D3DFMT_D16);
	CVAR_DEF(FMT_D32F_LOCKABLE, D3DFMT_D32F_LOCKABLE);
	CVAR_DEF(FMT_D24FS8, D3DFMT_D24FS8);


	//typedef enum _CVAR_DEF(D3DMULTISAMPLE_TYPE
	/*
	CVAR_DEF(D3DMULTISAMPLE_NONE, 0);
	CVAR_DEF(D3DMULTISAMPLE_NONMASKABLE, 1);
	CVAR_DEF(D3DMULTISAMPLE_2_SAMPLES, 2);
	CVAR_DEF(D3DMULTISAMPLE_3_SAMPLES, 3);
	CVAR_DEF(D3DMULTISAMPLE_4_SAMPLES, 4);
	CVAR_DEF(D3DMULTISAMPLE_5_SAMPLES, 5);
	CVAR_DEF(D3DMULTISAMPLE_6_SAMPLES, 6);
	CVAR_DEF(D3DMULTISAMPLE_7_SAMPLES, 7);
	CVAR_DEF(D3DMULTISAMPLE_8_SAMPLES, 8);
	CVAR_DEF(D3DMULTISAMPLE_9_SAMPLES, 9);
	CVAR_DEF(D3DMULTISAMPLE_10_SAMPLES,10);
	CVAR_DEF(D3DMULTISAMPLE_11_SAMPLES,11);
	CVAR_DEF(D3DMULTISAMPLE_12_SAMPLES,12);
	CVAR_DEF(D3DMULTISAMPLE_13_SAMPLES,13);
	CVAR_DEF(D3DMULTISAMPLE_14_SAMPLES,14);
	CVAR_DEF(D3DMULTISAMPLE_15_SAMPLES,15);
	CVAR_DEF(D3DMULTISAMPLE_16_SAMPLES,16);
	*/

	//typedef enum _D3DSWAPEFFECT
	CVAR_DEF(SWAP_DISCARD, D3DSWAPEFFECT_DISCARD);
	CVAR_DEF(SWAP_FLIP, D3DSWAPEFFECT_FLIP);
	CVAR_DEF(SWAP_COPY, D3DSWAPEFFECT_COPY);

	//PresentationIntervals
	/*
	CVAR_DEF(D3DPRESENT_INTERVAL_DEFAULT, 0);
	CVAR_DEF(D3DPRESENT_INTERVAL_ONE, 1);
	CVAR_DEF(D3DPRESENT_INTERVAL_TWO, 2);
	CVAR_DEF(D3DPRESENT_INTERVAL_THREE, 3);
	CVAR_DEF(D3DPRESENT_INTERVAL_FOUR, 4);
	CVAR_DEF(D3DPRESENT_INTERVAL_IMMEDIATE, -1);
	*/

	/* Register the definitions for the texture filter mode. */
	CVAR_DEF(POINT, FILTER_POINT);
	CVAR_DEF(LINEAR, FILTER_LINEAR);
	CVAR_DEF(BILINEAR, FILTER_BILINEAR);
	CVAR_DEF(TRILINEAR, FILTER_TRILINEAR);
	CVAR_DEF(ANISOTROPIC, FILTER_ANISOTROPIC);
	
	/* CVAR_PhysEngineDef */
	CVAR_DEF(PHYS_LEGACY, PHYS_ENGINE_LEGACY);
	CVAR_DEF(PHYS_NEWTON, PHYS_ENGINE_NEWTON);
#if 0
	CVAR_DEF(PHYS_PHYSX, PHYS_ENGINE_PHYSX);
#endif

	/* To register a cvar: cvar=REGISTER_CVAR("cvar name", "cvar default value", flags) */
	/* Which would be the same as REGCVAR "cvarname" "cvarvalue" [NOSAVE] [UPDATE]*/
	/********************************************
		Direct3D cvars, stuff that controls D3D. 
	*********************************************/
	//D3D Initialization stuff.
	CV_Register(CVAR_v_AdapterID, DEF_ADAPTER_DEFAULT, CVAR_SAVE);
	CV_Register(CVAR_v_DeviceType, DEF_HAL, 0);
	CV_Register(CVAR_v_VertexProc, DEF_SWVP, CVAR_SAVE);
	CV_Register(CVAR_v_FPU_Preserve, DEF_FALSE, 0);
	CV_Register(CVAR_v_MultiThread, DEF_FALSE, 0);
	CV_Register(CVAR_v_PureDevice, DEF_FALSE, 0);
	CV_Register(CVAR_v_DisableDriverManagement, DEF_FALSE, 0);
	CV_Register(CVAR_v_AdapterGroupDevice, DEF_FALSE, 0);
	CV_Register(CVAR_v_Managed, DEF_FALSE, 0);
	//D3DPRESENT_PARAMETERS
	CV_Register(CVAR_v_Width, "640", CVAR_SAVE);
	CV_Register(CVAR_v_Height, "480", CVAR_SAVE);
	CV_Register(CVAR_v_BitDepth, "16", CVAR_SAVE);
	CV_Register(CVAR_v_ScreenBuffers, "1", CVAR_SAVE);
	CV_Register(CVAR_v_FSAAQuality, "0", CVAR_SAVE);
	CV_Register(CVAR_v_SwapEffect, DEF_SWAP_DISCARD, 0);
	CV_Register(CVAR_v_Windowed, DEF_FALSE, CVAR_SAVE);
	CV_Register(CVAR_v_EnableAutoDepthStencil, DEF_TRUE, 0);
	CV_Register(CVAR_v_AutoDepthStencilFormat, DEF_FMT_D16, 0);
	//v_D3DPP_Flags:
	CV_Register(CVAR_v_LockableBackBuffer, DEF_FALSE, 0);
	CV_Register(CVAR_v_DiscardDepthStencil, DEF_FALSE, 0);
	CV_Register(CVAR_v_DeviceClip, DEF_FALSE, 0);
	CV_Register(CVAR_v_VideoHint, DEF_FALSE, 0);
	//More D3DPRESENT_PARAMETERS
	CV_Register(CVAR_v_RefreshRate, "0", CVAR_SAVE);
	CV_Register(CVAR_v_EnableVSync, DEF_FALSE, CVAR_SAVE);

	//Sampler CVars, the sampler cvars have the update attribute, so we will
	//see the results immediately.
	CV_Register(CVAR_v_TextureFilter, DEF_BILINEAR, CVAR_SAVE|CVAR_UPDATE);
	CV_Register(CVAR_v_MaxAnisotropy, "1", CVAR_SAVE|CVAR_UPDATE);
	
	//Texture loading related cvars.
	CV_Register(CVAR_tm_UseMipMaps, DEF_TRUE, CVAR_SAVE);
	//CV_Register(CVAR_tm_HWMipMaps, DEF_TRUE, CVAR_SAVE);
	//CV_Register(CVAR_tm_MipGenFilter, DEF_LINEAR, CVAR_SAVE);
	CV_Register(CVAR_tm_TextureSizeLimit, "0", CVAR_SAVE);
	CV_Register(CVAR_tm_Force16BitTextures, DEF_FALSE, CVAR_SAVE);
	/*v_32BitTextrueAlpha and v_16BitTextureAlpha are used to store information
	//as to whethere or not alpha textures are supported, these cvars shouldn't
	//be changed by the user, only by the application.*/
	//CV_Register(CVAR_tm_32BitTextureAlpha, DEF_TRUE, CVAR_ROM);
	//CV_Register(CVAR_tm_16BitTextureAlpha, DEF_TRUE, CVAR_ROM);
	//Should have a cvar to determine which pool textures are loaded into.
	CV_Register(CVAR_tm_DefaultTexture, "/dbase/textures/default.tga", 0);
	//Debug cvars
	CV_Register(CVAR_v_DebugWireframe, DEF_FALSE, CVAR_UPDATE);

	/******************************************
		Sound cvars, stuff that controls audio. 
	*******************************************/

	CV_Register(CVAR_s_Channels, "2", CVAR_SAVE);
	CV_Register(CVAR_s_Frequency, "22050", CVAR_SAVE);
	CV_Register(CVAR_s_BitsPerSample, "16", CVAR_SAVE);
	
	CV_Register(CVAR_s_MusicVolume, "100", CVAR_SAVE|CVAR_UPDATE);

	/************************************
		Game cvars, stuff about the game.
	*************************************/
	/* The path to the console background, note that this can only be changed by
		the user by modifying the cvar in the console and calling vrestart or
		setting a different path in the autoexec.cfg file. Same is true for the
		font.*/
	CV_Register(CVAR_lc_UseLegacyFont, DEF_FALSE, CVAR_SAVE);
	CV_Register(CVAR_lc_BG, "/dbase/textures/console/conbg.tga", 0);
	CV_Register(CVAR_lc_Font, "fixedsys", 0);
	CV_Register(CVAR_lc_FontColor, "0xFFFF00", 0);
	CV_Register(CVAR_lc_LegacyFont, "/dbase/font/lfontsm.tga", 0);
	CV_Register(CVAR_lc_LegacyFontSizeInFile, "15X8", 0); //Format is (H)X(W)
	CV_Register(CVAR_lc_FontSize, "16X0", 0); //Format is (H)X(W)

	cv_l_ShouldQuit=CV_Register(CVAR_lg_ShouldQuit, DEF_FALSE, CVAR_UPDATE);
	cv_l_AverageFPS=CV_Register(CVAR_lg_AverageFPS, "0", 0);
	CV_Register(CVAR_lg_DumpDebugData, DEF_FALSE, CVAR_SAVE);
	
	/*****************
		Input cvars ***
	*****************/
	CV_Register(CVAR_li_ExclusiveKB, DEF_FALSE, 0);
	CV_Register(CVAR_li_ExclusiveMouse, DEF_TRUE, 0);
	CV_Register(CVAR_li_DisableWinKey, DEF_TRUE, CVAR_SAVE);
	CV_Register(CVAR_li_MouseSmoothing, DEF_TRUE, CVAR_SAVE);
	
	//Server cvars
	CV_Register(CVAR_srv_MaxEntities, "10000", CVAR_ROM);
	CV_Register(CVAR_srv_Name, "Legacy", CVAR_SAVE);
	CV_Register(CVAR_srv_PhysEngine, DEF_PHYS_LEGACY, CVAR_ROM);
	
	//Additional game cvars
	CV_Register(CVAR_lg_MaxMeshes, "512", CVAR_ROM);
	CV_Register(CVAR_lg_MaxSkels, "2048", CVAR_ROM);
	CV_Register(CVAR_lg_MaxTex, "128", CVAR_ROM);
	CV_Register(CVAR_lg_MaxFx, "128", CVAR_ROM);
	CV_Register(CVAR_lg_MaxMtr, "128", CVAR_ROM);
	CV_Register(CVAR_lg_MaxSkin, "128", CVAR_ROM);
	
	CV_Register(CVAR_lg_GameLib, "game.dll", CVAR_ROM);
	
	//Effect manager cvars
	CV_Register(CVAR_fxm_DefaultFx, "/dbase/fx/basic_unlit.fx", 0);
	
	return;
}
