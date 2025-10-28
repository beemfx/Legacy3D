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
#include <lc_sys.h>
#include "lg_err.h"
#include "dxerr9.h"
#include "lg_sys.h"


HLCONSOLE g_console=L_null; /* Global, stores the console that will receive error reports.*/


/******************************************************
	Err_InitReporting()

	Initialized error reporting by setting the console
	to report to.
******************************************************/
void Err_InitReporting(HLCONSOLE hConsole)
{
	g_console=hConsole;
}

/*********************************************************
	Err_Printf()

	Prints formatted text to the specifed console, or if
	a console has not been initialized reports to stdout.
*********************************************************/
L_int Err_Printf(const char* format, ...)
{
	char szOutput[MAX_ERR_MSG_LEN];
	va_list arglist=L_null;
	int nResult=0;

	va_start(arglist, format);
	_vsnprintf(szOutput, MAX_ERR_MSG_LEN-1, format, arglist);
	if(g_console)
		nResult=Con_SendMessage(g_console, szOutput);
	else
		nResult=printf(szOutput);
	va_end(arglist);

	return nResult;
}

/************************************************
	Err_PrintDX()

	Prints a directx error code and description.
************************************************/
L_int Err_PrintDX(const char* function, L_result nResult)
{
	char szOutput[MAX_ERR_MSG_LEN];
	
	_snprintf(
		szOutput, 
		MAX_ERR_MSG_LEN-1, 
		"%s returned %s (%s).", 
		(char*)function, 
		DXGetErrorString9(nResult),
		DXGetErrorDescription9(nResult));
	
	return Con_SendMessage(g_console, szOutput);
}

L_int Err_PrintVersion()
{
	return Err_Printf("%s %s", LGAME_NAME, "BUILD "LGAME_VERSION" ("__DATE__" "__TIME__")");
}

void Err_ErrBox(const char* format, ...)
{
	char szOutput[1024];
	va_list arglist=L_null;
	int nResult=0;

	va_start(arglist, format);
	_vsnprintf(szOutput, 1024-1, format, arglist);
	MessageBox(0, szOutput, "Error Message", MB_OK);
	va_end(arglist);
}

void Err_PrintMatrix(D3DMATRIX* pM)
{
	Err_Printf("| %f\t  %f\t  %f\t  %f|", pM->_11, pM->_12, pM->_13, pM->_14);
	Err_Printf("| %f\t  %f\t  %f\t  %f|", pM->_21, pM->_22, pM->_23, pM->_24);
	Err_Printf("| %f\t  %f\t  %f\t  %f|", pM->_31, pM->_32, pM->_33, pM->_34);
	Err_Printf("| %f\t  %f\t  %f\t  %f|", pM->_41, pM->_42, pM->_43, pM->_44);
}
