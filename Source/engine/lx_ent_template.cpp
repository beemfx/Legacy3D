#include "lx_sys.h"

void LX_ETemplateStart(void* userData, const XML_Char* name, const XML_Char** atts);
void LX_ETemplateEnd(void* userData, const XML_Char* name);

struct ET_DATA
{
	lg_dword nCount;
	lx_ent_template* pTemplate;
};

lx_ent_template* LX_LoadEntTemplate(const lg_path szXMLFile, lg_dword* pCount)
{
	ET_DATA data;
	data.nCount=0;
	data.pTemplate=LG_NULL;
	
	LX_Process(szXMLFile, LX_ETemplateStart, LX_ETemplateEnd, LG_NULL, &data);

	*pCount=data.nCount;
	return data.pTemplate;
}

lg_void LX_DestroyEntTemplate(lx_ent_template* pTemplate)
{
	LG_SafeDeleteArray(pTemplate);
}


void LX_ETemplateStart(void* userData, const XML_Char* name, const XML_Char** atts)
{
	ET_DATA* pData=(ET_DATA*)userData;
	
	LX_START_TAG
		LX_TAG(ent_template)
		{
			if(pData->pTemplate)
				LG_SafeDeleteArray(pData->pTemplate);
				
			LX_START_ATTR
				LX_ATTR_DWORD(pData->nCount, count)
			LX_END_ATTR
			
			if(pData->nCount)
			{
				pData->pTemplate=new lx_ent_template[pData->nCount];
			}
		}
		LX_TAG(ent_type)
		{
			lg_dword nID;
			lg_path  szPath;
			
			LX_START_ATTR
				LX_ATTR_DWORD(nID, id)
				LX_ATTR_STRING(szPath, script, LG_MAX_PATH)
			LX_END_ATTR
			
			if(nID<pData->nCount)
			{
				pData->pTemplate[nID].nID=nID;
				LG_strncpy(pData->pTemplate[nID].szScript, szPath, LG_MAX_PATH);
			}
		}
	LX_END_TAG
}

void LX_ETemplateEnd(void* userData, const XML_Char* name)
{

}