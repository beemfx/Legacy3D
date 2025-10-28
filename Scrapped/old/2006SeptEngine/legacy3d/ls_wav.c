/* ls_wav.c - Fucntions for reading WAVE files.
	Copyright (c) 2006, Blaine Myers. */
#include "common.h"
#include <lf_sys.h>
#include "ls_wav.h"
#include "ls_load.h"
#include "lg_err.h"

L_bool Wav_OpenMMIO(WAVEFILE* lpWave);
L_bool Wav_ReadMMIO(WAVEFILE* lpWave);


WAVEFILE* Wav_CreateFromDisk(char* szFilename)
{
	WAVEFILE* lpNew=L_null;
	MMIOINFO mmioInfo;

	lpNew=malloc(sizeof(WAVEFILE));

	if(!lpNew)
		return L_null;

	memset(lpNew, 0, sizeof(WAVEFILE));

	// Open the wave file from disk.
	memset(&mmioInfo, 0, sizeof(mmioInfo));
	mmioInfo.pIOProc=LS_MMIOProc;
	lpNew->m_hmmio=mmioOpen(szFilename, &mmioInfo, MMIO_ALLOCBUF|MMIO_READ);

	if(lpNew->m_hmmio==L_null)
	{
		free(lpNew);
		return L_null;
	}
	if(!Wav_OpenMMIO(lpNew))
	{
		mmioClose(lpNew->m_hmmio, 0);
		free(lpNew);
		return L_null;
	}

	return lpNew;
}

/*
WAVEFILE* Wav_CreateFromData(void* lpData, L_dword dwDataSize)
{
	WAVEFILE* lpNew=L_null;
	MMIOINFO mmioInfo;

	lpNew=malloc(sizeof(WAVEFILE));

	if(!lpNew)
		return L_null;

	memset(lpNew, 0, sizeof(WAVEFILE));

	memset(&mmioInfo, 0, sizeof(MMIOINFO));
	mmioInfo.fccIOProc = FOURCC_MEM;
	mmioInfo.cchBuffer = dwDataSize;
	mmioInfo.pchBuffer = (char*)lpData;

	// Open the memory buffer
	lpNew->m_hmmio = mmioOpen(NULL, &mmioInfo, MMIO_READ);

	if(!lpNew->m_hmmio)
	{
		free(lpNew);
		return L_null;
	}

	if(!Wav_OpenMMIO(lpNew))
	{
		mmioClose(lpNew->m_hmmio, 0);
		free(lpNew);
		return L_null;
	}

	return lpNew;
}
*/

L_bool Wav_Delete(WAVEFILE* lpWave)
{
	if(!lpWave->m_hmmio)
		return L_false;

	
	mmioClose(lpWave->m_hmmio, 0);
	free(lpWave);
	return L_true;
}

L_dword Wav_Read(WAVEFILE* lpWave, void* lpDest, L_dword dwSizeToRead)
{
	MMIOINFO mmioinfoIn; //Current status of m_hmmio.

	//Data member specifying how many bytes to actually
	//read out of the wave file.
	L_dword dwDataIn=0;
	char* lpWaveBuffer=L_null;
	L_dword dwStreamRead=0;

	L_dword dwRead=0;
	L_dword dwCopySize=0;




	if(!lpWave || !lpDest)
		return 0;

	if(!lpWave->m_hmmio)
		return 0;

	if(0 != mmioGetInfo(lpWave->m_hmmio, &mmioinfoIn, 0))
		return 0;


	//If were not decompressing data, we can directly
	//use the buffer passed in to this function.
	lpWaveBuffer=(char*)lpDest;
	dwDataIn=dwSizeToRead;

	if(dwDataIn > lpWave->m_ck.cksize)
		dwDataIn=lpWave->m_ck.cksize;

	lpWave->m_ck.cksize -= dwDataIn;
	while(dwRead<dwDataIn)
	{
		//Copy the bytes from teh io to the buffer.
		if(0 != mmioAdvance(
			lpWave->m_hmmio,
			&mmioinfoIn,
			MMIO_READ))
		{
			return L_false;
		}

		if(mmioinfoIn.pchNext == mmioinfoIn.pchEndRead)
		{
			return L_false;
		}

		//Perform the copy.
		dwCopySize = mmioinfoIn.pchEndRead - mmioinfoIn.pchNext;
		dwCopySize = dwCopySize > (dwDataIn-dwRead) ? (dwDataIn-dwRead) : dwCopySize;
	
		memcpy(lpWaveBuffer+dwRead, mmioinfoIn.pchNext, dwCopySize);
		dwRead += dwCopySize;
		mmioinfoIn.pchNext += dwCopySize;
	}

	if(0 != mmioSetInfo(
		lpWave->m_hmmio,
		&mmioinfoIn,
		0))
	{
		return L_false;
	}

	if(lpWave->m_ck.cksize == 0)
		lpWave->m_bEOF=TRUE;

	return dwDataIn;
}

L_bool Wav_Reset(WAVEFILE* lpWave)
{
	if(!lpWave)
		return L_false;

	lpWave->m_bEOF=L_false;

	if(lpWave->m_hmmio == L_null)
		return L_false;

	//Seek to the data.
	if(-1 == mmioSeek(
		lpWave->m_hmmio,
		lpWave->m_ckRiff.dwDataOffset + sizeof(FOURCC),
		SEEK_SET))
	{
		return L_false;
	}

	//Search the input file for teh 'data' chunk.
	lpWave->m_ck.ckid = mmioFOURCC('d', 'a', 't', 'a');
	if(0 != mmioDescend(
		lpWave->m_hmmio,
		&lpWave->m_ck,
		&lpWave->m_ckRiff,
		MMIO_FINDCHUNK))
	{
		return L_false;
	}
	return L_true;
}

