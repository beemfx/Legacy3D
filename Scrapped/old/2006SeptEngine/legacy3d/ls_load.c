#include "common.h"
#include <dsound.h>
#include "lg_err.h"
#include "lg_sys.h"
#include "ls_wav.h"
#include "ls_ogg.h"
#include "ls_load.h"

L_result LS_FillBuffer(
	SNDCALLBACKS* pCB,
	IDirectSound8* lpDS,
	IDirectSoundBuffer8** lppBuffer,
	L_dword dwBufferFlags,
	char* szFilename)
{
	IDirectSoundBuffer* lpTempBuffer=L_null;
	DSBUFFERDESC desc;
	void* lpLockedBuffer=L_null;
	L_dword dwBufferBytes=0;
	L_dword dwBytesRead=0;

	void* lpSnd=L_null;
	L_result nResult=0;

	if(!pCB || !lpDS)
		return LG_FAIL;

	
	lpSnd=pCB->Snd_CreateFromDisk(szFilename);
	if(!lpSnd)
		return LG_FAIL;

	memset(&desc, 0, sizeof(desc));

	desc.dwSize=sizeof(DSBUFFERDESC);
	desc.dwFlags=dwBufferFlags;
	desc.dwBufferBytes=pCB->Snd_GetSize(lpSnd);
	desc.lpwfxFormat=pCB->Snd_GetFormat(lpSnd, L_null);
	
	nResult=lpDS->lpVtbl->CreateSoundBuffer(lpDS, &desc, &lpTempBuffer, L_null);
	if(L_failed(nResult))
	{
		pCB->Snd_Delete(lpSnd);
		Err_PrintDX("IDirectSound8::CreateSoundBuffer", nResult);
		return LG_FAIL;
	}

	lpTempBuffer->lpVtbl->QueryInterface(
		lpTempBuffer, 
		&IID_IDirectSoundBuffer8, 
		(void**)lppBuffer);

	IDirectSoundBuffer_Release(lpTempBuffer);

	nResult=IDirectSoundBuffer8_Lock(
		(*lppBuffer),
		0,
		0,
		&lpLockedBuffer,
		&dwBufferBytes,
		L_null,
		L_null,
		DSBLOCK_ENTIREBUFFER);

	if(L_failed(nResult))
	{
		Err_PrintDX("IDirectSoundBuffer8::Lock", nResult);
		pCB->Snd_Delete(lpSnd);
		IDirectSoundBuffer8_Release((*lppBuffer));
		return LG_FAIL;
	}

	dwBytesRead=pCB->Snd_Read(lpSnd, (void*)lpLockedBuffer, dwBufferBytes);

	if(dwBytesRead<1)
	{
		pCB->Snd_Delete(lpSnd);
		IDirectSoundBuffer8_Release((*lppBuffer));
		return LG_FAIL;
	}

	nResult=IDirectSoundBuffer8_Unlock((*lppBuffer), lpLockedBuffer, dwBufferBytes, L_null, 0);
	pCB->Snd_Delete(lpSnd);

	if(L_failed(nResult))
	{
		Err_PrintDX("IDirectSoundBuffer8::Unlock", nResult);
		/* Not sure what to do if we can't unlock it. */
	}
	return LG_OK;
}

L_result LS_LoadSound(
	IDirectSound8* lpDS,
	IDirectSoundBuffer8** lppBuffer,
	L_dword dwBufferFlags,
	char* szFilename)
{
	L_dword dwLen=0;
	L_dword i=0;
	char *szExt=szFilename;

	dwLen=L_strlen(szFilename);

	for(i=dwLen; i>0; i--)
	{
		if(szFilename[i]=='.')
		{
			szExt=&szFilename[i+1];
			break;
		}
	}

	if(L_strnicmp(szExt, "ogg", 0))
		return LS_FillBuffer(
			LS_GetCallbacks("ogg"), 
			lpDS, 
			lppBuffer, 
			dwBufferFlags, 
			szFilename);
	else /* We just try as a wave if, it isn't a previous type. */
		return LS_FillBuffer(
			LS_GetCallbacks("wav"), 
			lpDS, 
			lppBuffer, 
			dwBufferFlags, 
			szFilename);

	return LG_OK;
}

SNDCALLBACKS* LS_GetCallbacks(char* szType)
{
	static SNDCALLBACKS cbOgg={
		Ogg_CreateFromDisk,
		/*Ogg_CreateFromData,*/
		Ogg_Delete,
		Ogg_Read,
		Ogg_Reset,
		Ogg_GetFormat,
		Ogg_GetSize,
		Ogg_IsEOF};

	static SNDCALLBACKS cbWav={
		Wav_CreateFromDisk,
		/*Wav_CreateFromData,*/
		Wav_Delete,
		Wav_Read,
		Wav_Reset,
		Wav_GetFormat,
		Wav_GetSize,
		Wav_IsEOF};

	if(L_strnicmp(szType, "ogg", 0))
		return &cbOgg;
	else if(L_strnicmp(szType, "wav", 0))
		return &cbWav;
	
	return L_null;
}
