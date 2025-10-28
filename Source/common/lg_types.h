#ifndef __LG_TYPES_H__
#define __LG_TYPES_H__

//Universal definitions
#define LG_CACHE_ALIGN __declspec(align(4))

/***************************
	Legacy Engine types.
****************************/
typedef signed int lg_int, *lg_pint;
typedef unsigned int lg_uint, *lg_puint;
typedef char lg_char, *lg_pchar;
typedef unsigned char lg_byte, *lg_pbyte;
typedef signed char lg_sbyte, *lg_psbyte;
typedef signed char lg_schar, *lg_pschar;
typedef char *lg_str, *lg_pstr, *lg_lpstr;
typedef const char *lg_cstr, *lg_pcstr, *lg_lpcstr;
typedef unsigned short lg_word, *lg_pword, lg_ushort, *lg_pushort;
typedef signed short lg_short, *lg_pshort;
typedef unsigned long lg_dword, *lg_pdword, lg_ulong, *lg_pulong;
typedef signed long lg_long, *lg_plong;
typedef float lg_float, *lg_pfloat;
typedef double lg_double, *lg_pdouble;
typedef long double lg_ldouble, *lg_lpdouble;
typedef void lg_void, *lg_pvoid;

#ifndef __cplusplus
typedef unsigned short wchar_t;
#endif __cplusplus

typedef wchar_t lg_wchar_t, lg_wchar;
typedef lg_wchar_t *lg_wstr, *lg_pwstr, *lg_lpwstr;
typedef const lg_wchar_t *lg_cwstr, *lg_pcwstr, *lg_lpcwstr;

typedef unsigned int lg_size_t;

#ifdef UNICODE
typedef lg_wchar_t lg_tchar;
#else !UNICODE
typedef lg_char lg_tchar;
#endif UNICODE

#define LG_MAX_PATH 255
typedef lg_char LG_CACHE_ALIGN lg_pathA[LG_MAX_PATH+1];
typedef lg_wchar LG_CACHE_ALIGN lg_pathW[LG_MAX_PATH+1];

#define LG_MAX_STRING 127
typedef lg_char LG_CACHE_ALIGN lg_stringA[LG_MAX_STRING+1];
typedef lg_wchar LG_CACHE_ALIGN lg_stringW[LG_MAX_STRING+1];

#ifdef UNICODE
#define lg_path lg_pathW
#define lg_string lg_stringW
#else !UNICODE
#define lg_path lg_pathA
#define lg_string lg_stringA
#endif UNICODE


typedef struct _lg_large_integer{
	lg_dword dwLowPart;
	lg_long dwHighPart;
}lg_large_integer, *lg_plarge_integer, *lg_lplarge_integer;

typedef struct _lg_rect{
	lg_long left;
	lg_long top;
	lg_long right;
	lg_long bottom;
}lg_rect, *lg_prect;

typedef LG_CACHE_ALIGN int lg_bool;
#define LG_TRUE  (1)
#define LG_FALSE (0)

typedef long lg_result;
#define LG_SUCCEEDED(Status) ((lg_result)(Status)>=0)
#define LG_FAILED(Status) ((lg_result)(Status)<0)

/******************************
	Legacy Engine definitions.
*******************************/

#ifdef UNICODE
#define LG_TEXT(t) (L##t)
#else !UNICODE
#define LG_TEXT(t) (t)
#endif UNICODE

#ifdef __cplusplus
#define LG_NULL (0)
#else /* __cplusplus */
#define LG_NULL ((void*)0)
#endif __cplusplus

#endif __LG_TYPES_H__
