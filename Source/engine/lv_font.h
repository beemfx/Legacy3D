#ifndef __LV_FONT_H__
#define __LV_FONT_H__
#include <d3d9.h>
#include "common.h"
#include "lg_types.h"


class CLFont
{
protected:
	IDirect3DDevice9*  m_pDevice;
	lg_byte             m_nCharWidth;
	lg_byte             m_nCharHeight;
	lg_bool             m_bIsDrawing;
	char               m_szFont[LG_MAX_PATH+1];
public:
	CLFont();
	~CLFont();
	virtual void Delete(){};
	virtual lg_bool Validate(){return LG_TRUE;};
	virtual void Invalidate(){};
	virtual void Begin(){};
	virtual void End(){};
	virtual void DrawString(char* szString, lg_long x, lg_long y){};
	virtual void GetDims(lg_byte* nWidth, lg_byte* nHeight, lg_void* pExtra){};
	
public:
	static CLFont* Create(
		IDirect3DDevice9* lpDevice, 
		char* szFont, 
		lg_byte nCharWidth, 
		lg_byte nCharHeight,
		lg_bool bD3DXFont,
		lg_byte nCharWidthInFile,
		lg_byte nCharHeightInFile,
		lg_dword dwD3DColor);
};
#endif /*__LV_FONT_H__*/