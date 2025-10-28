#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include "tga_lib.h"
#include <math.h>

#define TGA_IMAGE_ORIGIN(p)  (((p)&0x30) >> 4)
#define TGA_ALPHABITS(p)     ((p)&0x0F)


typedef struct tagTGAHEADER
{
	unsigned char nIDSize;
	unsigned char nColorMapType;
	unsigned char nImageType;
	/* Color Map Info. */
	unsigned short nFirstCMIndex;
	unsigned short nNumCMEntries;
	unsigned char nCMEntrySize;
	/* Image Specifications. */
	unsigned short nXOrigin;
	unsigned short nYOrigin;
	unsigned short nWidth;
	unsigned short nHeight;
	unsigned char nBitsPerPixel;
	unsigned char nImageDesc;
}TGAHEADER, *LPTGAHEADER;

typedef struct tagTGAFOOTER
{
	unsigned long nExtensionOffset;
	unsigned long nDeveloperOffset;
	char szSignature[18];
}TGAFOOTER, *LPTGAFOOTER;

typedef struct tagTGAIMAGE
{
	TGAHEADER Header;
	char szImageID[255];
	void* ColorMap;
	void* ImageData;
	TGAFOOTER Footer;
	TGAFMT DataFmt;
	TGAFMT ColorMapFmt;
}TGAIMAGE, *LPTGAIMAGE;

/*************************
*** Private Functions. ***
*************************/
int TGA_ReadHeader(void* stream, TGA_CALLBACKS* lpCB, LPTGAHEADER lpHeader)
{
	lpCB->seek(stream, 0, SEEK_SET);
	/* Read the Header info. */
	lpCB->read(&lpHeader->nIDSize, 1, 1, stream);
	lpCB->read(&lpHeader->nColorMapType, 1, 1, stream);
	lpCB->read(&lpHeader->nImageType, 1, 1, stream);
	/* Read the Color map info. */
	lpCB->read(&lpHeader->nFirstCMIndex, 2, 1, stream);
	lpCB->read(&lpHeader->nNumCMEntries, 2, 1, stream);
	lpCB->read(&lpHeader->nCMEntrySize, 1, 1, stream);
	/* Read the image specifications. */
	lpCB->read(&lpHeader->nXOrigin, 2, 1, stream);
	lpCB->read(&lpHeader->nYOrigin, 2, 1, stream);
	lpCB->read(&lpHeader->nWidth, 2, 1, stream);
	lpCB->read(&lpHeader->nHeight, 2, 1, stream);
	lpCB->read(&lpHeader->nBitsPerPixel, 1, 1, stream);
	lpCB->read(&lpHeader->nImageDesc, 1, 1, stream);

	return 1;
}

int TGA_ReadFooter(void* stream, TGA_CALLBACKS* lpCB,  LPTGAFOOTER lpFooter)
{
	lpCB->seek(stream, -26, SEEK_END);

	lpCB->read(&lpFooter->nExtensionOffset, 4, 1, stream);
	lpCB->read(&lpFooter->nDeveloperOffset, 4, 1, stream);
	lpCB->read(&lpFooter->szSignature, 1, 18, stream);

	if(_strnicmp("TRUEVISION-XFILE.", lpFooter->szSignature, 17)==0)
	{
		return 1;
	}
	else
	{
		lpFooter->szSignature[0]=0;
		return 0;
	}
	

	return 0;
}

