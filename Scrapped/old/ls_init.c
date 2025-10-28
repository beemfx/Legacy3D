/* ls_init.c - Legacy engine audio initialization. */
#include "common.h"
#include <dsound.h>
#include "ls_init.h"
#include "ls_load.h"

L_bool LS_SetFormatFromCVars(IDirectSoundBuffer* lpBuffer, HCVARLIST cvarlist)
{
	L_result nResult=0;
	WAVEFORMATEX wf;

	memset(&wf, 0, sizeof(WAVEFORMATEX));
	wf.cbSize=0;
	wf.wFormatTag=WAVE_FORMAT_PCM;
	wf.nChannels=(L_word)CVar_GetValue(cvarlist, "s_Channels", L_null);
	wf.wBitsPerSample=(L_word)CVar_GetValue(cvarlist, "s_BitsPerSample", L_null);
	wf.nSamplesPerSec=(L_dword)CVar_GetValue(cvarlist, "s_Frequency", L_null);
	wf.nBlockAlign=wf.nChannels*(wf.wBitsPerSample/8);
	wf.nAvgBytesPerSec=wf.nSamplesPerSec*wf.nBlockAlign;

	Err_Printf(
		"Setting output format to: %i channels, %i bits per sample at %iHz...", 
		wf.nChannels, wf.wBitsPerSample, wf.nSamplesPerSec);
	nResult=lpBuffer->lpVtbl->SetFormat(lpBuffer, &wf);
	Err_PrintDX("IDirectSoundBuffer::SetFormat", nResult);

	if(L_succeeded(nResult))
		return L_true;
	else
		return L_false;
}

