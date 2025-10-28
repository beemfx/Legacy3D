#include <stdio.h>
#include <malloc.h>
#include <stdarg.h>
#include "common.h"
#include "lc_sys.h"
#include "lc_private.h"

#ifdef _DEBUG
#include <windows.h>
#include <stdlib.h>
#include <crtdbg.h>
#endif /*_DEBUG*/

unsigned long g_nNumEntries=0;

/* =================================
   Command Parsing Helper functions.
	=================================*/

/* CCParse_GetParam will get a parameter from the Parameter string, the way
	it works is that it assumes all parameters are separated by spaces, unless
	within qoutes, in which case spaces are ignored. */
int CCParse_GetParam(
	char* szParamOut,
	const char* szParams,
	unsigned long nParam)
{
	unsigned long dwLen=L_strlen(szParams), i=0, j=0;
	int bFoundParam=0;
	int bInQuotes=0;

	szParamOut[0]=0;

	if(nParam<1)
		return 0;

	if(dwLen<1)
		return 0;

	nParam--;


	for(i=0, j=0, bFoundParam=0, bInQuotes=0; i<=dwLen; i++)
	{
		if(szParams[i]=='\"')
		{
			if(bInQuotes && (nParam==0))
				break;
			bInQuotes=!bInQuotes;
			continue;
		}
		
		if(nParam==0)
		{
			if(szParams[i]==' ' && !bInQuotes)
				break;
			szParamOut[j]=szParams[i];
			j++;
		}
		else
		{
			if((szParams[i]==' ') && (!bInQuotes))
				nParam--;
		}
	}
	szParamOut[j]=0;
	
	if(L_strlen(szParamOut) < 1 )
		return 0;
	else
		return 1;
}

/* CCParse_GetFloat converts a parameter to a float, see above. */
float CCParse_GetFloat(
	char* szParams,
	unsigned short wParam)
{
	char szTemp[32];

	if(!CCParse_GetParam(szTemp, szParams, wParam))
		return 0.0f;

	return (float)L_atof(szTemp);
}

/* CCParse_GetInt converts a parameter to an int, see above. */
signed long CCParse_GetInt(
	char* szParams,
	unsigned short wParam)
{	
	char szTemp[32];

	if(!CCParse_GetParam(szTemp, szParams, wParam))
		return 0;
	
	return L_atol(szTemp);
}

/* CCParse_CheckParam - checks all params to see if any match
	a particular string. */
int CCParse_CheckParam(
	const char* szParams,
	const char* szAttrib,
	int nStart)
{
	int i=nStart;
	char* szTemp;

	if(!szParams || !szAttrib)
		return 0;

	szTemp=malloc(L_strlen(szParams)+1);
	if(!szTemp)
		return L_false;


	while(CCParse_GetParam(szTemp, szParams, i))
	{
		if(L_strnicmp(szTemp, (char*)szAttrib, 0))
		{
			free(szTemp);
			return L_true;
		}
		szTemp[0]=0;

		i++;
	}

	free(szTemp);
	return L_false;
}



/* ==========================================================
	Legacy Console private functions, the user never calls
	these functions.
	========================================================== */

/* Con_SimpleParse - does a few things to parse the text and
	identify the command, as well as the parameters. */