/*
	The idea behind TGAReadRLEData is that it reads RLE data
	out of the file decompressing it as it goes along.
*/
TGA_ReadRLEData(void* stream, TGA_CALLBACKS* lpCB, LPTGAIMAGE lpImage)
{
	unsigned char nBPP=0;
	int bCompressed=0;
	unsigned char nLen=0;
	unsigned char nCompress=0;
	unsigned long nDecodePos=0;
	unsigned long j=0;
	void * lpData=NULL;

	nBPP=lpImage->Header.nBitsPerPixel/8;

	/* Allocate some memory for the input data. */
	lpData=malloc(nBPP);

	nDecodePos=0;
	for(nDecodePos=0; nDecodePos<(unsigned long)(nBPP*lpImage->Header.nWidth*lpImage->Header.nHeight); )
	{
		/* Read the encodeing byte. */
		lpCB->read(&nCompress, 1, 1, stream);
		/* The compression mode is the last bit. */
		bCompressed=(nCompress&0x80)>>7;
		/* The length of the data is the first 7 bits plus 1. */
		nLen=(nCompress&0x7F)+1;
		if(bCompressed)
		{
			/* If compressed we decompress the data. */
			/* Read the compressed data. */
			lpCB->read(lpData, 1, nBPP, stream);
			for(j=0; j<nLen; j++)
			{
				/* Copy the data into the appropriate length of image. */
				memcpy((void*)((unsigned int)lpImage->ImageData+nDecodePos), lpData, nBPP);
				nDecodePos+=nBPP;
			}
		}
		else
		{
			/* Otherwize we simply read the data. */
			for(j=0; j<nLen; j++)
			{
				lpCB->read(lpData, 1, nBPP, stream);
				memcpy((void*)((unsigned int)lpImage->ImageData+nDecodePos), lpData, nBPP);
				nDecodePos+=nBPP;
			}
		}
	}

	/* Make the image type represent that which it now is. */
	switch(lpImage->Header.nImageType)
	{
	case 9:
		lpImage->Header.nImageType=1;
		break;
	case 10:
		lpImage->Header.nImageType=2;
		break;
	case 11:
		lpImage->Header.nImageType=3;
		break;
	}

	/* Free the memory from the input. */
	free(lpData);
	return 1;
}

/************************
*** Public Functions. ***
************************/

