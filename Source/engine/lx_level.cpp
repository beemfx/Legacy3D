#include "lx_sys.h"
#include "lg_stack.h"
#include "lg_err.h"
#include "lg_xml.h"
#include "lg_malloc.h"
#include "lg_func.h"
#include <string.h>

typedef enum _LX_LEVEL_MODE{
	LEVEL_MODE_NONE=0,
	LEVEL_MODE_UNKNOWN,
	LEVEL_MODE_LEVEL,
	LEVEL_MODE_MAP,
	LEVEL_MODE_SKYBOX,
	LEVEL_MODE_SKYBOX_SKIN,
	LEVEL_MODE_MUSIC,
	LEVEL_MODE_OBJECTS,
	LEVEL_MODE_OBJECT,
	LEVEL_MODE_ACTORS,
	LEVEL_MODE_ACTOR
}LX_LEVEL_MODE;

typedef struct _lx_level_data{
	lx_level* pLevel;
	lg_dword nCurObj;
	lg_dword nCurActor;
	CLStack<LX_LEVEL_MODE> stkMode;
}lx_level_data;

void LX_LevelStart(void* userData, const XML_Char* name, const XML_Char** atts);
void LX_LevelEnd(void* userData, const XML_Char* name);
void LX_LevelCharData(void* userData, const XML_Char*s, int len);

lx_level* LX_LoadLevel(const lg_path szXMLFile)
{
	Err_Printf("LX_LoadLevel: Reading \"%s\"...", szXMLFile);
	LF_FILE3 fileScript=LF_Open(szXMLFile, LF_ACCESS_READ|LF_ACCESS_MEMORY, LF_OPEN_EXISTING);
	if(!fileScript)
	{
		Err_Printf("LX_LoadLevel ERROR: Could not open \"%s\" for parsing.", szXMLFile);
		return LG_NULL;
	}
	
	//We need a new instance of the output data
	lx_level* pOut = (lx_level*)LG_Malloc(sizeof(lx_level));
	memset(pOut, 0, sizeof(lx_level));
	
	XML_Parser parser = XML_ParserCreate_LG(LG_NULL);
	if(!parser)
	{
		Err_Printf("LX_LoadLevel ERROR: Could not create XML parser. (%s)", szXMLFile);
		LF_Close(fileScript);
		return LG_NULL;
	}
	
	//Setup parsing.
	lx_level_data data;
	data.stkMode.Push(LEVEL_MODE_NONE);
	data.pLevel=pOut;
	XML_ParserReset(parser, LG_NULL);
	XML_SetUserData(parser, &data);
	XML_SetElementHandler(parser, LX_LevelStart, LX_LevelEnd);
	XML_SetCharacterDataHandler(parser, LX_LevelCharData);
	
	//Do parsing.
	if(!XML_Parse(parser, (const char*)LF_GetMemPointer(fileScript), LF_GetSize(fileScript), LG_TRUE))
	{
		Err_Printf("LX_LoadObject ERROR: Pares error at line %d: \"%s\".",
			XML_GetCurrentLineNumber(parser),
			XML_ErrorString(XML_GetErrorCode(parser)));
			
		LX_DestroyLevel(pOut);
		LF_Close(fileScript);
		XML_ParserFree(parser);
		return LG_NULL;
	}
	
	XML_ParserFree(parser);
	LF_Close(fileScript);
	return pOut;
}

lg_void LX_DestroyLevel(lx_level* pLevel)
{
	if(!pLevel)
		return;
		
	LG_SafeFree(pLevel->pObjs);
	LG_SafeFree(pLevel->pActors);
	LG_SafeFree(pLevel);
}

