#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include "lf_sys2.h"

LF_ERR_LEVEL g_nErrLevel=ERR_LEVEL_NOTICE;
void (__cdecl*g_pfnErrPrint)(lf_cwstr szFormat)=LF_NULL;

void LF_SetErrLevel(LF_ERR_LEVEL nLevel)
{
	g_nErrLevel=nLevel;
}

void LF_SYS2_EXPORTS LF_SetErrPrintFn(void (__cdecl*pfn)(lf_cwstr szFormat))
{
	g_pfnErrPrint=pfn;
}

#define LF_ERR_MAX_OUTPUT 511

void LF_ErrPrintW(lf_cwstr szFormat, LF_ERR_LEVEL nErrLevel, ...)
{
	
	lf_wchar_t szOutput[LF_ERR_MAX_OUTPUT+1];
	va_list arglist=LF_NULL;
	//Don't print the message if the current error level
	//is greater than the error level of the message.
	if((nErrLevel<g_nErrLevel) || !g_pfnErrPrint)
		return;
		
	va_start(arglist, nErrLevel);
	//_vsnwprintf(szOutput, 1022, szFormat, arglist);
	_vsnwprintf_s(szOutput, LF_ERR_MAX_OUTPUT, LF_ERR_MAX_OUTPUT, szFormat, arglist);
	va_end(arglist);
	
	szOutput[wcslen(szOutput)]=0;
	g_pfnErrPrint(szOutput);	
}

void LF_ErrPrintA(lf_cstr szFormat, LF_ERR_LEVEL nErrLevel, ...)
{
	
	lf_char szOutput[LF_ERR_MAX_OUTPUT+1];
	lf_wchar szOutputW[LF_ERR_MAX_OUTPUT+1];
	va_list arglist=LF_NULL;
	//Don't print the message if the current error level
	//is greater than the error level of the message.
	if((nErrLevel<g_nErrLevel) || !g_pfnErrPrint)
		return;
		
	va_start(arglist, nErrLevel);
	_vsnprintf_s(szOutput, LF_ERR_MAX_OUTPUT, LF_ERR_MAX_OUTPUT, szFormat, arglist);
	va_end(arglist);
	
	szOutput[strlen(szOutput)]=0;
	mbstowcs(szOutputW, szOutput, LF_ERR_MAX_OUTPUT);
	g_pfnErrPrint(szOutputW);	
}

void __cdecl LF_DefaultErrPrint(lf_cwstr szFormat)
{
	lf_char szText[LF_ERR_MAX_OUTPUT+1];
	wcstombs(szText, szFormat, LF_ERR_MAX_OUTPUT);
	printf("%s\n", szText);
}