int Con_SimpleParse(
	char* szCommand,
	char* szParams,
	char* szIgnore,
	char* szLineIn,
	unsigned long dwMaxLen)
{
	char* szTemp;
	unsigned long dwLen=0, i=0, j=0, k=0, dwSpaces=0;
	int bFoundChar=0, bIgnore=0, bInQuotes=0;

	unsigned long dwIgnoreLen=0;

	dwLen=L_strlen(szLineIn);
	dwIgnoreLen=L_strlen(szIgnore);

	if(dwLen<1)
		return 0;

	/* Allocate a temporary string for the output,
		this will be freed when the function exits. */
	szTemp=malloc(dwMaxLen);

	if(szTemp==NULL)
		return 0;

	/* First thing to do is remove any excess spaces. */
	for(i=0; i<dwLen; i++, bIgnore=0)
	{
		if(!bFoundChar)
			if((szLineIn[i]!=' '))
				bFoundChar=1;

		if(bFoundChar)
		{
			
			if(szLineIn[i]=='\"')
				bInQuotes=!bInQuotes;

			for(k=0; k<dwIgnoreLen; k++)
			{
				if((szLineIn[i]==szIgnore[k]) && !bInQuotes)
					bIgnore=1;
			}
			if(bIgnore)
			{
				if(dwSpaces==0)
				{
					szTemp[j]=' ';
					j++;
					dwSpaces++;
					continue;
				}
				else
				{
					dwSpaces++;
					continue;
				}
			}
			else
			{
				dwSpaces=0;
			}
				
			szTemp[j]=szLineIn[i];
			j++;
		}
	}

	szTemp[j]=0;

	dwLen=L_strlen(szTemp);


	/* Get the command (the first word). */
	for(i=0, j=0; i<dwLen; i++, j++){
		if(szTemp[i]==' ')
			break;
		szCommand[j]=szTemp[i];
	}
	szCommand[j]=0;

	/* Get the parameters. */
	j++;
	i++;
	for(j=0 ; i<dwLen; i++, j++){
		szParams[j]=szTemp[i];
	}
	szParams[j]=0;

	L_safe_free(szTemp);
	return 1;
}


/* Con_ClearOldestEntry - simply deletes the oldest entry in the console.
	Called when the number of allowed entries reaches it's max. */
int Con_ClearOldestEntry(HLCONSOLE hConsole)
{
	LPLCENTRY lpCurrent=NULL;
	LPLCENTRY lpPrev=NULL;

	LPLCONSOLE lpConsole=(LPLCONSOLE)hConsole;

	if(!lpConsole)
		return 0;

	if(lpConsole->lpEntryList==NULL)
		return 0;

	lpCurrent=lpConsole->lpEntryList;

	if(lpCurrent->lpNext==NULL)
		return 1;

	while(lpCurrent->lpNext)
	{
		lpPrev=lpCurrent;
		lpCurrent=lpCurrent->lpNext;
	}

	/* Remove the last entry and set it's previous entry as the
		last entry. */
	L_safe_free(lpCurrent->lpstrText);
	L_safe_free(lpCurrent);
	lpPrev->lpNext=NULL;
	g_nNumEntries--;
	lpConsole->dwNumEntries--;
	return 1;
}


/* Con_AddEntry - adds an entry to the console's text list. */
int Con_AddEntry(HLCONSOLE hConsole, char* szEntry)
{
	LPLCENTRY lpCurrent=NULL;
	LPLCENTRY lpNew=NULL;

	LPLCONSOLE lpConsole=(LPLCONSOLE)hConsole;

	if(!lpConsole)
		return 0;

	
	lpCurrent=lpConsole->lpEntryList;

	lpNew=malloc(sizeof(LCENTRY));

	if(lpNew==NULL)
	{
		return 0;
	}
	lpNew->lpstrText=malloc(sizeof(char)*lpConsole->nMaxStrLen);


	if(lpNew->lpstrText==NULL)
	{
		L_safe_free(lpNew);
		return 0;
	}
	lpNew->lpNext=lpConsole->lpEntryList;
	lpConsole->lpEntryList=lpNew;

	lpConsole->lpActiveEntry=lpConsole->lpEntryList;

	L_strncpy(lpConsole->lpEntryList->lpstrText, szEntry, lpConsole->nMaxStrLen);
	lpConsole->dwNumEntries++;
	g_nNumEntries++;
	if(lpConsole->dwNumEntries > lpConsole->nMaxEntries)
		Con_ClearOldestEntry(lpConsole);

	return 1;
}


/* ======================================================
	Legacy Console Public Functions.
	====================================================== */

