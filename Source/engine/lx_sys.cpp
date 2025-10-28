#include "lg_xml.h"
#include "lx_sys.h"

lg_bool LX_Process(const lg_path szXMLFile,
	XML_StartElementHandler startH,
	XML_EndElementHandler endH,
	XML_CharacterDataHandler charDataH,
	lg_void* pData)
{
	LF_FILE3 fin=LF_Open(szXMLFile, LF_ACCESS_READ|LF_ACCESS_MEMORY, LF_OPEN_EXISTING);
	if(!fin)
		return LG_FALSE;
		
	XML_Parser parser= XML_ParserCreate_LG(LG_NULL);
	if(!parser)
	{
		LF_Close(fin);
		return LG_FALSE;
	}
	
	XML_ParserReset(parser, LG_NULL);
	XML_SetUserData(parser, pData);
	XML_SetElementHandler(parser, startH, endH);
	XML_SetCharacterDataHandler(parser, charDataH);
	
	lg_bool bRes=XML_Parse(parser, (const char*)LF_GetMemPointer(fin), LF_GetSize(fin), LG_TRUE);
	
	if(!bRes)
	{
		Err_Printf("LX ERROR: Parse error at line %d: \"%s\"", 
			XML_GetCurrentLineNumber(parser),
			XML_ErrorString(XML_GetErrorCode(parser)));
	}
	
	XML_ParserFree(parser);
	LF_Close(fin);
	
	return bRes;
}