HTGAIMAGE TGA_OpenCallbacks(void* stream, TGA_CALLBACKS* lpCB)
{
	LPTGAIMAGE lpImage=NULL;
	int bValid=0;
	int bFooter=0;

	if(!stream || !lpCB)
		return NULL;


	lpImage=malloc(sizeof(TGAIMAGE));
	if(!lpImage)
		return NULL;

	lpImage->ColorMap=NULL;
	lpImage->ImageData=NULL;
	lpImage->szImageID[0]=0;

	TGA_ReadHeader(stream, lpCB, &lpImage->Header);
	bFooter=bValid=TGA_ReadFooter(stream, lpCB, &lpImage->Footer);
	
	//#define DEBUG_MSG
	#ifdef DEBUG_MSG
	{
	char szTemp[1000];
	sprintf(
		szTemp, 
		"ID Len: %i\nCM Type: %i\nImage Type: %i\nFirst CM Indes: %i\nNum CM Entires: %i\nCM Entry Size: %i\n(%i, %i)\n%ix%ix%i\n%i", 
		lpImage->Header.nIDSize, 
		lpImage->Header.nColorMapType, 
		lpImage->Header.nImageType,
		lpImage->Header.nFirstCMIndex,
		lpImage->Header.nNumCMEntries,
		lpImage->Header.nCMEntrySize,
		lpImage->Header.nXOrigin,
		lpImage->Header.nYOrigin,
		lpImage->Header.nWidth,
		lpImage->Header.nHeight,
		lpImage->Header.nBitsPerPixel,
		lpImage->Header.nImageDesc);
	MessageBox(0, szTemp, 0, 0);
	
	if(bFooter)
	{
		sprintf(
			szTemp, 
			"Extension Offset: %i\nDev Offset: %i\nSignature: %s\n", 
			lpImage->Footer.nExtensionOffset, 
			lpImage->Footer.nDeveloperOffset, 
			lpImage->Footer.szSignature);
	MessageBox(0, szTemp, 0, 0);
	}
	}
	#endif // DEBUG_MSG
	

	// Make sure we have a valid file.
	if(!bValid)
	{	
		if(lpImage->Header.nColorMapType==1)
		{
			if(
				(lpImage->Header.nImageType==1) || 
				(lpImage->Header.nImageType==9))
				bValid=1;
			else
				bValid=0;
		}
		else if(lpImage->Header.nColorMapType==0)
		{
			if( 
				(lpImage->Header.nImageType==0) ||
				(lpImage->Header.nImageType==2) ||
				(lpImage->Header.nImageType==3) ||
				(lpImage->Header.nImageType==10) ||
				(lpImage->Header.nImageType==11))
				bValid=1;
			else
				bValid=0;
		}
		else
		{
			bValid=0;
		}
	
	}

	//If the bits per pixel is not evenly divided by
	//8 we can't read the file.  In theory 15 bit tga
	//images exist, but I haven't found one, and they
	//aren't supported by this library.
	if(lpImage->Header.nBitsPerPixel%8)
		bValid=0;

	if(!bValid)
	{
		lpCB->close(stream);
		free(lpImage);
		return NULL;
	}

	// We assume a valid file now and read the id.
	lpCB->seek(stream, 18, SEEK_SET);

	lpCB->read(
		&lpImage->szImageID,
		1,
		lpImage->Header.nIDSize,
		stream);

	// Now read the color map data if it exists.
	if(lpImage->Header.nColorMapType)
	{
		lpImage->ColorMap=malloc(lpImage->Header.nNumCMEntries * (lpImage->Header.nCMEntrySize/8));
		if(lpImage->ColorMap==(void*)0)
		{
			lpCB->close(stream);
			free(lpImage);
			return NULL;
		}

		lpCB->read(
			lpImage->ColorMap, 
			1, 
			(lpImage->Header.nCMEntrySize/8)*lpImage->Header.nNumCMEntries, 
			stream);
	}

	// Read the image data.
	lpImage->ImageData=malloc(lpImage->Header.nHeight*lpImage->Header.nWidth*lpImage->Header.nBitsPerPixel/8);
	if(!lpImage->ImageData)
	{
		if(lpImage->ColorMap)free(lpImage->ColorMap);	
		lpCB->close(stream);
		if(lpImage)free(lpImage);
		return NULL;
	}
	// Read the data.  If the data is compressed we need to decode.
	if((lpImage->Header.nImageType >= 9) && (lpImage->Header.nImageType <=11))
	{
		TGA_ReadRLEData(stream, lpCB, lpImage);
	}
	else
	{
		lpCB->read(
			lpImage->ImageData,
			1,
			(lpImage->Header.nBitsPerPixel/8)*(lpImage->Header.nHeight*lpImage->Header.nWidth),
			stream);
	}
	lpCB->close(stream);

	switch(lpImage->Header.nBitsPerPixel)
	{
	case 8:
	{
		lpImage->DataFmt=TGAFMT_PALETTE;
		switch(lpImage->Header.nCMEntrySize)
		{
		case 16:
			lpImage->ColorMapFmt=TGAFMT_X1R5G5B5;
			break;
		case 24:
			lpImage->ColorMapFmt=TGAFMT_R8G8B8;
			break;
		case 32:
			lpImage->ColorMapFmt=TGAFMT_A8R8G8B8;
			break;
		default:
			lpImage->ColorMapFmt=TGAFMT_UNKNOWN;
		}
		break;
	}
	case 16:
		lpImage->DataFmt=TGAFMT_X1R5G5B5;
		lpImage->ColorMapFmt=TGAFMT_NONE;
		break;
	case 24:
		lpImage->DataFmt=TGAFMT_R8G8B8;
		lpImage->ColorMapFmt=TGAFMT_NONE;
		break;
	case 32:
		lpImage->DataFmt=TGAFMT_A8R8G8B8;
		lpImage->ColorMapFmt=TGAFMT_NONE;
		break;
	default:
		lpImage->DataFmt=TGAFMT_UNKNOWN;
		lpImage->ColorMapFmt=TGAFMT_UNKNOWN;
		break;
	}

	return (HTGAIMAGE)lpImage;
}


HTGAIMAGE TGA_Open(char* szFilename)
{
	TGA_CALLBACKS cb;
	FILE* fin=NULL;
	cb.close=fclose;
	cb.read=fread;
	cb.seek=fseek;
	cb.tell=ftell;

	fin=fopen(szFilename, "rb");
	if(!fin)
		return NULL;
	return TGA_OpenCallbacks(fin, &cb);
}

int TGA_Delete(HTGAIMAGE hImage)
{
	if(!hImage)
		return 0;
	/* Delete the color map data. */
	if( ((LPTGAIMAGE)hImage)->ColorMap )
		free( ((LPTGAIMAGE)hImage)->ColorMap);
	/* Delete the image data. */
	if( ((LPTGAIMAGE)hImage)->ImageData )
		free( ((LPTGAIMAGE)hImage)->ImageData);
	/* Free the structure. */
	free(hImage);

	return 1;
}

