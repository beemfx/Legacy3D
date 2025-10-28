/* lf_bdio.c - Basic Disk Input Output. These functions should be used
	internally only, and not by outside systems.
	
	Copyright (C) 2007 Blaine Myers
*/
/* NOTE: The bdio functions should be altered according to the operating
	system that the lf_sys2 library is being compiled for, this file
	is designed for windows operating systems (in all practicality the
	fopen, fclose, etc stdc functions could have been used, but the
	windows methods prove more stable for windows computers.
*/
#include <windows.h>
#include "lf_bdio.h"

/* Basic DIO stuff, should only be used internally only. */

typedef struct _BDIO{
	HANDLE hFile;
}BDIO;

BDIO_FILE BDIO_OpenA(lf_cstr szFilename, BDIO_CREATE_MODE nMode, lf_dword nAccess)
{
	BDIO* pFile=LF_NULL;
	lf_dword dwDesiredAccess=0;
	lf_dword dwCreateDisposition=0;
	lf_dword dwFlagsAndAttributes=0;
	dwDesiredAccess|=LF_CHECK_FLAG(nAccess, BDIO_ACCESS_READ)?GENERIC_READ:0;
	dwDesiredAccess|=LF_CHECK_FLAG(nAccess, BDIO_ACCESS_WRITE)?GENERIC_WRITE:0;
	
	pFile=LF_Malloc(sizeof(BDIO));
	if(!pFile)
		return LF_NULL;
	
	pFile->hFile=LF_NULL;
	
	switch(nMode)
	{
	case BDIO_CREATE_ALWAYS:
		dwCreateDisposition=CREATE_ALWAYS;
		break;
	case BDIO_CREATE_NEW:
		dwCreateDisposition=CREATE_NEW;
		break;
	case BDIO_OPEN_ALWAYS:
		dwCreateDisposition=OPEN_ALWAYS;
		break;
	case BDIO_OPEN_EXISTING:
		dwCreateDisposition=OPEN_EXISTING;
		break;
	}
	
	dwFlagsAndAttributes=FILE_ATTRIBUTE_NORMAL;
	if(LF_CHECK_FLAG(nAccess, BDIO_ACCESS_TEMP))
	{
		dwFlagsAndAttributes=FILE_FLAG_DELETE_ON_CLOSE|FILE_ATTRIBUTE_TEMPORARY;
	}
	
	pFile->hFile=CreateFileA(
		szFilename, 
		dwDesiredAccess,
		FILE_SHARE_READ,
		NULL,
		dwCreateDisposition,
		dwFlagsAndAttributes,
		NULL);
		
	if(pFile->hFile==INVALID_HANDLE_VALUE)
	{
		LF_Free(pFile);
		return LF_NULL;
	}
	return pFile;
}
BDIO_FILE BDIO_OpenW(lf_cwstr szFilename, BDIO_CREATE_MODE nMode, lf_dword nAccess)
{
	BDIO* pFile=LF_NULL;
	lf_dword dwDesiredAccess=0;
	lf_dword dwCreateDisposition=0;
	lf_dword dwFlagsAndAttributes=0;
	dwDesiredAccess|=LF_CHECK_FLAG(nAccess, BDIO_ACCESS_READ)?GENERIC_READ:0;
	dwDesiredAccess|=LF_CHECK_FLAG(nAccess, BDIO_ACCESS_WRITE)?GENERIC_WRITE:0;
	
	pFile=LF_Malloc(sizeof(BDIO));
	if(!pFile)
		return LF_NULL;
		
	pFile->hFile=LF_NULL;
	
	switch(nMode)
	{
	case BDIO_CREATE_ALWAYS:
		dwCreateDisposition=CREATE_ALWAYS;
		break;
	case BDIO_CREATE_NEW:
		dwCreateDisposition=CREATE_NEW;
		break;
	case BDIO_OPEN_ALWAYS:
		dwCreateDisposition=OPEN_ALWAYS;
		break;
	case BDIO_OPEN_EXISTING:
		dwCreateDisposition=OPEN_EXISTING;
		break;
	}
	
	dwFlagsAndAttributes=FILE_ATTRIBUTE_NORMAL;
	if(LF_CHECK_FLAG(nAccess, BDIO_ACCESS_TEMP))
	{
		dwFlagsAndAttributes=FILE_FLAG_DELETE_ON_CLOSE|FILE_ATTRIBUTE_TEMPORARY;
	}
	
	pFile->hFile=CreateFileW(
		szFilename, 
		dwDesiredAccess,
		FILE_SHARE_READ,
		NULL,
		dwCreateDisposition,
		dwFlagsAndAttributes,
		NULL);
		
	if(pFile->hFile==INVALID_HANDLE_VALUE)
	{
		LF_Free(pFile);
		return LF_NULL;
	}
	return pFile;
}
lf_bool BDIO_Close(BDIO_FILE File)
{
	BDIO* pFile=File;
	lf_bool bResult=CloseHandle(pFile->hFile);
	LF_Free(pFile);
	return bResult;
}

