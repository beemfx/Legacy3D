#ifndef __LVF_SYS_H__
#define __LVF_SYS_H__

#include "lf_sys2.h"
#include "lg_types.h"
#include "lg_xml.h"

class CLVFont
{
private:
	struct CHAR_DATA{
		lg_char c;
		lg_word x, y, w, h;
	};
	
	struct XML_FIND_DATA{
		lf_path    szFontFile;
		CHAR_DATA* pCharData;
		lg_char    c;
		lg_bool    bInChar;
	};
public:
	CLVFont();
	~CLVFont();
	
	CHAR_DATA m_CharData[96];
	
	lg_bool Load(lf_path szXMLFilename);
	
private:
	static void xml_font_data_start(void* userData, const XML_Char* name, const XML_Char** atrs);
	static void xml_font_data_end(void* userData, const XML_Char* name);
	static void xml_font_data_data(void* userData, const XML_Char*s, int len);
};

#endif __LVF_SYS_H__