/* ls_ogg.h - Header for ogg file support
	Copyright (c) 2006, Blaine Myers. */

#ifndef __LS_OGG_H__
#define __LS_OGG_H__
#include <windows.h>
#include "common.h"


#ifdef __cplusplus
extern "C"{
#endif /*__cplusplus*/


lg_void* Ogg_CreateFromDisk(lg_str szFilename);
/*lg_void* Ogg_CreateFromData(void* lpData, lg_dword dwDataSize);*/
lg_bool Ogg_Delete(lg_void* lpWave);
lg_dword Ogg_Read(lg_void* lpWave, void* lpDest, lg_dword dwSizeToRead);
lg_bool Ogg_Reset(lg_void* lpWave);
WAVEFORMATEX* Ogg_GetFormat(lg_void* lpWave, WAVEFORMATEX* lpFormat);
lg_dword Ogg_GetSize(lg_void* lpWave);
lg_bool Ogg_IsEOF(lg_void* lpWave);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /*__LS_OGG_H__*/
