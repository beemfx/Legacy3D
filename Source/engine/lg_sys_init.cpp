#include <stdio.h>
#include "lg_cmd.h"
#include "lg_sys.h"
#include "lg_err.h"
#include "lf_sys2.h"
#include "lg_cvars.h"
#include "lg_err_ex.h"
#include "lg_malloc.h"
#include "lg_func.h"
#include "../lc_sys2/lc_sys2.h"


/*********************************************************************
	LG_GameInit()

	Initializes various game things, including the console, cvarlist,
	graphics, and audio.
*********************************************************************/
lg_bool CLGame::LG_GameInit(lg_pstr szGameDir, HWND hwnd)
{	
	lg_long nResult=0;
	/* Attach the window to the game. */
	m_hwnd=hwnd;
	/********************************
		Initialzie the math library
	********************************/
	ML_Init(ML_INSTR_BEST);
	/**************************
		Initialize the console.
	 *************************/
	/* Create the console, if we fail to create the console, the who program
		isn't going to function correctly. Pretty much the only reason the
		console wouldn't start is if the user was out of memory, which means
		the program wouldn't work anyway.  We also need the cvarlist to get
		created for the game to work.*/
	if(!LC_Init())
		throw CLError(LG_ERR_DEFAULT, __FILE__, __LINE__, "LG: Could not initialize conosle.");
	LC_SetCommandFunc(LCG_ConCommand, this);
	Err_Printf("=== Initializing Legacy Game ===");
	Err_IncTab();
	
	Err_PrintVersion();
	
	//Err_Printf("Created console at 0x%08X.", s_console);
	
	/*************************
		Initialzie the timer.
	*************************/
	Err_Printf("Initializing timer...");
	s_Timer.Initialize();
	
	/******************************* 
		Initialze the file system. 
	 ******************************/
	/* We need to set the directory to the game data directory.  By default this
		is [legacy3ddirectory]\base, but we want to make it so that the user can 
		change this to run custom games for example*/
	Err_Printf("Initializing File System...");
	FS_Initialize(1024*5, LG_Malloc, LG_Free);
	LF_SetErrLevel(ERR_LEVEL_NOTICE);
	LF_SetErrPrintFn((LF_ERR_CALLBACK)Err_FSCallback);
	//Mount the base file system.
	FS_MountBase(szGameDir);
	//The mounted path /dbase represents the "default" base,
	//any mods should be mounted to /gbase for "game" base.
	
	//#define DBASE_PATH "base"
	#define DBASE_PATH "baseuncmp"
	FS_Mount(DBASE_PATH, "/dbase", MOUNT_FILE_OVERWRITE|MOUNT_MOUNT_SUBDIRS);
	
	//Expand all the lpk files in the default directory.
	//In theory we could search for all lpk files, but this
	//way it insures that lpk files get mounted in the correct
	//order, note that higher numbered lpks
	//will overwrite any files with the same name in lower numbered
	//lpks.
	for(lg_word i=0; i<99; i++)
	{
		lf_path szPakFile;
		sprintf(szPakFile, "/dbase/pak%u.lpk", i);
		if(!FS_MountLPK(szPakFile, MOUNT_FILE_OVERWRITELPKONLY))
			break;
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
	/* Register the console commands for Legacy 3D. */
	LGC_RegConCmds();
	/* Register the cvars for the game. */
	LG_RegisterCVars();

	/* Now load all the presets. */
	//Should load these from the main game base...
	LC_SendCommand("loadcfg \"/dbase/config.cfg\"");
	LC_SendCommand("loadcfg \"/dbase/autoexec.cfg\"");
	
	/*************************
		Initialize the video. 
	**************************/
	Err_Printf("Calling LV_Init...");
	if(!m_Video.Init(m_hwnd))
	{
		Err_Printf("LV_Init failed with result 0x%08X.", nResult);
		LC_SendCommand("condump \"/dbase/debug.txt\"");
		/*Destroy everything we've created so far. */
		//CVar_DeleteList(s_cvars);
		//s_cvars=LG_NULL;
		LC_Shutdown();
		//s_console=LG_NULL;
		m_nGameState=LGSTATE_SHUTDOWN;
		return LG_FALSE;
	}
	else
		Err_Printf("LV_Init succeeded.");
		
	#if 0
	Err_Printf("Initializing texture manager...");
	//Should get maximum textures from a cvar.
	m_pTMgr=new CLTMgr(s_pDevice, CV_Get(CVAR_lg_MaxTex)->nValue);
	m_pFxMgr=new CLFxMgr(s_pDevice, CV_Get(CVAR_lg_MaxFx)->nValue);
	m_pMtrMgr=new CLMtrMgr(CV_Get(CVAR_lg_MaxMtr)->nValue);
	#endif
	InitMgrs(s_pDevice);
	
	/********
		Initialize Game Windowing System
	*********/
	Err_Printf("=== Initialzing Windowing System ===");
	m_WndMgr.Init();
	Err_Printf("====================================");
	
	//Once the video is initialized, we can initalize the graphical console.
	Err_Printf("Initializing console graphical interface...");
	if(!m_VCon.Create(s_pDevice))
	{
		//If the creation of the graphical console fails, the game can still run,
		//but any calls to m_VCon methods will not do anything.
		Err_Printf("Graphical console creation failed, feature not available.");
	}

	/*************************
		Initialize the sound.
	**************************/
	Err_Printf("=== Audio System Initialization ===");
	Err_Printf("Calling LS_Init...");
	if(!m_SndMgr.LS_Init())
	{
		/* If the sound isn't initialized we just display an error message,
			as the game can still run without sound. */
		Err_Printf("LS_Init failed with, audio not available.");
	}
	else
		Err_Printf("LS_Init succeeded.");
	Err_Printf("===================================");

	#if 0
	//Begin initializing game structures.
	Err_Printf("Initializing the mesh & skeleton manager...");
	m_pMMgr=new CLMMgr(s_pDevice, CV_Get(CVAR_lg_MaxMeshes)->nValue, CV_Get(CVAR_lg_MaxSkels)->nValue);
	#endif
	/***********************
		Initialize input.
	***********************/
	Err_Printf("Calling LI_Init...");
	lg_dword nFlags=0;
	if(CV_Get(CVAR_li_ExclusiveKB)->nValue)
		nFlags|=LINPUT_EXCLUSIVEKB;
	if(CV_Get(CVAR_li_ExclusiveMouse)->nValue)
		nFlags|=LINPUT_EXCLUSIVEMOUSE;
	if(CV_Get(CVAR_li_DisableWinKey)->nValue)
		nFlags|=LINPUT_DISABLEWINKEY;
	if(CV_Get(CVAR_li_MouseSmoothing)->nValue)
		nFlags|=LINPUT_SMOOTHMOUSE;
	if(!m_Input.LI_Init(m_hwnd, nFlags, CON_KEYDX))
	{
		Err_Printf("LI_Init failed, input not available.");
	}
	else
		Err_Printf("LI_Init succeeded");
		
	
	
	/*****************************
		Initialize the game world
	******************************/
	Err_Printf("=== Initializing Game World ===");
	Err_IncTab();
	
	m_WorldSrv.Init();
	m_WorldCln.Init(&m_Input);
	
	#if 1
	/* Some test stuff */
	//m_WorldSrv.LoadLevel("/dbase/scripts/levels/very_simple.xml");
	m_WorldSrv.LoadLevel("/dbase/scripts/levels/primitive_test.xml");
	m_WorldCln.ConnectLocal(&m_WorldSrv);
	//m_WorldSrv.RemoveEnt(9999);
	lg_str szTestSound="InTown.ogg";
	LC_SendCommandf("MUSIC_START \"%s\"", szTestSound);
	#elif 0
	m_WorldSrv.LoadMap("/dbase/maps/lame_level.3dw");
	m_WorldCln.ConnectLocal(&m_WorldSrv);
	#endif
	
	Err_DecTab();
	Err_Printf("===============================");
	m_nGameState=LGSTATE_RUNNING;
	
	Err_DecTab();
	Err_Printf("================================");
	return LG_TRUE; 
}

/***************************************************************
	LG_GameShutdown()

	Shuts down and deletes everything created in LG_GameInit().
***************************************************************/
void CLGame::LG_GameShutdown()
{
	IDirect3D9* lpD3D=NULL;
	lg_int nResult=0;
	lg_ulong nNumLeft=0;
	lg_bool bDump=LG_FALSE;
	
	Err_Printf("Shutting down Legacy Engine.");
	Err_Printf("Shutting down the world...");
	m_WorldCln.Disconnect();
	m_WorldCln.Shutdown();
	m_WorldSrv.Shutdown();
	
	ShutdownMgrs();
	
	Err_Printf("Shutting down input system...");
	m_Input.LI_Shutdown();
	Err_Printf("=== Audio System Shutdown ===");
	Err_Printf("Calling LS_Shutdown...");
	m_SndMgr.LS_Shutdown();
	Err_Printf("=============================");
	
	/*********************************
		Raster object uninitalization. 
	*********************************/
	
	Err_Printf("Destroying graphic console...");
	m_VCon.Delete();
	s_pDevice->SetTexture(0, LG_NULL);
	s_pDevice->SetTexture(1, LG_NULL);
	
	Err_Printf("Destorying windowing system...");
	m_WndMgr.Shutdown();
	
	Err_Printf("Shutting down video processing...");
	m_Video.Shutdown();

	/* Dump the console, it should probably be determined by a cvar if
		the console should get dumped or not, but for now we always dump
		it. */
	bDump=CV_Get(CVAR_lg_DumpDebugData)->nValue;

	/* Save the cvars that want to be saved. */
	LC_SendCommand("savecfg \"/dbase/config.cfg\"");
	Err_Printf("Destroying console, goodbye!");
	if(bDump)
		LC_SendCommand("condump \"/dbase/debug.txt\"");
		
	/* Destroy the file system.  Any open files should be closed
		before this is called. Note that this must be done after
		the console is dump so that the data can be written.*/
	Err_Printf("Shutting down the file system...");
	FS_Shutdown();
	LC_Shutdown();
	/* Make sure there is no pointer in the error reporting. */
	m_nGameState=LGSTATE_SHUTDOWN;
}
