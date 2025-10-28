/************************************************************
	File: lg_init.c
	Copyright (c) 2006, Blaine Myers

	Purpose: Functions for initializing the Legacy 3D game.
************************************************************/
#include "common.h"
#include <d3d9.h>
#include <stdio.h>
#include "lg_cmd.h"
#include "lg_init.h"
#include <lc_sys.h>
#include "lv_init.h"
#include "lg_sys.h"
#include <lf_sys.h>
#include "ls_init.h"

extern L3DGame* g_game;

/***************************************************************************************
	LG_RegisterCVars()

	Register the cvars that the game will use, we set some default values for all
	of them. The cvarlist should report any errors in registering to the console, so
	we don't test it out ourselves.  If we need to save a cvar we can. All cvars
	should be registered in this function.  The user can register cvars on the fly
	using the regcvar console command, but this is discouraged.
***************************************************************************************/
void LG_RegisterCVars(L3DGame* lpGame, HCVARLIST cvarlist)
{
	/* We'll register the built in definitions.  The user can define definitions on the
		fly with define, which is dangerous because the define function will redefine
		values. */
	#define CVAR_DEF(a, b) CVar_AddDef(cvarlist, #a, (float)b)
	//Definitions, users should not change these.
	CVAR_DEF(TRUE, 1);
	CVAR_DEF(FALSE, 0);

	CVAR_DEF(ADAPTER_DEFAULT, D3DADAPTER_DEFAULT);

	//typedef enum _D3DDEVTYPE
	CVAR_DEF(HAL, D3DDEVTYPE_HAL);
	CVAR_DEF(REF, D3DDEVTYPE_REF);
	CVAR_DEF(SW, D3DDEVTYPE_SW);

	//Vertex processing methods.
	CVAR_DEF(SOFTWAREVP, D3DCREATE_SOFTWARE_VERTEXPROCESSING);
	CVAR_DEF(HARDWAREVP, D3DCREATE_HARDWARE_VERTEXPROCESSING);
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
	CVar_AddDef(cvarlist, "POINT", (float)FILTER_POINT);
	CVar_AddDef(cvarlist, "LINEAR", (float)FILTER_LINEAR);
	CVar_AddDef(cvarlist, "BILINEAR", (float)FILTER_BILINEAR);
	CVar_AddDef(cvarlist, "TRILINEAR", (float)FILTER_TRILINEAR);
	CVar_AddDef(cvarlist, "ANISOTROPIC", (float)FILTER_ANISOTROPIC);




	/* To register a cvar: cvar=REGISTER_CVAR("cvar name", "cvar default value", flags) */
	/* Which would be the same as REGCVAR "cvarname" "cvarvalue" [NOSAVE] [UPDATE]*/
	/********************************************
		Direct3D cvars, stuff that controls D3D. 
	*********************************************/
	//D3D Initialization stuff.
	CVar_Register(cvarlist, "v_AdapterID", "ADAPTER_DEFAULT", CVAREG_SAVE);
	CVar_Register(cvarlist, "v_DeviceType", "HAL", 0);
	CVar_Register(cvarlist, "v_VertexProc", "SOFTWAREVP", CVAREG_SAVE);
	CVar_Register(cvarlist, "v_FPU_Preserve", "FALSE", 0);
	CVar_Register(cvarlist, "v_MultiThread", "FALSE", 0);
	CVar_Register(cvarlist, "v_PureDevice", "FALSE", 0);
	CVar_Register(cvarlist, "v_DisableDriverManagement", "FALSE", 0);
	CVar_Register(cvarlist, "v_AdapterGroupDevice", "FALSE", 0);
	CVar_Register(cvarlist, "v_Managed", "FALSE", 0);
	//D3DPRESENT_PARAMETERS
	CVar_Register(cvarlist, "v_Width", "640", CVAREG_SAVE);
	CVar_Register(cvarlist, "v_Height", "480", CVAREG_SAVE);
	CVar_Register(cvarlist, "v_BitDepth", "16", CVAREG_SAVE);
	CVar_Register(cvarlist, "v_ScreenBuffers", "1", CVAREG_SAVE);
	CVar_Register(cvarlist, "v_FSAAQuality", "0", CVAREG_SAVE);
	CVar_Register(cvarlist, "v_SwapEffect", "SWAP_DISCARD", 0);
	CVar_Register(cvarlist, "v_Windowed", "FALSE", CVAREG_SAVE);
	CVar_Register(cvarlist, "v_EnableAutoDepthStencil", "TRUE", 0);
	CVar_Register(cvarlist, "v_AutoDepthStencilFormat", "FMT_D16", 0);
	//v_D3DPP_Flags:
	CVar_Register(cvarlist, "v_LockableBackBuffer", "FALSE", 0);
	CVar_Register(cvarlist, "v_DiscardDepthStencil", "FALSE", 0);
	CVar_Register(cvarlist, "v_DeviceClip", "FALSE", 0);
	CVar_Register(cvarlist, "v_VideoHint", "FALSE", 0);
	//More D3DPRESENT_PARAMETERS
	CVar_Register(cvarlist, "v_RefreshRate", "0", CVAREG_SAVE);
	CVar_Register(cvarlist, "v_EnableVSync", "FALSE", CVAREG_SAVE);

	//Sampler CVars, the sampler cvars have the update attribute, so we will
	//see the results immediately.
	CVar_Register(cvarlist, FILTER_MODE, "BILINEAR", CVAREG_SAVE|CVAREG_UPDATE);
	CVar_Register(cvarlist, FILTER_MAXANISOTROPY, "1", CVAREG_SAVE|CVAREG_UPDATE);
	
	//Texture loading related cvars.
	CVar_Register(cvarlist, "v_UseMipMaps", "TRUE", CVAREG_SAVE);
	CVar_Register(cvarlist, "v_HWMipMaps", "TRUE", CVAREG_SAVE);
	CVar_Register(cvarlist, "v_MipGenFilter", "LINEAR", CVAREG_SAVE);
	CVar_Register(cvarlist, "v_TextureSizeLimit", "0", CVAREG_SAVE);
	CVar_Register(cvarlist, "v_Force16BitTextures", "FALSE", CVAREG_SAVE);
	//v_32BitTextrueAlpha and v_16BitTextureAlpha are used to store information
	//as to whethere or not alpha textures are supported, these cvars shouldn't
	//be changed by the user, only by the application.
	CVar_Register(cvarlist, "v_32BitTextureAlpha", "TRUE", CVAREG_SETWONTCHANGE);
	CVar_Register(cvarlist, "v_16BitTextureAlpha", "TRUE", CVAREG_SETWONTCHANGE);
	//Should have a cvar to determine which pool textures are loaded into.

	//Debug cvars
	CVar_Register(cvarlist, "v_DebugWireframe", "FALSE", CVAREG_UPDATE);

	/******************************************
		Sound cvars, stuff that controls audio. 
	*******************************************/

	CVar_Register(cvarlist, "s_Channels", "2", CVAREG_SAVE);
	CVar_Register(cvarlist, "s_Frequency", "22050", CVAREG_SAVE);
	CVar_Register(cvarlist, "s_BitsPerSample", "16", CVAREG_SAVE);

	/************************************
		Game cvars, stuff about the game.
	*************************************/
	/* The path to the console background, note that this can only be changed by
		the user by modifying the cvar in the console and calling vrestart or
		setting a different path in the autoexec.cfg file. Same is true for the
		font.*/
	CVar_Register(cvarlist, "lc_UseLegacyFont", "FALSE", CVAREG_SAVE);
	CVar_Register(cvarlist, "lc_BG", "console\\conbg.tga", 0);
	CVar_Register(cvarlist, "lc_Font", "fixedsys", 0);
	CVar_Register(cvarlist, "lc_FontColor", "0xFFFF00", 0);
	CVar_Register(cvarlist, "lc_LegacyFont", "font\\lfontsm.tga", 0);
	CVar_Register(cvarlist, "lc_LegacyFontSizeInFile", "15X8", 0); //Format is (H)X(W)
	CVar_Register(cvarlist, "lc_FontSize", "16X0", 0); //Format is (H)X(W)

	lpGame->l_ShouldQuit=CVar_Register(cvarlist, "l_ShouldQuit", "FALSE", CVAREG_UPDATE);
	lpGame->l_AverageFPS=CVar_Register(cvarlist, "l_AverageFPS", "0", CVAREG_SETWONTCHANGE);
	CVar_Register(cvarlist, "l_DumpDebugData", "FALSE", CVAREG_SAVE);
	return;
}


