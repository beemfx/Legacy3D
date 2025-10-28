/**************************************************************
	File: lg_cmd.c
	Copyright (c) 2006, Blaine Myers

	Purpose: Contains the command function for the console.
	It also contains the function that registers the console
	commands, and various functions that are called by console
	commands.
**************************************************************/
#include "lg_func.h"
#include <stdio.h>
#include "lg_malloc.h"
#include "lg_cmd.h"
#include "lg_sys.h"
#include "lf_sys2.h"
#include "lg_err.h"

#include "../lc_sys2/lc_sys2.h"

#include "lg_cvars.h"

#define MAX_T_LEN (1024)

void LGC_DisplayDeviceCaps(CLGame* pGame);
void LGC_Dir(const lg_char* szLimit);
void LGC_Extract(const lg_char* szFile, const lg_char* szOutFile);
void LGC_Test(const lg_char* szParam);
void LGC_ConDump(lf_path szFilename);
void LGC_LoadCFG(const lg_char* szFilename);
void LGC_SaveCFG(const lg_char* szFilename, lg_bool bAppend, lg_bool bAll);
void LGC_Set(const lg_char* szCvar, const lg_char* szValue);
void LGC_Get(const lg_char* szCvar);
//void LGC_CvarUpdate(CLGame* pGame, const lg_char* szCvar);
void LGC_CvarList(const lg_char* szLimit);

/**************************************************************
	static CLGame::LGC_RegConCmds()

	Called from LG_Init(), it registers all of the console
	commands so that when a user types one it it will actually
	get processed.  As sort of a standard the value of all
	console commands start with CONF_ then are appended with
	the function name.
**************************************************************/
void CLGame::LGC_RegConCmds()
{
	#define REG_CMD(name) LC_RegisterCommand(#name, CONF_##name, 0)
	
	REG_CMD(QUIT);
	REG_CMD(VRESTART);
	REG_CMD(DIR);
	REG_CMD(EXTRACT);
	REG_CMD(D3DCAPS);
	REG_CMD(VIDMEM);
	//REG_CMD(HARDVRESTART);
	REG_CMD(TEXFORMATS);
	REG_CMD(LOADMODEL);
	REG_CMD(VERSION);
	REG_CMD(CONDUMP);
	REG_CMD(LOADCFG);
	REG_CMD(SAVECFG);
	REG_CMD(LOADMAP);
	REG_CMD(SETMESSAGE);
	REG_CMD(CVARUPDATE);
	REG_CMD(SET);
	REG_CMD(GET);
	REG_CMD(CLEAR);
	REG_CMD(ECHO);
	REG_CMD(CVARLIST);
	REG_CMD(CMDLIST);
	REG_CMD(MUSIC_START);
	REG_CMD(MUSIC_PAUSE);
	REG_CMD(MUSIC_STOP);
	REG_CMD(MUSIC_RESUME);
	REG_CMD(SHOW_INFO);
	REG_CMD(INIT_SRV);
	REG_CMD(SHUTDOWN_SRV);
	REG_CMD(CONNECT);
	REG_CMD(DISCONNECT);
	REG_CMD(PAUSE);
	
	REG_CMD(TEST);
}

/*************************************************************
	static CLGame::LGC_ConCommand()

	This is the command function for the console, by design
	the pExtra parameter is a pointe to the game structure.
	All of the console commands are managed in this function,
	except for the built in commands.
*************************************************************/

lg_bool CLGame::LCG_ConCommand(LC_CMD nCmd, lg_void* args, lg_void* pExtra)
{
	CLGame* pGame=(CLGame*)pExtra;
	switch(nCmd)
	{
	case CONF_PAUSE:
	{
		pGame->m_WorldSrv.TogglePause();
	}
	break;
	case CONF_INIT_SRV:
	{
		pGame->m_WorldSrv.Init();
		//Should reconnect the client?
	}
	break;
	case CONF_SHUTDOWN_SRV:
	{
		pGame->m_WorldSrv.Shutdown();
	}
	break;
	case CONF_CONNECT:
	{
		pGame->m_WorldCln.ConnectLocal(&pGame->m_WorldSrv);
	}
	break;
	case CONF_DISCONNECT:
	{
		pGame->m_WorldCln.Disconnect();
	}
	break;
	case CONF_SHOW_INFO:
	{
		const char szUsage[]="Usage: SHOW_INFO [MESH|SKEL|TEXTURE|ENT|FX|MTR]";
		const lg_char* szType=LC_GetArg(1, args);
		
		if(!szType)
			Err_Printf(szUsage);
		else if(stricmp(szType, "MESH")==0)
			pGame->m_pMMgr->PrintMeshes();
		else if(stricmp(szType, "SKEL")==0)
			pGame->m_pMMgr->PrintSkels();
		else if(stricmp(szType, "TEXTURE")==0)
			pGame->m_pTexMgr->PrintDebugInfo();
		else if(stricmp(szType, "ENT")==0)
			pGame->m_WorldSrv.PrintEntInfo();
		else if(stricmp(szType, "FX")==0)
			pGame->m_pFxMgr->PrintDebugInfo();
		else if(stricmp(szType, "MTR")==0)
			pGame->m_pMtrMgr->PrintDebugInfo();
		else
			Err_Printf(szUsage);
			
		break;
	}
	case CONF_CVARLIST:
		LGC_CvarList(LC_GetArg(1, args));
		break;
	case CONF_TEST:
		LGC_Test(LC_GetArg(1, args));
		break;
	case CONF_SET:
		LGC_Set(LC_GetArg(1, args), LC_GetArg(2, args));
		break;
	case CONF_GET:
		LGC_Get(LC_GetArg(1, args));
		break;
	case CONF_QUIT:
		CV_Set_l(pGame->cv_l_ShouldQuit, 1);
		break;
	case CONF_CVARUPDATE:
		LGC_CvarUpdate(pGame, LC_GetArg(1, args));
		break;
	case CONF_SAVECFG:
		LGC_SaveCFG(LC_GetArg(1, args), LC_CheckArg("APPEND", args), LC_CheckArg("ALL", args));
		break;
	case CONF_LOADCFG:
		LGC_LoadCFG(LC_GetArg(1, args));
		break;
	case CONF_SETMESSAGE:
	{
		const lg_char* szTemp=LC_GetArg(1, args);
		if(szTemp)
			pGame->m_VCon.SetMessage(2, (lg_char*)szTemp);
		break;
	}
	case CONF_VERSION:
		Err_PrintVersion();
		break;
	case CONF_LOADMAP:
	{
		const lg_char* szMapFilename=LC_GetArg(1, args);
		if(szMapFilename)
		{
			lf_path szTemp;
			if(szMapFilename[0]!='/')
			{
				_snprintf(szTemp, LF_MAX_PATH, "/dbase/maps/%s", szMapFilename);
			}
			else
			{
				_snprintf(szTemp, LF_MAX_PATH, "%s", szMapFilename);
			}
			#ifdef OLD_WORLD
			pGame->m_pWorld->LoadMap(szTemp);
			#endif
			pGame->m_WorldSrv.LoadMap(szTemp);
			pGame->m_WorldCln.ConnectLocal(&pGame->m_WorldSrv);
		}
		else
			Err_Printf("Usage: LOADMAP \"filename$\"");
			
		break;
	}
	case CONF_CLEAR:
		LC_Clear();
		break;
	case CONF_CMDLIST:
		LC_ListCommands();
		break;
	case CONF_CONDUMP:
		LGC_ConDump((lg_char*)LC_GetArg(1, args));
		break;
	case CONF_TEXFORMATS:
		//pGame->m_Video.SupportedTexFormats();
		Err_Printf("Not implemented.");
		break;
	case CONF_VIDMEM:
	{
		lg_dword nMem=IDirect3DDevice9_GetAvailableTextureMem(s_pDevice);
		Err_Printf("Estimated available video memory: %uMB", nMem/1024/1024);
		break;
	}
	case CONF_D3DCAPS:
		LGC_DisplayDeviceCaps(pGame);
		break;
	case CONF_EXTRACT:
		LGC_Extract(LC_GetArg(1, args), LC_GetArg(2, args));
		break;
	case CONF_DIR:
		LGC_Dir(LC_GetArg(1, args));
		break;
	case CONF_VRESTART:
		pGame->LV_Restart();
		break;
	/*
	case CONF_HARDVRESTART:
		// Should save game, and shut it down.
		LV_Shutdown(lpGame);
		LV_Init(lpGame);
		// Should load saved game.
		break;
	*/
	case CONF_ECHO:
		LC_Print((lg_char*)LC_GetArg(1, args));
		break;
	//Music control functions...
	case CONF_MUSIC_START:
	{
		lg_str szPath=(lg_str)LC_GetArg(1, args);
		lf_path szAdjPathname;
		if(!szPath)
		{
			Err_Printf("Usage: MUSIC_START \"$filename$\"");
			break;
		}
		if(szPath[0]!='/')
		{
			_snprintf(szAdjPathname, LF_MAX_PATH, "/dbase/music/%s", szPath);
			szPath=szAdjPathname;
		}
		pGame->m_SndMgr.Music_Start(szPath);
		break;
	}
	case CONF_MUSIC_PAUSE:
		pGame->m_SndMgr.Music_Pause();
		break;
	case CONF_MUSIC_STOP:
		pGame->m_SndMgr.Music_Stop();
		break;
	case CONF_MUSIC_RESUME:
		pGame->m_SndMgr.Music_Resume();
		break;
	default:
		return LG_FALSE;
	}
	return LG_TRUE;
}

