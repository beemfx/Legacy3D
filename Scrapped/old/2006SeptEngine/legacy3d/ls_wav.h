/* ls_wav.h - Header for WAVE file reading.
	Copyright (c) 2006, Blaine Myers */

#ifndef __LS_WAV_H__
#define __LS_WAV_H__

#include <windows.h>
#include "common.h"

#ifdef __cplusplus
extern "C"{
#endif /*__cplusplus*/


typedef struct _WAVEFILE{
	WAVEFORMATEX m_Format;
	HMMIO        m_hmmio;
	MMCKINFO     m_ck;
	MMCKINFO     m_ckRiff;
	L_dword      m_dwSize;
	L_dword      m_ReadPos;
	L_bool       m_bEOF;
	L_bool       m_bLoaded;
}WAVEFILE, *PWAVEFILE;


WAVEFILE* Wav_CreateFromDisk(char* szFilename);
/*WAVEFILE* Wav_CreateFromData(void* lpData, L_dword dwDataSize);*/
L_bool Wav_Delete(WAVEFILE* lpWave);
L_dword Wav_Read(WAVEFILE* lpWave, void* lpDest, L_dword dwSizeToRead);
L_bool Wav_Reset(WAVEFILE* lpWave);
WAVEFORMATEX* Wav_GetFormat(WAVEFILE* lpWave, WAVEFORMATEX* lpFormat);
L_dword Wav_GetSize(WAVEFILE* lpWave);
L_bool Wav_IsEOF(WAVEFILE* lpWave);

LRESULT CALLBACK LS_MMIOProc(
	LPSTR lpmmioinfo,  
	UINT uMsg,         
	LPARAM lParam1,      
	LPARAM lParam2);

#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /* __LS_WAV_H__ */