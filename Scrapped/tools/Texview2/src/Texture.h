// Texture.h: interface for the CTexture class.
//
//////////////////////////////////////////////////////////////////////

#include "img_lib.h"

#if !defined(AFX_TEXTURE_H__A9B8095A_5622_48A2_83DA_06A0D7AF237E__INCLUDED_)
#define AFX_TEXTURE_H__A9B8095A_5622_48A2_83DA_06A0D7AF237E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define BC_IMAGE    (0x00000010)
#define BC_ACHANNEL (0x00000020)
#define BC_USEAPPBG (0x00000040)

typedef struct _ARGBVALUE{
	BYTE b, g, r, a;
}ARGBVALUE;

class CTexture  
{
public:
	void GetBitmapDims(int* pWidth, int* pHeight);
	int m_nHeight;
	int m_nWidth;
	BOOL DrawBitmap(int x, int y, CDC* pdc);
	BOOL CreateCBitmap(int cx, int cy, IMGFILTER filter, CDC* pdc, DWORD Flags);
	BOOL Load(LPTSTR szFilename);
	void Unload();
	CTexture();
	virtual ~CTexture();

private:
	CBitmap m_bmImage;
	HIMG m_hTGAImage;
};

#endif // !defined(AFX_TEXTURE_H__A9B8095A_5622_48A2_83DA_06A0D7AF237E__INCLUDED_)
