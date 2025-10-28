/****************************************************
	File: lf_sys.c
	Copyright (c) 2006, Blaine Myers 
	
	Purpose: Functions that control disk input (and
	possibly in the future, output) for the Legacy
	File System. Functions starting with LF_ are
	used to effect the usage of the file system.
	Functions starting with File_ are used to access
	files within the file system.
****************************************************/

#include "common.h"
#include <stdio.h>
#include <direct.h>
#include <math.h>
#include <memory.h>
#include <malloc.h>
#include "lf_sys.h"
#include "lf_sys.h"

#include <windows.h>

/* A few global variables are used for the file system,
	one to indicate if the file system has been initialized
	another to store the current working directory, and also
	one to store the SEARCHPATHS. */
L_bool g_bFSInited=L_false;
char g_szCurDir[MAX_F_PATH];

SEARCHPATHS* g_SearchPaths=L_null;

L_bool g_bArchivesFirst=L_false;

/*********************************************************
	The LF_* functions are used to manipulate the file
	system, they initialize, shutdown, and can change
	the working directory.
**********************************************************/
/************************************************
	LF_SetError() && LF_GetLastError()

	Get and set the error text.
************************************************/
char g_szErrorText[1024];
void LF_SetError(char* format, ...)
{
	va_list arglist;
	va_start(arglist, format);
	_vsnprintf(g_szErrorText, 1023, format, arglist);
	va_end(arglist);
}
char* LF_GetLastError()
{
	return g_szErrorText;
}

/**********************************************************************
	LF_GetPath()

	This function goes through the search paths until it finds
	the specified filename.  If it finds it, it returns information
	about that file.  If it doesn't it returns L_null.
**********************************************************************/
SEARCHPATHS* LF_GetPath(const char* szFilename)
{
	SEARCHPATHS* lpPath=L_null;
	
	for(lpPath=g_SearchPaths; lpPath; lpPath=lpPath->m_next)
	{
		if(L_strnicmp((char*)szFilename, lpPath->m_szNameInLibrary, 0))
			return lpPath;
	}
	LF_SetError("LF_GetPath: No errors detected.");
	return L_null;
}

/*****************************************************************
	LF_ShowPaths()

	Calls Err_Printf for all of the files contained within the
	various archives.  This function is currently called when
	a console command is isued by the user.  A search limit string
	is allowed which will limit the filenames that are displayed
	to those beginning with the limit.  This takes an output 
	function as the second parameter which would be compatible
	with printf or Err_Printf.
******************************************************************/
L_bool LF_ShowPaths(const char* szLimit, L_int (*OutputFunc)(const char*, ...))
{
	SEARCHPATHS* lpPath=L_null;
	L_bool bLimit=L_false;

	FS_CHECK_EXIT

	bLimit=L_strlen(szLimit);
	for(lpPath=g_SearchPaths; lpPath; lpPath=lpPath->m_next)
	{
		if(L_strnicmp((char*)szLimit, lpPath->m_szFilename, L_strlen(szLimit)) || !bLimit)
			OutputFunc("   \"%s\", ARCHIVED\n", lpPath->m_szFilename);
	}
	LF_SetError("LF_ShowPaths: No problems detected.");
	return L_true;
}

/******************************************************************
	LF_Init()

	Initializes the file system and sets the current directory to
	the sepecified path.
******************************************************************/
L_bool LF_Init(char* dir, L_dword Flags)
{
	g_bFSInited=L_true;
	LF_ChangeDir(dir);
	if(L_CHECK_FLAG(Flags, LFINIT_CHECKARCHIVESFIRST))
		g_bArchivesFirst=L_true;
	else
		g_bArchivesFirst=L_false;
	LF_SetError("LF_Init: File System created.  Starting in \"%s\".", LF_GetDir(L_null, MAX_F_PATH));
	return L_true;
}

/********************************************************
	LF_Shutdown()

	Shuts down the file system, and deletes the archive
	index.
********************************************************/
L_bool LF_Shutdown()
{
	FS_CHECK_EXIT

	LF_CloseAllArchives();
	g_bFSInited=L_false;
	g_szCurDir[0]=0;

	LF_SetError("LF_Shutdown: No problems detected.");
	return L_true;
}