/*********************************************************************
	LG_GameInit()

	Initializes various game things, including the console, cvarlist,
	graphics, and audio.
*********************************************************************/
L3DGame* LG_GameInit(char* szGameDir, HWND hwnd)
{
	L3DGame* lpNewGame=NULL;
	L_long nResult=0;

	/* Allocate memory for the game, and clear it all out. */
	lpNewGame=malloc(sizeof(L3DGame));
	if(!lpNewGame)
		return L_null;
	memset(lpNewGame, 0, sizeof(L3DGame));


	/* Attach the window to the game. */
	lpNewGame->m_hwnd=hwnd;
	/**************************
		Initialize the console.
	 *************************/
	/* Create the console, if we fail to create the console, the who program
		isn't going to function correctly. Pretty much the only reason the
		console wouldn't start is if the user was out of memory, which means
		the program wouldn't work anyway.  We also need the cvarlist to get
		created for the game to work.*/
	lpNewGame->m_Console=Con_Create(
		LGC_ConCommand, 
		512, 
		2048, 
		CONCREATE_USEINTERNAL,
		lpNewGame);
	/* Set the global pointer to the game. */
	g_game=lpNewGame;
	/* If the console fails, we can't do anything else with the app. */
	if(!lpNewGame->m_Console)
	{
		free(lpNewGame);
		return L_null;
	}
	/* Attach the console to the error reporting. Which means we can just call
		Err_Printf to send a message to the console, as opposed to calling the
		Con_SendErrorMsg(*console, ...) function. */
	Err_InitReporting(lpNewGame->m_Console);
	Err_PrintVersion();
	Err_Printf("Created console at 0x%08X.", lpNewGame->m_Console);
	
	/******************************* 
		Initialze the file system. 
	 ******************************/
	/* We need to set the directory to the game data directory.  By default this
		is [legacy3ddirectory]\base, but we want to make it so that the user can 
		change this to run custom games for example*/
	Err_Printf("Calling LF_Init...");
	LF_Init(szGameDir, 0);//LFINIT_CHECKARCHIVESFIRST);
	Err_Printf(LF_GetLastError());
	/* Keep adding pak files incrementally til we can't find one.  In theory we could
		use _findfirst and _findnext to find all the pack files in the directory, but
		that wouldn't gaurantee what order they got put in. */
	{
	unsigned short i=0;
	char szPakFile[10];
	for(i=0; i<99; i++)
	{
		sprintf(szPakFile, "pak%.2i.lpk", i);
		if(LF_AddArchive(szPakFile))
		{
			Err_Printf(LF_GetLastError());//"Added %s to seach path.", szPakFile);
		}
		else
			break;
	}
	}
	
	/************************** 
		Create the cvarlist.  
	**************************/	
	/* We create the cvarlist, attach it to the console, so
		the console can use the built in functions with it
		we register all the commands for the console, and 
		we register all the cvars for teh game, and then 
		we set the cvars by attempting to load config.cfg
		as well as autoexec.cfg.   Note that any cvars that
		will be saved will be written to config.cfg, any cvars
		that the user wants to manipulate and save that aren't
		saved should be written in autoexec.cfg (for example
		v_VideoHint is not saved, but if the user wanted to
		set this variable to true, he should put
		SET v_VideoHint "TRUE" in the autoexec.cfg file.*/
	lpNewGame->m_cvars=CVar_CreateList(lpNewGame->m_Console);
	if(lpNewGame->m_cvars==NULL)
	{
		Err_Printf("Failed to initialze cvars.");
		Con_SendCommand(lpNewGame->m_Console, "condump \"debug.txt\"");
		Con_Delete(lpNewGame->m_Console);
		lpNewGame->m_Console=L_null;
		free(lpNewGame);
		return L_null;
	}
	Err_Printf("Created cvars at 0x%08X.", lpNewGame->m_cvars);
	Con_AttachCVarList(lpNewGame->m_Console, lpNewGame->m_cvars);
	/* Register the console commands for Legacy 3D. */
	LGC_RegConCmds(lpNewGame->m_Console);
	/* Register the cvars for the game. */
	LG_RegisterCVars(lpNewGame, lpNewGame->m_cvars);

	/* Now load all the presets. */
	Con_SendCommand(lpNewGame->m_Console, "loadcfg \"config.cfg\"");
	Con_SendCommand(lpNewGame->m_Console, "loadcfg \"autoexec.cfg\"");

	
	/*************************
		Initialize the video. 
	**************************/
	Err_Printf("Calling LV_Init...");
	nResult=LV_Init(lpNewGame);
	if(L_failed(nResult))
	{
		Err_Printf("LV_Init failed with result 0x%08X.", nResult);
		Con_SendCommand(lpNewGame->m_Console, "condump \"debug.txt\"");
		/*Destroy everything we've created so far. */
		CVar_DeleteList(lpNewGame->m_cvars);
		lpNewGame->m_cvars=NULL;
		Con_Delete(lpNewGame->m_Console);
		lpNewGame->m_Console=NULL;
		free(lpNewGame);
		return L_null;
	}
	else
		Err_Printf("LV_Init succeeded.");


	/*************************
		Initialize the sound.
	**************************/
	Err_Printf("Calling LS_Init...");
	nResult=LS_Init(lpNewGame);
	if(L_failed(nResult))
	{
		/* If the sound isn't initialized we just display an error message,
			as the game can still run without sound. */
		Err_Printf("LS_Init failed with result 0x%08X, audio not available.", nResult);
	}
	else
		Err_Printf("LS_Init succeeded.");
	/* Some test stuff. */

	return lpNewGame;
}


