#include "lx_sys.h"
#include "lg_malloc.h"
#include "lg_xml.h"
#include "lg_err.h"
#include "lg_stack.h"
#include "lp_sys2.h"
#include <memory.h>
#include <string.h>

typedef enum _LX_OBJ_MODE{
	OBJ_MODE_NONE=0,
	OBJ_MODE_OBJECT,
	OBJ_MODE_MODEL,
	OBJ_MODE_SKELS,
	OBJ_MODE_SKEL,
	OBJ_MODE_AI,
	OBJ_MODE_PHYS,
	OBJ_MODE_MODES,
	OBJ_MODE_MODE
}LX_OBJ_MODE;

typedef struct _lx_obj_data{
	lx_object*           pObject;
	CLStack<LX_OBJ_MODE> stkMode;     
}lx_obj_data;

void LX_ObjectStart(void* userData, const XML_Char* name, const XML_Char** atts);
void LX_ObjectEnd(void* userData, const XML_Char* name);
void LX_ObjectCharData(void* userData, const XML_Char*s, int len);

lx_object* LX_LoadObject(const lf_path szXMLFile)
{
	//First thing we need to do is open the file,
	//if the file is not available, then the object
	//cannot be loaded and null is returned.
	LF_FILE3 fileScript=LF_Open(szXMLFile, LF_ACCESS_READ|LF_ACCESS_MEMORY, LF_OPEN_EXISTING);
	if(!fileScript)
	{
		Err_Printf("LX_LoadObject ERROR: Could not open \"%s\" for parsing.", szXMLFile);
		return LG_NULL;
	}
	//We need a new instance of lx_object, to be filled as the
	//xml file is parsed.
	lx_object* pNew = (lx_object*)LG_Malloc(sizeof(lx_object));
	memset(pNew, 0, sizeof(lx_object));
	XML_Parser parser = XML_ParserCreate_LG(LG_NULL);
	if(!parser)
	{
		Err_Printf("LX_LoadObject ERROR: Could not create XML parser. (%s)", szXMLFile);
		LF_Close(fileScript);
		return LG_NULL;
	}
	
	//TODO: Parsing Goes Here
	lx_obj_data data;
	data.stkMode.Push(OBJ_MODE_NONE);
	data.pObject=pNew;
	XML_ParserReset(parser, LG_NULL);
	XML_SetUserData(parser, &data);
	XML_SetElementHandler(parser, LX_ObjectStart, LX_ObjectEnd);
	XML_SetCharacterDataHandler(parser, LX_ObjectCharData);
	
	//Do the parsing
	if(!XML_Parse(parser, (const char*)LF_GetMemPointer(fileScript), LF_GetSize(fileScript), LG_TRUE))
	{
		Err_Printf("LX_LoadObject ERROR: Parse error at line %d: \"%s\"", 
			XML_GetCurrentLineNumber(parser),
			XML_ErrorString(XML_GetErrorCode(parser)));
				
		LX_DestroyObject(pNew);
		LF_Close(fileScript);
		return LG_NULL;
	}
	
	//Destroy the parser, and close the file, then return the data.
	XML_ParserFree(parser);
	LF_Close(fileScript);
	#if 0
	Err_Printf("Model File: %s", pNew->szModelFile);
	Err_Printf("AI Function: %s", pNew->szAiFunction);
	Err_Printf("Mass: %f", pNew->fMass);
	Err_Printf("Scale: %f", pNew->fScale);
	Err_Printf("PHYS Flags: 0x%08X", pNew->nPhysFlags);
	Err_Printf("Center: %f, %f, %f", pNew->fXC, pNew->fYC, pNew->fZC);
	Err_Printf("Mode Count: %d", pNew->nModeCount);
	for(lg_dword i=0; i<pNew->nModeCount; i++)
	{
		Err_Printf("Mode %i: %s", i+1, pNew->pModes[i].szName);
	}
	Err_Printf("Skel Count: %d", pNew->nNumSkels);
	for(lg_dword i=0; i<pNew->nNumSkels; i++)
	{
		Err_Printf("Skel %i: File: %s Mesh: %s", i+1, pNew->pSkelFiles[i], pNew->pSkelMeshes[i]);
	}
	#endif
	return pNew;
}

lg_void LX_DestroyObject(lx_object* pObject)
{
	if(!pObject)
		return;
	if(pObject->pSkelFiles)
		LG_Free(pObject->pSkelFiles);
	if(pObject->pSkelMeshes)
		LG_Free(pObject->pSkelMeshes);
	if(pObject->pModes)
		LG_Free(pObject->pModes);
	if(pObject)
		LG_Free(pObject);
}

