/* ls_wav.c - Fucntions for reading WAVE files.
	Copyright (c) 2006, Blaine Myers. */
	
#include <lf_sys2.h>
#include "common.h"
#include "ls_wav.h"
//#include "ls_load.h"
//#include "lg_err.h"
#include "lg_malloc.h"


typedef struct _WAVEFILE{
	WAVEFORMATEX m_Format;
	HMMIO        m_hmmio;
	MMCKINFO     m_ck;
	MMCKINFO     m_ckRiff;
	lg_dword      m_dwSize;
	lg_dword      m_ReadPos;
	lg_bool       m_bEOF;
	lg_bool       m_bLoaded;
}WAVEFILE, *PWAVEFILE;

LRESULT CALLBACK LS_MMIOProc(
	LPSTR lpmmioinfo,  
	UINT uMsg,         
	LPARAM lParam1,      
	LPARAM lParam2);

lg_bool Wav_OpenMMIO(WAVEFILE* lpWave);
lg_bool Wav_ReadMMIO(WAVEFILE* lpWave);


lg_void* Wav_CreateFromDisk(lg_str szFilename)
{
	WAVEFILE* lpNew=LG_NULL;
	MMIOINFO mmioInfo;

	lpNew=LG_Malloc(sizeof(WAVEFILE));

	if(!lpNew)
		return LG_NULL;

	memset(lpNew, 0, sizeof(WAVEFILE));

	// Open the wave file from disk.
	memset(&mmioInfo, 0, sizeof(mmioInfo));
	mmioInfo.pIOProc=LS_MMIOProc;
	lpNew->m_hmmio=mmioOpen(szFilename, &mmioInfo, MMIO_ALLOCBUF|MMIO_READ);

	if(lpNew->m_hmmio==LG_NULL)
	{
		LG_Free(lpNew);
		return LG_NULL;
	}
	if(!Wav_OpenMMIO(lpNew))
	{
		mmioClose(lpNew->m_hmmio, 0);
		LG_Free(lpNew);
		return LG_NULL;
	}

	return lpNew;
}

/*
WAVEFILE* Wav_CreateFromData(void* lpData, lg_dword dwDataSize)
{
	WAVEFILE* lpNew=LG_NULL;
	MMIOINFO mmioInfo;

	lpNew=LG_Malloc(sizeof(WAVEFILE));

	if(!lpNew)
		return LG_NULL;

	memset(lpNew, 0, sizeof(WAVEFILE));

	memset(&mmioInfo, 0, sizeof(MMIOINFO));
	mmioInfo.fccIOProc = FOURCC_MEM;
	mmioInfo.cchBuffer = dwDataSize;
	mmioInfo.pchBuffer = (char*)lpData;

	// Open the memory buffer
	lpNew->m_hmmio = mmioOpen(NULL, &mmioInfo, MMIO_READ);

	if(!lpNew->m_hmmio)
	{
		LG_Free(lpNew);
		return LG_NULL;
	}

	if(!Wav_OpenMMIO(lpNew))
	{
		mmioClose(lpNew->m_hmmio, 0);
		LG_Free(lpNew);
		return LG_NULL;
	}

	return lpNew;
}
*/

lg_bool Wav_Delete(lg_void* lpWave)
{
	WAVEFILE* lpWaves=(WAVEFILE*)lpWave;
	if(!lpWaves->m_hmmio)
		return LG_FALSE;

	
	mmioClose(lpWaves->m_hmmio, 0);
	LG_Free(lpWaves);
	return LG_TRUE;
}

