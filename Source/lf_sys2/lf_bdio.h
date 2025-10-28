#ifndef _LF_BDIO_H__
#define _LF_BDIO_H__
#include "lf_sys2.h"

#ifdef __cplusplus
extern "C"{
#endif __cplusplus

/* Basic DIO stuff, should only be used internally only. */
typedef enum _BDIO_SEEK_MODE
{
	BDIO_SEEK_BEGIN=LF_SEEK_BEGIN,
	BDIO_SEEK_CURRENT=LF_SEEK_CURRENT,
	BDIO_SEEK_END=LF_SEEK_END
}BDIO_SEEK_MODE;

#define BDIO_ACCESS_READ  LF_ACCESS_READ
#define BDIO_ACCESS_WRITE LF_ACCESS_WRITE
#define BDIO_ACCESS_TEMP  0x00000080

typedef enum _BDIO_CREATE_MODE
{
	BDIO_CREATE_ALWAYS=LF_CREATE_ALWAYS, //Create a new file, deleting the old file if the file exists.
	BDIO_CREATE_NEW=LF_CREATE_NEW,       //Create a new file, failing if the file already exists.
	BDIO_OPEN_ALWAYS=LF_OPEN_ALWAYS,     //Open a file, creating a new file if the file doesn't exist.
	BDIO_OPEN_EXISTING=LF_OPEN_EXISTING  //OPen a file, failing if the file does not exist.
}BDIO_CREATE_MODE;

typedef void* BDIO_FILE;
BDIO_FILE LF_SYS2_EXPORTS BDIO_OpenA(lf_cstr szFilename, BDIO_CREATE_MODE nMode, lf_dword nAccess);
BDIO_FILE LF_SYS2_EXPORTS BDIO_OpenW(lf_cwstr szFilename, BDIO_CREATE_MODE nMode, lf_dword nAccess);

#ifdef UNICODE
#define BDIO_Open BDIO_OpenW
#else UNICODE
#define BDIO_Open BDIO_OpenA
#endif UNICODE

lf_bool LF_SYS2_EXPORTS BDIO_Close(BDIO_FILE File);
lf_dword LF_SYS2_EXPORTS BDIO_Read(BDIO_FILE File, lf_dword nSize, void* pOutBuffer);
lf_dword LF_SYS2_EXPORTS BDIO_Write(BDIO_FILE File, lf_dword nSize, void* pInBuffer);
lf_dword LF_SYS2_EXPORTS BDIO_Tell(BDIO_FILE File);
lf_dword LF_SYS2_EXPORTS BDIO_Seek(BDIO_FILE File, lf_long nOffset, BDIO_SEEK_MODE nOrigin);
lf_dword LF_SYS2_EXPORTS BDIO_GetSize(BDIO_FILE File);

lf_dword LF_SYS2_EXPORTS BDIO_WriteCompressed(BDIO_FILE File, lf_dword nSize, void* pInBuffer);
lf_dword LF_SYS2_EXPORTS BDIO_ReadCompressed(BDIO_FILE File, lf_dword nSize, void* pOutBuffer);

lf_bool LF_SYS2_EXPORTS BDIO_ChangeDirA(lf_cstr szDir);
lf_bool LF_SYS2_EXPORTS BDIO_ChangeDirW(lf_cwstr szDir);
lf_dword LF_SYS2_EXPORTS BDIO_GetCurrentDirA(lf_str szDir, lf_dword nSize);
lf_dword LF_SYS2_EXPORTS BDIO_GetCurrentDirW(lf_wstr szDir, lf_dword nSize);

BDIO_FILE LF_SYS2_EXPORTS BDIO_OpenTempFileA(BDIO_CREATE_MODE nMode, lf_dword nAccess);
BDIO_FILE LF_SYS2_EXPORTS BDIO_OpenTempFileW(BDIO_CREATE_MODE nMode, lf_dword nAccess);

lf_dword LF_SYS2_EXPORTS BDIO_CopyData(BDIO_FILE DestFile, BDIO_FILE SourceFile, lf_dword nSize);

lf_dword LF_SYS2_EXPORTS BDIO_GetFullPathNameW(lf_pathW szPath, const lf_pathW szFilename);
lf_dword LF_SYS2_EXPORTS BDIO_GetFullPathNameA(lf_pathA szPath, const lf_pathA szFilename);

#ifdef UNICODE
#define BDIO_ChangeDir BDIO_ChangeDirW
#define BDIO_GetCurrentDir BDIO_GetCurrentDirW
#define BDIO_OpenTempFile BDIO_OpenTempFileW
#define BDIO_GetFullPathName BDIO_GetFullPathNameW
#else UNICODE
#define BDIO_ChangeDir BDIO_ChangeDirA
#define BDIO_GetCurrentDir BDIO_GetCurrentDirA
#define BDIO_OpenTempFile BDIO_OpenTempFileA
#define BDIO_GetFullPathName BDIO_GetFullPathNameA
#endif UNICODE

typedef void* BDIO_FIND;

typedef struct _BDIO_FIND_DATAW{
	lf_bool bDirectory;
	lf_bool bReadOnly;
	lf_bool bHidden;
	lf_bool bNormal;
	lf_large_integer nFileSize;
	lf_pathW szFilename;
}BDIO_FIND_DATAW;

typedef struct _BDIO_FIND_DATAA{
	lf_bool bDirectory;
	lf_bool bReadOnly;
	lf_bool bHidden;
	lf_bool bNormal;
	lf_large_integer nFileSize;
	lf_pathA szFilename;
}BDIO_FIND_DATAA;

BDIO_FIND LF_SYS2_EXPORTS BDIO_FindFirstFileW(lf_cwstr szFilename, BDIO_FIND_DATAW* pFindData);
lf_bool LF_SYS2_EXPORTS BDIO_FindNextFileW(BDIO_FIND hFindFile, BDIO_FIND_DATAW* pFindData);
BDIO_FIND LF_SYS2_EXPORTS BDIO_FindFirstFileA(lf_cstr szFilename, BDIO_FIND_DATAA* pFindData);
lf_bool LF_SYS2_EXPORTS BDIO_FindNextFileA(BDIO_FIND hFindFile, BDIO_FIND_DATAA* pFindData);
lf_bool LF_SYS2_EXPORTS BDIO_FindClose(BDIO_FIND hFindFile);

#ifdef __cplusplus
}
#endif __cplusplus

#endif _LF_BDIO_H__