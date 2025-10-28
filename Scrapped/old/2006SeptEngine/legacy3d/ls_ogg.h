/* ls_ogg.h - Header for ogg file support
	Copyright (c) 2006, Blaine Myers. */

#ifndef __LS_OGG_H__
#define __LS_OGG_H__
#include <windows.h>
#include <vorbis\codec.h>
#include <vorbis\vorbisfile.h>
#include <lf_sys.h>
#include "common.h"
#ifdef __cplusplus
extern "C"{
#endif /*__cplusplus*/


typedef struct _OGGFILE{
	WAVEFORMATEX   m_Format;
	L_bool         m_bOpen;
	OggVorbis_File m_VorbisFile;
	vorbis_info*   m_pVorbisInfo;
	void*          m_pBufferPtr;
	void*          m_pBuffer;
	L_dword        m_nBufferSize;
	L_dword        m_nNumSamples;
	L_bool         m_bEOF;
}OGGFILE, *POGGFILE;


OGGFILE* Ogg_CreateFromDisk(char* szFilename);
/*OGGFILE* Ogg_CreateFromData(void* lpData, L_dword dwDataSize);*/
L_bool Ogg_Delete(OGGFILE* lpWave);
L_dword Ogg_Read(OGGFILE* lpWave, void* lpDest, L_dword dwSizeToRead);
L_bool Ogg_Reset(OGGFILE* lpWave);
WAVEFORMATEX* Ogg_GetFormat(OGGFILE* lpWave, WAVEFORMATEX* lpFormat);
L_dword Ogg_GetSize(OGGFILE* lpWave);
L_bool Ogg_IsEOF(OGGFILE* lpWave);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /*__LS_OGG_H__*/
