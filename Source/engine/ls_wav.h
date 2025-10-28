/* ls_wav.h - Header for WAVE file reading.
	Copyright (c) 2006, Blaine Myers */

#ifndef __LS_WAV_H__
#define __LS_WAV_H__

#include <windows.h>
#include "common.h"

#ifdef __cplusplus
extern "C"{
#endif /*__cplusplus*/


lg_void* Wav_CreateFromDisk(lg_str szFilename);
/*lg_void* Wav_CreateFromData(void* lpData, lg_dword dwDataSize);*/
lg_bool Wav_Delete(lg_void* lpWave);
lg_dword Wav_Read(lg_void* lpWave, void* lpDest, lg_dword dwSizeToRead);
lg_bool Wav_Reset(lg_void* lpWave);
WAVEFORMATEX* Wav_GetFormat(lg_void* lpWave, WAVEFORMATEX* lpFormat);
lg_dword Wav_GetSize(lg_void* lpWave);
lg_bool Wav_IsEOF(lg_void* lpWave);


#ifdef __cplusplus
}
#endif /*__cplusplus*/

#endif /* __LS_WAV_H__ */