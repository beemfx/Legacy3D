#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "lc_con.h"
#include "lc_sys2.h"
#include "lc_cvar.h"
#include "common.h"

CConsole* g_pCon=LG_NULL;
CCvarList* g_pCvars=LG_NULL;

extern "C" {

void LC_FUNC LC_ListCvars(const lg_char* szLimit)
{
	if(!g_pCon || !g_pCvars)
		return;
		
	g_pCon->Print("Registered cvars:");
	lg_cvar* cvar=g_pCvars->GetFirst();
	while(cvar)
	{
		if(szLimit)
		{
			if(strnicmp(szLimit, cvar->szName, strlen(szLimit))!=0)
			{
				cvar=g_pCvars->GetNext();
				continue;
			}
		}
		g_pCon->Printf("   %s (\"%s\", %.2f, %d)", cvar->szName, cvar->szValue, cvar->fValue, cvar->nValue);
		cvar=g_pCvars->GetNext();
	}
}

lg_bool LC_FUNC LC_Init()
{
	if(g_pCon)
	{
		g_pCon->Print("LC_Init WARNING: Console has already been initialized.");
		return LG_TRUE;
	}
	
	g_pCon=new CConsole;
	
	if(g_pCvars)
	{
		g_pCon->Print("LC_Init WARNING: Cvars have already been initialized.");
	}
	g_pCvars=new CCvarList;
	
	if(g_pCon && g_pCvars)
		return LG_TRUE;
	else
	{
		L_safe_delete(g_pCon);
		L_safe_delete(g_pCvars);
		return LG_FALSE;
	}
}

void LC_FUNC LC_Shutdown()
{
	L_safe_delete(g_pCvars);
	L_safe_delete(g_pCon);
}

void LC_FUNC LC_SetCommandFunc(LC_CMD_FUNC pfn, lg_void* pExtra)
{
	if(!g_pCon)
		return;
		
	g_pCon->SetCommandFunc(pfn, pExtra);
}

void LC_FUNC LC_Print(lg_char* szText)
{
	if(!g_pCon)
		return;
		
	g_pCon->Print(szText);
}
void LC_FUNC LC_Printf(lg_char* szFormat, ...)
{
	if(!g_pCon)
		return;
	
	va_list arglist;
	va_start(arglist, szFormat);
	_vsnprintf(CConsole::s_szTemp, LC_MAX_LINE_LEN, szFormat, arglist);
	va_end(arglist);
	LC_Print(CConsole::s_szTemp);
}

void LC_FUNC LC_SendCommand(const lg_char* szCmd)
{
	if(!g_pCon)
		return;
		
	g_pCon->SendCommand((lg_char*)szCmd);
}
void LC_FUNC LC_SendCommandf(lg_char* szFormat, ...)
{
	if(!g_pCon)
		return;
		
	va_list arglist;
	va_start(arglist, szFormat);
	_vsnprintf(CConsole::s_szTemp, LC_MAX_LINE_LEN, szFormat, arglist);
	va_end(arglist);
	LC_SendCommand(CConsole::s_szTemp);
}

const lg_char* LC_FUNC LC_GetLine(lg_dword nRef, const lg_bool bStartWithNew)
{
	if(!g_pCon)
		return LG_NULL;
		
	return g_pCon->GetLine(nRef, bStartWithNew);
}

const lg_char* LC_FUNC LC_GetOldestLine()
{
	if(!g_pCon)
		return LG_NULL;
		
	return g_pCon->GetOldestLine();
}
const lg_char* LC_FUNC LC_GetNewestLine()
{
	if(!g_pCon)
		return LG_NULL;
		
	return g_pCon->GetNewestLine();
}

const lg_char* LC_FUNC LC_GetNextLine()
{
	if(!g_pCon)
		return LG_NULL;
		
	return g_pCon->GetNextLine();
}

lg_bool LC_FUNC LC_RegisterCommand(lg_char* szCmd, lg_dword nID, lg_char* szHelpString)
{
	if(!g_pCon)
		return LG_NULL;
		
	return g_pCon->RegisterCommand(szCmd, nID, szHelpString);
}

void LC_FUNC LC_ListCommands()
{
	if(!g_pCon)
		return;
		
	g_pCon->ListCommands();
}

const lg_char* LC_FUNC LC_GetArg(lg_dword nParam, lg_void* pParams)
{
	return CConsole::GetArg(nParam, pParams);
}

lg_bool LC_FUNC LC_CheckArg(const lg_char* string, lg_void* args)
{
	return CConsole::CheckArg(string, args);
}

void LC_FUNC LC_Clear()
{
	if(!g_pCon)
		return;
		
	g_pCon->Clear();
}

lg_dword LC_FUNC LC_GetNumLines()
{
	if(!g_pCon)
		return 0;
		
	return g_pCon->m_nLineCount;
}

const lg_char* LC_FUNC LC_GetActiveLine()
{
	if(!g_pCon)
		return LG_NULL;
		
	return g_pCon->GetActiveLine();
}
void LC_FUNC LC_OnChar(lg_char c)
{
	if(!g_pCon)
		return;
		
	g_pCon->OnChar(c);
}

lg_cvar* LC_FUNC CV_Register(const lg_char* szName, const lg_char* szValue, lg_dword Flags)
{
	if(!g_pCvars)
		return LG_NULL;
			
	lg_cvar* pCvar=g_pCvars->Register(szName, szValue, Flags);
	if(pCvar)
		LC_Printf("Registered \"%s\" (\"%s\", %.2f, %d).", pCvar->szName, pCvar->szValue, pCvar->fValue, pCvar->nValue);
	else
		LC_Printf("CV_Register ERROR: Could not register \"%s\", check name.", szName);
	return pCvar;
}

lg_bool LC_FUNC CV_Define_l(lg_char* szDef, lg_long nValue)
{
	if(!g_pCvars)
		return LG_FALSE;
		
	return g_pCvars->Define(szDef, nValue);
}
lg_bool LC_FUNC CV_Define_f(const lg_char* szDef, lg_float fValue)
{
	if(!g_pCvars)
		return LG_FALSE;
		
	return g_pCvars->Define((lg_char*)szDef, fValue);
}

lg_cvar* LC_FUNC CV_Get(const lg_char* szName)
{
	if(!g_pCvars)
		return LG_NULL;
		
	return g_pCvars->Get(szName);
}
void LC_FUNC CV_Set(lg_cvar* cvar, const lg_char* szValue)
{
	if(!g_pCvars || !cvar)
		return;
		
	return CCvarList::Set(cvar, (lg_char*)szValue, &g_pCvars->m_Defs);
}
void LC_FUNC CV_Set_l(lg_cvar* cvar, lg_long nValue)
{
	if(!cvar)
		return;
		
	return CCvarList::Set(cvar, nValue);
}
void LC_FUNC CV_Set_f(lg_cvar* cvar, lg_float fValue)
{
	if(!cvar)
		return;
		
	return CCvarList::Set(cvar, fValue);
}

LC_EXP lg_cvar* LC_FUNC CV_GetFirst()
{
	if(!g_pCvars)
		return LG_NULL;
		
	return g_pCvars->GetFirst();
}
LC_EXP lg_cvar* LC_FUNC CV_GetNext()
{
	if(!g_pCvars)
		return LG_NULL;
		
	return g_pCvars->GetNext();
}

} //extern "C"


#if 0
#ifdef _DEBUG
#include <windows.h>
#include <crtdbg.h>
#include <common.h>
#include "lg_malloc.h"

BOOL WINAPI DllMain(HINSTANCE hInst, DWORD fdwReason, LPVOID lpReserved)
{
	static _CrtMemState s1, s2, s3;
	
	switch(fdwReason)
	{
	//case DLL_THREAD_ATTACH:
	case DLL_PROCESS_ATTACH:
		_CrtMemCheckpoint(&s1);
		break;
	//case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		Debug_printf("lc_sys2 Shutdown...\n");
		OutputDebugString("MEMORY USAGE FOR \"lc_sys2.dll\"\"\n");
		_CrtMemCheckpoint(&s2);
		_CrtMemDifference(&s3, &s1, &s2);
		_CrtMemDumpStatistics(&s3);
		_CrtDumpMemoryLeaks();
		
		LG_MemDbgOutput();
		break;
	}
	return TRUE;
}
#endif /*_DEBUG*/
#endif