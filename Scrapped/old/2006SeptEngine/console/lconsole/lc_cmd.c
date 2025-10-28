#include <stdio.h>
#include <malloc.h>
#include "lc_sys.h"
#include "lc_private.h"
#include "common.h"

#include <windows.h>

#define CVAR_CHECK_EXIT {if(lpConsole->cvarlist==NULL){Con_SendMessage(hConsole, "No lcvarlist loaded!");break;}}

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
	szString1=malloc(lpConsole->nMaxStrLen+1);
	szString2=malloc(lpConsole->nMaxStrLen+1);
	szString3=malloc(lpConsole->nMaxStrLen+1);

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
		L_dword dwLen=0;

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
	case CONF_CONDUMP:
	{
		LCENTRY* entry=NULL;
		FILE* fout=NULL;
		char* szFilename=szString1;
		L_dword i=0, j=0;
		CCParse_GetParam(szFilename, szParams, 1);
		if(L_strlen(szFilename)<1)
		{
			Con_SendMessage(hConsole, "Usage: CONDUMP filename$");
			break;
		}
		
		Con_SendErrorMsg(hConsole, "Dumping console to \"%s\".", szFilename);
		fout=fopen(szFilename, "w");
		if(!fout)
		{
			Con_SendErrorMsg(hConsole, "Could not open \"%s\" for saving.", szFilename);
			break;
		}
		/* We stop at entry 2, because that contains the lines that are printed by this
			function and the command line, which we don't want to print. */
		for(i=lpConsole->dwNumEntries; i>2; i--)
		{
			fprintf(fout, "%s\n", Con_GetEntry(hConsole, i, NULL));
		}
		fclose(fout);
		Con_SendMessage(hConsole, "Finished dumping console.");
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
		L_dword dwLen=0;

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
	case CONF_SAVECFG:
	{
		char* szFilename=szString1;
		char* szAppend=szString2;
		FILE* fout=NULL;
		HCVARLIST cvarlist=NULL;
		CVar* cvar=NULL;
		HLCONSOLE tempconsole=NULL;
		L_dword i=0, j=0;
		L_bool bAppend=L_false, bAll=L_false;

		CVAR_CHECK_EXIT

		CCParse_GetParam(szFilename, szParams, 1);
		if(L_strlen(szFilename)<1)
		{
			Con_SendMessage(hConsole, "Usage: SAVECFG filename$ [APPEND] [ALL]");
			break;
		}
		//CCParse_GetParam(szAppend, szParams, 2);
		if(CCParse_CheckParam(szParams, "APPEND", 2))
			bAppend=L_true;
		if(CCParse_CheckParam(szParams, "ALL", 2))
			bAll=L_true;

		Con_SendErrorMsg(hConsole, "Saving cvars as \"%s\"...", szFilename);
		cvarlist=lpConsole->cvarlist;
		if(!cvarlist)
		{
			Con_SendErrorMsg(hConsole, "Could not get cvarlist.", szFilename);
			break;
		}
		/* We need to reverse the order of the outputed text, the easyest way
			to do this is to use a def file to referse the order of the string. The
			reason we do this is so the cvars will be saved in the order they were
			created.*/
		tempconsole=Con_Create(NULL, lpConsole->nMaxStrLen, lpConsole->nMaxEntries, 0, NULL);
		if(tempconsole==NULL)
		{
			Con_SendMessage(hConsole, "Could not open temp console for dumping.");
			break;
		}
		for(cvar=CVar_GetFirstCVar(cvarlist), i=0; cvar; cvar=cvar->next)
		{
			if(L_CHECK_FLAG(cvar->flags, CVAREG_SAVE) || bAll)
			{
				Con_SendErrorMsg(tempconsole, "set \"%s\" \"%s\"", cvar->name, cvar->string);
				i++;
			}
		}
		fout=fopen(szFilename, bAppend?"r+":"w");
		if(!fout)
		{
			Con_Delete(tempconsole);
			Con_SendErrorMsg(hConsole, "Could not open \"%s\" for saving.", szFilename);
			break;
		}
		fseek(fout, 0, SEEK_END);
		for(j=0; j<i; j++)
		{
			fprintf(fout, "%s\n", Con_GetEntry(tempconsole, j+2, NULL));
		}
		fclose(fout);
		Con_Delete(tempconsole);
		Con_SendMessage(hConsole, "Finished saving cvars.");
		break;
	}
	case CONF_LOADCFG:
	{
		char* szFilename=szString1;
		char* szLine=szString2;
		FILE* fin=NULL;
		unsigned long nLen=0, i=0;

		CVAR_CHECK_EXIT
		
		CCParse_GetParam(szFilename, szParams, 1);
		if(L_strlen(szFilename)<1)
		{
			Con_SendMessage(hConsole, "Usage: LOADCFG filename$");
			break;
		}
		fin=fopen(szFilename, "r");
		if(fin==NULL)
		{
			Con_SendErrorMsg(hConsole, "Could not open \"%s\" for processing!", szFilename);
			break;
		}
		Con_SendErrorMsg(hConsole, "Processing \"%s\"...", szFilename);
		while(!feof(fin))
		{
			if(fgets(szLine, lpConsole->nMaxStrLen-1, fin)==NULL)
				continue;
			/* Ignore strings, that have nothing in them, and ignore strings
				that are just return carriages. */
			nLen=L_strlen(szLine);
			if(nLen<1)
				continue;

			if((szLine[0]=='\r') || (szLine[0]=='\n'))
				continue;
			/* Check to see for comments, and remove them. */
			for(i=0; i<=nLen; i++)
			{
				if((szLine[i]=='/')&&(szLine[i+1]=='/'))
				{
					szLine[i]=0;
					break;
				}
				if(szLine[i]=='\r' || szLine[i]=='\n')
				{
					szLine[i]=0;
					break;
				}
			}
			if(szLine[0]==0)
				continue;

			Con_SendCommand(hConsole, szLine);
		}
		fclose(fin);
		Con_SendErrorMsg(hConsole, "Finished processing \"%s\".", szFilename);
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
