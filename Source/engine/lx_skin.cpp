#include "lx_sys.h"
#include "lg_xml.h"
#include "lg_err.h"
#include "lg_err_ex.h"
#include "lm_skin.h"
#include "lg_malloc.h"
#include "lg_mtr_mgr.h"
#include <string.h>

struct LX_SKIN_DATA
{
	CLSkin* pSkin;
	lg_dword nCurrentGroup;
};


void LX_SkinEnd(void* userData, const XML_Char* name);

lg_bool LX_LoadSkin(const lg_path szFilename, void* pSkin)
{
	LF_FILE3 fileScript=LF_Open(szFilename, LF_ACCESS_READ|LF_ACCESS_MEMORY, LF_OPEN_EXISTING);
	if(!fileScript)
	{
		Err_Printf("LX_LoadSkin ERROR: Could not open \"%s\".", szFilename);
		return LG_FALSE;
	}
	
	XML_Parser parser = XML_ParserCreate_LG(LG_NULL);
	if(!parser)
	{
		Err_Printf("LX_LoadSkin ERROR: Could not create XML parser. (%s)", szFilename);
		LF_Close(fileScript);
		return LG_FALSE;
	}
	
	LX_SKIN_DATA data;
	data.pSkin=(CLSkin*)pSkin;
	data.nCurrentGroup=0;
	
	XML_ParserReset(parser, LG_NULL);
	XML_SetUserData(parser, &data);
	XML_SetElementHandler(parser, CLSkin::LX_SkinStart, LX_SkinEnd);
	
	//Do the parsing
	lg_bool bSuccess=XML_Parse(parser, (const char*)LF_GetMemPointer(fileScript), LF_GetSize(fileScript), LG_TRUE);
	
	if(!bSuccess)
	{
		Err_Printf("LX_LoadSkin ERROR: Parse error at line %d: \"%s\"", 
			XML_GetCurrentLineNumber(parser),
			XML_ErrorString(XML_GetErrorCode(parser)));
	}
	
	XML_ParserFree(parser);
	LF_Close(fileScript);
	
	//The output will be in pSkin
	return bSuccess;
}

void CLSkin::LX_SkinStart(void* userData, const XML_Char* name, const XML_Char** atts)
{
	CLSkin* pSkin=((LX_SKIN_DATA*)userData)->pSkin;
	lg_dword* pCurGrp=&((LX_SKIN_DATA*)userData)->nCurrentGroup;
	
	if(stricmp(name, "skin")==0)
	{
		//The skin flags should only be used once.
		if(LG_CheckFlag(pSkin->m_nFlags, LM_FLAG_LOADED))
		{
			Err_Printf("LX_LoadSkin ERROR: The skin tag should only be used once.");
			return;
		}
		
		
		LX_START_ATTR
			LX_ATTR_DWORD(pSkin->m_nMtrCount, material_count)
			LX_ATTR_DWORD(pSkin->m_nGrpCount, group_count)
		LX_END_ATTR
		
		//Should now allocate memory...
		//Err_Printf("Loading skin with %d materials and %d groups.", pSkin->m_nMtrCount, pSkin->m_nGrpCount);
		
		//Calculate the size of memory we need and allocate.
		lg_dword nMtrSize, nGrpSize, nRefSize;
		nMtrSize=pSkin->m_nMtrCount*sizeof(*pSkin->m_pMaterials);
		nGrpSize=pSkin->m_nGrpCount*sizeof(*pSkin->m_pGrps);
		nRefSize=pSkin->m_nGrpCount*sizeof(*pSkin->m_pCmpRefs);
		
		lg_dword nMemSize=nMtrSize+nGrpSize+nRefSize;
			
		pSkin->m_pMem=(lg_byte*)LG_Malloc(nMemSize);
		if(!pSkin->m_pMem)
			throw LG_ERROR(LG_ERR_OUTOFMEMORY, LG_NULL);
			
		pSkin->m_pMaterials=(lg_material*)&pSkin->m_pMem[0];
		pSkin->m_pGrps=(CLSkin::MtrGrp*)&pSkin->m_pMem[nMtrSize];
		pSkin->m_pCmpRefs=(lg_dword*)&pSkin->m_pMem[nMtrSize+nGrpSize];
		
		//Set all the data to 0s
		for(lg_dword i=0; i<pSkin->m_nGrpCount; i++)
		{
			pSkin->m_pGrps[i].nMtr=0;
			pSkin->m_pGrps[i].szName[0]=0;
			pSkin->m_pCmpRefs[i]=0;
		}
		
		for(lg_dword i=0; i<pSkin->m_nMtrCount; i++)
		{
			pSkin->m_pMaterials[i]=0;
		}
		//We'll flags it as loaded now.
		pSkin->m_nFlags=LM_FLAG_LOADED;
		
	}
	else if(stricmp(name, "material")==0)
	{
		lg_dword nNum=0;
		lg_path szFilename;
		szFilename[0]=0;
		
		LX_START_ATTR
			LX_ATTR_DWORD(nNum, mtr_num)
			LX_ATTR_STRING(szFilename, file, LG_MAX_PATH)
		LX_END_ATTR
		
		
		//If the material number was specifed as zero or 
		//greater than the mtr_count attribute we can't do
		//anything with it.
		if(!nNum || !pSkin->m_nMtrCount || nNum>pSkin->m_nMtrCount)
		{
			Err_Printf("LX_LoadSkin WARNING: A material was referenced as %d and mtr_count was set to %d.",
				nNum,
				pSkin->m_nMtrCount);
		}
		else
		{
			//We obtain the material, note that we'll get the default
			//material if we can't load it.
			pSkin->m_pMaterials[nNum-1]=CLMtrMgr::MTR_Load(szFilename, 0);
		}
		
	}
	else if(stricmp(name, "group")==0)
	{
		lg_dword nRef=0;
		LMName szName;
		
		LX_START_ATTR
			LX_ATTR_DWORD(nRef, mtr_ref)
			LX_ATTR_STRING(szName, name, LM_MAX_NAME-1)
		LX_END_ATTR
		
		if(*pCurGrp<pSkin->m_nGrpCount)
		{
			pSkin->m_pGrps[*pCurGrp].nMtr=nRef;
			LG_strncpy(pSkin->m_pGrps[*pCurGrp].szName, szName, LM_MAX_NAME-1);
			++(*pCurGrp);
		}
		else
		{
			Err_Printf("LX_LoadSkin WARNING: More group tags used than specifed by group_count attribute.");
		}
	}
	else
	{
		Err_Printf("LX_LoadSkin WARNING: %s is not a recognized tag.", name);
	}
}

void LX_SkinEnd(void* userData, const XML_Char* name)
{

}
