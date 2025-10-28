/* ls_ogg.c - ogg file support for the Legacy Sound System
	Copyright (c) 2006, Blaine Myers. */
#include "common.h"
#include "ls_ogg.h"
size_t ogg_read(void* ptr, size_t size, size_t nmemb, void* datasource);
int ogg_seek(void* datasource, ogg_int64_t offset, int whence);
int ogg_close(void* datasource);
long ogg_tell(void* datasource);

L_bool Ogg_GetStreamInfo(OGGFILE* lpOgg);


OGGFILE* Ogg_CreateFromDisk(char* szFilename)
{
	OGGFILE* lpNew=L_null;
	ov_callbacks cb;
	LF_FILE2 File=L_null;

	lpNew=malloc(sizeof(OGGFILE));

	if(!lpNew)
		return L_null;

	memset(lpNew, 0, sizeof(OGGFILE));
	memset(&cb, 0, sizeof(cb));
	cb.read_func=ogg_read;
	cb.seek_func=ogg_seek;
	cb.close_func=ogg_close;
	cb.tell_func=ogg_tell;

	
	File=File_Open(szFilename, 0, LF_ACCESS_READ, LFCREATE_OPEN_EXISTING);

	if(!File)
	{
		free(lpNew);
		return L_null;
	}

	/* We don't ever call the File_Close function here,
		because teh callbacks functions will call it
		on ogg_close callback. */
	if(ov_open_callbacks(
		File,
		&lpNew->m_VorbisFile,
		L_null,
		0,
		cb)<0)
	{
		free(lpNew);
		return L_null;
	}

	if(!Ogg_GetStreamInfo(lpNew))
	{
		ov_clear(&lpNew->m_VorbisFile);
		free(lpNew);
		return L_null;
	}
	lpNew->m_bOpen=L_true;
	return lpNew;
}

/*
OGGFILE* Ogg_CreateFromData(void* lpData, L_dword dwDataSize)
{
	return L_null;
}
*/


L_bool Ogg_Delete(OGGFILE* lpOgg)
{
	if(!lpOgg)
		return L_false;
	ov_clear(&lpOgg->m_VorbisFile);
	free(lpOgg);
	return L_true;
}


L_dword Ogg_Read(OGGFILE* lpOgg, void* lpDest, L_dword dwSizeToRead)
{
	char* pCurBuffer=(char*)lpDest;
	L_dword nBytesRead=0;
	int iSection=0;

	if(!lpOgg || !lpDest)
		return 0;

	
	while((nBytesRead<dwSizeToRead) && !lpOgg->m_bEOF)
	{
		L_long iRet=ov_read(
			&lpOgg->m_VorbisFile, 
			pCurBuffer, 
			dwSizeToRead-nBytesRead, 
			0, 
			2, 
			1, 
			&iSection);
		
		if(iRet==0 || iSection !=0)
			lpOgg->m_bEOF=L_true;

		nBytesRead+=iRet;
		pCurBuffer+=iRet;
	}
	return nBytesRead;
}


L_bool Ogg_Reset(OGGFILE* lpOgg)
{
	if(!lpOgg)
		return L_false;

	lpOgg->m_bEOF=L_false;
	ov_pcm_seek(&lpOgg->m_VorbisFile, 0);
	return L_true;
}


WAVEFORMATEX* Ogg_GetFormat(OGGFILE* lpOgg, WAVEFORMATEX* lpFormat)
{
	if(!lpOgg)
		return L_null;

	if(lpFormat)
	{
		memcpy(lpFormat, &lpOgg->m_Format, sizeof(WAVEFORMATEX));
		return lpFormat;
	}
	else
		return &lpOgg->m_Format;
}

L_dword Ogg_GetSize(OGGFILE* lpOgg)
{
	if(!lpOgg)
		return 0;

	return lpOgg->m_nNumSamples*lpOgg->m_Format.nChannels*lpOgg->m_Format.wBitsPerSample/8;
}
L_bool Ogg_IsEOF(OGGFILE* lpOgg)
{
	if(!lpOgg)
		return L_true;
	return lpOgg->m_bEOF;
}




L_bool Ogg_GetStreamInfo(OGGFILE* lpOgg)
{
	if(!lpOgg)
		return L_false;

	lpOgg->m_pVorbisInfo=ov_info(&lpOgg->m_VorbisFile, -1);
	lpOgg->m_nNumSamples=(long)ov_pcm_total(&lpOgg->m_VorbisFile, -1);
	lpOgg->m_Format.wFormatTag=WAVE_FORMAT_PCM;
	lpOgg->m_Format.nChannels=lpOgg->m_pVorbisInfo->channels;
	lpOgg->m_Format.nSamplesPerSec=lpOgg->m_pVorbisInfo->rate;
	lpOgg->m_Format.wBitsPerSample=16;
	lpOgg->m_Format.nBlockAlign=lpOgg->m_Format.nChannels*lpOgg->m_Format.wBitsPerSample/8;
	lpOgg->m_Format.nAvgBytesPerSec=lpOgg->m_Format.nSamplesPerSec*lpOgg->m_Format.nBlockAlign;
	lpOgg->m_Format.cbSize=0;

	return L_true;
}



/****************************************
	OGG Callback Functions for reading.
****************************************/

size_t ogg_read(void* ptr, size_t size, size_t nmemb, void* datasource)
{
	return File_Read(datasource, size*nmemb, ptr);
}
int ogg_seek(void* datasource, ogg_int64_t offset, int whence)
{
	return File_Seek(datasource, (L_long)offset, whence);
}
int ogg_close(void* datasource)
{
	return File_Close(datasource);
}
long ogg_tell(void* datasource)
{
	return File_Seek(datasource, 0, MOVETYPE_CUR);
}