void CLGame::LGC_CvarUpdate(CLGame* pGame, const lg_char* szCvar)
{
	/* Check to see if a sampler or render state cvar has been
		changed if so, call LV_SetStates.*/
	if(
		L_strnicmp(szCvar, CVAR_v_TextureFilter, 0) || 
		L_strnicmp(szCvar, CVAR_v_MaxAnisotropy, 0) ||
		L_strnicmp(szCvar, CVAR_v_DebugWireframe, 0)
	)
	{
		/* Break if the device is not initialized, this happens because the set command
			gets called when we are loading config.cfg, and the device hasn't been*/
		Err_Printf("Calling LV_SetStates...");
		if(!pGame->m_Video.SetStates())
			Err_Printf("An error occured while setting the sampler and render states.");	
	}
	else if(stricmp(szCvar, CVAR_s_MusicVolume)==0)
	{
		pGame->m_SndMgr.Music_UpdateVolume();
	}
}

void LGC_CvarList(const lg_char* szLimit)
{
	LC_ListCvars(szLimit);
}

void LGC_Get(const lg_char* szCvar)
{
	if(!szCvar)
	{
		Err_Printf("USAGE: get \"$cvarname$\"");
		return;
	}
	
	lg_cvar* cvar=CV_Get(szCvar);
	if(cvar)
		Err_Printf("   %s (\"%s\", %.2f, %d)", cvar->szName, cvar->szValue, cvar->fValue, cvar->nValue);
	else
		Err_Printf("Could not get %s, no such cvar.", szCvar);
}

void LGC_Set(const lg_char* szCvar, const lg_char* szValue)
{
	if(!szCvar || !szValue)
	{
		Err_Printf("USAGE: set \"$cvarname$\" \"$value$\"");
		return;
	}
		
	lg_cvar* cvar=CV_Get(szCvar);
	if(cvar)
	{
		CV_Set(cvar, szValue);
		Err_Printf("   %s (\"%s\", %.2f, %d)", cvar->szName, cvar->szValue, cvar->fValue, cvar->nValue);
		if(L_CHECK_FLAG(cvar->Flags, CVAR_UPDATE))
			LC_SendCommandf("CVARUPDATE %s", cvar->szName);
	}
	else
		Err_Printf("Could not set %s, no such cvar.", szCvar);
}

//LGC Test is only for debugging purposes and it
//is a place in which test code can be placed.
void LGC_Test(const lg_char* szParam)
{
	if(szParam)
	{
		char szTemp[LF_MAX_PATH];
		LF_GetDirFromPath(szTemp, szParam);
		Err_Printf("The path is: \"%s\"", szTemp);
	}
	else
	{
		char szTemp[LF_MAX_PATH];
		LF_GetDirFromPath(szTemp, "");
		Err_Printf("The path is: \"%s\"", szTemp);
	}
	return;
}

//LGC_ConDump dumps the console to the specified file.
void LGC_ConDump(lf_path szFilename)
{
	if(!szFilename)
	{
		Err_Printf("Usage: CONDUMP filename$");
		return;
	}
	
	LF_FILE3 fOut=LF_Open(szFilename, LF_ACCESS_WRITE, LF_CREATE_ALWAYS);
	if(!fOut)
	{
		Err_Printf("CONDUMP Error: Could not open file for writing.");
		return;
	}
	
	Err_Printf("Dumping console to \"%s\"...", szFilename);
	const lg_char* szLine=LC_GetOldestLine();
	
	while(szLine)
	{
		LF_Write(fOut, (lf_void*)szLine, strlen(szLine));
		LF_Write(fOut, "\r\n", 2);
		szLine=LC_GetNextLine();
	}
		
	LF_Close(fOut);
	Err_Printf("Finnished dumping console.");
}

void LGC_LoadCFG(const lg_char* szFilename)
{
	if(!szFilename)
	{
		Err_Printf("Usage: LOADCFG filename$");
		return;
	}
	
	LF_FILE3 fIn=LF_Open(szFilename, LF_ACCESS_READ, LF_OPEN_EXISTING);
	if(!fIn)
	{
		Err_Printf("LOADCFG Error: Could not open file for reading.");
		return;
	}
	Err_Printf("Processing \"%s\"...", szFilename);
	while(!LF_IsEOF(fIn))
	{
		lg_char szLine[512];
		for(lg_dword i=0; i<512; i++)
		{
			if(!LF_Read(fIn, &szLine[i], 1) || szLine[i]=='\n')
			{
				szLine[i]=0;
				break;
			}
		}
		
		//Ignore lines that aren't anything.
		lg_dword nLen=L_strlen(szLine);
		if(nLen<1)
			continue;
		
		//Err_Printf("LINE: %s", szLine);
		
		if(szLine[0]=='\r' || szLine[0]=='\n')
			continue;
			
		/* Check to see for comments, and remove them. */
		for(lg_dword i=0; i<=nLen; i++)
		{
			if((szLine[i]=='/')&&(szLine[i+1]=='/'))
			{
				szLine[i]=0;
				break;
			}
			if(szLine[i]=='\r' || szLine[i]=='\n')
			{
				szLine[i]=0;
				break;
			}
		}
			
		if(szLine[0]==0)
			continue;

			
		LC_SendCommand(szLine);
	}
	LF_Close(fIn);
}

