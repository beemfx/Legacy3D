/*********************************************************
	File: lg_err.c
	Copyright (c) 2006, Blaine Myers

	Purpose: Contains various functions for initializing
	error reporting, and for reporting errors.
*********************************************************/
#include "common.h"
#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include <d3d9.h>
#include "lg_err.h"
//#include "dxerr9.h"
#include "lg_sys.h"

lg_dword g_nTabSize=0;

/*********************************************************
	Err_Printf()

	Prints formatted text to the specifed console, or if
	a console has not been initialized reports to stdout.
*********************************************************/
void Err_Printf(const char* format, ...)
{
	char szOutput[MAX_ERR_MSG_LEN];
	va_list arglist=LG_NULL;
	int nResult=0;
	
	for(lg_dword i=0; i<g_nTabSize; i++)
	{
		szOutput[i]=' ';
	}

	va_start(arglist, format);
	_vsnprintf(&szOutput[g_nTabSize], MAX_ERR_MSG_LEN-1-g_nTabSize, format, arglist);
	
	
	LC_Print(szOutput);
	#if defined(_DEBUG)
	OutputDebugString("CONSOLE: ");
	OutputDebugString(szOutput);
	OutputDebugString("\n");
	#endif
	va_end(arglist);
}

void Err_IncTab()
{
	g_nTabSize+=3;
}
void Err_DecTab()
{
	if(g_nTabSize)
		g_nTabSize-=3;
}

void Err_MsgPrintf(const char* format, ...)
{
	char szOutput[128];
	char szMsg[128];
	va_list arglist=LG_NULL;
	int nResult=0;

	va_start(arglist, format);
	_vsnprintf(szOutput, 127, format, arglist);
	va_end(arglist);
	
	_snprintf(szMsg, 127, "setmessage \"%s\"", szOutput);
	
	LC_SendCommand(szMsg);
}

void Err_PrintfDebug(const char* format, ...)
{
	char szOutput[MAX_ERR_MSG_LEN];
	va_list arglist=LG_NULL;
	int nResult=0;

	va_start(arglist, format);
	_vsnprintf(szOutput, MAX_ERR_MSG_LEN-1, format, arglist);
	OutputDebugString(szOutput);
	va_end(arglist);
}


/************************************************
	Err_PrintDX()

	Prints a directx error code and description.
************************************************/

void Err_PrintDX(lg_cstr function, lg_result nResult)
{
	Err_Printf(
		"%s returned %s (%s).",
		(lg_str)function,
		"Error", //DXGetErrorString9(nResult),
		"Desc");//DXGetErrorDescription9(nResult));
}

void Err_PrintVersion()
{
	Err_Printf(g_GameString);
	//Err_Printf("%s %s", LGAME_NAME, "BUILD "LGAME_VERSION" ("__DATE__" "__TIME__")");
}

void Err_ErrBox(const char* format, ...)
{
	char szOutput[1024];
	va_list arglist=LG_NULL;
	int nResult=0;

	va_start(arglist, format);
	_vsnprintf(szOutput, 1024-1, format, arglist);
	MessageBox(0, szOutput, "Error Message", MB_OK);
	va_end(arglist);
}

void Err_PrintMatrix(void* pM1)
{
	D3DMATRIX* pM=(D3DMATRIX*)pM1;
	Err_Printf("MATRIX:");
	Err_Printf("| %f\t  %f\t  %f\t  %f|", pM->_11, pM->_12, pM->_13, pM->_14);
	Err_Printf("| %f\t  %f\t  %f\t  %f|", pM->_21, pM->_22, pM->_23, pM->_24);
	Err_Printf("| %f\t  %f\t  %f\t  %f|", pM->_31, pM->_32, pM->_33, pM->_34);
	Err_Printf("| %f\t  %f\t  %f\t  %f|", pM->_41, pM->_42, pM->_43, pM->_44);
}

//File system error reporting callback function
void __cdecl Err_FSCallback(lg_cwstr szString)
{
	char szOutput[512];
	wcstombs(szOutput, (const wchar_t*)szString, 511);
	Err_Printf(szOutput);
}