/***************************************************************
	LG_GameShutdown()

	Shuts down and deletes everything created in LG_GameInit().
***************************************************************/
L_bool LG_GameShutdown(L3DGame* lpGame)
{
	IDirect3D9* lpD3D=NULL;
	L_int nResult=0;
	L_ulong nNumLeft=0;
	L_bool bDump=L_false;

	if(!lpGame)
		return L_false;


	Err_Printf("Shutting down Legacy Engine.");
	Err_Printf("Calling LS_Shutdown...");
	nResult=LS_Shutdown(lpGame);
	Err_Printf("LS_Shutdown resulted with 0x%08X.", nResult);
	Err_Printf("Calling LV_Shutdown...");
	nResult=LV_Shutdown(lpGame);
	Err_Printf("LV_Shutdown resulted with 0x%08X.", nResult);

	/* Dump the console, it should probably be determined by a cvar if
		the console should get dumped or not, but for now we always dump
		it. */
	bDump=(int)CVar_GetValue(lpGame->m_cvars, "l_DumpDebugData", L_null);

	/* Save the cvars that want to be saved. */

	Con_SendCommand(lpGame->m_Console, "savecfg \"config.cfg\"");
	Err_Printf("Destroying cvarlist.");
	CVar_DeleteList(lpGame->m_cvars);
	lpGame->m_cvars=L_null;
	/* Destroy the file system.  Any open files should be closed
		before this is called. */
	LF_Shutdown();
	Err_Printf("Destroying console, goodbye!");
	if(bDump)
		Con_SendCommand(
			lpGame->m_Console,
			"condump \"debug.txt\"");
	Con_Delete(lpGame->m_Console);
	lpGame->m_Console=L_null;
	/* Make sure there is no pointer in the error reporting. */
	Err_InitReporting(L_null);
	free(lpGame);
	return L_true;
}