int TGA_GetPalette(
	HTGAIMAGE hImage,
	void* lpDataOut)
{
	if( (!lpDataOut) || (!((LPTGAIMAGE)hImage)->ColorMap) )
		return 0;

	memcpy(
		lpDataOut, 
		((LPTGAIMAGE)hImage)->ColorMap, 
		(((LPTGAIMAGE)hImage)->Header.nCMEntrySize/8)*((LPTGAIMAGE)hImage)->Header.nNumCMEntries);
	return 1;
}

#define TGA_scale_float(value, scale) (long)(((float)value)*(scale))
#define TGA_scale_int(value, scale) (long)((((long)value)*(scale))

int TGA_ChangeBitDepth(
	unsigned long* color, 
	TGAFMT prevdepth, 
	TGAFMT newdepth, 
	unsigned char extra)
{
	unsigned long r=0, g=0, b=0, a=0;
	unsigned long newcolor=0x00000000l;

	if(!color)
		return 0;

	if(prevdepth==newdepth)
		return 1;

	if(newdepth==TGAFMT_PALETTE)
		return 0;

	switch(prevdepth)
	{
	case TGAFMT_X1R5G5B5:
		/* Using floating point, for the conversion proccess, is more accurate,
			but slower. */
		a=((*color)&0x8000)>>15;
		r=((*color)&0x7C00)>>10;
		g=((*color)&0x03E0)>>5;
		b=((*color)&0x001F)>>0;

		//a*=255;
		a=extra;
		r=TGA_scale_float(r, 255.0f/31.0f);
		g=TGA_scale_float(g, 255.0f/31.0f);
		b=TGA_scale_float(b, 255.0f/31.0f);
		break;
	case TGAFMT_R5G6B5:
		a = extra;
		r = ((*color)&0xF800)>>10;
		g = ((*color)&0x07E0)>>5;
		b = ((*color)&0x001F)>>0;

		a = extra;
		r=TGA_scale_float(r, 255.0f/31.0f);
		g=TGA_scale_float(g, 255.0f/63.0f);
		b=TGA_scale_float(b, 255.0f/31.0f);
	case TGAFMT_A8R8G8B8:
		//Not 100% sure this is getting the right value.
		a=(char)(((*color)&0xFF000000)>>24);
		extra=(char)a;
	case TGAFMT_R8G8B8:
		a=extra;
		r=((*color)&0x00FF0000)>>16;
		g=((*color)&0x0000FF00)>>8;
		b=((*color)&0x000000FF)>>0;
		break;
	case TGAFMT_PALETTE:
		return 0;
		break;
	}

	switch(newdepth)
	{
		case TGAFMT_X1R5G5B5:
			r=TGA_scale_float(r, 31.0f/255.0f);
			g=TGA_scale_float(g, 31.0f/255.0f);
			b=TGA_scale_float(b, 31.0f/255.0f);
			*color=0;
			*color=((a>0?1:0)<<15)|(r<<10)|(g<<5)|(b<<0);
			break;
		case TGAFMT_R5G6B5:
			r=TGA_scale_float(r, 31.0f/255.0f);
			g=TGA_scale_float(g, 63.0f/255.0f);
			b=TGA_scale_float(b, 31.0f/255.0f);
			*color=0;
			*color=(r<<11)|(g<<5)|(b<<0);
			break;
		case TGAFMT_R8G8B8:
		case TGAFMT_A8R8G8B8:
			*color=(a<<24)|(r<<16)|(g<<8)|(b<<0);
			break;
	}
	return 1;
}

