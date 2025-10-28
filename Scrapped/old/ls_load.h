#ifndef __LS_LOAD_H__
#define __LS_LOAD_H__
#include <dsound.h>
#include "common.h"
#ifdef __cplusplus
extern "C"
#endif /*__cplusplus*/

lg_bool LS_LoadSound(
	IDirectSound8* lpDS,
	IDirectSoundBuffer8** lppBuffer,
	lg_dword dwBufferFlags,
	char* szFilename);

#endif /* __LS_LOAD_H__*/