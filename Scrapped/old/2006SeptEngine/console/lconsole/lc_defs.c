/* ldefs.c - code for definitions for the console. */
#include <stdio.h>
#include <malloc.h>
#include <common.h>
#include "lc_private.h"

unsigned long g_nDefRefs=0;


HLDEFS Defs_CreateDefs()
{
	LDefs* newdef=NULL;

	newdef=malloc(sizeof(LDefs));

	if(newdef==NULL)
		return NULL;

	newdef->list=NULL;
	return newdef;
}

int Defs_DeleteDefs(HLDEFS hDefs)
{
	LDefs* lpDefs=(LDefs*)hDefs;
	LDef* def=NULL;
	LDef* nextdef=NULL;

	if(!lpDefs)
		return 0;

	def=lpDefs->list;
	while(def)
	{
		nextdef=def->next;
		L_safe_free(def->name);
		L_safe_free(def);
		g_nDefRefs--;
		def=nextdef;
	}
	Debug_printf("Number of definitions left: %i\n", g_nDefRefs);
	L_safe_free(lpDefs);
	hDefs=NULL;
	return 1;
}

int Defs_Add(HLDEFS* hDefs, char* szDef, float fValue)
{
	LDefs* lpDefs=(LDefs*)hDefs;
	LDef* newdef=NULL;
	int bCheck=0;

	if(lpDefs==NULL)
		return 0;

	/* First check to see if the definition already exists.  If it does the function
		fails.*/
	Defs_Get(hDefs, szDef, &bCheck);
	if(bCheck)
	{
		//return Defs_ReDef(hDefs, szDef, fValue);
		return 0;
	}

	/* Now we need to check if the name is valid.  We don't want the first letter to
		be a number, and we don't want selected characters in our def.*/
	if(!L_CheckValueName(szDef, NULL, NULL))
		return 0;
	/* Allocate memory for the new def. */
	newdef=malloc(sizeof(LDef));
	if(!newdef)
		return 0;

	newdef->name=malloc(L_strlen(szDef)+1);
	if(!newdef->name)
	{
		free(newdef);
		return 0;
	}
	L_strncpy(newdef->name, szDef, L_strlen(szDef)+1);
	newdef->value=fValue;
	newdef->next=lpDefs->list;
	lpDefs->list=newdef;
	g_nDefRefs++;
	return 1;
}

int Defs_ReDef(HLDEFS hDef, char* szName, float fNewValue)
{
	
	LDefs* lpDefs=(LDefs*)hDef;
	LDef* def=NULL;

	if(!lpDefs)
		return 0;

	for(def=lpDefs->list; def; def=def->next)
	{
		if(L_strnicmp(szName, def->name, L_strlen(szName)))
		{
			def->value=fNewValue;
			return 1;
		}
	}
	return 0;
}

float Defs_Get(HLDEFS hDef, char* szName, int* bGotDef)
{
	LDefs* lpDefs=(LDefs*)hDef;
	LDef* def=NULL;
	unsigned long nLen=0, nLen2=0;;

	if(bGotDef)
		*bGotDef=0;

	if(!lpDefs)
		return 0.0f;

	nLen=L_strlen(szName);

	for(def=lpDefs->list; def; def=def->next)
	{
		if(L_strnicmp(szName, def->name, 0))
		{
			if(bGotDef)
				*bGotDef=1;

			return def->value;
		}
	}
	return 0.0f;
}

/*
LDef* Defs_GetFirst(HLDEFS hDef)
{
	LDefs *lpDefs=(LDefs*)hDef;

	if(!lpDefs)
		return L_null;
	else return lpDefs->list;
}
*/