L_result LS_Init(L3DGame* lpGame)
{
	WAVEFORMATEX wf;
	DSBUFFERDESC dsdesc;
	L_result nResult=0;

	if(!lpGame)
		return LG_ERR;

	memset(&lpGame->s, 0, sizeof(lpGame->s));

	/* Create the IDirectSound8 inteface. */
	Err_Printf("Creating IDirectSound8 interface...");
	nResult=DirectSoundCreate8(&DSDEVID_DefaultPlayback, &lpGame->s.m_lpDS, L_null);
	Err_PrintDX("DirectSoundCreate8", nResult);
	if(L_failed(nResult))
	{
		Err_Printf("Failed to create IDirectSound8 interface, audio not available.");
		lpGame->s.m_lpDS=L_null;
		lpGame->s.m_bAvailable=L_false;
		return LG_ERR;
	}
	else
		Err_Printf("IDirectSound8 interface created at 0x%08X.", lpGame->s.m_lpDS);

	/* Set the cooperative level, for a game we want priority. */
	Err_Printf("Setting audio cooperative level to priority...");
	nResult=lpGame->s.m_lpDS->lpVtbl->SetCooperativeLevel(
		lpGame->s.m_lpDS, 
		lpGame->m_hwnd, 
		DSSCL_PRIORITY);
	Err_PrintDX("IDirectSound8::SetCooperativeLevel", nResult);
	if(L_failed(nResult))
	{
		Err_Printf("Could not set cooperative level, audio not available.");
		L_safe_release(lpGame->s.m_lpDS);
		lpGame->s.m_bAvailable=L_false;
		return LG_ERR;
	}

	/* Get the primary sound buffer. */
	memset(&dsdesc, 0, sizeof(dsdesc));
	Err_Printf("Creating primary sound buffer...");
	dsdesc.dwSize=sizeof(dsdesc);
	dsdesc.dwFlags=DSBCAPS_PRIMARYBUFFER|DSBCAPS_CTRL3D;
	nResult=lpGame->s.m_lpDS->lpVtbl->CreateSoundBuffer(
		lpGame->s.m_lpDS,
		&dsdesc,
		&lpGame->s.m_lpPrimaryBuffer,
		L_null);
	Err_PrintDX("IDirectSound8::CreateSoundBuffer", nResult);
	if(L_failed(nResult))
	{
		Err_Printf("Failed to create primary sound buffer, audio not available.");
		L_safe_release(lpGame->s.m_lpDS);
		lpGame->s.m_bAvailable=L_false;
		return LG_ERR;
	}
	Err_Printf("Created primary sound buffer at 0x%08X.", lpGame->s.m_lpPrimaryBuffer);
	
	/* Set the format of the primary buffer, this really only matters
		for older hardware.  We set the output format from the cvars.*/
	Err_Printf("Setting primary sound buffer format...");
	
	memset(&wf, 0, sizeof(wf));
	/*
	wf.cbSize=0;
	wf.wFormatTag=WAVE_FORMAT_PCM;
	wf.nChannels=(L_word)CVar_GetValue(lpGame->m_cvars, "s_Channels", L_null);
	wf.wBitsPerSample=(L_word)CVar_GetValue(lpGame->m_cvars, "s_BitsPerSample", L_null);
	wf.nSamplesPerSec=(L_dword)CVar_GetValue(lpGame->m_cvars, "s_Frequency", L_null);
	wf.nBlockAlign=wf.nChannels*(wf.wBitsPerSample/8);
	wf.nAvgBytesPerSec=wf.nSamplesPerSec*wf.nBlockAlign;
	*/
	if(!LS_SetFormatFromCVars(lpGame->s.m_lpPrimaryBuffer, lpGame->m_cvars))
	{
		CVar_Set(lpGame->m_cvars, "s_Channels", "2");
		if(!LS_SetFormatFromCVars(lpGame->s.m_lpPrimaryBuffer, lpGame->m_cvars))
		{
			CVar_Set(lpGame->m_cvars, "s_Channels", "1");
			if(!LS_SetFormatFromCVars(lpGame->s.m_lpPrimaryBuffer, lpGame->m_cvars))
			{
				Err_Printf("Could not initialize audio format.");
				L_safe_release(lpGame->s.m_lpPrimaryBuffer);
				L_safe_release(lpGame->s.m_lpDS);
				lpGame->s.m_bAvailable=L_false;
				return LG_ERR;
			}
		}
	}
	

	/* Play the primary buffer, most drivers will play the primary buffer
		anyway, but just to be safe we call the Play method anyway. */
	Err_Printf("Playing the primary sound buffer for sound output.");
	nResult=lpGame->s.m_lpPrimaryBuffer->lpVtbl->Play(lpGame->s.m_lpPrimaryBuffer, 0, 0, DSBPLAY_LOOPING);
	Err_PrintDX("DirectSoundBuffer::Play", nResult);
	/* Tell the game that sound is enalbled.*/
	lpGame->s.m_bAvailable=L_true;

	/* Some test stuff. */
	LS_LoadSound(
		lpGame->s.m_lpDS,
		&lpGame->s.m_lpTestSound,
		0,
		"music\\m2theme.ogg");
		//"test\\jungle_loop_02.ogg");
		//"music\\largewave01.wav");

	if(lpGame->s.m_lpTestSound)
		lpGame->s.m_lpTestSound->lpVtbl->Play(lpGame->s.m_lpTestSound, 0, 0, DSBPLAY_LOOPING);
	
	return LG_OK;
}

L_result LS_Shutdown(L3DGame* lpGame)
{
	L_long nCount=0;
	/* Make sure the audio was initialized, because if it hasn't
		been there is no point in shutting it donw. */
	if(!lpGame->s.m_bAvailable)
	{
		Err_Printf("Audio was not initialized, nothing to shut down.");
		return LG_OK;
	}
	L_safe_release(lpGame->s.m_lpTestSound);
	/* Release Primary sound buffer. */
	Err_Printf("Releasing primary sound buffer...");
	if(lpGame->s.m_lpPrimaryBuffer)
		nCount=lpGame->s.m_lpPrimaryBuffer->lpVtbl->Release(lpGame->s.m_lpPrimaryBuffer);
	Err_Printf(
		"Released IDirectSoundBuffer interface at 0x%08X with %i references left.",
		lpGame->s.m_lpPrimaryBuffer,
		nCount);
	lpGame->s.m_lpPrimaryBuffer=L_null;
	/* Release DirectSound8 interface. */
	Err_Printf("Releasing IDirectSound8 interface...");
	if(lpGame->s.m_lpDS)
		nCount=lpGame->s.m_lpDS->lpVtbl->Release(lpGame->s.m_lpDS);
	Err_Printf(
		"Released IDirectSound8 interface at 0x%08X with %i references left.", 
		lpGame->s.m_lpDS, 
		nCount);
	lpGame->s.m_lpDS=L_null;

	return LG_OK;
}