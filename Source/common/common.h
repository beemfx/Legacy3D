#ifndef __COMMON_H__
#define __COMMON_H__

#ifdef _DEBUG
#define CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#endif//_DEBUG

#ifdef __cplusplus
extern "C"{
#endif /* __cplusplus */

#include "lg_types.h"

/********************************
	Disable some of VCs warnings
*********************************/
#pragma warning(disable: 4267)
#pragma warning(disable: 4996)

#define L_CHECK_FLAG(var, flag) ((var&flag))

#define LG_Max(v1, v2) ((v1)>(v2))?(v1):(v2)
#define LG_Min(v1, v2) ((v1)<(v2))?(v1):(v2)
#define LG_Clamp(v1, min, max) ( (v1)>(max)?(max):(v1)<(min)?(min):(v1) )

/******************************* 
	Legacy Engine Helper Macros.
********************************/
/* L_safe_free is a macro to safely free allocated memory. */
#ifndef L_safe_free
#define L_safe_free(p) {if(p){LG_Free(p);(p)=LG_NULL;}}
#endif /* SAFE_FREE */

#ifndef L_safe_delete_array
#define L_safe_delete_array(p) {if(p){delete [] (p);(p)=LG_NULL;}}
#endif //L_safe_delete_array

#ifndef L_safe_delete
#define L_safe_delete(p) {if(p){delete (p);(p)=LG_NULL;}}
#endif //L_safe_delete

/* L_safe_release is to release com objects safely. */
#ifdef __cplusplus
#define L_safe_release(p) {if(p){(p)->Release();(p)=LG_NULL;}}
#else /* __cplusplus */
#define L_safe_release(p) {if(p){(p)->lpVtbl->Release((p));(p)=LG_NULL;}}
#endif /* __cplusplus */


/****************************************
	Legacy Engine replacement functions.
****************************************/

unsigned long L_strncpy(char* szOut, const char* szIn, unsigned long nMax);
char* L_strncat(char* szDest, const char* szSrc, unsigned long nMax);
int L_strnicmp(const char* szOne, const char* szTwo, unsigned long nNum);
float L_atovalue(char* string);
signed long L_atol(char* string);
float L_atof(char* string);
unsigned long L_axtol(char* string);
lg_dword L_strlenA(const lg_char* string);
lg_dword L_strlenW(const lg_wchar* string);
int L_mkdir(const char* szDir);
char* L_strtokA(char* strToken, char* strDelimit, char cIgIns);
lg_wchar* L_strtokW(lg_wchar* strToken, lg_wchar* strDelimit, lg_wchar cIgIns);

#ifdef UNICODE
#define L_strlen L_strlenW
#else !UNICODE
#define L_strlen L_strlenA
#endif

/*************************************
	Legacy Engine special functions.
*************************************/

//L_GetPathFromPath returns the [drive:\]path\to\file portion of a [drive:\]path\to\file\name.ext.
//eg. c:\My Documents\readme.txt ==> c:\My Documents\ OR base\textures\face.tga ==> base\textures\.
//includes the trailing \ or /.
char* L_GetPathFromPath(char* szDir, const char* szFilename);
//L_GetShortNameFromPath returns the name portion of a [drive:\]path\to\file\name.ext.
//eg. c:\My Documents\readme.txt ==> readme OR base\textures\face.tga ==> face.
lg_char* L_GetShortNameFromPath(lg_char* szName, const char* szFilename);
//Same as above but with the extension.
lg_char* L_GetNameFromPath(lg_char* szName, const char* szFilename);

void Debug_printfA(lg_char* format, ...);
void Debug_printfW(lg_wchar_t* format, ...);
#ifdef _UNICODE
#define Debug_printf Debug_printfW
#else _UNICODE
#define Debug_printf Debug_printfA
#endif _UNICODE

#ifdef _DEBUG
#define L_BEGIN_D_DUMP {_CrtMemState s1, s2, s3;_CrtMemCheckpoint(&s1);
#define L_END_D_DUMP(txt) _CrtMemCheckpoint(&s2);\
							     _CrtMemDifference(&s3, &s1, &s2);\
							     OutputDebugString("MEMORY USAGE FOR: \""txt"\":\n");\
	                       _CrtMemDumpStatistics(&s3);}
#else /*_DEBUG*/
#define L_BEGIN_D_DUMP
#define L_END_D_DUMP(txt)
#endif /*_DEBUG*/

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /*__COMMON_H__*/