void LGC_SaveCFG(const lg_char* szFilename, lg_bool bAppend, lg_bool bAll)
{
	if(!szFilename)
	{
		Err_Printf("Usage: SAVECFG filename$ [APPEND] [ALL]");
		return;
	}
	LF_FILE3 fOut=LF_Open(szFilename, LF_ACCESS_WRITE, bAppend?LF_OPEN_ALWAYS:LF_CREATE_ALWAYS);
	if(!fOut)
	{
		Err_Printf("SAVECFG Error: Could not open file for writing.");
		return;
	}
	//If appending, seek to the end of the file (and add a new line, just in case
	//we need one.
	if(bAppend)
	{
		LF_Seek(fOut, LF_SEEK_END, 0);
		LF_Write(fOut, "\r\n", 2);
	}
		
	Err_Printf("Saving cvars to \"%s\"...", szFilename);
	
	lg_cvar* cvar=CV_GetFirst();
	while(cvar)
	{
		if(bAll || (L_CHECK_FLAG(cvar->Flags, CVAR_SAVE)) )
		{
			//Write the line...
			LF_Write(fOut, "set \"", 5);
			LF_Write(fOut, cvar->szName, L_strlen(cvar->szName));
			LF_Write(fOut, "\" \"", 3);
			LF_Write(fOut, cvar->szValue, L_strlen(cvar->szValue));
			LF_Write(fOut, "\"\r\n", 3);
		}
		
		cvar=CV_GetNext();
	}
	//In the future key binding data will need to be saved as well.
	Err_Printf("Finnished saving cvars.");
	
	LF_Close(fOut);
}

/*********************************************************
	LGC_Extract()

	Routine for the EXTRACT console command, note that
	extract is designed to extract files from lpks, if
	a output file is specified the file will be renamed
	if not it will maintain it's original path and
	name.  Also note that if extract is called on a file
	that is not in an LPK it will still "extract" the
	in that it will read the file, and write the new file.
**********************************************************/
void LGC_Extract(const lg_char* szFile, const lg_char* szOutFile)
{
	LF_FILE3 fin=LG_NULL;
	LF_FILE3 fout=LG_NULL;
	lg_byte nByte=0;
	lg_dword i=0;

	if(!szFile)
	{
		Err_Printf("Usage: EXTRACT filename$ [outputfile$]");
		return;
	}

	if(!szOutFile)
	{	
		szOutFile=szFile;
	}

	Err_Printf("Extracting \"%s\" to \"%s\"...", szFile, szOutFile);
	/* We have to open with the LF_ACCESS_MEMORY flag, in case our output
		file is a file with the exact same name, this happens if we
		call extract on a file, that is not in an archive. */

	fin=LF_Open(szFile, LF_ACCESS_READ|LF_ACCESS_MEMORY, LF_OPEN_EXISTING);
	if(!fin)
	{
		Err_Printf("Could not open \"%s\".", szFile);
		return;
	}
	
	fout=LF_Open(szOutFile, LF_ACCESS_WRITE, LF_CREATE_NEW);

	if(!fout)
	{
		LF_Close(fin);
		Err_Printf("Could not open \"%s\" for writing.", szOutFile);
		return;
	}
	
	LF_Write(fout, (lf_void*)LF_GetMemPointer(fin), LF_GetSize(fin));
		
	LF_Close(fout);
	LF_Close(fin);
	Err_Printf("Finnished extracting \"%s\"", szFile);
	
	return;
}

void LGC_Dir(const lg_char* szLimit)
{
	//The dir command needs only print the mount info on the file system
	//information will be printed depending on the error level for
	//the file system.
	FS_PrintMountInfo();
	return;
}


/*****************************************************************
	LGC_DisplayDeviceCaps()

	Called by the console command D3DCAPS, this dumps all of the
	D3DCAPS9 information to the console.
*****************************************************************/