WAVEFORMATEX* Wav_GetFormat(WAVEFILE* lpWave, WAVEFORMATEX* lpFormat)
{
	if(!lpWave || !lpWave->m_bLoaded)
		return L_false;
	if(lpFormat)
	{
		memcpy(lpFormat, &lpWave->m_Format, sizeof(WAVEFORMATEX));
		return lpFormat;
	}
	else
		return &lpWave->m_Format;
}

L_dword Wav_GetSize(WAVEFILE* lpWave)
{
	if(!lpWave)
		return 0;
	else
		return lpWave->m_dwSize;
}

L_bool Wav_IsEOF(WAVEFILE* lpWave)
{
	if(!lpWave)
		return L_true;
	else
		return lpWave->m_bEOF;
}


L_bool Wav_OpenMMIO(WAVEFILE* lpWave)
{
	if(!Wav_ReadMMIO(lpWave))
	{
		return L_false;
	}

	//Reset the file to prepare for reading
	if(!Wav_Reset(lpWave))
	{
		return L_false;
	}

	lpWave->m_dwSize=lpWave->m_ck.cksize;

	return L_true;
}

L_bool Wav_ReadMMIO(WAVEFILE* lpWave)
{
	//Chunk info for general use.
	MMCKINFO ckIn;
	//Temp PCM structure to load in.
	PCMWAVEFORMAT pcmWaveFormat;

	//Make sure this structure has been deallocated.
	//SAFE_DELETE_ARRAY(m_pwfx);
	
	if(mmioSeek(lpWave->m_hmmio, 0, SEEK_SET) == -1)
		return L_false;

	if((0 != mmioDescend(
		lpWave->m_hmmio,
		&lpWave->m_ckRiff,
		NULL,
		0)))
	{
		return L_false;
	}

	//Check to make sure this is a valid wave file.
	if((lpWave->m_ckRiff.ckid != FOURCC_RIFF) || 
		(lpWave->m_ckRiff.fccType != mmioFOURCC('W', 'A', 'V', 'E')))
	{
		return L_false;
	}

	//Search the input file for the 'fmt' chunk.
	ckIn.ckid = mmioFOURCC('f', 'm', 't', ' ');

	if(0 != mmioDescend(
		lpWave->m_hmmio,
		&ckIn,
		&lpWave->m_ckRiff,
		MMIO_FINDCHUNK))
	{
		return L_false;
	}

	//The 'fmt ' chunk will be at least
	//as large as PCMWAVEFORMAT if there are
	//extra parameters at the end, we'll ignore them.
	if(ckIn.cksize < (LONG) sizeof(PCMWAVEFORMAT))
		return L_false;

	//Read the 'fmt ' chunk into pcmWaveFormat.
	if(mmioRead(
		lpWave->m_hmmio,
		(HPSTR)&pcmWaveFormat,
		sizeof(PCMWAVEFORMAT)) != sizeof(PCMWAVEFORMAT))
	{
		return L_false;
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
		return L_false;
	}

	//Ascend teh input file out of the 'fmt ' chucnk.
	if(0 != mmioAscend(lpWave->m_hmmio, &ckIn, 0))
	{
		//SAFE_DELETE(m_pwfx);
		return L_false;
	}

	lpWave->m_bLoaded=L_true;

	return L_true;
}


/* LS_MMIOProc is the mmio reading procedure that will read from the Legacy 3D
	file system.  It should work I think. This code is not bug proof by any means
	so it still needs some work. */

LRESULT CALLBACK LS_MMIOProc(LPSTR lpmmioinfo, UINT uMsg, LPARAM lParam1, LPARAM lParam2)
{
	LPMMIOINFO info=(LPMMIOINFO)lpmmioinfo;

	switch(uMsg)
	{
	case MMIOM_OPEN:
	{
		LF_FILE2 lpIn=L_null;
		/* When we open a wave file, we don't open it with
			the LFF_MEMBUF attribute, this is because we may
			want to stream from the disk. However if the file
			is opened from an archive it will be opened
			LFF_MEMBUF anyway, so long music soundtracks should
			not be archived in the game, they should be in their
			own directory. */
		lpIn=File_Open((char*)lParam1, 0, LF_ACCESS_READ, LFCREATE_OPEN_EXISTING);
		if(!lpIn)
			return MMIOERR_CANNOTOPEN;
		
		info->dwReserved1=(L_dword)lpIn;
		return MMSYSERR_NOERROR;
	}
	case MMIOM_CLOSE:
		File_Close((LF_FILE2)info->dwReserved1);
		return 0;
	case MMIOM_READ:
	{
		L_dword nNumRead=0;
		nNumRead=File_Read((LF_FILE2)info->dwReserved1, lParam2, (int*)lParam1);
		return nNumRead;
	}
	case MMIOM_SEEK:
	{
		L_long nType=0;
		if(lParam2==SEEK_SET)
			nType=MOVETYPE_SET;
		else if(lParam2==SEEK_END)
			nType=MOVETYPE_END;
		else if(lParam2==SEEK_CUR)
			nType=MOVETYPE_CUR;

		File_Seek((LF_FILE2)info->dwReserved1, lParam1, nType);
		return 0;
	}
	case MMIOM_WRITE:
	case MMIOM_WRITEFLUSH:
		Err_Printf("Writing to MMIO is not supported in Legacy3D.");
		return -1;
	default:
		return -1;
	}
	return 0;
}