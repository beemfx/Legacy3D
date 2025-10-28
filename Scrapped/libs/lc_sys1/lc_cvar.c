#include <stdio.h>
#include "lg_malloc.h"
#include "lc_sys.h"
#include "lc_private.h"
#include "common.h"

#include <windows.h>

unsigned long g_nNumCVars=0;


/* Private function declarations. */
float CVar_TextToFloat(char* szValue);

typedef struct tagCVarList{
	CVar* varlist; /* Pointer to the list. */
	void* console; /* Pointer to a console. */
	HLDEFS defs; /* Definitions list. */
}CVarList, *LPCVarList;

/* ============================================
	Public Functions.
	============================================ */
HCVARLIST CVar_CreateList(void* hConsole)
{
	CVarList* lpNewList=NULL;
	/* All we really need to do to generate a new cvar list, is allocate the
		memory and set a few default values, and attach the console if there is
		one.  We set the actual list to NULL since there are no variables yet. */
	lpNewList=LG_Malloc(sizeof(CVarList));
	if(lpNewList==NULL)
	{
		if(hConsole)
			Con_SendMessage(hConsole, "Could not allocate memory for cvarlist!");
	
		return NULL;
	}

	lpNewList->varlist=NULL;
	/* Note that if the console is NULL, the cvarlist knows not to use it. */
	lpNewList->console=hConsole;


	/* Attempt to create a definitions list, if the call doesn't work the feature
		will be unusable. */
	lpNewList->defs=Defs_CreateDefs();

	if(lpNewList->defs==NULL)
		if(hConsole)
			Con_SendMessage(hConsole, "Could not allocate memory for Definitions, feature unusable!");

	return lpNewList;
}

int CVar_DeleteList(HCVARLIST hList)
{
	/* We need to delete all the values, and then delete the list. */
	CVar* var=NULL;
	CVar* oldvar=NULL;
	CVarList* lpList=(CVarList*)hList;

	if(lpList==NULL)
		return 0;
	
	/* Free all allocated memory. */
	
	var=lpList->varlist;
	while(var)
	{
		L_safe_free(var->name);
		L_safe_free(var->string);
		g_nNumCVars--;
		oldvar=var;
		var=var->next;
		L_safe_free(oldvar);
	}

	Debug_printf("Number of CVars left: %i\n", g_nNumCVars);
	

	Defs_DeleteDefs(lpList->defs);
	lpList->console=NULL;
	L_safe_free(hList);
	return 1;
}


/* CVar_Register registers a cvar into the list. */
CVar* CVar_Register(HCVARLIST hList, const char* szName, const char* szValue, unsigned long dwFlags)
{
	CVar* newvar=NULL;
	CVarList* lpList=(CVarList*)hList;

	char szOutOfMem[]="Cannot register \"%s\", out of memory!";

	if(lpList==NULL)
		return NULL;

	/* Need to check if the cvar has a valid name, if not we can't create it. */
	if(!L_CheckValueName(szName, NULL, NULL))
	{
		if(lpList->console)
			Con_SendErrorMsg(lpList->console, "Cannot register \"%s\", invalid name!", szName);

		return NULL;
	}

	/* Check to see if a CVar with the same name is already registered. */
	if(CVar_GetCVar(hList, szName))
	{
		if(lpList->console)
			Con_SendErrorMsg(lpList->console, "Cannot register \"%s\", cvar already exists!", szName);

		return NULL;
	}

	/* Allocate memory vor CVarList. */
	newvar=LG_Malloc(sizeof(CVar));
	if(!newvar)
	{
		if(lpList->console)
			Con_SendErrorMsg(lpList->console, szOutOfMem, szName);
		return NULL;
	}

	/* Allocate space for the cvar's name. */
	newvar->name=LG_Malloc(L_strlen(szName)+1);
	if(newvar->name==NULL)
	{
		L_safe_free(newvar);
		if(lpList->console)
			Con_SendErrorMsg(lpList->console, szOutOfMem, szName);
		return NULL;
	}
	L_strncpy(newvar->name, szName, L_strlen(szName)+1);
	/* Allocate space for the cvar's value. */
	
	newvar->string=NULL;
	newvar->value=0.0f;
	newvar->flags=dwFlags;
	newvar->next=LG_NULL;
	//newvar->update=((dwFlags&CVAREG_UPDATE)==CVAREG_UPDATE);
	//newvar->save=((dwFlags&CVAREG_SAVE)==CVAREG_SAVE);
	//newvar->nosetchange=L_CHECK_FLAG(dwFlags, CVAREG_SETWONTCHANGE);

	/* Add to the linked list. */
	/* TODO: We'll add the cvars in alphabetical order so search through
		the list til we find one that is higher in lower in the alphabed
		and at the cvar there.*/
	/* For now the new cvar is added on the end, this is so they
		get saved in the same order they were registered. */
	if(lpList->varlist==LG_NULL)
	{
		newvar->next=LG_NULL;
		lpList->varlist=newvar;
	}
	else
	{
		CVar* pItem;
		for(pItem=lpList->varlist; pItem; pItem=pItem->next)
		{
			CVar* pNext=pItem->next;
			if(!pNext)
			{
				pItem->next=newvar;
				break;
			}
			/*
			if((stricmp(newvar->name, pItem->name)<0))
			{
				pItem->next=newvar;
				newvar->next=pNext;
				break;
			}
			*/
		}
	}
	//newvar->next=lpList->varlist;
	//lpList->varlist=newvar;
	g_nNumCVars++;
	/* Now set the new value using the set function. */
	
	CVar_Set(lpList, szName, szValue);

	if(lpList->console)
		Con_SendErrorMsg(lpList->console, "Registered %s = \"%s\", %.0f", newvar->name, newvar->string, newvar->value);
	return newvar;
}

