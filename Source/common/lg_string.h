#ifndef __LG_STRING_H__
#define __LG_STRING_H__

#include "lg_types.h"

#ifdef __cplusplus
extern "C"{
#endif __cplusplus

lg_int LG_StrNcCmpA(const lg_char* s1, const lg_char* s2, lg_long n);
lg_int LG_StrNcCmpW(const lg_wchar* s1, const lg_wchar* s2, lg_long n);

lg_dword LG_StrLenA(const lg_char* s);
lg_dword LG_StrLenW(const lg_wchar* s);

lg_char* LG_StrCopySafeA(lg_char* dest, const lg_char* src, lg_dword destsize);
lg_wchar* LG_StrCopySafeW(lg_wchar* dest, const lg_wchar* src, lg_dword destsize);

#ifdef __cplusplus
}
#endif __cplusplus

#ifdef UNICODE
#define LG_StrNcCmp LG_StrNcCmpW
#define LG_StrLen   LG_StrLenW
#else !UNICODE
#define LG_StrNcCmp LG_StrNcCmpA
#define LG_StrLen   LG_StrLenA
#endif UNICODE

#endif __LG_STRING_H__