lg_dword Wav_Read(lg_void* lpWave, void* lpDest, lg_dword dwSizeToRead)
{
	WAVEFILE* lpWaves=(WAVEFILE*)lpWave;
	
	MMIOINFO mmioinfoIn; //Current status of m_hmmio.

	//Data member specifying how many bytes to actually
	//read out of the wave file.
	lg_dword dwDataIn=0;
	char* lpWaveBuffer=LG_NULL;
	lg_dword dwStreamRead=0;

	lg_dword dwRead=0;
	lg_dword dwCopySize=0;




	if(!lpWave || !lpDest)
		return 0;

	if(!lpWaves->m_hmmio)
		return 0;

	if(0 != mmioGetInfo(lpWaves->m_hmmio, &mmioinfoIn, 0))
		return 0;


	//If were not decompressing data, we can directly
	//use the buffer passed in to this function.
	lpWaveBuffer=(char*)lpDest;
	dwDataIn=dwSizeToRead;

	if(dwDataIn > lpWaves->m_ck.cksize)
		dwDataIn=lpWaves->m_ck.cksize;

	lpWaves->m_ck.cksize -= dwDataIn;
	while(dwRead<dwDataIn)
	{
		//Copy the bytes from teh io to the buffer.
		if(0 != mmioAdvance(
			lpWaves->m_hmmio,
			&mmioinfoIn,
			MMIO_READ))
		{
			return LG_FALSE;
		}

		if(mmioinfoIn.pchNext == mmioinfoIn.pchEndRead)
		{
			return LG_FALSE;
		}

		//Perform the copy.
		dwCopySize = (lg_dword)(mmioinfoIn.pchEndRead - mmioinfoIn.pchNext);
		dwCopySize = dwCopySize > (dwDataIn-dwRead) ? (dwDataIn-dwRead) : dwCopySize;
	
		memcpy(lpWaveBuffer+dwRead, mmioinfoIn.pchNext, dwCopySize);
		dwRead += dwCopySize;
		mmioinfoIn.pchNext += dwCopySize;
	}

	if(0 != mmioSetInfo(
		lpWaves->m_hmmio,
		&mmioinfoIn,
		0))
	{
		return LG_FALSE;
	}

	if(lpWaves->m_ck.cksize == 0)
		lpWaves->m_bEOF=TRUE;

	return dwDataIn;
}

lg_bool Wav_Reset(lg_void* lpWave)
{
	WAVEFILE* lpWaves=(WAVEFILE*)lpWave;
	
	if(!lpWave)
		return LG_FALSE;

	lpWaves->m_bEOF=LG_FALSE;

	if(lpWaves->m_hmmio == LG_NULL)
		return LG_FALSE;

	//Seek to the data.
	if(-1 == mmioSeek(
		lpWaves->m_hmmio,
		lpWaves->m_ckRiff.dwDataOffset + sizeof(FOURCC),
		SEEK_SET))
	{
		return LG_FALSE;
	}

	//Search the input file for teh 'data' chunk.
	lpWaves->m_ck.ckid = mmioFOURCC('d', 'a', 't', 'a');
	if(0 != mmioDescend(
		lpWaves->m_hmmio,
		&lpWaves->m_ck,
		&lpWaves->m_ckRiff,
		MMIO_FINDCHUNK))
	{
		return LG_FALSE;
	}
	return LG_TRUE;
}

WAVEFORMATEX* Wav_GetFormat(lg_void* lpWave, WAVEFORMATEX* lpFormat)
{
	WAVEFILE* lpWaves=(WAVEFILE*)lpWave;
	
	if(!lpWave || !lpWaves->m_bLoaded)
		return LG_FALSE;
	if(lpFormat)
	{
		memcpy(lpFormat, &lpWaves->m_Format, sizeof(WAVEFORMATEX));
		return lpFormat;
	}
	else
		return &lpWaves->m_Format;
}

lg_dword Wav_GetSize(lg_void* lpWave)
{
	WAVEFILE* lpWaves=(WAVEFILE*)lpWave;
	if(!lpWave)
		return 0;
	else
		return lpWaves->m_dwSize;
}

lg_bool Wav_IsEOF(lg_void* lpWave)
{
	WAVEFILE* lpWaves=(WAVEFILE*)lpWave;
	if(!lpWave)
		return LG_TRUE;
	else
		return lpWaves->m_bEOF;
}


lg_bool Wav_OpenMMIO(WAVEFILE* lpWave)
{
	if(!Wav_ReadMMIO(lpWave))
	{
		return LG_FALSE;
	}

	//Reset the file to prepare for reading
	if(!Wav_Reset(lpWave))
	{
		return LG_FALSE;
	}

	lpWave->m_dwSize=lpWave->m_ck.cksize;

	return LG_TRUE;
}