/* CVar_Set sets the selected cvar. */
int CVar_Set(HCVARLIST hList, const char* szName, const char* szValue)
{
	CVarList* list=(CVarList*)hList;
	CVar* cvar=NULL;
	int bGotDef=0;

	if(!list)
		return 0;

	cvar=CVar_GetCVar(hList, szName);
	if(!cvar)
	{
		if(list->console)
			Con_SendErrorMsg(list->console, "Could not set \"%s\", no such cvar!", szName);

		return 0;
	}

	L_safe_free(cvar->string);

	cvar->string=LG_Malloc(L_strlen(szValue)+1);
	if(cvar->string==NULL)
	{
		if(list->console)
			Con_SendErrorMsg(list->console, "Could not reallocate memory for \"%s\"!", szName);

		return 0;
	}

	L_strncpy(cvar->string, szValue, L_strlen(szValue)+1);
	/* We also want to check to see if the value, is a definition, if it is we,
		give the value that parameter. If it is not a definition, then we try to
		convert the value to a number, if it isn't a number, the value will be 0.0f.*/

	cvar->value=Defs_Get(list->defs, szValue, &bGotDef);

	if(!bGotDef)
		cvar->value=CVar_TextToFloat(cvar->string);

	return 1;
}

int CVar_SetValue(HCVARLIST hList, const char* szName, const float fValue)
{
	char sz[32];
	_snprintf(sz, 32, "%f", fValue);
	return CVar_Set(hList, szName, sz);
}

/* CVar_Get gets the string value of a cvar. */
char* CVar_Get(HCVARLIST hList, const char* szName, char* szOutput)
{
	CVar* cvar=NULL;
	if(hList==NULL)
		return NULL;

	cvar=CVar_GetCVar(hList, szName);

	if(cvar==NULL)
		return NULL;

	if(szOutput)
		L_strncpy(szOutput, cvar->string, L_strlen(cvar->string)+1);

	return cvar->string;
}

float CVar_GetValue(HCVARLIST hList, const char*szName, int* bGotValue)
{
	CVar* cvar=NULL;
	if(hList==NULL)
	{
		if(bGotValue)
			*bGotValue=0;
		return 0.0f;
	}

	cvar=CVar_GetCVar(hList, szName);

	if(cvar==NULL)
	{
		if(bGotValue)
			*bGotValue=0;
		return 0.0f;
	}

	if(bGotValue)
		*bGotValue=1;
	return cvar->value;
}


/* CVar_GetCVar returns a pointer to the cvar with the slected name. */
CVar* CVar_GetCVar(HCVARLIST hList, const char* szName)
{
	CVar* var=NULL;
	CVarList* lpList=(CVarList*)hList;

	if(!lpList)
		return NULL;

	for(var=lpList->varlist; var; var=var->next)
	{
		if(L_strnicmp(szName, var->name, 0))
			return var;
	}

	return NULL;
}


CVar* CVar_GetFirstCVar(HCVARLIST hList)
{
	if(!hList)
		return NULL;

	return ((CVarList*)hList)->varlist;
}

int CVar_AddDef(HCVARLIST hList, const char* szDef, const float fValue)
{
	CVarList* lpList=(CVarList*)hList;

	if(!lpList)
		return 0;

	return Defs_Add(lpList->defs, szDef, fValue);
}

float CVar_GetDef(HCVARLIST hList, const char* szDef, int* bGotDef)
{
	if(!hList)
	{
		if(bGotDef)*bGotDef=0;
		return 0.0f;
	}
	return Defs_Get(((CVarList*)hList)->defs, szDef, bGotDef);
}

/* ===============================================
	Private Functions.
	=============================================== */

/* CVar_TextToFloat converts a text value to a float value. */
float CVar_TextToFloat(char* szValue)
{
	return (float)L_atovalue(szValue);
}
