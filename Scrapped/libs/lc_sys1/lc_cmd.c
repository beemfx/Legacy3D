#include <stdio.h>
#include "lg_malloc.h"
#include "lc_sys.h"
#include "lc_private.h"
#include "common.h"

#include <windows.h>

#define CVAR_CHECK_EXIT {if(lpConsole->cvarlist==NULL){Con_SendMessage(hConsole, "No cvarlist loaded!");break;}}

/* Con_InternalCommands attempts to process internal commands. */
/* As of right now this is the only function that uses the very max, setting, but I do want to
	eliminate it from this. */
int Con_InternalCommands(unsigned long nCommand, const char* szParams, HLCONSOLE hConsole)
{
	LPLCONSOLE lpConsole=hConsole;
	/* unsigned long nCommand=0; */
	char* szString1=NULL;
	char* szString2=NULL;
	char* szString3=NULL;
	int nResult=0;

	if(lpConsole==NULL)
		return 0;

	/* nCommand=Con_CmdNameToValue(hConsole, szCommand); */
	
	if(nCommand==0)
		return 0;

	/* Go ahead, and allocate three strings for the use of these functions. */
	szString1=LG_Malloc(lpConsole->nMaxStrLen+1);
	szString2=LG_Malloc(lpConsole->nMaxStrLen+1);
	szString3=LG_Malloc(lpConsole->nMaxStrLen+1);

	if(!szString1 || !szString2 || !szString3)
	{
		L_safe_free(szString1);
		L_safe_free(szString2);
		L_safe_free(szString3);
		Con_SendMessage(hConsole, "Could not allocate memory for temp strings.");
		return 1;
	}

	/* We'll set the nResult to 1, but then if nothing gets processed, the default
		switch with change it to a zero, which means we didn't process a message. */
	nResult=1;
	switch(nCommand)
	{
	case CONF_ECHO:
	{
		char* szEcho=szString1;
		CCParse_GetParam(szEcho, szParams, 1);
		Con_SendMessage(hConsole, szEcho);
		break;
	}
	case CONF_CLEAR:
	{
		Con_Clear(hConsole);
		break;
	}
	case CONF_CMDLIST:
	{
		LDefs* defs=lpConsole->commands;
		LDef* def=NULL;
		char* szLimit=szString1;
		lg_dword dwLen=0;

		if(!defs)
		{
			Con_SendMessage(hConsole, "Could not obtain list of commands.");
			break;
		}
		CCParse_GetParam(szLimit, szParams, 1);
		/* If there is a parameter, it is to limit the listed commands
			to the ones starting with the specified string. */
		dwLen=L_strlen(szLimit);
		Con_SendMessage(hConsole, "Registered commands:");
		for(def=defs->list; def; def=def->next)
		{
			if(dwLen)
				if(!L_strnicmp(szLimit, def->name, dwLen))
					continue;

			Con_SendErrorMsg(hConsole, "   %s", def->name);
		}
		break;
	}
	case CONF_REGCVAR:
	{
		char* szCVarName=szString1;
		char* szCVarValue=szString2;
		char* szUpdate=szString3;
		unsigned long dwCreateFlags=0x00000000l;

		CVAR_CHECK_EXIT

		CCParse_GetParam(szCVarName, szParams, 1);
		CCParse_GetParam(szCVarValue, szParams, 2);
		/* Make sure that we at least have two parameters for the value and name. */
		if( (L_strlen(szCVarName)<1) || (L_strlen(szCVarValue)<1) )
		{
			Con_SendMessage(hConsole, "Usage: REGCVAR name$ value$ [UPDATE] [NOSAVE]");
			break;
		}
		//CCParse_GetParam(szUpdate, szParams, 3);
		dwCreateFlags=CVAREG_SAVE;
		/* If the update value is set to true then, we update. */
		if(CCParse_CheckParam(szParams, "UPDATE", 3))
			dwCreateFlags=dwCreateFlags|CVAREG_UPDATE;
		//CCParse_GetParam(szUpdate, szParams, 4);
		if(CCParse_CheckParam(szParams, "NOSAVE", 3))
			dwCreateFlags=dwCreateFlags^CVAREG_SAVE;
		CVar_Register(lpConsole->cvarlist, szCVarName, szCVarValue, dwCreateFlags);
		break;
	}
	case CONF_SET:
	{
		/* Changes a cvars value. */
		char* cvarname=szString1;
		char* newvalue=szString2;
		CVar* cvar=NULL;

		CVAR_CHECK_EXIT

		CCParse_GetParam(cvarname, szParams, 1);
		CCParse_GetParam(newvalue, szParams, 2);

		if((L_strlen(cvarname)<1) || (L_strlen(newvalue)<1))
		{
			Con_SendMessage(hConsole, "Usage: SET name$ newvalue$");
			break;
		}
		cvar=CVar_GetCVar(lpConsole->cvarlist, cvarname);
		if(!cvar)
		{
			Con_SendErrorMsg(
				hConsole,
				"Could not set \"%s\", no such cvar!",
				cvarname);
			break;
		}

		if(L_CHECK_FLAG(cvar->flags, CVAREG_SETWONTCHANGE))
		{
			Con_SendErrorMsg(hConsole, "Cannot change \"%s\" permission denied by app.", cvarname);
			break;
		}
		if(CVar_Set(lpConsole->cvarlist, cvarname, newvalue))
		{
			if(cvar)
			{
				Con_SendErrorMsg(
					hConsole, 
					"   %s = \"%s\", %.0f", 
					cvarname, 
					newvalue, 
					cvar->value);

				if(L_CHECK_FLAG(cvar->flags, CVAREG_UPDATE))
				{
					sprintf(szString1, "cvarupdate \"%s\"", cvar->name);
					Con_SendCommand(hConsole, szString1);
				}
			}
			else
				Con_SendErrorMsg(
					hConsole,
					"Could not check to see if \"%s\" was successfully set.", cvarname);
			
		}
		break;
	}
	case CONF_GET:
	{
		/* Retrieves a cvar. */
		char* szCVarName=szString1;
		char* lpValue=NULL;
		CVar* value=NULL;

		CVAR_CHECK_EXIT

		CCParse_GetParam(szCVarName, szParams, 1);
		if(L_strlen(szCVarName)<1)
		{
			Con_SendMessage(hConsole, "Usage: GET name$ [FP]");
			break;
		}
		value=CVar_GetCVar(lpConsole->cvarlist, szCVarName);
		if(value)
		{
			if(CCParse_CheckParam(szParams, "FP", 2))
				Con_SendErrorMsg(hConsole, "   %s = \"%s\", %f", value->name, value->string, value->value);
			else
				Con_SendErrorMsg(hConsole, "   %s = \"%s\", %.0f", value->name, value->string, value->value);
		}
		else
			Con_SendErrorMsg(hConsole, "Cannot get \"%s\", no such cvar!", szCVarName);
		break;
	}
	case CONF_DEFINE:
	{
		char* szDef=szString1;
		char* szValue=szString2;

		CVAR_CHECK_EXIT

		CCParse_GetParam(szDef, szParams, 1);
		CCParse_GetParam(szValue, szParams, 2);

		if( (L_strlen(szDef)<1) || (L_strlen(szValue)<1))
		{
			Con_SendMessage(hConsole, "Usage: DEFINE name$ value%");
			break;
		}

		if(CVar_AddDef(lpConsole->cvarlist, szDef, L_atof(szValue)))
			Con_SendErrorMsg(lpConsole, "Defined \"%s\" as %f", szDef, L_atof(szValue));
		else
			Con_SendErrorMsg(hConsole, "Couldn't define \"%s\", possible bad name, or already defined.", szDef);
		break;
	}
	case CONF_CVARLIST:
	{
		CVar* cvarlist=NULL;
		char* szLimit=szString1;
		lg_dword dwLen=0;

		CVAR_CHECK_EXIT

		Con_SendMessage(hConsole, "Registered cvars:");
		cvarlist=CVar_GetFirstCVar(lpConsole->cvarlist);
		CCParse_GetParam(szLimit, szParams, 1);
		/* We can limit the number of cvars displayed, by putting
			a parameter with the first leters we want. */
		dwLen=L_strlen(szLimit);
		for(cvarlist=CVar_GetFirstCVar(lpConsole->cvarlist); cvarlist; cvarlist=cvarlist->next)
		{
			if(dwLen)
				if(!L_strnicmp(szLimit, cvarlist->name, dwLen))
					continue;
			Con_SendErrorMsg(hConsole, "   %s = \"%s\", %.0f", cvarlist->name, cvarlist->string, cvarlist->value);
		}
		break;
	}
	default:
		L_safe_free(szString1);
		L_safe_free(szString2);
		L_safe_free(szString3);
		nResult=0;
	}
	L_safe_free(szString1);
	L_safe_free(szString2);
	L_safe_free(szString3);
	return nResult;
}
