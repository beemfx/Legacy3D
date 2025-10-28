#include <stdio.h>
#include <conio.h>
#include <tchar.h>
#include <windows.h>
#include "lc_sys2.h"

typedef struct _LCDEMO_INFO{
	lg_bool bQuit;
}LCDEMO_INFO;

lg_bool LCDemoCmd(LC_CMD nCmd, lg_void* pParams, lg_void* pExtra)
{
	LCDEMO_INFO* pInfo=(LCDEMO_INFO*)pExtra;

	switch(nCmd)
	{
	case 7:
	{
		const lg_char* szCvar=LC_GetArg(1, pParams);
		const lg_char* szValue=LC_GetArg(2, pParams);
		lg_cvar* cvar;
		
		if(!szCvar || !szValue)
		{
			LC_Printf("USAGE: set \"$cvarname$\" \"$value$\"");
			break;
		}
		cvar=CV_Get(szCvar);
		if(!cvar)
		{
			LC_Printf("\"%s\" is not a registered cvar.", szCvar);
			break;
		}
		
		CV_Set_s(cvar, szValue);
		LC_Printf("Set %s (\"%s\", %.2f, %d)", cvar->szName, cvar->szValue, cvar->fValue, cvar->nValue);
		break;
	}
	case 1:
		LC_Printf(TEXT("Quit command."));
		pInfo->bQuit=1;
		break;
	case 2:
		LC_ListCommands();
		break;
	case 3:
		LC_Printf(TEXT("List second command."));
		break;
	case 4:
	{
		const lg_char* szString=LC_GetArg(1, pParams);
		if(!szString)
			LC_Printf(TEXT("USAGE: echo \"$string$\"."));
		else
			LC_Printf(TEXT("%s"), szString);
		return LG_TRUE;
	}
	case 9:
		LC_Clear();
		break;
	case 6:
		LC_Print(TEXT("cvarlist command"));
		break;
	case 13:
		LC_Print(TEXT("Lose command"));
		break;
	default:
		return LG_FALSE;
	}
	return LG_TRUE;
}

void main()
{
	LCDEMO_INFO info;
	lg_dword nMessage=0;
	lg_char szCmd[1024];
	
	info.bQuit=0;
	LC_Init();
	
	LC_Printf(TEXT("Cosole Tester v1.0"));
	
	LC_SetCommandFunc(LCDemoCmd, &info);
	LC_RegisterCommand(TEXT("quit"), 1, 0);
	LC_RegisterCommand(TEXT("cmdlist"), 2, 0);
	LC_RegisterCommand(TEXT("cmdlist"), 3, 0);
	LC_RegisterCommand(TEXT("echo"), 4, 0);
	LC_RegisterCommand(TEXT("extract"), 5, 0);
	LC_RegisterCommand(TEXT("cvarlist"), 6, 0);
	LC_RegisterCommand(TEXT("set"), 7, 0);
	LC_RegisterCommand(TEXT("get"), 8, 0);
	LC_RegisterCommand(TEXT("clear"), 9, 0);
	LC_RegisterCommand(TEXT("hello"), 10, 0);
	LC_RegisterCommand(TEXT("whatup"), 11, 0);
	LC_RegisterCommand(TEXT("win"), 12, 0);
	LC_RegisterCommand(TEXT("lose"), 13, 0);
	LC_RegisterCommand(TEXT("regcvar"), 14, 0);
	
	CV_Define_l("TRUE", 1);
	CV_Define_l("FALSE", 0);
	
	CV_Define_l("ONE", 1);
	CV_Define_l("TWO", 2);
	CV_Define_l("THREE", 3);
	
	CV_Register("v_Windowed", "TRUE", CVAR_SAVE|CVAR_ROM);
	CV_Register("v_VidSetting", "THREE", CVAR_SAVE);
	CV_Register("hi{}", "HI", CVAR_ROM);
	
	while(LG_TRUE)
	{
		const lg_char* szLine=LC_GetOldestLine();
		system("cls");
		while(szLine)
		{
			_tprintf(TEXT("%s\n"), szLine);
			szLine=LC_GetNextLine();
		}
		
		_tprintf(TEXT("ct:> "));
		_getts(szCmd);
		LC_SendCommand(szCmd);
		
		if(_tcsicmp(szCmd, TEXT("x"))==0)
			break;
		
		if(info.bQuit)
			break;
	}
	LC_Shutdown();
	_tprintf(TEXT("press any key to exit\n"));
	getch();
}