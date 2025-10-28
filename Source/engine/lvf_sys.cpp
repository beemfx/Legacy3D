#include <string.h>
#include "lvf_sys.h"
#include "lg_err.h"
#include "common.h"
#include "lg_func.h"

#include <stdio.h>


CLVFont::CLVFont()
{
}

CLVFont::~CLVFont()
{

}

lg_bool CLVFont::Load(lf_path szXMLFile)
{
	#if 0
	//Temp thing to create output
	LF_FILE3 fout=LF_Open("/dbase/font/font.xml", LF_ACCESS_WRITE, LF_CREATE_ALWAYS);
	
	char szLine[128];
	LG_strncpy(szLine, "<?xml version=\"1.0\" encoding=\"utf-8\"?>\r\n", 127);
	LF_Write(fout, szLine, L_strlen(szLine));
	LG_strncpy(szLine, "<lfont file=\"lfontsm.tga\">\r\n", 127);
	LF_Write(fout, szLine, L_strlen(szLine));
	
	lg_dword x=0, y=0;
	for(lg_dword i=0; i<96; i++)
	{
		sprintf(szLine, "\t<c%d>%d %d %d %d</c%d> <!-- '%c' -->\r\n", i+32, x, y, 8, 15, i+32, i+32);
		LF_Write(fout, szLine, L_strlen(szLine));
		
		x+=8;
		if(x>120)
		{
			x=0;
			y+=15;
		}
	}
	
	LG_strncpy(szLine, "</lfont>\r\n", 127);
	LF_Write(fout, szLine, L_strlen(szLine));
	
	LF_Close(fout);
	#endif
	
	
	Err_Printf("Loading \"%s\" font...", szXMLFile);
	LF_FILE3 fileXML=LF_Open(szXMLFile, LF_ACCESS_READ|LF_ACCESS_MEMORY, LF_OPEN_EXISTING);
	
	if(!fileXML)
	{
		Err_Printf("   ERROR: Could not open XML file.");
		return LG_FALSE;
	}
	
	XML_Parser xmlParser=XML_ParserCreate_LG(LG_NULL);
	if(!xmlParser)
	{
		Err_Printf("   ERROR: Could not create XML parser.");
		LF_Close(fileXML);
		return LG_FALSE;
	}
	
	XML_SetCharacterDataHandler(xmlParser, xml_font_data_data);
	XML_SetElementHandler(xmlParser, xml_font_data_start, xml_font_data_end);
	
	XML_FIND_DATA sFindData;
	
	sFindData.bInChar=LG_FALSE;
	sFindData.c=0;
	sFindData.pCharData=m_CharData;
	sFindData.szFontFile[0]=0;
	memset(m_CharData, 0, sizeof(m_CharData));
	
	XML_SetUserData(xmlParser, &sFindData);
	
	if(!XML_Parse(xmlParser, (const char*)LF_GetMemPointer(fileXML), LF_GetSize(fileXML), LG_TRUE))
	{
		Err_Printf("   ERROR: Parse error at line %d: \"%s\"",
			XML_GetCurrentLineNumber(xmlParser),
			XML_ErrorString(XML_GetErrorCode(xmlParser)));
		
		XML_ParserFree(xmlParser);
		LF_Close(fileXML);
		return LG_FALSE;	
	}
	
	XML_ParserFree(xmlParser);
	LF_Close(fileXML);
	
	if(sFindData.szFontFile[0]==0)
	{
		Err_Printf("   ERROR: Could not find font file.");
		return LG_FALSE;
	}
	
	//We now have information about all the characters...
	//Adjust the font to the same path as the tga file
	lf_path szTemp;
	LF_GetDirFromPath(szTemp, szXMLFile);
	L_strncat(szTemp, sFindData.szFontFile, LF_MAX_PATH);
	LG_strncpy(sFindData.szFontFile, szTemp, LF_MAX_PATH);
	
	for(lg_byte i=0; i<96; i++)
	{
		Err_Printf("x: %u y: %u w: %u h:%u",
			m_CharData[i].x,
			m_CharData[i].y,
			m_CharData[i].w,
			m_CharData[i].h);
	}
	
	Err_Printf("The font file is: %s", sFindData.szFontFile);
	
	return LG_TRUE;
}

void CLVFont::xml_font_data_start(void* userData, const XML_Char* name, const XML_Char** atrs)
{
	XML_FIND_DATA* pFindData=(XML_FIND_DATA*)userData;
	const XML_Char* szFilename=LG_NULL;
	
	for(lg_dword i=0; atrs[i]; i+=2)
	{
		if(strcmp(atrs[i], "file")==0)
			szFilename=atrs[i+1];
	}
	
	if(name[0]=='c')
	{
		lg_char c=(lg_char)L_atol((char*)&name[1]);
		pFindData->bInChar=LG_TRUE;
		pFindData->c=c;
	}
	if(stricmp(name, "lfont")==0)
	{
		if(!szFilename)
		{
			Err_Printf("   ERROR: lfont declared but no file specified");
			return;
		}
		
		strncpy(pFindData->szFontFile, szFilename, LF_MAX_PATH);
		return;
	}
}
void CLVFont::xml_font_data_end(void* userData, const XML_Char* name)
{
	XML_FIND_DATA* pFindData=(XML_FIND_DATA*)userData;
	if(name[0]=='c')
		pFindData->bInChar=LG_FALSE;
}
void CLVFont::xml_font_data_data(void* userData, const XML_Char*s, int len)
{
	XML_FIND_DATA* pFindData=(XML_FIND_DATA*)userData;
	if(!pFindData->bInChar || !pFindData->szFontFile[0])
		return;
		
	lg_char c=pFindData->c-32;
	
	if(c>=96)
		return;
		
	//Parse the data:
	lg_char szTemp[64];
	LG_strncpy(szTemp, s, LG_Min(63, len+1));
	pFindData->pCharData[c].x=(lg_word)L_atol(L_strtokA(szTemp, " \t", 0));
	pFindData->pCharData[c].y=(lg_word)L_atol(L_strtokA(0, 0, 0));
	pFindData->pCharData[c].w=(lg_word)L_atol(L_strtokA(0, 0, 0));
	pFindData->pCharData[c].h=(lg_word)L_atol(L_strtokA(0, 0, 0));
	pFindData->pCharData[c].c=pFindData->c;
}
