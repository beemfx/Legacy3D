// Texture.cpp: implementation of the CTexture class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Texview2.h"
#include "Texture.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CTexture::CTexture():
m_hTGAImage(NULL), m_nWidth(0), m_nHeight(0)
{

}

CTexture::~CTexture()
{
	Unload();
}

void CTexture::Unload()
{
	m_bmImage.DeleteObject();
	IMG_Delete(m_hTGAImage);
	m_hTGAImage=NULL;
}

BOOL CTexture::Load(LPTSTR szFilename)
{
	Unload();
	
	m_hTGAImage=IMG_Open(szFilename);
	if(m_hTGAImage)
		return TRUE;
	else
		return FALSE;
}

BOOL CTexture::CreateCBitmap(int cx, int cy, IMGFILTER filter, CDC* pdc, DWORD Flags)
{
/* The idea behind converting an imaged to a DDB that can be used
	with windows is the get necessary data and use CreateDIBitmap
	with specified info. */
	HBITMAP hFinal=NULL;
	LPVOID lpImageData=NULL;

	BITMAPINFOHEADER bmih;
	BITMAPINFO bmi;

	IMG_DESC descript;

	unsigned char nExtra=255;
	//unsigned short finalwidth=0, finalheight=0;

	memset(&bmih, 0, sizeof(BITMAPINFOHEADER));
	memset(&bmi, 0, sizeof(BITMAPINFO));

	if(!m_hTGAImage)
		return FALSE;

	IMG_GetDesc(
		m_hTGAImage,
		&descript);

	m_nWidth=cx?cx:descript.Width;
	m_nHeight=cy?cy:descript.Height;

	lpImageData=malloc(m_nWidth*m_nHeight*4);
	if(!lpImageData)
	{
		return FALSE;
	}

	//Set up the destination rect.
	IMG_DEST_RECT rcd={0};
	rcd.nWidth=m_nWidth;
	rcd.nHeight=m_nHeight;
	rcd.nPitch=m_nWidth*4;
	rcd.nFormat=IMGFMT_A8R8G8B8;
	rcd.nOrient=IMGORIENT_BOTTOMLEFT;
	rcd.rcCopy.top=0;
	rcd.rcCopy.left=0;
	rcd.rcCopy.right=m_nWidth;
	rcd.rcCopy.bottom=m_nHeight;
	rcd.pImage=(img_void*)lpImageData;

	IMG_CopyBits(
		m_hTGAImage,
		&rcd,
		filter,
		NULL,
		0xFF);
		

	int i=0;
	//If we want the alpha channel we simply copy the alpha
	//value into all other values.  This will give us the gray
	//scale image we need.
	if(CHECK_FLAG(Flags, BC_ACHANNEL))
	{
		ARGBVALUE colrBG={0, 0, 0, 0xFF};
		DWORD   bgclr=CHECK_FLAG(Flags, BC_USEAPPBG)?GetSysColor(COLOR_APPWORKSPACE):0xFF000000;
		memcpy(&colrBG, &bgclr, 4);
		ARGBVALUE* pColVals=(ARGBVALUE*)lpImageData;
		for(i=0; i<(m_nWidth*m_nHeight); i++)
		{
			//If we want to render both the image and the alpah channel
			//we do an alpha blend.
			if(CHECK_FLAG(Flags, BC_IMAGE))
			{
				//newColor = backColor + (overlayColor – backColor) * (alphaByte div 255)
				pColVals[i].r=pColVals[i].r+(colrBG.r-pColVals[i].r)*(255-pColVals[i].a)/255;
				pColVals[i].g=pColVals[i].g+(colrBG.g-pColVals[i].g)*(255-pColVals[i].a)/255;
				pColVals[i].b=pColVals[i].b+(colrBG.b-pColVals[i].b)*(255-pColVals[i].a)/255;
			}
			else
			{
				//If not we use white as the image color and do the same alpha blend op.

				pColVals[i].r=255+(colrBG.r-255)*(255-pColVals[i].a)/255;
				pColVals[i].g=255+(colrBG.g-255)*(255-pColVals[i].a)/255;
				pColVals[i].b=255+(colrBG.b-255)*(255-pColVals[i].a)/255;
			}
		}
		
	}


	bmih.biSize=sizeof(BITMAPINFOHEADER);
	bmih.biWidth=m_nWidth;
	bmih.biHeight=m_nHeight;
	bmih.biPlanes=1;
	bmih.biBitCount=32;
	bmih.biCompression=BI_RGB;
	bmih.biSizeImage=m_nWidth*m_nHeight*4;//BI_RGB;
	bmih.biXPelsPerMeter=0;
	bmih.biYPelsPerMeter=0;
	bmih.biClrUsed=0;
	bmih.biClrImportant=0;

	bmi.bmiHeader=bmih;

	
	hFinal=CreateDIBitmap(
		pdc->m_hDC,
		&bmih,
		CBM_INIT,
		lpImageData,
		&bmi,
		DIB_RGB_COLORS);

	free(lpImageData);

	m_bmImage.m_hObject=hFinal;

	return TRUE;
}

BOOL CTexture::DrawBitmap(int x, int y, CDC *pdc)
{
	if(!m_bmImage.m_hObject)
		return FALSE;

	BITMAP bm;
	m_bmImage.GetBitmap(&bm);
	CDC cImageDC;
	cImageDC.CreateCompatibleDC(pdc);
	CBitmap* bmOld=(CBitmap*)cImageDC.SelectObject(&m_bmImage);


	int xOffset=x;
	int yOffset=y;

	pdc->StretchBlt(
		0,
		0,
		bm.bmWidth-xOffset,
		bm.bmHeight-yOffset,
		&cImageDC,
		xOffset,
		yOffset,
		bm.bmWidth-xOffset,
		bm.bmHeight-yOffset,
		SRCCOPY);
	

	cImageDC.SelectObject(bmOld);
	cImageDC.DeleteDC();
	return TRUE;
}

void CTexture::GetBitmapDims(int *pWidth, int *pHeight)
{
	*pWidth=m_nWidth;
	*pHeight=m_nHeight;
}