/**********************************************************************
	LF_AddArchive()
	
	This function opens a legacy pak file, and stores the
	name of all the files that are in it, that way when the File_Open
	function trys to open a file, it will check to see if it is in a 
	pack first. 
**********************************************************************/

L_bool LF_AddArchive(L_lpstr szFilename)
{
	HLARCHIVE hArchive=L_null;
	L_dword dwNumFiles=0;
	L_dword i=0;
	L_dword nOffset=0;
	SEARCHPATHS* lpNewPath=L_null;

	FS_CHECK_EXIT

	hArchive=Arc_Open(szFilename);
	if(!hArchive)
	{
		LF_SetError("Could not add \"%s\" to search path.", szFilename);
		return L_false;
	}
	
	dwNumFiles=Arc_GetNumFiles(hArchive);
	for(i=1; i<=dwNumFiles; i++)
	{
		/* Check to see if there is a file already by that name in the search path,
			if there is we replace it with the new one. */
		lpNewPath=LF_GetPath(Arc_GetFilename(hArchive, i));
		if(lpNewPath)
		{
			L_strncpy(lpNewPath->m_szLibrary, szFilename, MAX_F_PATH);
			continue;
		}

		lpNewPath=malloc(sizeof(SEARCHPATHS));
		if(lpNewPath)
		{
			L_strncpy(lpNewPath->m_szNameInLibrary, (char*)Arc_GetFilename(hArchive, i), MAX_F_PATH);
			L_strncpy(lpNewPath->m_szLibrary, szFilename, MAX_F_PATH);
			if(lpNewPath->m_szNameInLibrary[0]=='.')
				nOffset=2;
			else
				nOffset=0;
					
			L_strncpy(lpNewPath->m_szFilename, lpNewPath->m_szNameInLibrary+nOffset, MAX_F_PATH);
			lpNewPath->m_next=g_SearchPaths;
			g_SearchPaths=lpNewPath;
		}
	}
	Arc_Close(hArchive);
	LF_SetError("Added \"%s\" to search path.", szFilename);
	return L_true;
}

/************************************************************************
	LF_CloseAllArchives()

	Deletes all of the search path information, this should usually be
	called from LF_Shutdown, but for various reasons could be called
	from elswhere in the code.
************************************************************************/
L_bool LF_CloseAllArchives()
{
	SEARCHPATHS* lpPath=L_null;
	SEARCHPATHS* lpTemp=L_null;
	FS_CHECK_EXIT

	/* Just delete all the search paths. */
	for(lpPath=g_SearchPaths; lpPath; )
	{
		lpTemp=lpPath->m_next;
		free(lpPath);
		lpPath=lpTemp;
	}
	g_SearchPaths=L_null;
	LF_SetError("LF_CloseAllArchives: No errors detected.");
	return L_true;
}

/*******************************************************
	LF_ChangeDir()

	Changes the working directory to the specified one.
*******************************************************/
L_bool LF_ChangeDir(const char* dirname)
{
	FS_CHECK_EXIT

	_chdir(dirname);
	/* We need to call File_GetDir, to update our directory string. */
	LF_GetDir(L_null, 0);
	LF_SetError("LF_ChangeDir: No errors detected.");
	return L_true;
}

/*********************************************
	LF_GetDir()

	Returns the current working directory.
*********************************************/
char* LF_GetDir(char* buffer, int nMaxLen)
{
	FS_CHECK_EXIT
	/* Whenever we get the dir we always renew it
		with _getdcwd, to make sure it is correct. */
	if(!nMaxLen)
		nMaxLen=MAX_F_PATH;

	_getdcwd(_getdrive(), g_szCurDir, MAX_F_PATH);
	LF_SetError("LF_GetDir: No errors detected.");
	if(buffer)
	{
		L_strncpy(buffer, g_szCurDir, nMaxLen);
		return buffer;
	}
	else
	{
		return g_szCurDir;
	}
}