/* PRE: Accepts a name for the mode.
	POST: Returns the numerical value for
	the mode.
*/
LX_LEVEL_MODE LX_LevelNameToMode(const XML_Char* name)
{
	LX_LEVEL_MODE nMode=LEVEL_MODE_UNKNOWN;
	
	if(stricmp(name, "level")==0)
		nMode=LEVEL_MODE_LEVEL;
	else if(stricmp(name, "map")==0)
		nMode=LEVEL_MODE_MAP;
	else if(stricmp(name, "skybox")==0)
		nMode=LEVEL_MODE_SKYBOX;
	else if(stricmp(name, "skybox_skin")==0)
		nMode=LEVEL_MODE_SKYBOX_SKIN;
	else if(stricmp(name, "music")==0)
		nMode=LEVEL_MODE_MUSIC;
	else if(stricmp(name, "objects")==0)
		nMode=LEVEL_MODE_OBJECTS;
	else if(stricmp(name, "object")==0)
		nMode=LEVEL_MODE_OBJECT;
	else if(stricmp(name, "actors")==0)
		nMode=LEVEL_MODE_ACTORS;
	else if(stricmp(name, "actor")==0)
		nMode=LEVEL_MODE_ACTOR;
	
	return nMode;
}

void LX_LevelStart(void* userData, const XML_Char* name, const XML_Char** atts)
{
	lx_level_data* pData=(lx_level_data*)userData;
	LX_LEVEL_MODE nMode=LX_LevelNameToMode(name);
	//We'll always push the mode onto the stack even it was invalid
	//that way when the values get popped off the stack they
	//will always match.
	pData->stkMode.Push(nMode);
	
	//If the mode was NONE or UNKNWOWN there is nothing to do.
	if(nMode==LEVEL_MODE_UNKNOWN || nMode==LEVEL_MODE_NONE)
	{
		Err_Printf("LX_Level Parse ERROR: Invalid tag (%s).", name);
	}
	else if(nMode==LEVEL_MODE_MAP)
	{
		pData->pLevel->szMapFile[0]=0;
	}
	else if(nMode==LEVEL_MODE_SKYBOX)
	{
		pData->pLevel->szSkyBoxFile[0]=0;
	}
	else if(nMode==LEVEL_MODE_SKYBOX_SKIN)
	{
		pData->pLevel->szSkyBoxSkin[0]=0;
	}
	else if(nMode==LEVEL_MODE_MUSIC)
	{
		pData->pLevel->szMusicFile[0]=0;
	}
	else if(nMode==LEVEL_MODE_OBJECTS)
	{
		for(lg_dword i=0; atts[i]; i+=2)
		{
			if(stricmp(atts[i], "object_count")==0)
			{
				pData->pLevel->nObjCount=atoi(atts[i+1]);
			}
		}
		
		//Should allocate memory for objects here:
		//Free the data in case there is more than one call to the object tag.
		LG_SafeFree(pData->pLevel->pObjs);
		pData->pLevel->pObjs=(lx_level_object*)LG_Malloc(sizeof(lx_level_object)*pData->pLevel->nObjCount);
		pData->nCurObj=0;
	}
	else if(nMode==LEVEL_MODE_ACTORS)
	{
		for(lg_dword i=0; atts[i]; i+=2)
		{
			if(stricmp(atts[i], "actor_count")==0)
			{
				pData->pLevel->nActorCount=atoi(atts[i+1]);
			}
		}
		
		//Should allocate memory:
		LG_SafeFree(pData->pLevel->pActors);
		pData->pLevel->pActors=(lx_level_object*)LG_Malloc(sizeof(lx_level_object)*pData->pLevel->nActorCount);
		pData->nCurActor=0;
	}
	else if(nMode==LEVEL_MODE_OBJECT || nMode==LEVEL_MODE_ACTOR)
	{
		//Err_Printf("%d/%d ACTORS.", pData->nCurActor, pData->pLevel->nActorCount);
		//Err_Printf("%d/%d OBJECTS.", pData->nCurObj, pData->pLevel->nObjCount);
		
		//We can use the same code for actors and objects because
		//the both have the same parameters (script and position).
		lg_path szScript;
		lg_float fPos[3];
		
		for(lg_dword i=0; atts[i]; i+=2)
		{
			if(stricmp(atts[i], "script")==0)
			{
				LG_strncpy(szScript, atts[i+1], LG_MAX_PATH);
			}
			else if(stricmp(atts[i], "position")==0)
			{
				lg_string szTemp;
				LG_strncpy(szTemp, atts[i+1], LG_MAX_STRING);
				lg_char* szTok=strtok(szTemp, ", ");
				lg_dword nPos=0;
				while(szTok && nPos<3)
				{
					fPos[nPos++]=(lg_float)atof(szTok);
					szTok=strtok(LG_NULL, ", ");
				}
			}
		}
		
		//Err_Printf("(%f, %f, %f)", fPos[0], fPos[1], fPos[2]);
		
		//Now copy the obtained data to the propper position.
		lx_level_object* pObj=LG_NULL;
		
		if(nMode==LEVEL_MODE_OBJECT && (pData->nCurObj<pData->pLevel->nObjCount))
		{
			pObj=&pData->pLevel->pObjs[pData->nCurObj++];
		}
		else if(pData->nCurActor<pData->pLevel->nActorCount)
		{
			pObj=&pData->pLevel->pActors[pData->nCurActor++];
		}
		
		//Copy the data.
		if(pObj)
		{
			LG_strncpy(pObj->szObjFile, szScript, LG_MAX_PATH);
			memcpy(pObj->fPosition, fPos, sizeof(fPos));
		}
		else
		{
			Err_Printf("LX_Level Parse ERROR: There are either more actors or objects present than specified.");
		}
	}
	
}
void LX_LevelEnd(void* userData, const XML_Char* name)
{
	lx_level_data* pData=(lx_level_data*)userData;
	LX_LEVEL_MODE nMode=LX_LevelNameToMode(name);
	LX_LEVEL_MODE nPrevMode=pData->stkMode.Pop();
	
	if(nPrevMode!=nMode){
		Err_Printf("LX_Level Parse ERROR: Starting tage did not match end tage.");
	}
}