lf_dword BDIO_Read(BDIO_FILE File, lf_dword nSize, void* pOutBuffer)
{
	BDIO* pFile=File;
	lf_dword dwRead=0;
	if(ReadFile(pFile->hFile, pOutBuffer, nSize, &dwRead, NULL))
		return dwRead;
	else
		return 0;
}


lf_dword BDIO_Write(BDIO_FILE File, lf_dword nSize, void* pInBuffer)
{
	BDIO* pFile=File;
	lf_dword dwWrote=0;
	if(WriteFile(pFile->hFile, pInBuffer, nSize, &dwWrote, NULL))
		return dwWrote;
	else
		return 0;
}

lf_dword BDIO_Tell(BDIO_FILE File)
{
	BDIO* pFile=File;
	return SetFilePointer(
		pFile->hFile,
		0,
		NULL,
		FILE_CURRENT);
}

lf_dword BDIO_Seek(BDIO_FILE File, lf_long nOffset, BDIO_SEEK_MODE nOrigin)
{
	BDIO* pFile=File;
	lf_dword dwMoveMethod=0;
	switch(nOrigin)
	{
	case BDIO_SEEK_BEGIN:
		dwMoveMethod=FILE_BEGIN;
		break;
	case BDIO_SEEK_CURRENT:
		dwMoveMethod=FILE_CURRENT;
		break;
	case BDIO_SEEK_END:
		dwMoveMethod=FILE_END;
		break;
	}
	return SetFilePointer(
		pFile->hFile,
		nOffset,
		NULL,
		dwMoveMethod);
}

lf_dword BDIO_GetSize(BDIO_FILE File)
{
	BDIO* pFile=File;
	lf_large_integer nFileSize={0,0};
	nFileSize.dwLowPart=GetFileSize(pFile->hFile, &nFileSize.dwHighPart);
	if(nFileSize.dwHighPart)
		return 0xFFFFFFFF;
	else
		return nFileSize.dwLowPart;
}

BDIO_FILE LF_SYS2_EXPORTS BDIO_OpenTempFileA(BDIO_CREATE_MODE nMode, lf_dword nAccess)
{
	lf_pathA szTempPath, szTempFile;
	GetTempPathA(LF_MAX_PATH, szTempPath);
	GetTempFileNameA(szTempPath, "BDIO", 0, szTempFile);
	return BDIO_OpenA(szTempFile, nMode, nAccess|BDIO_ACCESS_TEMP);
}
BDIO_FILE LF_SYS2_EXPORTS BDIO_OpenTempFileW(BDIO_CREATE_MODE nMode, lf_dword nAccess)
{
	lf_pathW szTempPath, szTempFile;
	GetTempPathW(LF_MAX_PATH, szTempPath);
	GetTempFileNameW(szTempPath, L"BDIO", 0, szTempFile);
	return BDIO_OpenW(szTempFile, nMode, nAccess|BDIO_ACCESS_TEMP);
}

/* Directory manipulation...*/

lf_bool LF_SYS2_EXPORTS BDIO_ChangeDirA(lf_cstr szDir)
{
	return SetCurrentDirectoryA(szDir);
}
lf_bool LF_SYS2_EXPORTS BDIO_ChangeDirW(lf_cwstr szDir)
{
	return SetCurrentDirectoryW(szDir);
}

lf_dword LF_SYS2_EXPORTS BDIO_GetCurrentDirA(lf_str szDir, lf_dword nSize)
{
	return GetCurrentDirectoryA(nSize, szDir);
}
lf_dword LF_SYS2_EXPORTS BDIO_GetCurrentDirW(lf_wstr szDir, lf_dword nSize)
{
	return GetCurrentDirectoryW(nSize, szDir);
}

/* BDIO_GetFullPathname converts a filename to it's full path according to the 
	current directory
*/
lf_dword LF_SYS2_EXPORTS BDIO_GetFullPathNameW(lf_pathW szPath, const lf_pathW szFilename)
{
	return GetFullPathNameW(szFilename, LF_MAX_PATH, szPath, NULL);
}
lf_dword LF_SYS2_EXPORTS BDIO_GetFullPathNameA(lf_pathA szPath, const lf_pathA szFilename)
{
	return GetFullPathNameA(szFilename, LF_MAX_PATH, szPath, NULL);;
}

