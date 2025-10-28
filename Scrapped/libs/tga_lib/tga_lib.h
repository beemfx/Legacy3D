/* lv_tga.h - TGA file support for Legacy 3D. */
#ifndef __LV_TGA_H__
#define __LV_TGA_H__

#ifdef __cplusplus
extern "C"{
#endif /*__cplusplus*/

typedef void* HTGAIMAGE;

typedef struct _TGA_CALLBACKS{
	int (__cdecl *close)(void* stream);
	int (__cdecl *seek)(void* stream, long offset, int origin);
	long (__cdecl *tell)(void* stream);
	unsigned int (__cdecl *read)(void* buffer, unsigned int size, unsigned int count, void* stream);
}TGA_CALLBACKS, *PTGA_CALLBACKS;


typedef enum _TGAORIENT{
	TGAORIENT_BOTTOMLEFT=0,
	TGAORIENT_BOTTOMRIGHT=1,
	TGAORIENT_TOPLEFT=2,
	TGAORIENT_TOPRIGHT=3
}TGAORIENT;

typedef enum _TGAFMT{
	TGAFMT_UNKNOWN=0,
	TGAFMT_NONE=0,
	TGAFMT_PALETTE=1,
	TGAFMT_X1R5G5B5=2,
	TGAFMT_R5G6B5=3,
	TGAFMT_R8G8B8=4,
	TGAFMT_A8R8G8B8=5
}TGAFMT;

typedef enum _TGAFILTER{
	TGAFILTER_NONE=0,
	TGAFILTER_POINT=0,
	TGAFILTER_LINEAR=1
}TGAFILTER;

typedef struct _TGA_DESC{
	unsigned short Width;
	unsigned short Height;
	unsigned char BitsPerPixel;
	unsigned short NumCMEntries;
	unsigned char ColorMapBitDepth;
}TGA_DESC, *PTGA_DESC;

HTGAIMAGE TGA_OpenCallbacks(void* stream, TGA_CALLBACKS* lpFuncs);
HTGAIMAGE TGA_Open(char* szFilename);
int TGA_Delete(HTGAIMAGE hImage);

int TGA_GetDesc(
	HTGAIMAGE hImage, 
	TGA_DESC* lpDescript);

int TGA_CopyBits(
	HTGAIMAGE hImage, 
	void* lpOut, 
	TGAORIENT nOrient, 
	TGAFMT Format,
	unsigned short nWidth, 
	unsigned short nHeight,
	unsigned short nPitch,
	unsigned char nExtra);

int TGA_CopyBitsStretch(
	HTGAIMAGE hImage,
	TGAFILTER Filter,
	void* lpOut,
	TGAORIENT nOrient,
	TGAFMT Format,
	unsigned short nWidth,
	unsigned short nHeight,
	unsigned short nPitch,
	unsigned char nExtra);
	
int TGA_GetPalette(
	HTGAIMAGE hImage,
	void* lpDataOut);

void* TGA_CreateDIBitmap(char* szFilename, void* hdc);

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*__LV_TGA_H__*/