void LX_LevelCharData(void* userData, const XML_Char* s, int len)
{
	lx_level_data* pData = (lx_level_data*)userData;
	LX_LEVEL_MODE nMode = pData->stkMode.Peek();
	
	lg_bool bRead=LG_FALSE;
	lg_char* szFile=LG_NULL;
	if(nMode==LEVEL_MODE_MAP)
	{
		szFile=&pData->pLevel->szMapFile[0];
		bRead=LG_TRUE;
	}
	else if(nMode==LEVEL_MODE_SKYBOX)
	{
		szFile=&pData->pLevel->szSkyBoxFile[0];
		bRead=LG_TRUE;
	}
	else if(nMode==LEVEL_MODE_SKYBOX_SKIN)
	{
		szFile=&pData->pLevel->szSkyBoxSkin[0];
		bRead=LG_TRUE;
	}
	else if(nMode==LEVEL_MODE_MUSIC)
	{
		szFile=&pData->pLevel->szMusicFile[0];
		bRead=LG_TRUE;
	}
	if(bRead)
	{
		lg_path szTemp;
		strncpy(szTemp, s, len);
		szTemp[len]=0;
		
		//Take off any return carriages.
		while(szTemp[len-1]=='\n' || szTemp[len-1]=='\r')
		{
			szTemp[--len]=0;
		}

		//Attach the remainder of the string to teh level.
		strncat(szFile, szTemp, LG_MAX_PATH-strlen(szFile));
		szFile[LG_MAX_PATH]=0;
	}
}

#if 0

void LX_ObjectEnd(void* userData, const XML_Char* name)
{
	lx_obj_data* pData = (lx_obj_data*)userData;
	LX_OBJ_MODE nMode=LX_ObjectNameToMode(name);
	LX_OBJ_MODE nPrevMode=pData->stkMode.Pop();
	
	if(nPrevMode!=nMode){
		Err_Printf("LX_Object Parse ERROR: Starting tag did not match end tag.");
	}
}

void LX_ObjectCharData(void* userData, const XML_Char*s, int len)
{
	lx_obj_data* pData = (lx_obj_data*)userData;

	lg_char* szText=(lg_char*)LG_Malloc((len+1)*sizeof(lg_char));
	memcpy(szText, s, len+1);
	szText[len]=0;
	if(pData->stkMode.Peek()==OBJ_MODE_MODE)
	{
		#if 0
		Err_Printf(szText);
		#endif
	}
	LG_Free(szText);
}


#endif