/* Con_Create - Creates the console. */
HLCONSOLE Con_Create(
	LPCONCOMMAND lpCommandFunc, 
	unsigned long nMaxStrlen, 
	unsigned long nMaxEntries,
	unsigned long dwFlags,
	void* pExtraData)
{
	/* Allocate the console to memory. */
	LPLCONSOLE lpConsole=NULL;
	lpConsole=malloc(sizeof(LCONSOLE));
	if(lpConsole==NULL)
	{
		/*printf("Failed to initialize Legacy Console!\n");*/
		return NULL;
	}
	
	/* We have a few requirements for the console's limits, despite what the user
		asks for.*/
	#define CMIN_REQUIRED (64)
	#define CMAX_ALLOWED (2048)


	/* Set the initial values for the console. */
	lpConsole->CommandFunction=lpCommandFunc;
	lpConsole->pExtraData=pExtraData;
	lpConsole->dwNumEntries=0;
	lpConsole->lpActiveEntry=NULL;
	lpConsole->lpEntryList=NULL;
	lpConsole->nMaxEntries=nMaxEntries;
	/* Insure that the string lenth is greater than 64, but less than 1024. */
	lpConsole->nMaxStrLen=nMaxStrlen<CMIN_REQUIRED ? CMIN_REQUIRED : (nMaxStrlen>CMAX_ALLOWED) ? CMAX_ALLOWED : nMaxStrlen;
	if((CONCREATE_USEINTERNAL&dwFlags)==CONCREATE_USEINTERNAL)
		lpConsole->bProcessInternal=1; /* Whether or not the conosle should process internal commands. */
	else 
		lpConsole->bProcessInternal=0;

	/* Add a blank entry. */
	Con_AddEntry((HLCONSOLE)lpConsole, "");

	lpConsole->cvarlist=NULL;
	
	lpConsole->commands=Defs_CreateDefs();
	if(!lpConsole->commands)
		Con_SendMessage(lpConsole, "Could not register command defs, some functions may be unusable.");
	
	/* If using built in commands register them. */
	if((CONCREATE_USEINTERNAL&dwFlags)==CONCREATE_USEINTERNAL)
	{
		#define CON_REG(a) Con_RegisterCmd(lpConsole, #a, CONF_##a)

		CON_REG(SET);
		CON_REG(GET);
		CON_REG(LOADCFG);
		CON_REG(CLEAR);
		CON_REG(ECHO);
		CON_REG(DEFINE);
		CON_REG(CVARLIST);
		CON_REG(REGCVAR);
		CON_REG(SAVECFG);
		CON_REG(CONDUMP);
		CON_REG(CMDLIST);
		CON_REG(CVARUPDATE);
	}

	/* Check to see if we should create a cvarlist. */
	/*
	if((CONCREATE_USECVARLIST&dwFlags)==CONCREATE_USECVARLIST)
	{
		lpConsole->cvarlist=CVar_CreateList(lpConsole);	
	}
	*/
	/*#define COPYRIGHT_TEXT*/
	#ifdef COPYRIGHT_TEXT
	Con_SendMessage((HLCONSOLE)lpConsole, "Legacy Console copyright (c) 2006, Blaine Myers.");
	#endif /* COPYRIGHT_TEXT */
	/* Con_SendMessage((HLCONSOLE)lpConsole, "Successfully initialized Legacy Console."); */
	return (HLCONSOLE)lpConsole;
}

/* Con_Delete - Deletes the console. */
int Con_Delete(HLCONSOLE hConsole)
{
	LPLCENTRY lpCurrent=NULL, lpNext=NULL;
	LPLCONSOLE lpConsole=(LPLCONSOLE)hConsole;
	/* Delete the list of entrys. */
	if(!hConsole)
		return 0;


	lpCurrent=((LPLCONSOLE)hConsole)->lpEntryList;
	while(lpCurrent!=NULL)
	{
		lpNext=lpCurrent->lpNext;
		/* Clear the text string. */
		L_safe_free(lpCurrent->lpstrText);
		/* Clear the entry. */
		L_safe_free(lpCurrent);
		lpConsole->dwNumEntries--;
		g_nNumEntries--;
		lpCurrent=lpNext;
		
	}
	Debug_printf("Console entries left: %i\n", g_nNumEntries);
	((LPLCONSOLE)hConsole)->lpActiveEntry=NULL;
	/* Delete the bg, and font. */
	
	((LPLCONSOLE)hConsole)->CommandFunction=NULL;
	if(((LPLCONSOLE)hConsole)->commands)
		Defs_DeleteDefs(((LPLCONSOLE)hConsole)->commands);

	/*
	if(((LPLCONSOLE)hConsole)->cvarlist)
		CVar_DeleteList(((LPLCONSOLE)hConsole)->cvarlist);
	*/

	L_safe_free(hConsole);

	return 1;
}


