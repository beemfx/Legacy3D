/**********************************************************************
*** lf_bdio_ex.c - Includes extra functions for basic input output
***
**********************************************************************/
#include <zlib.h>
#include "lf_bdio.h"

voidpf ZLIB_Alloc(voidpf opaque, uInt items, uInt size)
{
	return LF_Malloc(items*size);
}

void ZLIB_Free(voidpf opaque, voidpf address)
{
	LF_Free(address);
}

voidpf zcalloc(voidpf opaque, uInt items, uInt size)
{
	return ZLIB_Alloc(opaque, items, size);
}

void zcfree(voidpf opaque, voidpf address)
{
	ZLIB_Free(opaque, address);
}

lf_dword BDIO_WriteCompressed(BDIO_FILE File, lf_dword nSize, void* pInBuffer)
{
	lf_dword nWritten=0;
	z_stream stream;
	int err;
	lf_byte* pCmpBuffer=LF_Malloc(nSize);
	if(!pCmpBuffer)
		return 0;
		
	stream.next_in=(Bytef*)pInBuffer;
	stream.avail_in=(uInt)nSize;
	stream.next_out=pCmpBuffer;
	stream.avail_out=nSize;
	stream.zalloc=ZLIB_Alloc;
	stream.zfree=ZLIB_Free;
	stream.opaque=(voidpf)0;
	
	err=deflateInit(&stream, Z_DEFAULT_COMPRESSION);
	if(err!=Z_OK)
	{
		LF_Free(pCmpBuffer);
		return 0;
	}
	err=deflate(&stream, Z_FINISH);
	if(err!=Z_STREAM_END)
	{
		deflateEnd(&stream);
		LF_Free(pCmpBuffer);
		return 0;
	}
	nWritten=stream.total_out;
	deflateEnd(&stream);
	nWritten=BDIO_Write(File, nWritten, pCmpBuffer);
	
	LF_Free(pCmpBuffer);
	return nWritten;
}


lf_dword BDIO_ReadCompressed(BDIO_FILE File, lf_dword nSize, void* pInBuffer)
{
	//The BDIO_ReadCompressed function is somewhat complicated.  Unlike the
	//BDIO_WriteCompressed method. Also note that this function returns the 
	//size of the data that was decompressed, and not the actual amount of 
	//data read from the file.
	z_stream in_stream;
	int err;
	lf_dword nRead;
	lf_dword nPos;
	lf_byte* pCmpBuffer=LF_Malloc(nSize);
	
	if(!pCmpBuffer)
		return 0;
	
	nPos=BDIO_Tell(File);
	nRead=BDIO_Read(File, nSize, pCmpBuffer);
	
	in_stream.zalloc=ZLIB_Alloc;
	in_stream.zfree=ZLIB_Free;
	in_stream.opaque=(voidpf)0;
		
	in_stream.next_out=pInBuffer;
	in_stream.avail_out=nSize;
	in_stream.next_in=pCmpBuffer;
	in_stream.avail_in=nRead;
		
	err=inflateInit(&in_stream);
	if(Z_OK != err)
	{
		LF_Free(pCmpBuffer);
		BDIO_Seek(File, nPos, BDIO_SEEK_BEGIN);
		return 0;
	}
	
	err=inflate(&in_stream, Z_FINISH);
	nRead=in_stream.total_out;
	BDIO_Seek(File, nPos+in_stream.total_in, BDIO_SEEK_BEGIN);
	inflateEnd(&in_stream);
	
	LF_Free(pCmpBuffer);
	return nRead;
}

lf_dword LF_SYS2_EXPORTS BDIO_CopyData(BDIO_FILE DestFile, BDIO_FILE SourceFile, lf_dword nSize)
{
	lf_dword i=0;
	lf_byte byte=0;
	lf_dword nWritten=0;
	lf_byte* pData=LF_Malloc(nSize);
	if(!pData)
		return 0;
		
	nWritten=BDIO_Read(SourceFile, nSize, pData);
	nWritten=BDIO_Write(DestFile, nWritten, pData);
	LF_Free(pData);
	return nWritten;
}

