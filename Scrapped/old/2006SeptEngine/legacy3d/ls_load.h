#ifndef __LS_LOAD_H__
#define __LS_LOAD_H__
#include <dsound.h>
#include "common.h"
#ifdef __cplusplus
extern "C"{
#endif /*__cplusplus*/

typedef struct _SNDCALLBACKS{
	void* (*Snd_CreateFromDisk)(char*);
	/*void* (*Snd_CreateFromData)(void*, L_dword);*/
	L_bool (*Snd_Delete)(void*);
	L_dword (*Snd_Read)(void*, void*, L_dword);
	L_bool (*Snd_Reset)(void*);
	WAVEFORMATEX* (*Snd_GetFormat)(void*, WAVEFORMATEX*);
	L_dword (*Snd_GetSize)(void*);
	L_bool (*Snd_IsEOF)(void*);
}SNDCALLBACKS, *PSNDCALLBACKS;

SNDCALLBACKS* LS_GetCallbacks(char* szType);

L_result LS_LoadSound(
	IDirectSound8* lpDS,
	IDirectSoundBuffer8** lppBuffer,
	L_dword dwBufferFlags,
	char* szFilename);

L_result LS_FillBuffer(
	SNDCALLBACKS* pCB,
	IDirectSound8* lpDS,
	IDirectSoundBuffer8** lppBuffer,
	L_dword dwBufferFlags,
	char* szFilename);


#ifdef __cplusplus
extern "C"{
#endif /*__cplusplus*/
#endif /* __LS_LOAD_H__*/