/* Con_SendMessage - Adds a string to the console. */
int Con_SendMessage(HLCONSOLE hConsole, char* szMessage)
{
	char* szTemp=NULL;

	int nResult=0;

	LPLCONSOLE lpConsole=(LPLCONSOLE)hConsole;
	if(!lpConsole)
		return 0;

	/* Allocate memory for the temporary string.
		We free this when the function returns. */
	szTemp=malloc(lpConsole->nMaxStrLen);

	if((szTemp==NULL))
		return 0;

	/* Note that when we copy strings, we do one less than the max string length, because
		the null terminating 0, is not counted as a character count in L_strncpy. */
	L_strncpy(szTemp, lpConsole->lpActiveEntry->lpstrText, lpConsole->nMaxStrLen);
	/* Copy in the new line. */
	L_strncpy(lpConsole->lpActiveEntry->lpstrText, szMessage, lpConsole->nMaxStrLen);

	/* Re-Add the old line. */
	nResult=Con_AddEntry(lpConsole, szTemp);

	L_safe_free(szTemp);
	return nResult;
}

/* Con_SendErrorMessage, outputs formated text to the console. 
	Use as you would sprintf except that the first paramter is
	the console, not a string.*/
int Con_SendErrorMsg(HLCONSOLE hConsole, char* format, ...)
{
	char* szOutput=NULL;
	va_list arglist=NULL;
	int nResult=0;

	if(!hConsole)
		return 0;

	/* Allocate memory for output, we free when we exit the function. */
	szOutput=malloc( ((LPLCONSOLE)hConsole)->nMaxStrLen );

	if(!szOutput)
		return 0;

	/* We use _vsnprintf so we can print arguments, and also so we can limit the
		number of characters, put to the output buffer by the max string length
		allowed in the console. */
	va_start(arglist, format);
	_vsnprintf(szOutput, ((LPLCONSOLE)hConsole)->nMaxStrLen-1, format, arglist);
	va_end(arglist);
	nResult=Con_SendMessage(hConsole, szOutput);
	L_safe_free(szOutput);
	return nResult;
}

/* Con_SendCommand - Sends a command to the console, if internal functions are
	active then it will attempt to process one of those, then it will send the
	function to the external command, the function command will be changed to 
	the appropriate value, based of the definition.  The parameters are all in
	one string.*/
int Con_SendCommand(HLCONSOLE hConsole, char* szCmdLine)
{
	char* szParams=NULL;
	char* szCommand=NULL;
	char szParseIgnore[]=" ,()\n\r\t";
	unsigned long dwLen=0;
	LPLCONSOLE lpConsole=NULL;
	int nResult=0;
	int nCommand=0;

	lpConsole=(LPLCONSOLE)hConsole;

	if(lpConsole==NULL)
		return 0;

	dwLen=L_strlen(szCmdLine);

	if(dwLen<1)
	{
		Con_SendMessage(hConsole, "");
	}

	/* Allocate memory for the string buffers, 
		we need to free these when the function exits. */
	szParams=malloc(lpConsole->nMaxStrLen);
	szCommand=malloc(lpConsole->nMaxStrLen);

	if( (!szParams) || (!szCommand) )
	{
		L_safe_free(szParams);
		L_safe_free(szCommand);
		return 0;
	}

	if(!Con_SimpleParse(szCommand, szParams, szParseIgnore, szCmdLine, lpConsole->nMaxStrLen))
	{
		L_safe_free(szParams);
		L_safe_free(szCommand);
		return 0;
	}

	if((nCommand=Con_CmdNameToValue(hConsole, szCommand))==0)
	{
		Con_SendErrorMsg(hConsole, "\"%s\" not recognized as a valid command.", szCommand);
		L_safe_free(szParams);
		L_safe_free(szCommand);
		return 0;
	}

	/* First attempt to process internal commands, if asked to if we
		process an internal command we return, if not we process the
		command function. */
	if(lpConsole->bProcessInternal)
	{
		if((nResult=Con_InternalCommands(nCommand, szParams, hConsole)))
		{
			L_safe_free(szParams);
			L_safe_free(szCommand);
			return nResult;
		}
	}

	if(!lpConsole->CommandFunction)
	{
		Con_SendMessage(hConsole, "Warning: No CommandFunction registered.");
	}
	else
	{
		if(!(nResult=lpConsole->CommandFunction(nCommand, szParams, hConsole, lpConsole->pExtraData)))
			Con_SendErrorMsg(hConsole, "Could not process \"%s\" command.", szCommand);
	}

	L_safe_free(szParams);
	L_safe_free(szCommand);
	return nResult;
}