int TGA_GetPixel(
	HTGAIMAGE hImage,
	unsigned long* lpPix,
	signed short x,
	signed short y,
	TGAFMT Format,
	unsigned char nExtra)
{
	TGAIMAGE* lpImage=hImage;
	TGAORIENT nSrcOrient=lpImage?TGA_IMAGE_ORIGIN(lpImage->Header.nImageDesc):0;
	TGAFMT nSrcFormat=0;
	unsigned short iHeight=0, iWidth=0;
	unsigned long nSource=0;
	unsigned long nSourceColor=0, nSourceCMEntry=0;

	if(!lpImage)
		return 0;

	iHeight=lpImage->Header.nHeight;
	iWidth=lpImage->Header.nWidth;

	if(x>=iWidth)
		x=iWidth-1;
	if(y>=iHeight)
		y=iHeight-1;

	if(x<0)
		x=0;
	if(y<0)
		y=0;

	switch(nSrcOrient)
	{
	case TGAORIENT_BOTTOMLEFT:
		nSource=iWidth*(iHeight-y-1) + x;
		break;
	case TGAORIENT_BOTTOMRIGHT:
		nSource=iWidth*(iHeight-1-y) + (iWidth-x-1);
		break;
	case TGAORIENT_TOPLEFT:
		nSource=iWidth*y + x;
		break;
	case TGAORIENT_TOPRIGHT:
		nSource=iWidth*y + (iWidth-x-1);
		break;
	default:
		return 0;
	}

	nSource*=lpImage->Header.nBitsPerPixel/8;

	memcpy(&nSourceColor, (void*)((unsigned int)lpImage->ImageData+nSource), lpImage->Header.nBitsPerPixel/8);
	if(lpImage->DataFmt==TGAFMT_PALETTE)
	{
		nSourceCMEntry=nSourceColor;
		nSourceColor=0;
		memcpy(&nSourceColor, (void*)((unsigned int)lpImage->ColorMap+nSourceCMEntry*(lpImage->Header.nCMEntrySize/8)), (lpImage->Header.nCMEntrySize/8)); 
		nSrcFormat=lpImage->ColorMapFmt;
	}
	else
		nSrcFormat=lpImage->DataFmt;
	TGA_ChangeBitDepth(&nSourceColor, nSrcFormat, Format, nExtra);
	if(lpPix)
		*lpPix=nSourceColor;

	return 1;
}


int TGA_GetPixelFilter(
	HTGAIMAGE hImage,
	unsigned short nSampleLevel,
	unsigned long* lpPix,
	unsigned short x,
	unsigned short y,
	TGAFMT Format,
	unsigned char nExtra)
{
	unsigned long* lpPixels;
	unsigned short over=0, under=0;
	unsigned long i=0;
	unsigned long nCTR=0, nCTG=0, nCTB=0, nCTA=0;
	unsigned long nNumPix=0;

	nNumPix=(unsigned long)pow((double)2, (double)(nSampleLevel+1));

	lpPixels=malloc(nNumPix*sizeof(unsigned long));

	if(nNumPix<=1 || !lpPixels || nSampleLevel==0)
	{
		if(lpPixels)
			free(lpPixels);

		return TGA_GetPixel(
			hImage,
			lpPix,
			x,
			y,
			Format,
			nExtra);
	}

	for(under=0, i=0; under<(nNumPix/2); under++)
	{
		for(over=0; over<(nNumPix/2); over++, i++)
		{
			if(i>=nNumPix)
				break;
			lpPixels[i]=0;
			TGA_GetPixel(hImage, &lpPixels[i], (short)(x+over/*-nNumPix/4*/), (short)(y+under/*-nNumPix/4*/), TGAFMT_A8R8G8B8, nExtra);
		}
	}

	for(i=0; i<(nNumPix); i++)
	{
		nCTA+=(0xFF000000&lpPixels[i])>>24;
		nCTR+=(0x00FF0000&lpPixels[i])>>16;
		nCTG+=(0x0000FF00&lpPixels[i])>>8;
		nCTB+=(0x000000FF&lpPixels[i])>>0;
	}

	free(lpPixels);

	nCTA/=(nNumPix);
	nCTR/=(nNumPix);
	nCTG/=(nNumPix);
	nCTB/=(nNumPix);
	
	nCTA<<=24;
	nCTR<<=16;
	nCTG<<=8;
	nCTB<<=0;

	if(lpPix)
	{
		*lpPix=(nCTA|nCTR|nCTG|nCTB);
		TGA_ChangeBitDepth(lpPix, TGAFMT_A8R8G8B8, Format, nExtra);
	}
	return 1;
}


