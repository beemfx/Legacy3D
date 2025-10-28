#include <windows.h>
#include "tga_lib.h"

/* The idea behind converting an imaged to a DDB that can be used
	with windows is the get necessary data and use CreateDIBitmap
	with specified info. */
void* TGA_CreateDIBitmap(char* szFilename, void* hdc)
{
	HBITMAP hFinal=NULL;
	HTGAIMAGE hImage=NULL;
	LPVOID lpImageData=NULL;

	BITMAPINFOHEADER bmih;
	BITMAPINFO bmi;

	TGA_DESC descript;

	unsigned char nExtra=255;
	unsigned short finalwidth=0, finalheight=0;

	ZeroMemory(&bmih, sizeof(BITMAPINFOHEADER));
	ZeroMemory(&bmi, sizeof(BITMAPINFO));


	hImage=TGA_Open(szFilename);
	if(!hImage)
		return NULL;

	TGA_GetDesc(
		hImage,
		&descript);

	finalwidth=descript.Width;
	finalheight=descript.Height;

	lpImageData=malloc(finalwidth*finalheight*4);//descript.Width*descript.Height*32/8);
	if(!lpImageData)
	{
		TGA_Delete(hImage);
		return NULL;
	}

	TGA_CopyBitsStretch(
		hImage,
		TGAFILTER_LINEAR,
		lpImageData,
		TGAORIENT_BOTTOMLEFT,
		TGAFMT_A8R8G8B8,
		finalwidth,
		finalheight,
		(unsigned short)(finalwidth*4),
		0xFF);

	TGA_Delete(hImage);


	bmih.biSize=sizeof(BITMAPINFOHEADER);
	bmih.biWidth=finalwidth;
	bmih.biHeight=finalheight;
	bmih.biPlanes=1;
	bmih.biBitCount=32;
	bmih.biCompression=BI_RGB;
	bmih.biSizeImage=BI_RGB;
	bmih.biXPelsPerMeter=0;
	bmih.biYPelsPerMeter=0;
	bmih.biClrUsed=0;
	bmih.biClrImportant=0;

	bmi.bmiHeader=bmih;

	
	hFinal=CreateDIBitmap(
		hdc,
		&bmih,
		CBM_INIT,
		lpImageData,
		&bmi,
		DIB_RGB_COLORS);

	free(lpImageData);

	return hFinal;
}