void LGC_DisplayDeviceCaps(CLGame* pGame)
{
	D3DCAPS9 d3dcaps;
	D3DADAPTER_IDENTIFIER9 adi;

	memset(&d3dcaps, 0, sizeof(D3DCAPS9));
	lg_result nResult=IDirect3DDevice9_GetDeviceCaps(pGame->s_pDevice, &d3dcaps);
	if(LG_FAILED(nResult))
	{
		Err_PrintDX("IDirect3DDevice9::GetDeviceCaps", nResult);
		return;
	}
		
	memset(&adi, 0, sizeof(adi));
	pGame->s_pD3D->GetAdapterIdentifier(
		pGame->s_nAdapterID,
		0,
		&adi);
			
	char szDeviceType[4];
	nResult=0;

	#define CAPFLAG(a, b) Err_Printf("   "#b"=%s", (L_CHECK_FLAG(d3dcaps.a, b)?"YES":"NO"))

	switch(d3dcaps.DeviceType)
	{
	case D3DDEVTYPE_HAL:
		LG_strncpy(szDeviceType, "HAL", 3);
		break;
	case D3DDEVTYPE_REF:
		LG_strncpy(szDeviceType, "REF", 3);
		break;
	case D3DDEVTYPE_SW:
		LG_strncpy(szDeviceType, "SW", 3);
		break;
	}
	Err_Printf("\"%s\" (%s) Capabilities:", adi.Description, szDeviceType);

	Err_Printf("   Adapter Ordinal: %i", d3dcaps.AdapterOrdinal);
	//Err_Printf("   Caps: %s", L_CHECK_FLAG(d3dcaps.Caps, D3DCAPS_READ_SCANLINE)?"D3DCAPS_READ_SCANLINE":"");
	Err_Printf("Driver Caps:");
	CAPFLAG(Caps, D3DCAPS_READ_SCANLINE);
	Err_Printf("Driver Caps 2:");
	CAPFLAG(Caps2, D3DCAPS2_CANAUTOGENMIPMAP),
	CAPFLAG(Caps2, D3DCAPS2_CANCALIBRATEGAMMA),
	CAPFLAG(Caps2, D3DCAPS2_CANMANAGERESOURCE),
	CAPFLAG(Caps2, D3DCAPS2_DYNAMICTEXTURES),
	CAPFLAG(Caps2, D3DCAPS2_FULLSCREENGAMMA),
	Err_Printf("Driver Caps 3:");
	CAPFLAG(Caps2, D3DCAPS2_RESERVED);
	CAPFLAG(Caps3, D3DCAPS3_ALPHA_FULLSCREEN_FLIP_OR_DISCARD);
	CAPFLAG(Caps3, D3DCAPS3_COPY_TO_VIDMEM);
	CAPFLAG(Caps3, D3DCAPS3_COPY_TO_SYSTEMMEM);
	CAPFLAG(Caps3, D3DCAPS3_LINEAR_TO_SRGB_PRESENTATION);
	CAPFLAG(Caps3, D3DCAPS3_RESERVED);
	Err_Printf("Available Presentation Intervals:");
	CAPFLAG(PresentationIntervals, D3DPRESENT_INTERVAL_IMMEDIATE);
	CAPFLAG(PresentationIntervals, D3DPRESENT_INTERVAL_ONE);
	CAPFLAG(PresentationIntervals, D3DPRESENT_INTERVAL_TWO);
	CAPFLAG(PresentationIntervals, D3DPRESENT_INTERVAL_THREE);
	CAPFLAG(PresentationIntervals, D3DPRESENT_INTERVAL_FOUR);
	Err_Printf("Cursor Caps:");
	CAPFLAG(CursorCaps, D3DCURSORCAPS_COLOR);
	CAPFLAG(CursorCaps, D3DCURSORCAPS_LOWRES);
	Err_Printf("Device Caps:");
	CAPFLAG(DevCaps, D3DDEVCAPS_CANBLTSYSTONONLOCAL);
	CAPFLAG(DevCaps, D3DDEVCAPS_CANRENDERAFTERFLIP);
	CAPFLAG(DevCaps, D3DDEVCAPS_DRAWPRIMITIVES2);
	CAPFLAG(DevCaps, D3DDEVCAPS_DRAWPRIMITIVES2EX);
	CAPFLAG(DevCaps, D3DDEVCAPS_DRAWPRIMTLVERTEX);
	CAPFLAG(DevCaps, D3DDEVCAPS_EXECUTESYSTEMMEMORY);
	CAPFLAG(DevCaps, D3DDEVCAPS_EXECUTEVIDEOMEMORY);
	CAPFLAG(DevCaps, D3DDEVCAPS_HWRASTERIZATION);
	CAPFLAG(DevCaps, D3DDEVCAPS_HWTRANSFORMANDLIGHT);
	CAPFLAG(DevCaps, D3DDEVCAPS_NPATCHES);
	CAPFLAG(DevCaps, D3DDEVCAPS_PUREDEVICE);
	CAPFLAG(DevCaps, D3DDEVCAPS_QUINTICRTPATCHES);
	CAPFLAG(DevCaps, D3DDEVCAPS_RTPATCHES);
	CAPFLAG(DevCaps, D3DDEVCAPS_RTPATCHHANDLEZERO);
	CAPFLAG(DevCaps, D3DDEVCAPS_SEPARATETEXTUREMEMORIES);
	CAPFLAG(DevCaps, D3DDEVCAPS_TEXTURENONLOCALVIDMEM);
	CAPFLAG(DevCaps, D3DDEVCAPS_TEXTURESYSTEMMEMORY);
	CAPFLAG(DevCaps, D3DDEVCAPS_TEXTUREVIDEOMEMORY);
	CAPFLAG(DevCaps, D3DDEVCAPS_TLVERTEXSYSTEMMEMORY);
	CAPFLAG(DevCaps, D3DDEVCAPS_TLVERTEXVIDEOMEMORY);
	Err_Printf("Miscellaneous Driver Primitive Caps:");
	CAPFLAG(PrimitiveMiscCaps, D3DPMISCCAPS_MASKZ);
	CAPFLAG(PrimitiveMiscCaps, D3DPMISCCAPS_CULLNONE);
	CAPFLAG(PrimitiveMiscCaps, D3DPMISCCAPS_CULLCW);
	CAPFLAG(PrimitiveMiscCaps, D3DPMISCCAPS_CULLCCW);
	CAPFLAG(PrimitiveMiscCaps, D3DPMISCCAPS_COLORWRITEENABLE);
	CAPFLAG(PrimitiveMiscCaps, D3DPMISCCAPS_CLIPPLANESCALEDPOINTS);
	CAPFLAG(PrimitiveMiscCaps, D3DPMISCCAPS_CLIPTLVERTS);
	CAPFLAG(PrimitiveMiscCaps, D3DPMISCCAPS_TSSARGTEMP);
	CAPFLAG(PrimitiveMiscCaps, D3DPMISCCAPS_BLENDOP);
	CAPFLAG(PrimitiveMiscCaps, D3DPMISCCAPS_NULLREFERENCE);
	CAPFLAG(PrimitiveMiscCaps, D3DPMISCCAPS_INDEPENDENTWRITEMASKS);
	CAPFLAG(PrimitiveMiscCaps, D3DPMISCCAPS_PERSTAGECONSTANT);
	CAPFLAG(PrimitiveMiscCaps, D3DPMISCCAPS_FOGANDSPECULARALPHA);
	CAPFLAG(PrimitiveMiscCaps, D3DPMISCCAPS_SEPARATEALPHABLEND);
	CAPFLAG(PrimitiveMiscCaps, D3DPMISCCAPS_MRTINDEPENDENTBITDEPTHS);
	CAPFLAG(PrimitiveMiscCaps, D3DPMISCCAPS_MRTPOSTPIXELSHADERBLENDING);
	CAPFLAG(PrimitiveMiscCaps, D3DPMISCCAPS_FOGVERTEXCLAMPED);
	Err_Printf("Raster Drawing Caps:");
	CAPFLAG(RasterCaps, D3DPRASTERCAPS_ANISOTROPY);
	CAPFLAG(RasterCaps, D3DPRASTERCAPS_COLORPERSPECTIVE);
	CAPFLAG(RasterCaps, D3DPRASTERCAPS_DITHER);
	CAPFLAG(RasterCaps, D3DPRASTERCAPS_DEPTHBIAS);
	CAPFLAG(RasterCaps, D3DPRASTERCAPS_FOGRANGE);
	CAPFLAG(RasterCaps, D3DPRASTERCAPS_FOGTABLE);
	CAPFLAG(RasterCaps, D3DPRASTERCAPS_FOGVERTEX);
	CAPFLAG(RasterCaps, D3DPRASTERCAPS_MIPMAPLODBIAS);
	CAPFLAG(RasterCaps, D3DPRASTERCAPS_MULTISAMPLE_TOGGLE);
	CAPFLAG(RasterCaps, D3DPRASTERCAPS_SCISSORTEST);
	CAPFLAG(RasterCaps, D3DPRASTERCAPS_SLOPESCALEDEPTHBIAS);
	CAPFLAG(RasterCaps, D3DPRASTERCAPS_WBUFFER);
	CAPFLAG(RasterCaps, D3DPRASTERCAPS_WFOG);
	CAPFLAG(RasterCaps, D3DPRASTERCAPS_ZBUFFERLESSHSR);
	CAPFLAG(RasterCaps, D3DPRASTERCAPS_ZFOG);
	CAPFLAG(RasterCaps, D3DPRASTERCAPS_ZTEST);
	Err_Printf("Z-Buffer Comparison Caps:");
	CAPFLAG(ZCmpCaps, D3DPCMPCAPS_ALWAYS);
	CAPFLAG(ZCmpCaps, D3DPCMPCAPS_EQUAL);
	CAPFLAG(ZCmpCaps, D3DPCMPCAPS_GREATER);
	CAPFLAG(ZCmpCaps, D3DPCMPCAPS_GREATEREQUAL);
	CAPFLAG(ZCmpCaps, D3DPCMPCAPS_LESS);
	CAPFLAG(ZCmpCaps, D3DPCMPCAPS_LESSEQUAL);
	CAPFLAG(ZCmpCaps, D3DPCMPCAPS_NEVER);
	CAPFLAG(ZCmpCaps, D3DPCMPCAPS_NOTEQUAL);
	Err_Printf("Source Blend Caps:");
	CAPFLAG(SrcBlendCaps, D3DPBLENDCAPS_BLENDFACTOR);
	CAPFLAG(SrcBlendCaps, D3DPBLENDCAPS_BOTHINVSRCALPHA);
	CAPFLAG(SrcBlendCaps, D3DPBLENDCAPS_BOTHSRCALPHA);
	CAPFLAG(SrcBlendCaps, D3DPBLENDCAPS_DESTALPHA);
	CAPFLAG(SrcBlendCaps, D3DPBLENDCAPS_DESTCOLOR);
	CAPFLAG(SrcBlendCaps, D3DPBLENDCAPS_INVDESTALPHA);
	CAPFLAG(SrcBlendCaps, D3DPBLENDCAPS_INVDESTCOLOR);
	CAPFLAG(SrcBlendCaps, D3DPBLENDCAPS_INVSRCALPHA);
	CAPFLAG(SrcBlendCaps, D3DPBLENDCAPS_INVSRCCOLOR);
	CAPFLAG(SrcBlendCaps, D3DPBLENDCAPS_ONE);
	CAPFLAG(SrcBlendCaps, D3DPBLENDCAPS_SRCALPHA);
	CAPFLAG(SrcBlendCaps, D3DPBLENDCAPS_SRCALPHASAT);
	CAPFLAG(SrcBlendCaps, D3DPBLENDCAPS_SRCCOLOR);
	CAPFLAG(SrcBlendCaps, D3DPBLENDCAPS_ZERO);
	Err_Printf("Dest Blend Caps:");
	CAPFLAG(DestBlendCaps, D3DPBLENDCAPS_BLENDFACTOR);
	CAPFLAG(DestBlendCaps, D3DPBLENDCAPS_BOTHINVSRCALPHA);
	CAPFLAG(DestBlendCaps, D3DPBLENDCAPS_BOTHSRCALPHA);
	CAPFLAG(DestBlendCaps, D3DPBLENDCAPS_DESTALPHA);
	CAPFLAG(DestBlendCaps, D3DPBLENDCAPS_DESTCOLOR);
	CAPFLAG(DestBlendCaps, D3DPBLENDCAPS_INVDESTALPHA);
	CAPFLAG(DestBlendCaps, D3DPBLENDCAPS_INVDESTCOLOR);
	CAPFLAG(DestBlendCaps, D3DPBLENDCAPS_INVSRCALPHA);
	CAPFLAG(DestBlendCaps, D3DPBLENDCAPS_INVSRCCOLOR);
	CAPFLAG(DestBlendCaps, D3DPBLENDCAPS_ONE);
	CAPFLAG(DestBlendCaps, D3DPBLENDCAPS_SRCALPHA);
	CAPFLAG(DestBlendCaps, D3DPBLENDCAPS_SRCALPHASAT);
	CAPFLAG(DestBlendCaps, D3DPBLENDCAPS_SRCCOLOR);
	CAPFLAG(DestBlendCaps, D3DPBLENDCAPS_ZERO);
	Err_Printf("Alpha Test Comparison Caps:");
	CAPFLAG(AlphaCmpCaps, D3DPCMPCAPS_ALWAYS);
	CAPFLAG(AlphaCmpCaps, D3DPCMPCAPS_EQUAL);
	CAPFLAG(AlphaCmpCaps, D3DPCMPCAPS_GREATER);
	CAPFLAG(AlphaCmpCaps, D3DPCMPCAPS_GREATEREQUAL);
	CAPFLAG(AlphaCmpCaps, D3DPCMPCAPS_LESS);
	CAPFLAG(AlphaCmpCaps, D3DPCMPCAPS_LESSEQUAL);
	CAPFLAG(AlphaCmpCaps, D3DPCMPCAPS_NEVER);
	CAPFLAG(AlphaCmpCaps, D3DPCMPCAPS_NOTEQUAL);
	Err_Printf("Shading Operations Caps:");
	CAPFLAG(ShadeCaps, D3DPSHADECAPS_ALPHAGOURAUDBLEND);
	CAPFLAG(ShadeCaps, D3DPSHADECAPS_COLORGOURAUDRGB);
	CAPFLAG(ShadeCaps, D3DPSHADECAPS_FOGGOURAUD);
	CAPFLAG(ShadeCaps, D3DPSHADECAPS_SPECULARGOURAUDRGB);
	Err_Printf("Texture Mapping Caps:");
	CAPFLAG(TextureCaps, D3DPTEXTURECAPS_ALPHA);
	CAPFLAG(TextureCaps, D3DPTEXTURECAPS_ALPHAPALETTE);
	CAPFLAG(TextureCaps, D3DPTEXTURECAPS_CUBEMAP);
	CAPFLAG(TextureCaps, D3DPTEXTURECAPS_CUBEMAP_POW2);
	CAPFLAG(TextureCaps, D3DPTEXTURECAPS_MIPCUBEMAP);
	CAPFLAG(TextureCaps, D3DPTEXTURECAPS_MIPMAP);
	CAPFLAG(TextureCaps, D3DPTEXTURECAPS_MIPVOLUMEMAP);
	CAPFLAG(TextureCaps, D3DPTEXTURECAPS_NONPOW2CONDITIONAL);
	CAPFLAG(TextureCaps, D3DPTEXTURECAPS_NOPROJECTEDBUMPENV);
	CAPFLAG(TextureCaps, D3DPTEXTURECAPS_PERSPECTIVE);
	CAPFLAG(TextureCaps, D3DPTEXTURECAPS_POW2);
	CAPFLAG(TextureCaps, D3DPTEXTURECAPS_PROJECTED);
	CAPFLAG(TextureCaps, D3DPTEXTURECAPS_SQUAREONLY);
	CAPFLAG(TextureCaps, D3DPTEXTURECAPS_TEXREPEATNOTSCALEDBYSIZE);
	CAPFLAG(TextureCaps, D3DPTEXTURECAPS_VOLUMEMAP);
	CAPFLAG(TextureCaps, D3DPTEXTURECAPS_VOLUMEMAP_POW2);
	Err_Printf("Texture Filter Caps:");
	CAPFLAG(TextureFilterCaps, D3DPTFILTERCAPS_MINFPOINT);    
	CAPFLAG(TextureFilterCaps, D3DPTFILTERCAPS_MINFLINEAR);        
	CAPFLAG(TextureFilterCaps, D3DPTFILTERCAPS_MINFANISOTROPIC);     
	CAPFLAG(TextureFilterCaps, D3DPTFILTERCAPS_MINFPYRAMIDALQUAD);   
	CAPFLAG(TextureFilterCaps, D3DPTFILTERCAPS_MINFGAUSSIANQUAD); 
	CAPFLAG(TextureFilterCaps, D3DPTFILTERCAPS_MIPFPOINT);  
	CAPFLAG(TextureFilterCaps, D3DPTFILTERCAPS_MIPFLINEAR);        
	CAPFLAG(TextureFilterCaps, D3DPTFILTERCAPS_MAGFPOINT);        
	CAPFLAG(TextureFilterCaps, D3DPTFILTERCAPS_MAGFLINEAR);         
	CAPFLAG(TextureFilterCaps, D3DPTFILTERCAPS_MAGFANISOTROPIC);
	CAPFLAG(TextureFilterCaps, D3DPTFILTERCAPS_MAGFPYRAMIDALQUAD);
	CAPFLAG(TextureFilterCaps, D3DPTFILTERCAPS_MAGFGAUSSIANQUAD);
	Err_Printf("Cube Texture Filter Caps:");
	CAPFLAG(CubeTextureFilterCaps, D3DPTFILTERCAPS_MINFPOINT);    
	CAPFLAG(CubeTextureFilterCaps, D3DPTFILTERCAPS_MINFLINEAR);        
	CAPFLAG(CubeTextureFilterCaps, D3DPTFILTERCAPS_MINFANISOTROPIC);     
	CAPFLAG(CubeTextureFilterCaps, D3DPTFILTERCAPS_MINFPYRAMIDALQUAD);   
	CAPFLAG(CubeTextureFilterCaps, D3DPTFILTERCAPS_MINFGAUSSIANQUAD); 
	CAPFLAG(CubeTextureFilterCaps, D3DPTFILTERCAPS_MIPFPOINT);  
	CAPFLAG(CubeTextureFilterCaps, D3DPTFILTERCAPS_MIPFLINEAR);        
	CAPFLAG(CubeTextureFilterCaps, D3DPTFILTERCAPS_MAGFPOINT);        
	CAPFLAG(CubeTextureFilterCaps, D3DPTFILTERCAPS_MAGFLINEAR);         
	CAPFLAG(CubeTextureFilterCaps, D3DPTFILTERCAPS_MAGFANISOTROPIC);
	CAPFLAG(CubeTextureFilterCaps, D3DPTFILTERCAPS_MAGFPYRAMIDALQUAD);
	CAPFLAG(CubeTextureFilterCaps, D3DPTFILTERCAPS_MAGFGAUSSIANQUAD);
	Err_Printf("Volume Texture Filter Caps:");
	CAPFLAG(VolumeTextureFilterCaps, D3DPTFILTERCAPS_MINFPOINT);    
	CAPFLAG(VolumeTextureFilterCaps, D3DPTFILTERCAPS_MINFLINEAR);        
	CAPFLAG(VolumeTextureFilterCaps, D3DPTFILTERCAPS_MINFANISOTROPIC);     
	CAPFLAG(VolumeTextureFilterCaps, D3DPTFILTERCAPS_MINFPYRAMIDALQUAD);   
	CAPFLAG(VolumeTextureFilterCaps, D3DPTFILTERCAPS_MINFGAUSSIANQUAD); 
	CAPFLAG(VolumeTextureFilterCaps, D3DPTFILTERCAPS_MIPFPOINT);  
	CAPFLAG(VolumeTextureFilterCaps, D3DPTFILTERCAPS_MIPFLINEAR);        
	CAPFLAG(VolumeTextureFilterCaps, D3DPTFILTERCAPS_MAGFPOINT);        
	CAPFLAG(VolumeTextureFilterCaps, D3DPTFILTERCAPS_MAGFLINEAR);         
	CAPFLAG(VolumeTextureFilterCaps, D3DPTFILTERCAPS_MAGFANISOTROPIC);
	CAPFLAG(VolumeTextureFilterCaps, D3DPTFILTERCAPS_MAGFPYRAMIDALQUAD);
	CAPFLAG(VolumeTextureFilterCaps, D3DPTFILTERCAPS_MAGFGAUSSIANQUAD);
	Err_Printf("Texture Address Caps:");
	CAPFLAG(TextureAddressCaps, D3DPTADDRESSCAPS_BORDER);
	CAPFLAG(TextureAddressCaps, D3DPTADDRESSCAPS_CLAMP);
	CAPFLAG(TextureAddressCaps, D3DPTADDRESSCAPS_INDEPENDENTUV);
	CAPFLAG(TextureAddressCaps, D3DPTADDRESSCAPS_MIRROR);
	CAPFLAG(TextureAddressCaps, D3DPTADDRESSCAPS_MIRRORONCE);
	CAPFLAG(TextureAddressCaps, D3DPTADDRESSCAPS_WRAP);
	Err_Printf("Volume Texture Address Caps:");
	CAPFLAG(VolumeTextureAddressCaps, D3DPTADDRESSCAPS_BORDER);
	CAPFLAG(VolumeTextureAddressCaps, D3DPTADDRESSCAPS_CLAMP);
	CAPFLAG(VolumeTextureAddressCaps, D3DPTADDRESSCAPS_INDEPENDENTUV);
	CAPFLAG(VolumeTextureAddressCaps, D3DPTADDRESSCAPS_MIRROR);
	CAPFLAG(VolumeTextureAddressCaps, D3DPTADDRESSCAPS_MIRRORONCE);
	CAPFLAG(VolumeTextureAddressCaps, D3DPTADDRESSCAPS_WRAP);
	Err_Printf("Line Caps:");
	CAPFLAG(LineCaps, D3DLINECAPS_ALPHACMP);
	CAPFLAG(LineCaps, D3DLINECAPS_ANTIALIAS);
	CAPFLAG(LineCaps, D3DLINECAPS_BLEND);
	CAPFLAG(LineCaps, D3DLINECAPS_FOG);
	CAPFLAG(LineCaps, D3DLINECAPS_TEXTURE);
	CAPFLAG(LineCaps, D3DLINECAPS_ZTEST);
	Err_Printf("Device Limits:");
	Err_Printf("   Max Texture Width: %i", d3dcaps.MaxTextureWidth);
	Err_Printf("   Max Texture Height: %i", d3dcaps.MaxTextureHeight);
	Err_Printf("   Max Volume Extent: %i", d3dcaps.MaxVolumeExtent);
	Err_Printf("   Max Texture Repeat: %i", d3dcaps.MaxTextureRepeat);
	Err_Printf("   Max Texture Aspect Ratio: %i", d3dcaps.MaxTextureAspectRatio);
	Err_Printf("   Max Anisotropy: %i", d3dcaps.MaxAnisotropy);
	Err_Printf("   Max W-Based Depth Value: %f", d3dcaps.MaxVertexW);
	Err_Printf("   Guard Band Left: %f", d3dcaps.GuardBandLeft);
	Err_Printf("   Guard Band Top: %f", d3dcaps.GuardBandTop);
	Err_Printf("   Gaurd Band Right: %f", d3dcaps.GuardBandRight);
	Err_Printf("   Guard Band Bottom: %f", d3dcaps.GuardBandBottom);
	Err_Printf("   Extents Adjust: %f", d3dcaps.ExtentsAdjust);
	Err_Printf("Stencil Caps:");
	CAPFLAG(StencilCaps, D3DSTENCILCAPS_KEEP);
	CAPFLAG(StencilCaps, D3DSTENCILCAPS_ZERO);
	CAPFLAG(StencilCaps, D3DSTENCILCAPS_REPLACE);
	CAPFLAG(StencilCaps, D3DSTENCILCAPS_INCRSAT);
	CAPFLAG(StencilCaps, D3DSTENCILCAPS_DECRSAT);
	CAPFLAG(StencilCaps, D3DSTENCILCAPS_INVERT);
	CAPFLAG(StencilCaps, D3DSTENCILCAPS_INCR);
	CAPFLAG(StencilCaps, D3DSTENCILCAPS_DECR);
	CAPFLAG(StencilCaps, D3DSTENCILCAPS_TWOSIDED);
	Err_Printf("FVF Caps:");
	CAPFLAG(FVFCaps, D3DFVFCAPS_DONOTSTRIPELEMENTS);
	CAPFLAG(FVFCaps, D3DFVFCAPS_PSIZE);
	CAPFLAG(FVFCaps, D3DFVFCAPS_TEXCOORDCOUNTMASK);
	Err_Printf("Texture Operation Caps:");
	CAPFLAG(TextureOpCaps, D3DTEXOPCAPS_ADD);
	CAPFLAG(TextureOpCaps, D3DTEXOPCAPS_ADDSIGNED);
	CAPFLAG(TextureOpCaps, D3DTEXOPCAPS_ADDSIGNED2X);
	CAPFLAG(TextureOpCaps, D3DTEXOPCAPS_ADDSMOOTH);
	CAPFLAG(TextureOpCaps, D3DTEXOPCAPS_BLENDCURRENTALPHA);
	CAPFLAG(TextureOpCaps, D3DTEXOPCAPS_BLENDDIFFUSEALPHA);
	CAPFLAG(TextureOpCaps, D3DTEXOPCAPS_BLENDFACTORALPHA);
	CAPFLAG(TextureOpCaps, D3DTEXOPCAPS_BLENDTEXTUREALPHA);
	CAPFLAG(TextureOpCaps, D3DTEXOPCAPS_BLENDTEXTUREALPHAPM);
	CAPFLAG(TextureOpCaps, D3DTEXOPCAPS_BUMPENVMAP);
	CAPFLAG(TextureOpCaps, D3DTEXOPCAPS_BUMPENVMAPLUMINANCE);
	CAPFLAG(TextureOpCaps, D3DTEXOPCAPS_DISABLE);
	CAPFLAG(TextureOpCaps, D3DTEXOPCAPS_DOTPRODUCT3);
	CAPFLAG(TextureOpCaps, D3DTEXOPCAPS_LERP);
	CAPFLAG(TextureOpCaps, D3DTEXOPCAPS_MODULATE);
	CAPFLAG(TextureOpCaps, D3DTEXOPCAPS_MODULATE2X);
	CAPFLAG(TextureOpCaps, D3DTEXOPCAPS_MODULATE4X);
	CAPFLAG(TextureOpCaps, D3DTEXOPCAPS_MODULATEALPHA_ADDCOLOR);
	CAPFLAG(TextureOpCaps, D3DTEXOPCAPS_MODULATECOLOR_ADDALPHA);
	CAPFLAG(TextureOpCaps, D3DTEXOPCAPS_MODULATEINVALPHA_ADDCOLOR);
	CAPFLAG(TextureOpCaps, D3DTEXOPCAPS_MODULATEINVCOLOR_ADDALPHA);
	CAPFLAG(TextureOpCaps, D3DTEXOPCAPS_MULTIPLYADD);
	CAPFLAG(TextureOpCaps, D3DTEXOPCAPS_PREMODULATE);
	CAPFLAG(TextureOpCaps, D3DTEXOPCAPS_SELECTARG1);
	CAPFLAG(TextureOpCaps, D3DTEXOPCAPS_SELECTARG2);
	CAPFLAG(TextureOpCaps, D3DTEXOPCAPS_SUBTRACT);
	Err_Printf("Multi-Texture Limits:");
	Err_Printf("   Max Texture Blend Stages: %i", d3dcaps.MaxTextureBlendStages);
	Err_Printf("   Max Simultaneous Textures: %i", d3dcaps.MaxSimultaneousTextures);
	Err_Printf("Vertex Processing Caps:");
	CAPFLAG(VertexProcessingCaps, D3DVTXPCAPS_DIRECTIONALLIGHTS); 
	CAPFLAG(VertexProcessingCaps, D3DVTXPCAPS_LOCALVIEWER);
	CAPFLAG(VertexProcessingCaps, D3DVTXPCAPS_MATERIALSOURCE7);
	CAPFLAG(VertexProcessingCaps, D3DVTXPCAPS_NO_TEXGEN_NONLOCALVIEWER);
	CAPFLAG(VertexProcessingCaps, D3DVTXPCAPS_POSITIONALLIGHTS);
	CAPFLAG(VertexProcessingCaps, D3DVTXPCAPS_TEXGEN);
	CAPFLAG(VertexProcessingCaps, D3DVTXPCAPS_TEXGEN_SPHEREMAP);
	CAPFLAG(VertexProcessingCaps, D3DVTXPCAPS_TWEENING);
	Err_Printf("Device Limits:");
	Err_Printf("   Max Active Lights: %i", d3dcaps.MaxActiveLights);
	Err_Printf("   Max User Clip Planes: %i", d3dcaps.MaxUserClipPlanes);
	Err_Printf("   Max Vertex Blend Matrices: %i", d3dcaps.MaxVertexBlendMatrices);
	Err_Printf("   Max Vertex Blend Matrix Index: %i", d3dcaps.MaxVertexBlendMatrixIndex);
	Err_Printf("   Max Point Size: %f", d3dcaps.MaxPointSize);
	Err_Printf("   Max Primitive Count: %i", d3dcaps.MaxPrimitiveCount);
	Err_Printf("   Max Vertex Index: %i", d3dcaps.MaxVertexIndex);
	Err_Printf("   Max Streams: %i", d3dcaps.MaxStreams);
	Err_Printf("   Max Stream Stride: %i", d3dcaps.MaxStreamStride);
	Err_Printf("   Vertex Shader Version: 0x%04X 0x%04X", HIWORD(d3dcaps.VertexShaderVersion), LOWORD(d3dcaps.VertexShaderVersion));
	Err_Printf("   Max Vertex Shader Const: %i", d3dcaps.MaxVertexShaderConst);
	Err_Printf("   Pixel Shader Version: 0x%04X 0x%04X", HIWORD(d3dcaps.PixelShaderVersion), LOWORD(d3dcaps.PixelShaderVersion));
	Err_Printf("   Pixel Shader 1x Max Value: %f", d3dcaps.PixelShader1xMaxValue);
	Err_Printf("Device Caps 2:");
	CAPFLAG(DevCaps2, D3DDEVCAPS2_ADAPTIVETESSRTPATCH);
	CAPFLAG(DevCaps2, D3DDEVCAPS2_ADAPTIVETESSNPATCH);
	CAPFLAG(DevCaps2, D3DDEVCAPS2_CAN_STRETCHRECT_FROM_TEXTURES);
	CAPFLAG(DevCaps2, D3DDEVCAPS2_DMAPNPATCH);
	CAPFLAG(DevCaps2, D3DDEVCAPS2_PRESAMPLEDDMAPNPATCH);
	CAPFLAG(DevCaps2, D3DDEVCAPS2_STREAMOFFSET);
	CAPFLAG(DevCaps2, D3DDEVCAPS2_VERTEXELEMENTSCANSHARESTREAMOFFSET);
	Err_Printf("Device Limits:");
	Err_Printf("   Max Npatch Tessellation Level: %f", d3dcaps.MaxNpatchTessellationLevel);
	//Err_Printf("   Min Antialiased Line Width: %i", d3dcaps.MinAntialiasedLineWidth);
	//Err_Printf("   Max Antialiased Line Width: %i", d3dcaps.MaxAntialiasedLineWidth);
	Err_Printf("   Master Adapter Ordinal: %i", d3dcaps.MasterAdapterOrdinal);
	Err_Printf("   Adapter Ordinal In Group: %i", d3dcaps.AdapterOrdinalInGroup);
	Err_Printf("   Number Of Adapters In Group: %i", d3dcaps.NumberOfAdaptersInGroup);
	Err_Printf("Vertex Data Bytes:");
	CAPFLAG(DeclTypes, D3DDTCAPS_UBYTE4);
	CAPFLAG(DeclTypes, D3DDTCAPS_UBYTE4N);
	CAPFLAG(DeclTypes, D3DDTCAPS_SHORT2N);
	CAPFLAG(DeclTypes, D3DDTCAPS_SHORT4N);
	CAPFLAG(DeclTypes, D3DDTCAPS_USHORT2N);
	CAPFLAG(DeclTypes, D3DDTCAPS_USHORT4N);
	CAPFLAG(DeclTypes, D3DDTCAPS_UDEC3);
	CAPFLAG(DeclTypes, D3DDTCAPS_DEC3N);
	CAPFLAG(DeclTypes, D3DDTCAPS_FLOAT16_2);
	CAPFLAG(DeclTypes, D3DDTCAPS_FLOAT16_4);
	Err_Printf("Device Limits:");
	Err_Printf("   Num Simultaneous Render Targets: %i", d3dcaps.NumSimultaneousRTs);
	Err_Printf("StretchRect Filter Caps:");
	CAPFLAG(StretchRectFilterCaps, D3DPTFILTERCAPS_MINFPOINT);
	CAPFLAG(StretchRectFilterCaps, D3DPTFILTERCAPS_MAGFPOINT);
	CAPFLAG(StretchRectFilterCaps, D3DPTFILTERCAPS_MINFLINEAR);
	CAPFLAG(StretchRectFilterCaps, D3DPTFILTERCAPS_MAGFLINEAR);
	Err_Printf("Vertex Shader 2.0 Extended Caps:");
	CAPFLAG(VS20Caps.Caps, D3DVS20CAPS_PREDICATION);
	Err_Printf("   Dynamic Flow Control Depth: %i", d3dcaps.VS20Caps.DynamicFlowControlDepth);
	Err_Printf("   Num Temps: %i", d3dcaps.VS20Caps.NumTemps);
	Err_Printf("   Static Flow Control Depth: %i", d3dcaps.VS20Caps.StaticFlowControlDepth);
	Err_Printf("Pixel Shader 2.0 Extended Caps:");
	CAPFLAG(PS20Caps.Caps, D3DPS20CAPS_ARBITRARYSWIZZLE);
	CAPFLAG(PS20Caps.Caps, D3DPS20CAPS_GRADIENTINSTRUCTIONS);
	CAPFLAG(PS20Caps.Caps, D3DPS20CAPS_PREDICATION);
	CAPFLAG(PS20Caps.Caps, D3DPS20CAPS_NODEPENDENTREADLIMIT);
	CAPFLAG(PS20Caps.Caps, D3DPS20CAPS_NOTEXINSTRUCTIONLIMIT);
	Err_Printf("   Dynamic Flow Control Depth: %i", d3dcaps.PS20Caps.DynamicFlowControlDepth);
	Err_Printf("   Num Temps: %i", d3dcaps.PS20Caps.NumTemps);
	Err_Printf("   Static Flow Control Depth: %i", d3dcaps.PS20Caps.StaticFlowControlDepth);
	Err_Printf("   Num Instruction Slots: %i", d3dcaps.PS20Caps.NumInstructionSlots);
	
	Err_Printf("Vertex Texture Filter Caps:");
	CAPFLAG(VertexTextureFilterCaps, D3DPTFILTERCAPS_MINFPOINT);    
	CAPFLAG(VertexTextureFilterCaps, D3DPTFILTERCAPS_MINFLINEAR);        
	CAPFLAG(VertexTextureFilterCaps, D3DPTFILTERCAPS_MINFANISOTROPIC);     
	CAPFLAG(VertexTextureFilterCaps, D3DPTFILTERCAPS_MINFPYRAMIDALQUAD);   
	CAPFLAG(VertexTextureFilterCaps, D3DPTFILTERCAPS_MINFGAUSSIANQUAD); 
	CAPFLAG(VertexTextureFilterCaps, D3DPTFILTERCAPS_MIPFPOINT);  
	CAPFLAG(VertexTextureFilterCaps, D3DPTFILTERCAPS_MIPFLINEAR);        
	CAPFLAG(VertexTextureFilterCaps, D3DPTFILTERCAPS_MAGFPOINT);        
	CAPFLAG(VertexTextureFilterCaps, D3DPTFILTERCAPS_MAGFLINEAR);         
	CAPFLAG(VertexTextureFilterCaps, D3DPTFILTERCAPS_MAGFANISOTROPIC);
	CAPFLAG(VertexTextureFilterCaps, D3DPTFILTERCAPS_MAGFPYRAMIDALQUAD);
	CAPFLAG(VertexTextureFilterCaps, D3DPTFILTERCAPS_MAGFGAUSSIANQUAD);

	Err_Printf("Device Limits:");
	Err_Printf("   Max Vertex Shader Instructions Executed: %i", d3dcaps.MaxVShaderInstructionsExecuted);
	Err_Printf("   Max Pixel Shader Instructions Executed: %i", d3dcaps.MaxPShaderInstructionsExecuted);
	Err_Printf("   Max Vertex Shader 3.0 Instruction Slots: %i", d3dcaps.MaxVertexShader30InstructionSlots);
	Err_Printf("   Max Pixel Shader 3.0 Instruction Slots: %i", d3dcaps.MaxPixelShader30InstructionSlots);
}