lg_bool Wav_ReadMMIO(WAVEFILE* lpWave)
{
	//Chunk info for general use.
	MMCKINFO ckIn;
	//Temp PCM structure to load in.
	PCMWAVEFORMAT pcmWaveFormat;

	//Make sure this structure has been deallocated.
	//SAFE_DELETE_ARRAY(m_pwfx);
	
	if(mmioSeek(lpWave->m_hmmio, 0, SEEK_SET) == -1)
		return LG_FALSE;

	if((0 != mmioDescend(
		lpWave->m_hmmio,
		&lpWave->m_ckRiff,
		NULL,
		0)))
	{
		return LG_FALSE;
	}

	//Check to make sure this is a valid wave file.
	if((lpWave->m_ckRiff.ckid != FOURCC_RIFF) || 
		(lpWave->m_ckRiff.fccType != mmioFOURCC('W', 'A', 'V', 'E')))
	{
		return LG_FALSE;
	}

	//Search the input file for the 'fmt' chunk.
	ckIn.ckid = mmioFOURCC('f', 'm', 't', ' ');

	if(0 != mmioDescend(
		lpWave->m_hmmio,
		&ckIn,
		&lpWave->m_ckRiff,
		MMIO_FINDCHUNK))
	{
		return LG_FALSE;
	}

	//The 'fmt ' chunk will be at least
	//as large as PCMWAVEFORMAT if there are
	//extra parameters at the end, we'll ignore them.
	if(ckIn.cksize < (LONG) sizeof(PCMWAVEFORMAT))
		return LG_FALSE;

	//Read the 'fmt ' chunk into pcmWaveFormat.
	if(mmioRead(
		lpWave->m_hmmio,
		(HPSTR)&pcmWaveFormat,
		sizeof(PCMWAVEFORMAT)) != sizeof(PCMWAVEFORMAT))
	{
		return LG_FALSE;
	}

	//Only handling PCM formats.
	if(pcmWaveFormat.wf.wFormatTag == WAVE_FORMAT_PCM)
	{
		memcpy(
			&lpWave->m_Format,
			&pcmWaveFormat,
			sizeof(PCMWAVEFORMAT));

		lpWave->m_Format.cbSize = 0;
	}
	else
	{
		//NON PCM wave would be handled here.
		return LG_FALSE;
	}

	//Ascend teh input file out of the 'fmt ' chucnk.
	if(0 != mmioAscend(lpWave->m_hmmio, &ckIn, 0))
	{
		//SAFE_DELETE(m_pwfx);
		return LG_FALSE;
	}

	lpWave->m_bLoaded=LG_TRUE;

	return LG_TRUE;
}


/* LS_MMIOProc is the mmio reading procedure that will read from the Legacy
	file system.  It should work I think. This code is not bug proof by any means
	so it still needs some work. */

LRESULT CALLBACK LS_MMIOProc(LPSTR lpmmioinfo, UINT uMsg, LPARAM lParam1, LPARAM lParam2)
{
	LPMMIOINFO info=(LPMMIOINFO)lpmmioinfo;

	switch(uMsg)
	{
	case MMIOM_OPEN:
	{
		LF_FILE3 lpIn=LG_NULL;
		/* When we open a wave file, we don't open it with
			the LF_ACCESS_MEMORY attribute, this is because we may
			want to stream from the disk.*/
		lpIn=LF_Open((char*)lParam1, LF_ACCESS_READ, LF_OPEN_EXISTING);
		if(!lpIn)
			return MMIOERR_CANNOTOPEN;
		
		info->hmmio=lpIn;
		return MMSYSERR_NOERROR;
	}
	case MMIOM_CLOSE:
		LF_Close((LF_FILE3)info->hmmio);
		return 0;
	case MMIOM_READ:
	{
		lg_dword nNumRead=0;
		nNumRead=LF_Read((LF_FILE3)info->hmmio, (int*)lParam1, (lg_dword)lParam2);
		return nNumRead;
	}
	case MMIOM_SEEK:
	{
		lg_long nType=0;
		if(lParam2==SEEK_SET)
			nType=LF_SEEK_BEGIN;
		else if(lParam2==SEEK_END)
			nType=LF_SEEK_END;
		else if(lParam2==SEEK_CUR)
			nType=LF_SEEK_CURRENT;

		LF_Seek((LF_FILE3)info->hmmio, nType, (lg_long)lParam1);
		return 0;
	}
	case MMIOM_WRITE:
	case MMIOM_WRITEFLUSH:
		//Err_Printf("Writing to MMIO is not supported in the Legacy Engine.");
		return -1;
	default:
		return -1;
	}
	return 0;
}