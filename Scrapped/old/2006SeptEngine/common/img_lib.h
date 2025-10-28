/* img_lib.h - The Image Loader library.
	Copyright (c) 2006 Blaine Myers*/
#ifndef __IMG_LIB_H__
#define __IMG_LIB_H__

/* Insure that if a c++ app is calling the img_lib functions
	that it knows they were written in C.  This is for VC++
	and is not ANSI C.*/
#ifdef __cplusplus
extern "C"{
#endif /*__cplusplus*/

/* Some types that we will use.*/
typedef unsigned char  img_byte;
typedef signed char    img_sbyte;
typedef unsigned short img_word;
typedef signed short   img_short;
typedef unsigned long  img_dword;
typedef signed long    img_long;
typedef int            img_bool;
typedef unsigned int   img_uint;
typedef signed int     img_int;
typedef void           img_void;

/* A few definitions. */
#define IMG_TRUE  (1)
#define IMG_FALSE (0)


/* A definition for the function type (all functions will be __cdecl, but
	if for some reason that needed to be changed only the following line needs
	to be changed.  This is not ANSI C, but to make this file ANSI C compatible
	IMG_FUNC need only to be changed to a blank defintion.*/
#define IMG_FUNC __cdecl

#ifdef __cplusplus
#define IMG_NULL (0)
#else /*__cplusplus*/
#define IMG_NULL ((void*)0)
#endif /*__cplusplus*/

/* IMG_SAFE_FREE safely frees memory allocated using malloc (or calloc, etc).
	IMG_CLAMP insures that the value of v1 is within min and max.*/
#define IMG_SAFE_FREE(p) if(p){free(p);p=IMG_NULL;}
#define IMG_CLAMP(v1, min, max) ( (v1)>(max)?(max):(v1)<(min)?(min):(v1) )

/* HIMG is the handle for our image.  Obtained by a call
	to IMG_Open or IMG_OpenCallbacks*/
typedef void* HIMG;

/* The structure for the callback functions used when opening the
	image.*/
typedef struct _IMG_CALLBACKS{
	img_int (__cdecl *close)(void* stream);
	img_int (__cdecl *seek)(void* stream, long offset, int origin);
	img_long (__cdecl *tell)(void* stream);
	img_uint (__cdecl *read)(void* buffer, img_uint size, img_uint count, void* stream);
}IMG_CALLBACKS, *PIMG_CALLBACKS;

/* The seeking parametres for the seek function.*/
#define IMG_SEEK_CUR    1
#define IMG_SEEK_END    2
#define IMG_SEEK_SET    0

/* The IMGORIENT enumeration describes where the upper-left pixel
	is located in the file.*/
typedef enum _IMGORIENT{
	IMGORIENT_BOTTOMLEFT=0,
	IMGORIENT_BOTTOMRIGHT=1,
	IMGORIENT_TOPLEFT=2,
	IMGORIENT_TOPRIGHT=3,
	IMGORIENT_FORCE_DWORD=0xFFFFFFFF
}IMGORIENT;

/* The IMGFMT enumeration describes the images pixel format.*/
typedef enum _IMGFMT{
	IMGFMT_UNKNOWN=0,
	IMGFMT_NONE=0,
	IMGFMT_PALETTE=1,
	IMGFMT_X1R5G5B5=2,
	IMGFMT_R5G6B5=3,
	IMGFMT_R8G8B8=4,
	IMGFMT_A8R8G8B8=5,
	IMGFMT_R8G8B8A8=6,
	IMGFMT_FORCE_DWORD=0xFFFFFFFF
}IMGFMT;

/* The IMGFILTER enumeration is used to determine
	how an images pixels are filter when calls IMG_CopyBits
	involve shrinking and stretching.*/
typedef enum _IMGFILTER{
	IMGFILTER_NONE=0,
	IMGFILTER_POINT=0,
	IMGFILTER_LINEAR=1,
	IMGFILTER_FORCE_DWORD=0xFFFFFFFF
}IMGFILTER;


/* The IMG_RECT structure is used to describe the
	rectangluar area that is being copied from on
	the source image.*/
typedef struct _IMG_RECT{
	img_long left;
	img_long top;
	img_long right;
	img_long bottom;
}IMG_RECT, *PIMG_RECT;

/* The IMG_DES_RECT structure describes the destination
	images format (size, pixel format, and area to be
	copied to) for the IMG_CopyBits function.*/
typedef struct _IMG_DEST_RECT{
	img_dword nWidth;
	img_dword nHeight;
	img_dword nPitch;
	IMGFMT    nFormat;
	IMGORIENT nOrient;
	IMG_RECT  rcCopy;
	void*     pImage;
}IMG_DEST_RECT, *PIMG_DEST_RECT;

/* IMG_DESC is used when retreiving information
	about the image stored in an HIMG.*/
typedef struct _IMG_DESC{
	img_word Width;
	img_word Height;
	img_byte BitsPerPixel;
	IMGFMT   Format;
	img_word NumPaletteEntries;
	img_byte PaletteBitDepth;
	IMGFMT   PaletteFormat;
}IMG_DESC, *PIMG_DESC;

/*********************************
	The img_lib public functions.
*********************************/


HIMG IMG_FUNC IMG_OpenCallbacks(void* stream, IMG_CALLBACKS* lpFuncs);
HIMG IMG_FUNC IMG_Open(char* szFilename);
img_bool IMG_FUNC IMG_Delete(HIMG hImage);

img_bool IMG_FUNC IMG_GetDesc(
	HIMG hImage, 
	IMG_DESC* lpDescript);

img_bool IMG_FUNC IMG_CopyBits(
	HIMG hImage,
	IMG_DEST_RECT* pDest,
	IMGFILTER Filter,
	IMG_RECT* rcSrc,
	img_byte nExtra);

img_bool IMG_FUNC IMG_GetPalette(
	HIMG hImage,
	void* lpDataOut);

/* Terminate the extern "C" (see above).*/
#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /*__IMG_LIB_H__*/