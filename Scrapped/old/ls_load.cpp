#include <dsound.h>
#include "common.h"
#include "lg_err.h"
#include "ls_load.h"
#include "ls_sndfile.h"



extern "C" lg_bool LS_LoadSound(
	IDirectSound8* lpDS,
	IDirectSoundBuffer8** lppBuffer,
	lg_dword dwBufferFlags,
	char* szFilename)
{
CLSndFile cSndFile;
	IDirectSoundBuffer* lpTempBuffer=LG_NULL;
	DSBUFFERDESC desc;
	void* lpLockedBuffer=LG_NULL;
	lg_dword dwBufferBytes=0;
	lg_dword dwBytesRead=0;

	void* lpSnd=LG_NULL;
	lg_result nResult=0;

	if(!lpDS)
		return LG_FALSE;
	
	if(!cSndFile.Open(szFilename))
		return LG_FALSE;
	
	//lpSnd=pCB->Snd_CreateFromDisk(szFilename);
	//if(!lpSnd)
	//	return LG_FALSE;
		

	memset(&desc, 0, sizeof(desc));

	desc.dwSize=sizeof(DSBUFFERDESC);
	desc.dwFlags=dwBufferFlags;
	desc.dwBufferBytes=cSndFile.GetDataSize();
	WAVEFORMATEX wf;
	wf.cbSize=0;
	wf.wFormatTag=0x0001;
	wf.nChannels=(WORD)cSndFile.GetNumChannels();
	wf.nSamplesPerSec=cSndFile.GetSamplesPerSecond();
	wf.wBitsPerSample=(WORD)cSndFile.GetBitsPerSample();
	wf.nBlockAlign=wf.nChannels*wf.wBitsPerSample/8;
	wf.nAvgBytesPerSec=wf.nSamplesPerSec*wf.nBlockAlign;
	
	desc.lpwfxFormat=&wf;
	
	nResult=lpDS->CreateSoundBuffer(&desc, &lpTempBuffer, LG_NULL);
	
	if(LG_FAILED(nResult))
	{
		cSndFile.Close();
		Err_PrintDX("IDirectSound8::CreateSoundBuffer", nResult);
		return LG_FALSE;
	}
	lpTempBuffer->QueryInterface(
		IID_IDirectSoundBuffer8, 
		(void**)lppBuffer);

	IDirectSoundBuffer_Release(lpTempBuffer);

	nResult=IDirectSoundBuffer8_Lock(
		(*lppBuffer),
		0,
		0,
		&lpLockedBuffer,
		&dwBufferBytes,
		LG_NULL,
		LG_NULL,
		DSBLOCK_ENTIREBUFFER);

	if(LG_FAILED(nResult))
	{
		Err_PrintDX("IDirectSoundBuffer8::Lock", nResult);
		cSndFile.Close();
		IDirectSoundBuffer8_Release((*lppBuffer));
		return LG_FALSE;
	}

	dwBytesRead=cSndFile.Read((void*)lpLockedBuffer, dwBufferBytes);

	if(dwBytesRead<1)
	{
		cSndFile.Close();
		IDirectSoundBuffer8_Release((*lppBuffer));
		return LG_FALSE;
	}

	nResult=IDirectSoundBuffer8_Unlock((*lppBuffer), lpLockedBuffer, dwBufferBytes, LG_NULL, 0);
	cSndFile.Close();

	if(LG_FAILED(nResult))
	{
		Err_PrintDX("IDirectSoundBuffer8::Unlock", nResult);
		/* Not sure what to do if we can't unlock it. */
	}
	return LG_TRUE;
}