//Find File functions for finding all files in a directory.
BDIO_FIND LF_SYS2_EXPORTS BDIO_FindFirstFileW(lf_cwstr szFilename, BDIO_FIND_DATAW* pFindData)
{
	HANDLE hFindFile;
	WIN32_FIND_DATAW findData;
	
	hFindFile=FindFirstFileW(szFilename, &findData);
	if(INVALID_HANDLE_VALUE==hFindFile)
		return LF_NULL;
	
	pFindData->nFileSize.dwHighPart=findData.nFileSizeHigh;
	pFindData->nFileSize.dwLowPart=findData.nFileSizeLow;
	wcsncpy(pFindData->szFilename, findData.cFileName, LF_MAX_PATH);
	pFindData->bNormal=LF_CHECK_FLAG(findData.dwFileAttributes, FILE_ATTRIBUTE_NORMAL);
	pFindData->bDirectory=LF_CHECK_FLAG(findData.dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY);
	pFindData->bHidden=LF_CHECK_FLAG(findData.dwFileAttributes, FILE_ATTRIBUTE_HIDDEN);
	pFindData->bReadOnly=LF_CHECK_FLAG(findData.dwFileAttributes, FILE_ATTRIBUTE_READONLY);
	return hFindFile;
}
lf_bool LF_SYS2_EXPORTS BDIO_FindNextFileW(BDIO_FIND hFindFile, BDIO_FIND_DATAW* pFindData)
{
	WIN32_FIND_DATAW findData;
	lf_bool bResult=FindNextFileW((HANDLE)hFindFile, &findData);
	if(!bResult)
		return bResult;
	
	pFindData->nFileSize.dwHighPart=findData.nFileSizeHigh;
	pFindData->nFileSize.dwLowPart=findData.nFileSizeLow;
	wcsncpy(pFindData->szFilename, findData.cFileName, LF_MAX_PATH);
	pFindData->bNormal=LF_CHECK_FLAG(findData.dwFileAttributes, FILE_ATTRIBUTE_NORMAL);
	pFindData->bDirectory=LF_CHECK_FLAG(findData.dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY);
	pFindData->bHidden=LF_CHECK_FLAG(findData.dwFileAttributes, FILE_ATTRIBUTE_HIDDEN);
	pFindData->bReadOnly=LF_CHECK_FLAG(findData.dwFileAttributes, FILE_ATTRIBUTE_READONLY);
	return bResult;
}


BDIO_FIND LF_SYS2_EXPORTS BDIO_FindFirstFileA(lf_cstr szFilename, BDIO_FIND_DATAA* pFindData)
{
	HANDLE hFindFile;
	WIN32_FIND_DATAA findData;
	
	hFindFile=FindFirstFileA(szFilename, &findData);
	if(INVALID_HANDLE_VALUE==hFindFile)
		return LF_NULL;
	
	pFindData->nFileSize.dwHighPart=findData.nFileSizeHigh;
	pFindData->nFileSize.dwLowPart=findData.nFileSizeLow;
	strncpy(pFindData->szFilename, findData.cFileName, LF_MAX_PATH);
	pFindData->bNormal=LF_CHECK_FLAG(findData.dwFileAttributes, FILE_ATTRIBUTE_NORMAL);
	pFindData->bDirectory=LF_CHECK_FLAG(findData.dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY);
	pFindData->bHidden=LF_CHECK_FLAG(findData.dwFileAttributes, FILE_ATTRIBUTE_HIDDEN);
	pFindData->bReadOnly=LF_CHECK_FLAG(findData.dwFileAttributes, FILE_ATTRIBUTE_READONLY);
	return hFindFile;
}
lf_bool LF_SYS2_EXPORTS BDIO_FindNextFileA(BDIO_FIND hFindFile, BDIO_FIND_DATAA* pFindData)
{
	WIN32_FIND_DATAA findData;
	lf_bool bResult=FindNextFileA((HANDLE)hFindFile, &findData);
	if(!bResult)
		return bResult;
	
	pFindData->nFileSize.dwHighPart=findData.nFileSizeHigh;
	pFindData->nFileSize.dwLowPart=findData.nFileSizeLow;
	strncpy(pFindData->szFilename, findData.cFileName, LF_MAX_PATH);
	pFindData->bNormal=LF_CHECK_FLAG(findData.dwFileAttributes, FILE_ATTRIBUTE_NORMAL);
	pFindData->bDirectory=LF_CHECK_FLAG(findData.dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY);
	pFindData->bHidden=LF_CHECK_FLAG(findData.dwFileAttributes, FILE_ATTRIBUTE_HIDDEN);
	pFindData->bReadOnly=LF_CHECK_FLAG(findData.dwFileAttributes, FILE_ATTRIBUTE_READONLY);
	return bResult;
}

lf_bool LF_SYS2_EXPORTS BDIO_FindClose(BDIO_FIND hFindFile)
{
	return FindClose((HANDLE)hFindFile);
}