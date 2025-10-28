#include "lg_xml.h"
#include "lx_sys.h"
#include "lg_func.h"
#include "lg_err.h"
#include <string.h>

void LX_MtrStart(void* userData, const XML_Char* name, const XML_Char** atts);
void LX_MtrEnd(void* userData, const XML_Char* name);

lg_bool LX_LoadMtr(const lg_path szFilename, lx_mtr* pMtr)
{
	LF_FILE3 fileScript=LF_Open(szFilename, LF_ACCESS_READ|LF_ACCESS_MEMORY, LF_OPEN_EXISTING);
	if(!fileScript)
	{
		Err_Printf("LX_LoadMtr ERROR: Could not open \"%s\".", szFilename);
		return LG_FALSE;
	}
	
	XML_Parser parser = XML_ParserCreate_LG(LG_NULL);
	if(!parser)
	{
		Err_Printf("LX_LoadMtr ERROR: Could not create XML parser. (%s)", szFilename);
		LF_Close(fileScript);
		return LG_FALSE;
	}
	
	pMtr->szFx[0]=0;
	pMtr->szTexture[0]=0;
	
	XML_ParserReset(parser, LG_NULL);
	XML_SetUserData(parser, pMtr);
	XML_SetElementHandler(parser, LX_MtrStart, LX_MtrEnd);
	
	//Do the parsing
	lg_bool bSuccess=XML_Parse(parser, (const char*)LF_GetMemPointer(fileScript), LF_GetSize(fileScript), LG_TRUE);
	
	if(!bSuccess)
	{
		Err_Printf("LX_LoadMtr ERROR: Parse error at line %d: \"%s\"", 
			XML_GetCurrentLineNumber(parser),
			XML_ErrorString(XML_GetErrorCode(parser)));
	}
	
	LF_Close(fileScript);
	XML_ParserFree(parser);
	
	//The output will be in pMtr
	return bSuccess;
}

void LX_MtrStart(void* userData, const XML_Char* name, const XML_Char** atts)
{
	lx_mtr* pMtr=(lx_mtr*)userData;
	
	if(stricmp(name, "material")==0)
	{
		for(lg_dword i=0; atts[i]; i+=2)
		{
			if(stricmp(atts[i], "texture")==0)
			{
				LG_strncpy(pMtr->szTexture, atts[i+1], LG_MAX_PATH);
			}
			else if(stricmp(atts[i], "fx")==0)
			{
				LG_strncpy(pMtr->szFx, atts[i+1], LG_MAX_PATH);
			}
		}
	}
	else
	{
		Err_Printf("LX_LoadMtr WARNING: %s is not a recognized tag.", name);
	}
}

void LX_MtrEnd(void* userData, const XML_Char* name)
{

}