int TGA_CopyBits(
	HTGAIMAGE hImage, 
	void* lpOut, 
	TGAORIENT nOrient, 
	TGAFMT destFormat,
	unsigned short nWidth, 
	unsigned short nHeight,
	unsigned short nPitch,
	unsigned char nExtra)
{
	return TGA_CopyBitsStretch(
		hImage,
		TGAFILTER_NONE,
		lpOut,
		nOrient,
		destFormat,
		nWidth,
		nHeight,
		nPitch,
		nExtra);
}

int TGA_CopyBitsStretch(
	HTGAIMAGE hImage,
	TGAFILTER Filter,
	void* lpOut,
	TGAORIENT nOrient,
	TGAFMT Format,
	unsigned short nWidth,
	unsigned short nHeight,
	unsigned short nPitch,
	unsigned char nExtra)
{
	TGAIMAGE* lpImage=hImage;

	unsigned short x=0, y=0;
	unsigned char nDestBitDepth=0;
	unsigned long nPix=0, nCMEntry=0;
	unsigned long nDest=0;
	float fWidthRatio=0.0f, fHeightRatio=0.0f;
	float fSrcX=0.0f, fSrcY=0.0f;
	unsigned short nMipLevel=0;

	if(!lpImage)
		return 0;

	switch(Format)
	{
	case TGAFMT_PALETTE:
		nDestBitDepth=8;
		break;
	case TGAFMT_X1R5G5B5:
	case TGAFMT_R5G6B5:
		nDestBitDepth=16;
		break;
	case TGAFMT_R8G8B8:
		nDestBitDepth=24;
		break;
	case TGAFMT_A8R8G8B8:
		nDestBitDepth=32;
		break;
	}

	if(nPitch==0)
		nPitch=nWidth*nDestBitDepth;

	fWidthRatio=(float)(lpImage->Header.nWidth-1)/(float)(nWidth-1);
	fHeightRatio=(float)(lpImage->Header.nHeight-1)/(float)(nHeight-1);

	if(Filter==TGAFILTER_LINEAR)
	{
		short nAveRatio=(short)(fWidthRatio+fHeightRatio)/2;
		nMipLevel=0;
		while((nAveRatio/=2)>0)
			nMipLevel++;
	}
	else
	{
		nMipLevel=0;
	}

	for(y=0; y<nHeight; y++)
	{
		for(x=0; x<nWidth; x++)
		{	
			switch(nOrient)
			{
			case TGAORIENT_BOTTOMLEFT:
				nDest=nPitch*(nHeight-(y+1))+x*(nDestBitDepth/8);
				break;
			case TGAORIENT_BOTTOMRIGHT:
				nDest=nPitch*(nHeight-1-y) + (nWidth-x-1)*(nDestBitDepth/8);
				break;
			case TGAORIENT_TOPLEFT:
				nDest=y*nPitch + x*(nDestBitDepth/8);
				//nDest=nWidth*y + x;
				break;
			case TGAORIENT_TOPRIGHT:
				nDest=nPitch*y + (nWidth-x-1)*(nDestBitDepth/8);
				//nDest=nWidth*y + (nWidth-x-1);
				break;
			default:
				return 0;
			}
			
			//nDest*=nBitDepth/8;
			nPix=0x00000000l;
			/* We now need to calculate where we actually want to get the pixel
				from. */
			fSrcX=x*fWidthRatio;
			fSrcY=y*fHeightRatio;

			TGA_GetPixelFilter(hImage, nMipLevel, &nPix, (short)fSrcX, (short)fSrcY, Format, nExtra);
			memcpy((void*)((unsigned int)lpOut+nDest), &nPix, nDestBitDepth/8);
		}
	}



	return 1;
}


int TGA_GetDesc(
	HTGAIMAGE hImage, 
	TGA_DESC* lpDescript)
{
	LPTGAIMAGE lpImage=hImage;
	if(!lpImage || !lpDescript)
		return 0;

	lpDescript->Width=lpImage->Header.nWidth;
	lpDescript->Height=lpImage->Header.nHeight;
	lpDescript->BitsPerPixel=lpImage->Header.nBitsPerPixel;
	lpDescript->NumCMEntries=lpImage->Header.nNumCMEntries;
	lpDescript->ColorMapBitDepth=lpImage->Header.nCMEntrySize;
	return 1;
}
