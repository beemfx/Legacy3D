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

/***************************
	Legacy Engine types.
****************************/
typedef signed int L_int, *L_pint;
typedef unsigned int L_uint, *L_puint;
typedef unsigned char L_char, *L_pchar;
typedef unsigned char L_byte, *L_pbyte;
typedef signed char L_sbyte, *L_psbyte;
typedef signed char L_schar, *L_pschar;
typedef char *L_str, *L_pstr, *L_lpstr;
typedef unsigned short L_word, *L_pword, L_ushort, *L_pushort;
typedef signed short L_short, *L_pshort;
typedef unsigned long L_dword, *L_pdword, L_ulong, *L_pulong;
typedef signed long L_long, *L_plong;
typedef float L_float, *L_pfloat;
typedef double L_double, *L_pdouble;
typedef long double L_ldouble, *L_lpdouble;
typedef void L_void, *L_pvoid;


typedef struct _L_large_integer{
	L_dword dwLowPart;
	L_long dwHighPart;
}L_large_integer, *L_plarge_integer, *L_lplarge_integer;

typedef struct _L_rect{
	L_long left;
	L_long top;
	L_long right;
	L_long bottom;
}L_rect, *L_prect;

typedef int L_bool;
#define L_true  (1)
#define L_false (0)

typedef long L_result;
#define L_succeeded(Status) ((L_result)(Status)>=0)
#define L_failed(Status) ((L_result)(Status)<0)

/******************************
	Legacy Engine definitions.
*******************************/

#ifdef __cplusplus
#define L_null (0)
#else /* __cplusplus */
#define L_null ((void*)0)
#endif __cplusplus


#define L_CHECK_FLAG(var, flag) ((var&flag))

#define L_max(v1, v2) ((v1)>(v2))?(v1):(v2)
#define L_min(v1, v2) ((v1)<(v2))?(v1):(v2)
#define L_clamp(v1, min, max) ( (v1)>(max)?(max):(v1)<(min)?(min):(v1) )

/******************************* 
	Legacy Engine Helper Macros.
********************************/
/* L_safe_free is a macro to safely free allocated memory. */
#ifndef L_safe_free
#define L_safe_free(p) {if(p){free(p);(p)=L_null;}}
#endif /* SAFE_FREE */

#ifndef L_safe_delete_array
#define L_safe_delete_array(p) {if(p){delete [] (p);(p)=L_null;}}
#endif //L_safe_delete_array

#ifndef L_safe_delete
#define L_safe_delete(p) {if(p){delete (p);(p)=L_null;}}
#endif //L_safe_delete

/* L_safe_release is to release com objects safely. */
#ifdef __cplusplus
#define L_safe_release(p) {if(p){(p)->Release();(p)=L_null;}}
#else /* __cplusplus */
#define L_safe_release(p) {if(p){(p)->lpVtbl->Release((p));(p)=L_null;}}
#endif /* __cplusplus */


/****************************************
	Legacy Engine replacement functions.
****************************************/

unsigned long L_strncpy(char* szOut, const char* szIn, unsigned long nMax);
char* L_strncat(char* szDest, const char* szSrc, unsigned long nMax);
int L_strnicmp(char* szOne, char* szTwo, unsigned long nNum);
float L_atovalue(char* string);
signed long L_atol(char* string);
float L_atof(char* string);
unsigned long L_axtol(char* string);
unsigned long L_strlen(const char* string);
int L_mkdir(const char* szDir);

void* L_fopen(const char* filename, const char* mode);
int L_fclose(void* stream);
L_uint L_fread(void* buffer, L_uint size, L_uint count, void* stream);
L_uint L_fwrite(const void* buffer, L_uint size, L_uint count, void* stream);
L_long L_ftell(void* stream);
int L_fseek(void* stream, long offset, int origin);

/*************************************
	Legacy Engine special functions.
*************************************/

char* L_getfilepath(char* szDir, const char* szFilename);


void Debug_printf(char* format, ...);
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