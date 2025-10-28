/**************************************************************
	File: lg_cmd.c
	Copyright (c) 2006, Blaine Myers

	Purpose: Contains the command function for the console.
	It also contains the function that registers the console
	commands, and various functions that are called by console
	commands.
**************************************************************/
#include "common.h"
#include <stdio.h>
#include <io.h>
//#include "lv_init.h"
//#include "lv_reset.h"
#include "lg_cmd.h"
#include "lg_sys_2.h"
#include <lf_sys.h>
#include "lg_err.h"
#include "lv_tex.h"

#include "lv_test.h"

#define MAX_T_LEN (1024)

int LGC_DisplayDeviceCaps(char* szDeviceName, D3DCAPS9* lpcaps);
int LGC_Dir(const char* szParams);
int LGC_Extract(const char* szParams);
int LGC_Test(char* szParam);

/**************************************************************
	LGC_RegConCmds()

	Called from LG_Init(), it registers all of the console
	commands so that when a user types one it it will actually
	get processed.  As sort of a standard the value of all
	console commands start with CONF_ then are appended with
	the function name.
**************************************************************/
int LGC_RegConCmds(HLCONSOLE hConsole)
{
	#define REG_CMD(name) Con_RegisterCmd(hConsole, #name, CONF_##name)

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
	
	REG_CMD(TEST);
	return 1;
}

/*************************************************************
	LGC_ConCommand()

	This is the command function for the console, by design
	the pExtra parameter is a pointe to the game structure.
	All of the console commands are managed in this function,
	except for the built in commands.
*************************************************************/
int LGC_ConCommand(unsigned long nCommand, const char* szParams, HLCONSOLE hConsole, void* pExtra)
{
	int nResult=1;
	CLGame* lpGame=(CLGame*)pExtra;
	/* This function isn't tested, so I don't know if it works or not. */
	switch(nCommand)
	{
	case CONF_TEST:
	{
		char szTemp[1024];
		if(CCParse_GetParam(szTemp, szParams, 1))
			LGC_Test(szTemp);
		else
			LGC_Test(L_null);
		break;
	}
	case CONF_VERSION:
	{
		Err_PrintVersion();
		break;
	}
	case CONF_LOADMODEL:
	{
		/*char szFilename[1024];
		if(CCParse_GetParam(szFilename, szParams, 1))
			LVT_LoadModel(szFilename);
		else*/
			Err_Printf("Usage: LOADMODEL filename$");
		break;
	}
	case CONF_TEXFORMATS:
		lpGame->LV_SupportedTexFormats(
			lpGame->m_cvars, 
			lpGame->m_pD3D, 
			lpGame->m_nAdapterID, 
			lpGame->m_nDeviceType, 
			lpGame->m_nBackBufferFormat);
		break;
	case CONF_VIDMEM:
	{
		L_dword nMem=IDirect3DDevice9_GetAvailableTextureMem(lpGame->m_pDevice);
		Err_Printf("Estimated available video memory: %iMB", nMem/1024/1024);
		break;
	}
	case CONF_D3DCAPS:
	{
		D3DCAPS9 d3dcaps;
		D3DADAPTER_IDENTIFIER9 adi;

		memset(&d3dcaps, 0, sizeof(D3DCAPS9));
		nResult=IDirect3DDevice9_GetDeviceCaps(lpGame->m_pDevice, &d3dcaps);
		if(L_failed(nResult))
		{
			Err_PrintDX("IDirect3DDevice9::GetDeviceCaps", nResult);
			return 0;
		}
		
		memset(&adi, 0, sizeof(adi));
		lpGame->m_pD3D->GetAdapterIdentifier(
			lpGame->m_nAdapterID,
			0,
			&adi);

		LGC_DisplayDeviceCaps(adi.Description, &d3dcaps);
		nResult=1;
		break;
	}
	case CONF_EXTRACT:
		LGC_Extract(szParams);
		break;
	case CONF_DIR:
		LGC_Dir(szParams);
		break;
	/*
	case CONF_HARDVRESTART:
		// Should save game, and shut it down.
		LV_Shutdown(lpGame);
		LV_Init(lpGame);
		// Should load saved game.
		break;
	*/
	case CONF_VRESTART:
		lpGame->LV_Restart();
		break;
	case CONF_CVARUPDATE:
	{
		/* Means the game console told us a cvar was updated, so we may need to
			change a setting. */
		char szCVarName[MAX_T_LEN];
		if(!CCParse_GetParam(szCVarName, szParams, 1))
		{
			Err_Printf("Could not get name of cvar to update.");
			break;
		}
		/* Check to see if a sampler or render state cvar has been
			changed if so, call LV_SetStates.*/
		if(
			L_strnicmp(szCVarName, FILTER_MODE, 0) || 
			L_strnicmp(szCVarName, FILTER_MAXANISOTROPY, 0) ||
			L_strnicmp(szCVarName, "v_DebugWireframe", 0))
		{
			/* Break if the device is not initialized, this happens because the set command
				gets called when we are loading config.cfg, and the device hasn't been
				initialized yet. */
			if(!lpGame->m_pDevice)
				break;
			else
			{
				Err_Printf("Calling LV_SetStates...");
				if(!lpGame->LV_SetStates())
					Err_Printf("An error occured while setting the sampler and render states.");
			}
			
		}
		break;
	}
	case CONF_QUIT:
		CVar_Set(lpGame->m_cvars, "l_ShouldQuit", "TRUE");
		break;
	default:
		nResult=0;
		break;
	}
	return nResult;
}

