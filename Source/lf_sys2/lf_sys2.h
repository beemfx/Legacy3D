#ifndef __LF_SYS2_H__
#define __LF_SYS2_H__
/* lf_sys2 - The Legacy File System version 2.0 */

/********************************
Definitions for the file system
********************************/

//#define ARC_VERSION 102	
//#define ARC_TYPE    0x314B504C //(*(lf_dword*)"LPK1")

#define MOUNT_MOUNT_SUBDIRS         0x00000002
#define MOUNT_FILE_OVERWRITE        0x00000004
#define MOUNT_FILE_OVERWRITELPKONLY 0x00000008

/*********************************
	Definitions for file access
*********************************/

typedef enum _LF_SEEK_TYPE{
	LF_SEEK_BEGIN=0,
	LF_SEEK_CURRENT=1,
	LF_SEEK_END=2
}LF_SEEK_TYPE;
	
typedef enum _LF_CREATE_MODE{
	LF_CREATE_ALWAYS=1,
	LF_CREATE_NEW=2,
	LF_OPEN_ALWAYS=3,
	LF_OPEN_EXISTING=4
}LF_CREATE_MODE;
	
#define LF_ACCESS_READ   0x00000002 //Open file for reading.
#define LF_ACCESS_WRITE  0x00000004 //Open file for writing.
#define LF_ACCESS_MEMORY 0x00000008 //Open file and access it from memory, file must be read only.
/********************************
	Disable some of VCs warnings
*********************************/
#pragma warning(disable: 4267)
#pragma warning(disable: 4996)

#if LF_SYS2_BUILD_DLL
#define LF_SYS2_EXPORTS __declspec(dllexport)
#else
#define LF_SYS2_EXPORTS
#endif

