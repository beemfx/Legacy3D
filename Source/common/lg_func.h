/* lg_func.h - Additional functions for the usage
	in the legacy engine.
*/
#ifndef __LG_FUNC_H__
#define __LG_FUNC_H__

#include "lg_types.h"

/********************************
	Disable some of VCs warnings
*********************************/
#pragma warning(disable: 4267)
#pragma warning(disable: 4996)

#ifdef __cplusplus
extern "C"{
#endif __cplusplus

#define LG_CheckFlag(var, flag) (var&flag)
#define LG_SetFlag(var, flag)   (var|=flag)
#define LG_UnsetFlag(var, flag) (var&=~flag)

#define LG_Max(v1, v2) ((v1)>(v2))?(v1):(v2)
#define LG_Min(v1, v2) ((v1)<(v2))?(v1):(v2)
#define LG_Clamp(v1, min, max) ( (v1)>(max)?(max):(v1)<(min)?(min):(v1) )

/******************************* 
	Legacy Engine Helper Macros.
********************************/
/* L_safe_free is a macro to safely free allocated memory. */
#ifndef LG_SafeFree
#define LG_SafeFree(p) {if(p){LG_Free(p);(p)=LG_NULL;}}
#endif /* LG_SafeFree */

#ifndef LG_SafeDeleteArray
#define LG_SafeDeleteArray(p) {if(p){delete [] (p);(p)=LG_NULL;}}
#endif /* LG_SafeDeleteArray */

#ifndef LG_SafeDelete
#define LG_SafeDelete(p) {if(p){delete (p);(p)=LG_NULL;}}
#endif /* LG_SafeDelete */

/* L_safe_release is to release com objects safely. */
#ifdef __cplusplus
#define LG_SafeRelease(p) {if(p){(p)->Release();(p)=LG_NULL;}}
#else /* __cplusplus */
#define LG_SafeRelease(p) {if(p){(p)->lpVtbl->Release((p));(p)=LG_NULL;}}
#endif /* __cplusplus */

/* LG_strncpy & LG_wcsncpy
	Notes: Same as c runtime functions strncpy & wcsncpy but 
	insures that a null termination character is appended.
*/
lg_char* LG_strncpy(lg_char* strDest, const lg_char* strSrc, lg_size_t count);
lg_wchar_t* LG_wcsncpy(lg_wchar_t* strDest, const lg_wchar_t* strSrc, lg_size_t count);


/* Just a filename manipulation function, gets just the name of the
	file and not the path or extension.
*/
lg_char* LG_GetShortNameFromPathA(lg_char* szName, const lg_char* szFilename);


/*LG_RandomSeed
	PRE:N/A
	POST: Sets the seed for the LG_Random* functions.
*/
void LG_RandomSeed(lg_uint Seed);
/*LG_RandomFloat
	PRE:Any float values are fine.
	POST:A random number float is returned with the
	value between fMin and fMax, inclusive.
*/
lg_float LG_RandomFloat(lg_float fMin, lg_float fMax);
/*LG_RandomLong
	PRE:Any long values are fine.
	POST: A random long is returned between nMin and nMac
	inclusive.
*/
lg_long LG_RandomLong(lg_long nMin, lg_long nMax);

/*LG_Hash - hasing Function*/
lg_dword LG_Hash(lg_char* szString);
/*LG_HashFilename - Hashes only the filename, ignores directories.*/
lg_dword LG_HashFilename(lg_char* szPath);

#ifdef __cplusplus
} /* extern "C" */
#endif __cplusplus

#endif __LG_FUNC_H__