/* Con_SetCurrent - Sets the active entry. */
int Con_SetCurrent(HLCONSOLE hConsole, char* szText)
{
	if(szText==NULL)
		return 0;

	if(hConsole==NULL)
		return 0;

	if(!((LPLCONSOLE)hConsole)->lpActiveEntry)
		return 0;

	L_strncpy(
		((LPLCONSOLE)hConsole)->lpActiveEntry->lpstrText, 
		szText, 
		((LPLCONSOLE)hConsole)->nMaxStrLen);
	return 1;
}

/* Con_EraseCurrent - Clears the active entry. */
int Con_EraseCurrent(
	HLCONSOLE lpConsole)
{
	if(lpConsole==NULL)
		return 0;

	if(!((LPLCONSOLE)lpConsole)->lpActiveEntry)
		return 0;

	((LPLCONSOLE)lpConsole)->lpActiveEntry->lpstrText[0]=0;
	return 1;
}


/* Con_Clear - clears all entries out of the console. */
int Con_Clear(HLCONSOLE hConsole)
{
	LPLCENTRY lpCurrent=NULL;
	LPLCENTRY lpNext=NULL;

	LPLCONSOLE lpConsole=(LPLCONSOLE)hConsole;

	if(!lpConsole)
		return 0;

	lpCurrent=lpConsole->lpEntryList;
	while(lpCurrent!=NULL)
	{
		lpNext=lpCurrent->lpNext;
		/* Clear the text string. */
		L_safe_free(lpCurrent->lpstrText);
		/* Clear the entry. */
		L_safe_free(lpCurrent);
		lpCurrent=lpNext;
	}
	lpConsole->lpEntryList=NULL;
	lpConsole->lpActiveEntry=NULL;
	lpConsole->dwNumEntries=0;
	Con_AddEntry(lpConsole, "");
	return 1;
}


/* Con_GetNumEntries - returns the number of strings currently in the console,
	this includes the active string. */
unsigned long Con_GetNumEntries(HLCONSOLE hConsole)
{
	if(hConsole==NULL)
		return 0;

	return ((LPLCONSOLE)hConsole)->dwNumEntries;
}

/* Con_GetEntry - Gets the selected string from the console. */
char* Con_GetEntry(HLCONSOLE hConsole, unsigned long nEntry, char* szOut)
{
	LPLCONSOLE lpConsole=NULL;
	LPLCENTRY lpCurrent=NULL;
	unsigned long i;

	if(hConsole==NULL)
		return NULL;

	lpConsole=(LPLCONSOLE)hConsole;

	if(nEntry>lpConsole->dwNumEntries)
		return NULL;


	lpCurrent=lpConsole->lpEntryList;

	if(lpCurrent==NULL)
		return NULL;

	if(nEntry<1)
		return NULL;

	if(nEntry==1)
	{
		if(szOut)
			L_strncpy(
				szOut, 
				lpConsole->lpActiveEntry->lpstrText, 
				L_strlen(lpConsole->lpActiveEntry->lpstrText)+1);

		return lpConsole->lpActiveEntry->lpstrText;
	}


	for(i=1; i<nEntry; i++)
	{
		lpCurrent=lpCurrent->lpNext;
	}

	if(szOut)
		L_strncpy(szOut, lpCurrent->lpstrText, lpConsole->nMaxStrLen);

	return lpCurrent->lpstrText;
}

/* Con_RegisterCmd is used to register a command name, with a value, the value
	will be passed to the CommandFunction as the command parameter, then the user
	can use a switch to find out what to do. */