//LGC Test is only for debugging purposes and it
//is a place in which test code can be placed.
int LGC_Test(char* szParam)
{
	if(szParam)
	{
		char szTemp[MAX_F_PATH];
		L_GetPathFromPath(szTemp, szParam);
		Err_Printf("The path is: \"%s\"", szTemp);
	}
	else
	{
		char szTemp[MAX_F_PATH];
		L_GetPathFromPath(szTemp, "");
		Err_Printf("The path is: \"%s\"", szTemp);
	}
	return 0;
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
int LGC_Extract(const char* szParams)
{
	LF_FILE2 fin=L_null;
	LF_FILE2 fout=L_null;
	L_byte nByte=0;
	L_dword i=0;

	char szFile[MAX_T_LEN];
	char szOutputFile[MAX_T_LEN];
	char szOutputDir[MAX_T_LEN];

	if(!CCParse_GetParam(szFile, szParams, 1))
	{
		Err_Printf("Usage: EXTRACT filename$ [outputfile$]");
		return 1;
	}

	if(!CCParse_GetParam(szOutputFile, szParams, 2))
	{	
		_snprintf(szOutputFile, MAX_T_LEN-1, "%s", szFile);
	}

	/* Make sure the output directory exists. */
	for(i=L_strlen(szOutputFile); i>=0; i--)
	{
		if(szOutputFile[i]=='\\')
		{
			L_strncpy(szOutputDir, szFile, i+1);
			break;
		}
		if(i==0)
		{
			L_strncpy(szOutputDir, ".\\", MAX_T_LEN-1);
			break;
		}
	}
	L_mkdir(szOutputDir);

	Err_Printf("Extracting \"%s\" to \"%s\"...", szFile, szOutputFile);
	/* We have to open with the LF_ACCESS_MEMORY flag, in case our output
		file is a file with the exact same name, this happens if we
		call extract on a file, that is not in an archive. */
	fin=File_Open(szFile, 0, LF_ACCESS_READ|LF_ACCESS_MEMORY, LFCREATE_OPEN_EXISTING);
	if(!fin)
	{
		Err_Printf("Could not open \"%s\".", szFile);
		return 1;
	}
	fout=File_Open(szOutputFile, 0, LF_ACCESS_WRITE, LFCREATE_CREATE_NEW);

	if(!fout)
	{
		File_Close(fin);
		Err_Printf("Could not open \"%s\" for writing.", szOutputFile);
		return 1;
	}

	/* This is one way to copy a file,
		it isn't the fastest though. */
	while(File_Read(fin, 1, &nByte))
	{
		File_Write(fout, 1, &nByte);
	}
	File_Close(fout);
	File_Close(fin);
	Err_Printf("Finnished extracting \"%s\"", szFile);
	return 1;
}

/***************************************************
	LGC_Dir() && LGC_Files()

	Functions that display all of the files in the
	search path.  LGC_Dir is the master function,
	and will first show all files in archives, and
	then LGC_Files goes through all the files on 
	the disk.
***************************************************/
int LGC_Files(char* szPath, char* szLimit)
{
	long Handle=0;
	struct _finddata_t data;
	char szDir[MAX_T_LEN];
	L_bool bLimit=L_false;
	szDir[0]=0;

	bLimit=L_strlen(szLimit);

	memset(&data, 0, sizeof(data));

	_snprintf(szDir, MAX_T_LEN-1, "%s*.*", szPath);
	Handle=(long)_findfirst(szDir, &data);
	if(Handle==-1)
		return L_false;

	do
	{
		if(L_CHECK_FLAG(data.attrib, _A_SUBDIR))
		{
			if(L_strnicmp("..", data.name, 0) || (L_strnicmp(".", data.name, 0)))
				continue;
			_snprintf(szDir, MAX_T_LEN-1, "%s%s\\", szPath, data.name);
			LGC_Files(szDir, szLimit);
		}
		else
		{
			_snprintf(szDir, MAX_T_LEN-1, "%s%s", szPath+2, data.name);
			if(bLimit)
			{
				if(L_strnicmp(szLimit, szDir, L_strlen(szLimit)))
					Err_Printf("   \"%s\", DISK", szDir);
			}
			else
				Err_Printf("   \"%s\", DISK", szDir);
		}
	}
	while(_findnext(Handle, &data)!=-1);

	_findclose(Handle);
	return L_true;
}

int LGC_Dir(const char* szParams)
{
	char szLimit[MAX_T_LEN];

	szLimit[0]=0;
	CCParse_GetParam(szLimit, szParams, 1);
	Err_Printf("Directory of \"%s\"", LF_GetDir(L_null, 0));
	//Err_Printf("Files found in archives:");
	LF_ShowPaths(szLimit, Err_Printf);
	//Err_Printf("Files found on disk:");
	LGC_Files(".\\", szLimit);
	return 1;
}


/*****************************************************************
	LGC_DisplayDeviceCaps()

	Called by the console command D3DCAPS, this dumps all of the
	D3DCAPS9 information to the console.
*****************************************************************/

int LGC_DisplayDeviceCaps(char* szDeviceName, D3DCAPS9* lpcaps)
{
	D3DCAPS9 d3dcaps=*lpcaps;
	char szDeviceType[4];
	L_result nResult=0;

	if(!lpcaps)
		return 0;

	#define CAPFLAG(a, b) Err_Printf("   "#b"=%s", (L_CHECK_FLAG(d3dcaps.a, b)?"YES":"NO"))

	switch(d3dcaps.DeviceType)
	{
	case D3DDEVTYPE_HAL:
		L_strncpy(szDeviceType, "HAL", 4);
		break;
	case D3DDEVTYPE_REF:
		L_strncpy(szDeviceType, "REF", 4);
		break;
	case D3DDEVTYPE_SW:
		L_strncpy(szDeviceType, "SW", 4);
		break;
	}
	Err_Printf("\"%s\" (%s) Capabilities:", szDeviceName, szDeviceType);

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

	return 1;

}