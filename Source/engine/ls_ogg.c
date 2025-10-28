/* ls_ogg.c - ogg file support for the Legacy Sound System
	Copyright (c) 2006, Blaine Myers. */
#include <vorbis\codec.h>
#include <vorbis\vorbisfile.h>
#include "lf_sys2.h"
#include "common.h"
#include "ls_ogg.h"
#include "lg_malloc.h"


typedef struct _OGGFILE{
	WAVEFORMATEX    m_Format;
	lg_bool         m_bOpen;
	OggVorbis_File  m_VorbisFile;
	vorbis_info*    m_pVorbisInfo;
	void*           m_pBufferPtr;
	void*           m_pBuffer;
	lg_dword        m_nBufferSize;
	lg_dword        m_nNumSamples;
	lg_bool         m_bEOF;
}OGGFILE, *POGGFILE;


size_t ogg_read(void* ptr, size_t size, size_t nmemb, void* datasource);
int ogg_seek(void* datasource, ogg_int64_t offset, int whence);
int ogg_close(void* datasource);
long ogg_tell(void* datasource);

lg_bool Ogg_GetStreamInfo(OGGFILE* lpOggs);


lg_void* Ogg_CreateFromDisk(lg_str szFilename)
{
	OGGFILE* lpNew=LG_NULL;
	ov_callbacks cb;
	LF_FILE3 File=LG_NULL;
	int nErr;

	lpNew=LG_Malloc(sizeof(OGGFILE));

	if(!lpNew)
		return LG_NULL;

	memset(lpNew, 0, sizeof(OGGFILE));
	memset(&cb, 0, sizeof(cb));
	cb.read_func=ogg_read;
	cb.seek_func=ogg_seek;
	cb.close_func=ogg_close;
	cb.tell_func=ogg_tell;

	
	File=LF_Open(szFilename, LF_ACCESS_READ, LF_OPEN_EXISTING);

	if(!File)
	{
		LG_Free(lpNew);
		return LG_NULL;
	}

	/* We don't ever call the File_Close function here,
		because the callbacks functions will call it
		on ogg_close callback. */
	nErr=ov_open_callbacks(
		File,
		&lpNew->m_VorbisFile,
		LG_NULL,
		0,
		cb);
		
	if(nErr<0)
	{
		LG_Free(lpNew);
		return LG_NULL;
	}

	if(!Ogg_GetStreamInfo(lpNew))
	{
		ov_clear(&lpNew->m_VorbisFile);
		LG_Free(lpNew);
		return LG_NULL;
	}
	lpNew->m_bOpen=LG_TRUE;
	return lpNew;
}

/*
OGGFILE* Ogg_CreateFromData(void* lpData, lg_dword dwDataSize)
{
	return LG_NULL;
}
*/


lg_bool Ogg_Delete(lg_void* lpOgg)
{
	OGGFILE* lpOggs=(OGGFILE*)lpOgg;
	if(!lpOggs)
		return LG_FALSE;
	ov_clear(&lpOggs->m_VorbisFile);
	LG_Free(lpOggs);
	return LG_TRUE;
}


lg_dword Ogg_Read(lg_void* lpOgg, void* lpDest, lg_dword dwSizeToRead)
{
	OGGFILE* lpOggs=(OGGFILE*)lpOgg;
	char* pCurBuffer=(char*)lpDest;
	lg_dword nBytesRead=0;
	int iSection=0;

	if(!lpOggs || !lpDest)
		return 0;

	
	while((nBytesRead<dwSizeToRead) && !lpOggs->m_bEOF)
	{
		lg_long iRet=ov_read(
			&lpOggs->m_VorbisFile, 
			pCurBuffer, 
			dwSizeToRead-nBytesRead, 
			0, 
			2, 
			1, 
			&iSection);
		
		if(iRet==0 || iSection !=0)
			lpOggs->m_bEOF=LG_TRUE;

		nBytesRead+=iRet;
		pCurBuffer+=iRet;
	}
	return nBytesRead;
}


lg_bool Ogg_Reset(lg_void* lpOgg)
{
	OGGFILE* lpOggs=(OGGFILE*)lpOgg;
	if(!lpOggs)
		return LG_FALSE;

	lpOggs->m_bEOF=LG_FALSE;
	ov_pcm_seek(&lpOggs->m_VorbisFile, 0);
	return LG_TRUE;
}


WAVEFORMATEX* Ogg_GetFormat(lg_void* lpOgg, WAVEFORMATEX* lpFormat)
{
	OGGFILE* lpOggs=(OGGFILE*)lpOgg;
	if(!lpOggs)
		return LG_NULL;

	if(lpFormat)
	{
		memcpy(lpFormat, &lpOggs->m_Format, sizeof(WAVEFORMATEX));
		return lpFormat;
	}
	else
		return &lpOggs->m_Format;
}

lg_dword Ogg_GetSize(lg_void* lpOgg)
{
	OGGFILE* lpOggs=(OGGFILE*)lpOgg;
	if(!lpOggs)
		return 0;

	return lpOggs->m_nNumSamples*lpOggs->m_Format.nChannels*lpOggs->m_Format.wBitsPerSample/8;
}
lg_bool Ogg_IsEOF(lg_void* lpOgg)
{
	OGGFILE* lpOggs=(OGGFILE*)lpOgg;
	if(!lpOggs)
		return LG_TRUE;
	return lpOggs->m_bEOF;
}




lg_bool Ogg_GetStreamInfo(OGGFILE* lpOggs)
{
	if(!lpOggs)
		return LG_FALSE;

	lpOggs->m_pVorbisInfo=ov_info(&lpOggs->m_VorbisFile, -1);
	lpOggs->m_nNumSamples=(long)ov_pcm_total(&lpOggs->m_VorbisFile, -1);
	lpOggs->m_Format.wFormatTag=WAVE_FORMAT_PCM;
	lpOggs->m_Format.nChannels=lpOggs->m_pVorbisInfo->channels;
	lpOggs->m_Format.nSamplesPerSec=lpOggs->m_pVorbisInfo->rate;
	lpOggs->m_Format.wBitsPerSample=16;
	lpOggs->m_Format.nBlockAlign=lpOggs->m_Format.nChannels*lpOggs->m_Format.wBitsPerSample/8;
	lpOggs->m_Format.nAvgBytesPerSec=lpOggs->m_Format.nSamplesPerSec*lpOggs->m_Format.nBlockAlign;
	lpOggs->m_Format.cbSize=0;

	return LG_TRUE;
}



/****************************************
	OGG Callback Functions for reading.
****************************************/

size_t ogg_read(void* ptr, size_t size, size_t nmemb, void* datasource)
{
	return LF_Read(datasource, ptr, size*nmemb);
}
int ogg_seek(void* datasource, ogg_int64_t offset, int whence)
{
	int nMode;

	if(whence==SEEK_SET)
		nMode=LF_SEEK_BEGIN;
	else if(whence==SEEK_CUR)
		nMode=LF_SEEK_CURRENT;
	else if(whence==SEEK_END)
		nMode=LF_SEEK_END;
	else
		return 1;
		
	LF_Seek(datasource, nMode, (lg_long)offset);
	return 0;
}
int ogg_close(void* datasource)
{
	return !LF_Close(datasource);
}
long ogg_tell(void* datasource)
{
	return LF_Tell(datasource);
}