#ifdef __cplusplus
extern "C"{
#endif __cplusplus

/* File System Types */
typedef signed int lf_int, *lf_pint;
typedef unsigned int lf_uint, *lf_puint;
typedef char lf_char, *lf_pchar;
typedef unsigned char lf_byte, *lf_pbyte;
typedef signed char lf_sbyte, *lf_psbyte;
typedef signed char lf_schar, *lf_pschar;
typedef char *lf_str, *lf_pstr, *lf_lpstr;
typedef const char *lf_cstr, *lf_pcstr, *lf_lpcstr;
typedef unsigned short lf_word, *lf_pword, lf_ushort, *lf_pushort;
typedef signed short lf_short, *lf_pshort;
typedef unsigned long lf_dword, *lf_pdword, lf_ulong, *lf_pulong;
typedef signed long lf_long, *lf_plong;
typedef float lf_float, *lf_pfloat;
typedef double lf_double, *lf_pdouble;
typedef long double lf_ldouble, *lf_lpdouble;
typedef void lf_void, *lf_pvoid;
typedef const void lf_cvoid, *lf_pcvoid;
typedef unsigned int lf_size_t;

#ifdef __cplusplus
typedef wchar_t wchar_t_d;
#else
typedef unsigned short wchar_t_d;
#endif

typedef wchar_t_d lf_wchar, lf_wchar_t;
typedef lf_wchar_t *lf_wstr, *lf_pwstr, *lf_lpwstr;
typedef const lf_wchar_t *lf_cwstr, *lf_pcwstr, *lf_lpcwstr;

typedef struct _lf_large_integer{
	lf_dword dwLowPart;
	lf_long dwHighPart;
}lf_large_integer, *lf_plarge_integer, *lf_lplarge_integer;

typedef int lf_bool;
#define LF_TRUE  (1)
#define LF_FALSE (0)

#ifdef __cplusplus
#define LF_NULL (0)
#else __cplusplus
#define LF_NULL ((void*)0)
#endif __cplusplus

#define LF_CHECK_FLAG(flag, var) ((flag&var))

#define LF_MAX_PATH 255
typedef lf_char lf_pathA[LF_MAX_PATH+1];
typedef lf_wchar lf_pathW[LF_MAX_PATH+1];

#ifdef UNICODE
#define lf_path lf_pathW
#else !UNICODE
#define lf_path lf_pathA
#endif UNICODE

#define LF_max(v1, v2) ((v1)>(v2))?(v1):(v2)
#define LF_min(v1, v2) ((v1)<(v2))?(v1):(v2)
#define LF_clamp(v1, min, max) ( (v1)>(max)?(max):(v1)<(min)?(min):(v1) )

#define LF_SAFE_DELETE_ARRAY(p) if(p){delete [] p; p=LF_NULL;}
#define LF_SAFE_DELETE(p) if(p){delete p; p=LF_NULL;}

#define LF_SAFE_FREE(p) if(p){LF_Free(p);p=LF_NULL;}

//Path name parsing functions...
lf_str  LF_SYS2_EXPORTS LF_GetFileNameFromPathA(lf_str szFileName, lf_cstr szFullPath);
lf_wstr LF_SYS2_EXPORTS LF_GetFileNameFromPathW(lf_wstr szFileName, lf_cwstr szFullPath);
lf_str  LF_SYS2_EXPORTS LF_GetShortNameFromPathA(lf_str szName, lf_cstr szFullPath);
lf_wstr LF_SYS2_EXPORTS LF_GetShortNameFromPathW(lf_wstr szName, lf_cwstr szFullPath);
lf_str  LF_SYS2_EXPORTS LF_GetDirFromPathA(lf_str szDir, lf_cstr szFullPath);
lf_wstr LF_SYS2_EXPORTS LF_GetDirFromPathW(lf_wstr szDir, lf_cwstr szFullPath);

/* Memory Functions */
typedef lf_void* ( * LF_ALLOC_FUNC)(lf_size_t size);
typedef lf_void  ( * LF_FREE_FUNC)(lf_void* p);
lf_void* LF_Malloc(lf_size_t size);
lf_void  LF_Free(lf_void* p);
lf_void  LF_SetMemFuncs(LF_ALLOC_FUNC pAlloc, LF_FREE_FUNC pFree);


#ifdef UNICODE
#define LF_GetFileNameFromPath  LF_GetFileNameFromPathW
#define LF_GetShortNameFromPath LF_GetShortNameFromPathW
#define LF_GetDirFromPath       LF_GetDirFromPathW
#else UNICODE
#define LF_GetFileNameFromPath  LF_GetFileNameFromPathA
#define LF_GetShortNameFromPath LF_GetShortNameFromPathA
#define LF_GetDirFromPath       LF_GetDirFromPathA
#endif UNICODE


/*************************
	Error Handling Stuff
*************************/
typedef enum _LF_ERR_LEVEL{
	ERR_LEVEL_UNKNOWN=0, //Messages are always printed.
	ERR_LEVEL_SUPERDETAIL=1,
	ERR_LEVEL_DETAIL,
	ERR_LEVEL_NOTICE,
	ERR_LEVEL_WARNING,
	ERR_LEVEL_ERROR,
	ERR_LEVEL_ALWAYS //Message is is printed.
}LF_ERR_LEVEL;

void LF_SYS2_EXPORTS LF_SetErrLevel(LF_ERR_LEVEL nLevel);
void LF_SYS2_EXPORTS LF_ErrPrintW(lf_cwstr szFormat, LF_ERR_LEVEL nErrLevel, ...);
void LF_SYS2_EXPORTS LF_ErrPrintA(lf_cstr szFormat, LF_ERR_LEVEL nErrLevel, ...);
void LF_SYS2_EXPORTS __cdecl LF_DefaultErrPrint(lf_cwstr szFormat);
void LF_SYS2_EXPORTS LF_SetErrPrintFn(void (__cdecl*pfn)(lf_cwstr szFormat));

typedef void (__cdecl * LF_ERR_CALLBACK)(lf_cwstr);

#ifdef UNICODE
#define LF_ErrPrint LF_ErrPrintW
#else !UNICODE
#define LF_ErrPrint LF_ErrPrintA
#endif UNICODE


/**************************
	File System Functions
	Mounting, etc.
**************************/

lf_bool  LF_SYS2_EXPORTS FS_Initialize(lf_dword nMaxFiles, LF_ALLOC_FUNC pAlloc, LF_FREE_FUNC pFree);
lf_dword LF_SYS2_EXPORTS FS_MountBaseA(lf_cstr szOSPath);
lf_dword LF_SYS2_EXPORTS FS_MountBaseW(lf_cwstr szOSPath);
lf_dword LF_SYS2_EXPORTS FS_MountA(lf_cstr szOSPath, lf_cstr szMountPath, lf_dword Flags);
lf_dword LF_SYS2_EXPORTS FS_MountW(lf_cwstr szOSPath, lf_cwstr szMountPath, lf_dword Flags);
lf_dword LF_SYS2_EXPORTS FS_MountLPKA(lf_cstr szMountedFile, lf_dword Flags);
lf_dword LF_SYS2_EXPORTS FS_MountLPKW(lf_cwstr szMountedFile, lf_dword Flags);
lf_bool  LF_SYS2_EXPORTS FS_UnMountAll();
lf_bool  LF_SYS2_EXPORTS FS_Shutdown();
void     LF_SYS2_EXPORTS FS_PrintMountInfo();

#ifdef UNICODE
#define FS_MountBase FS_MountBaseW
#define FS_Mount     FS_MountW
#define FS_MountLPK  FS_MountLPKW
#else !UNICODE
#define FS_MountBase FS_MountBaseA
#define FS_Mount     FS_MountA
#define FS_MountLPK  FS_MountLPKA
#endif UNICODE

/********************************************
	File Access Functions
	Opening, Closing, Reading, Writing, etc.
********************************************/
typedef lf_pvoid LF_FILE3;

LF_FILE3  LF_SYS2_EXPORTS LF_OpenA(lf_cstr szFilename, lf_dword Access, lf_dword CreateMode);
LF_FILE3  LF_SYS2_EXPORTS LF_OpenW(lf_cwstr szFilename, lf_dword Access, lf_dword CreateMode);
lf_bool   LF_SYS2_EXPORTS LF_Close(LF_FILE3 File);
lf_dword  LF_SYS2_EXPORTS LF_Read(LF_FILE3 File, lf_void* pOutBuffer, lf_dword nSize);
lf_dword  LF_SYS2_EXPORTS LF_Write(LF_FILE3 File, lf_void* pInBuffer, lf_dword nSize);
lf_dword  LF_SYS2_EXPORTS LF_Tell(LF_FILE3 File);
lf_dword  LF_SYS2_EXPORTS LF_Seek(LF_FILE3 File, LF_SEEK_TYPE nMode, lf_long nOffset);
lf_dword  LF_SYS2_EXPORTS LF_GetSize(LF_FILE3 File);
lf_pcvoid LF_SYS2_EXPORTS LF_GetMemPointer(LF_FILE3 File);
lf_bool   LF_SYS2_EXPORTS LF_IsEOF(LF_FILE3 File);

#ifdef UNICODE
#define LF_Open LF_OpenW
#else !UNICODE
#define LF_Open LF_OpenA
#endif UNICODE

#ifdef __cplusplus
} //extern "C"
#endif __cplusplus

#endif __LF_SYS2_H__