int Con_RegisterCmd(HLCONSOLE hConsole, char* szCmdName, unsigned long nValue)
{
	LPLCONSOLE lpConsole=(LPLCONSOLE)hConsole;

	if(!lpConsole)
		return 0;

	if(!lpConsole->commands)
	{
		Con_SendMessage(hConsole, "No command definitions loaded.");
		return 0;
	}

	if(!Defs_Add(lpConsole->commands, szCmdName, (float)nValue))
	{
		Con_SendErrorMsg(hConsole, "Could not register command \"%s\", name may be invalid.", szCmdName);
		return 0;
	}
	return 1;
}

/* Con_CmdNameToValue will change a command name to it's value that was registered with
	Con_RegisterCmd. */
unsigned long Con_CmdNameToValue(HLCONSOLE hConsole, const char* szCmdName)
{
	LPLCONSOLE lpConsole=(LPLCONSOLE)hConsole;
	int bGotValue=0;
	float fValue=0;

	if(!lpConsole)
		return 0;

	if(!lpConsole->commands)
	{
		Con_SendMessage(hConsole, "No command definitions loaded.");
		return 0;
	}

	fValue=Defs_Get(lpConsole->commands, (char*)szCmdName, &bGotValue);
	if(!bGotValue)
	{
		return 0;
	}
	return (unsigned long)fValue;
}



/* Con_OnChar does is for the user to call when a button is pressed, for
	most characters it will append it to the end of the active string, but
	for backspace it will delete one character, and for return it will send
	the line to the command function. A lot of characters are ignored, though.*/
int Con_OnChar(
	HLCONSOLE hConsole,
	unsigned short cChar)
{
	unsigned long dwLen=0;
	LPLCONSOLE lpConsole=NULL;

	lpConsole=(LPLCONSOLE)hConsole;

	if(lpConsole==NULL)
		return 0;

	/* If no active entry, then add an entry. */
	if(lpConsole->lpActiveEntry==NULL)
		Con_AddEntry(lpConsole, "");

	dwLen=L_strlen(lpConsole->lpActiveEntry->lpstrText);
	/* Do any necessary actions depending on cChar. */
	switch(cChar)
	{
	case '\r':
		Con_SendCommand(lpConsole, lpConsole->lpActiveEntry->lpstrText);
		Con_EraseCurrent(lpConsole);
		break;
	case '\b':
		if(dwLen>0)
		{
			/* Remove last char, and add null-termination. */
			lpConsole->lpActiveEntry->lpstrText[dwLen-1]=0;
		}
		break;
	default:
		/* The funcion made it here if a standard key was pressed. */
		/* Break if line has reached it's max limit. */
		if(dwLen >= (lpConsole->nMaxStrLen-1))
			return 0;
		
		/* Add the char into the string and a null-termination. */
		if((cChar >= ' ') && (cChar <= '~'))
		{
			lpConsole->lpActiveEntry->lpstrText[dwLen]=(char)cChar;
			lpConsole->lpActiveEntry->lpstrText[dwLen+1]=0;
		}
		break;
	};


	return 1;
}

int Con_AttachCVarList(HLCONSOLE hConsole, void* cvarlist)
{
	LPLCONSOLE lpConsole=NULL;
	lpConsole=(LPLCONSOLE)hConsole;
	if(lpConsole==NULL)
		return 0;

	lpConsole->cvarlist=cvarlist;
	Con_SendErrorMsg(hConsole, "Attached cvarlist at address 0x%08X to console.", cvarlist);
	return 1;
}

/*
void* Con_GetCVar(HLCONSOLE hConsole, char* name)
{
	LPLCONSOLE lpConsole=NULL;
	lpConsole=(LPLCONSOLE)hConsole;
	if(lpConsole==NULL)
		return 0;

	return CVar_GetCVar(lpConsole->cvarlist, name);
}

void* Con_GetCVarList(HLCONSOLE hConsole)
{
	LPLCONSOLE lpConsole=NULL;
	lpConsole=(LPLCONSOLE)hConsole;
	if(lpConsole==NULL)
		return 0;
	return lpConsole->cvarlist;
}
*/



#ifdef _DEBUG
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
		OutputDebugString("MEMORY USAGE FOR \"lc_sys.dll\"\"\n");
		_CrtMemCheckpoint(&s2);
		_CrtMemDifference(&s3, &s1, &s2);
		_CrtMemDumpStatistics(&s3);
		_CrtDumpMemoryLeaks();
		break;
	}
	return TRUE;
}
#endif /*_DEBUG*/