LX_OBJ_MODE LX_ObjectNameToMode(const XML_Char* name)
{
	LX_OBJ_MODE nMode=OBJ_MODE_NONE;
	
	if(stricmp(name, "object")==0)
		nMode=OBJ_MODE_OBJECT;
	else if(stricmp(name, "model")==0)
		nMode=OBJ_MODE_MODEL;
	else if(stricmp(name, "ai")==0)
		nMode=OBJ_MODE_AI;
	else if(stricmp(name, "skels")==0)
		nMode=OBJ_MODE_SKELS;
	else if(stricmp(name, "skel")==0)
		nMode=OBJ_MODE_SKEL;
	else if(stricmp(name, "phys")==0)
		nMode=OBJ_MODE_PHYS;
	else if(stricmp(name, "modes")==0)
		nMode=OBJ_MODE_MODES;
	else if(stricmp(name, "mode")==0)
		nMode=OBJ_MODE_MODE;
	
	return nMode;
}

void LX_ObjectStart(void* userData, const XML_Char* name, const XML_Char** atts)
{
	lx_obj_data* pData = (lx_obj_data*)userData;
	LX_OBJ_MODE nMode=LX_ObjectNameToMode(name);
	//We'll always push the mode onto the stack even it was invalid
	//that way when the values get popped off the stack they
	//will always match.
	pData->stkMode.Push(nMode);
	
	//Convert the tage to an integer value, if the value is
	//OBJ_MODE_NONE then it was an invalid tage and there
	//is nothing to do.
	if(nMode==OBJ_MODE_NONE)
	{
		Err_Printf("LX_Object Parse ERROR: Invalid object tag (%s).", name);
		return;
	}
	
	if(nMode==OBJ_MODE_MODEL)
	{
		for(lg_dword i=0; atts[i]; i+=2)
		{
			if(stricmp(atts[i], "file")==0)
			{
				strncpy(pData->pObject->szModelFile, atts[i+1], LF_MAX_PATH);
				pData->pObject->szModelFile[LF_MAX_PATH]=0;
			}
		}
	}
	else if(nMode==OBJ_MODE_SKELS)
	{
		for(lg_dword i=0; atts[i]; i+=2)
		{
			if(stricmp(atts[i], "count")==0)
			{
				pData->pObject->nNumSkels=atoi(atts[i+1]);
			}
		}
		
		if(pData->pObject->nNumSkels>0)
		{
			pData->pObject->pSkelFiles = (lf_path*)LG_Malloc(sizeof(lf_path)*pData->pObject->nNumSkels);
			pData->pObject->pSkelMeshes = (lx_name*)LG_Malloc(sizeof(lx_name)*pData->pObject->nNumSkels);
		}
	}
	else if(nMode==OBJ_MODE_SKEL)
	{
		lg_dword nID=0;
		lg_char* szFile=LG_NULL;
		lg_char* szMesh=LG_NULL;
		
		for(lg_dword i=0; atts[i]; i+=2)
		{
			if(stricmp(atts[i], "id")==0)
				nID=atoi(atts[i+1]);
			else if(stricmp(atts[i], "file")==0)
				szFile=(lg_char*)atts[i+1];
			else if(stricmp(atts[i], "mesh")==0)
				szMesh=(lg_char*)atts[i+1];
		}
		
		if(nID>0 && nID<=pData->pObject->nNumSkels)
		{
			strncpy(pData->pObject->pSkelFiles[nID-1], szFile, LF_MAX_PATH);
			pData->pObject->pSkelFiles[nID-1][LF_MAX_PATH]=0;
			strncpy(pData->pObject->pSkelMeshes[nID-1], szMesh, LF_MAX_PATH);
			pData->pObject->pSkelMeshes[nID-1][LF_MAX_PATH]=0;
		}
	}
	else if(nMode==OBJ_MODE_AI)
	{
		for(lg_dword i=0; atts[i]; i+=2)
		{
			if(stricmp(atts[i], "function")==0)
			{
				strncpy(pData->pObject->szAiFunction, atts[i+1], LF_MAX_PATH);
				pData->pObject->szAiFunction[LF_MAX_PATH]=0;
			}
		}
	}
	else if(nMode==OBJ_MODE_PHYS)
	{
		LX_START_ATTR
			LX_ATTR_FLOAT(pData->pObject->fHeight, height)
			LX_ATTR_FLOAT(pData->pObject->fRadius, radius)
			LX_ATTR_FLOAT3(pData->pObject->fShapeOffset, shape_offset)
			LX_ATTR_FLOAT3(pData->pObject->fMassCenter, mass_center)
			LX_ATTR_FLOAT(pData->pObject->fMass, mass)
			//LX_ATTR_FLOAT(pData->pObject->fScale, scale)
			LX_MISC_ATTR(shape)
				if(stricmp(atts[i+1], "capsule")==0)
					pData->pObject->nShape=PHYS_SHAPE_CAPSULE;//pData->pObject->nPhysFlags|=LX_OBJ_PHYS_SHAPE_CAPSULE;
				else if(stricmp(atts[i+1], "box")==0)
					pData->pObject->nShape=PHYS_SHAPE_BOX;//pData->pObject->nPhysFlags|=LX_OBJ_PHYS_SHAPE_BOX;
				else if(stricmp(atts[i+1], "sphere")==0)
					pData->pObject->nShape=PHYS_SHAPE_SPHERE;//pData->pObject->nPhysFlags|=LX_OBJ_PHYS_SHAPE_SPHERE;
				else if(stricmp(atts[i+1], "cylinder")==0)
					pData->pObject->nShape=PHYS_SHAPE_CYLINDER;//pData->pObject->nPhysFlags|=LX_OBJ_PHYS_SHAPE_CYLINDER;
			LX_END_MISC_ATTR
			LX_MISC_ATTR(bounds)
				if(stricmp(atts[i+1], "skel")==0)
					pData->pObject->nPhysFlags|=LX_OBJ_PHYS_BOUNDS_SKEL;
				else if(stricmp(atts[i+1], "mesh")==0)
					pData->pObject->nPhysFlags|=LX_OBJ_PHYS_BOUNDS_MESH;
			LX_END_MISC_ATTR
		LX_END_ATTR
		/*
		for(lg_dword i=0; atts[i]; i+=2)
		{
			if(stricmp(atts[i], "mass")==0)
				pData->pObject->fMass=(lg_float)atof(atts[i+1]);
			else if(stricmp(atts[i], "scale")==0)
				pData->pObject->fScale=(lg_float)atof(atts[i+1]);
			else if(stricmp(atts[i], "shape")==0)
			{
				if(stricmp(atts[i+1], "capsule")==0)
					pData->pObject->nPhysFlags|=LX_OBJ_PHYS_SHAPE_CAPSULE;
				else if(stricmp(atts[i+1], "box")==0)
					pData->pObject->nPhysFlags|=LX_OBJ_PHYS_SHAPE_BOX;
				else if(stricmp(atts[i+1], "sphere")==0)
					pData->pObject->nPhysFlags|=LX_OBJ_PHYS_SHAPE_SPHERE;
				else if(stricmp(atts[i+1], "cylinder")==0)
					pData->pObject->nPhysFlags|=LX_OBJ_PHYS_SHAPE_CYLINDER;
			}
			else if(stricmp(atts[i], "bounds")==0)
			{
				if(stricmp(atts[i+1], "skel")==0)
					pData->pObject->nPhysFlags|=LX_OBJ_PHYS_BOUNDS_SKEL;
				else if(stricmp(atts[i+1], "mesh")==0)
					pData->pObject->nPhysFlags|=LX_OBJ_PHYS_BOUNDS_MESH;
			}
			else if(stricmp(atts[i], "center")==0)
			{
				if(stricmp(atts[i+1], "auto")==0)
					pData->pObject->nPhysFlags|=LX_OBJ_PHYS_AUTO_CENTER;
				else
				{
					char* szTok=strtok((char*)atts[i+1], ", ");
					pData->pObject->fXC=szTok?(lg_float)atof(szTok):0.0f;
					szTok=strtok(LG_NULL, ", ");
					pData->pObject->fYC=szTok?(lg_float)atof(szTok):0.0f;
					szTok=strtok(LG_NULL, ", ");
					pData->pObject->fZC=szTok?(lg_float)atof(szTok):0.0f;
				}
			}
		}
		*/
	}
	else if(nMode==OBJ_MODE_MODES)
	{
		//If the nModeCount is set, then the modes have already been
		//declared an in that case, more modes cannot be declared.
		if(pData->pObject->nModeCount>0)
		{
			Err_Printf("LX_Object Parse ERROR: <modes> tag can only be used once.");
			return;
		}
		
		//Get the number of modes (count) and allocate memory to store
		//the modes.
		for(lg_dword i=0; atts[i]; i+=2)
		{
			if(stricmp(atts[i], "count")==0)
			{
				pData->pObject->nModeCount=atoi(atts[i+1]);
			}
		}
		
		if(pData->pObject->nModeCount>0)
		{
			pData->pObject->pModes=(lx_object_mode*)LG_Malloc(pData->pObject->nModeCount*sizeof(lx_object_mode));
		}
	}
	else if(nMode==OBJ_MODE_MODE)
	{
		//A mode can only be declared if we are already inside
		//the <modes> tage, so make sure that that is the case.
		LX_OBJ_MODE nModeCurrent=pData->stkMode.Pop();
		LX_OBJ_MODE nModePrev=pData->stkMode.Peek();
		pData->stkMode.Push(nModeCurrent);
		if(nModePrev!=OBJ_MODE_MODES)
		{
			Err_Printf("LX_Object Parse ERROR: <mode> delcared outside <modes>.");
			return;
		}
		
		//Get the mode id and the name of the mode.
		lg_dword nID=0;
		lg_char* szName=LG_NULL;
		
		for(lg_dword i=0; atts[i]; i+=2)
		{
			if(stricmp(atts[i], "id")==0)
				nID=atoi(atts[i+1]);
			else if(stricmp(atts[i], "name")==0)
				szName=(lg_char*)atts[i+1];
		}
		
		if(nID>0 && nID<=pData->pObject->nModeCount)
		{
			strncpy(pData->pObject->pModes[nID-1].szName, szName, LX_MAX_NAME);
			pData->pObject->pModes[nID-1].szName[LX_MAX_NAME]=0;
		}
	}
}
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

