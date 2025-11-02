#include "ls_sys.h"
#include "lg_err.h"
#include "lg_cvars.h"
#include "lg_malloc.h"

#include "ls_sndfile.h"

#include "../lc_sys2/lc_sys2.h"

CLSndMgr::CLSndMgr()
#if L3D_WITH_OPENAL_AUDIO
	: m_pAudioContext(LG_NULL)
	, m_pAudioDevice(LG_NULL)
#endif
{

}
CLSndMgr::~CLSndMgr()
{
	if(m_bSndAvailable)
		LS_Shutdown();
}

void CLSndMgr::Music_Start(lg_str szPath)
{
	if(!m_MusicTrack.Load(szPath))
	{
		Err_Printf("Music_Start ERROR: Could not load music track \"%s\".", szPath);
		return;
	}
	Music_UpdateVolume();//m_MusicTrack.SetVolume(CV_Get(CVAR_s_MusicVolume)->nValue);
	m_MusicTrack.Play(LG_TRUE);
}

void CLSndMgr::Music_UpdateVolume()
{
	m_MusicTrack.SetVolume(CV_Get(CVAR_s_MusicVolume)->nValue);
}

void CLSndMgr::Music_Pause()
{
	m_MusicTrack.Pause();
}

void CLSndMgr::Music_Stop()
{
	m_MusicTrack.Stop();
}

void CLSndMgr::Music_Resume()
{
	m_MusicTrack.Play(LG_TRUE);
}

void CLSndMgr::Update()
{
	//Update streaming audio.
	//Process audio...
	//m_TestSnd2.Update();
	m_MusicTrack.Update();
#if L3D_WITH_OPENAL_AUDIO
	alcProcessContext(m_pAudioContext);
	//Suspend the context till the next frame...
	alcSuspendContext(m_pAudioContext);
#endif
}

lg_bool CLSndMgr::LS_Init()
{
#if L3D_WITH_OPENAL_AUDIO
	ALCenum nALCError;
	const ALCchar* szDevice=alcGetString(LG_NULL, ALC_DEFAULT_DEVICE_SPECIFIER);// szDevice[]="DirectSound3D";
	
	m_pAudioDevice=alcOpenDevice((const ALCchar*)szDevice);
	nALCError=alcGetError(LG_NULL);
	if(!m_pAudioDevice)
	{
		Err_Printf("Sound Init ERROR %u: Could not create audio device.", nALCError);
		m_bSndAvailable=LG_FALSE;
		return LG_FALSE;
	}
	Err_Printf("Created \"%s\" audio device at 0x%08X", alcGetString(m_pAudioDevice, ALC_DEVICE_SPECIFIER), m_pAudioDevice);
	
	m_pAudioContext=alcCreateContext(m_pAudioDevice, LG_NULL);
	nALCError=alcGetError(m_pAudioDevice);
	if(!m_pAudioContext)
	{
		Err_Printf("Sound Init ERROR %u: Could not create audio context.", nALCError);
		alcCloseDevice(m_pAudioDevice);
		m_bSndAvailable=LG_FALSE;
		return LG_FALSE;
	}
	
	Err_Printf("Created audio context at 0x%08X", m_pAudioContext);
	nALCError=alcGetError(m_pAudioDevice);
	if(!alcMakeContextCurrent(m_pAudioContext))
	{
		Err_Printf("Sount Init ERROR %u: Could not make sound context current.");
		alcDestroyContext(m_pAudioContext);
		alcCloseDevice(m_pAudioDevice);
		m_bSndAvailable=LG_FALSE;
		return LG_FALSE;
	}
	Err_Printf("Audio system extensions:");
	if(alcIsExtensionPresent(m_pAudioDevice, "ALC_EXT_EFX"))
		Err_Printf("ACL_EXT_EFX");
	if(alcIsExtensionPresent(m_pAudioDevice, "ALC_EXT_CAPTURE"))
		Err_Printf("ALC_EXT_CAPTURE");
		
	//const ALCchar* szExt=alcGetString(m_pAudioDevice, ALC_EXTENSIONS);
	//Err_Printf("%s", szExt);
	
	alGetError();
	m_bSndAvailable=LG_TRUE;
	
	return LG_TRUE;
#else
	m_bSndAvailable = LG_TRUE;
	return LG_TRUE;
#endif
}

void CLSndMgr::LS_Shutdown()
{
#if L3D_WITH_OPENAL_AUDIO
	//Close the music track...
	m_MusicTrack.Close();
	
	ALCenum nALCError;
	
	/* Make sure the audio was initialized, because if it hasn't
		been there is no point in shutting it donw. */
	if(!m_bSndAvailable)
	{
		Err_Printf("Sound Shutdown ERROR: Audio was not initialized, nothing to shut down.");
		return;
	}
	Err_Printf("Destroying audio device at 0x%08X...", m_pAudioContext);
	alcMakeContextCurrent(LG_NULL);
	alcDestroyContext(m_pAudioContext);
	Err_Printf("Closing device at 0x%08X...", m_pAudioDevice);
	alcCloseDevice(m_pAudioDevice);
	nALCError=alcGetError(LG_NULL);
	if(nALCError!=ALC_NO_ERROR)
	{
		Err_Printf("Sound Shutdown ERROR %u: Could not close device properly.", nALCError);
	}
	m_bSndAvailable=LG_FALSE;
	return;
#else
	m_bSndAvailable = LG_FALSE;
#endif
}

#if L3D_WITH_OPENAL_AUDIO
lg_bool CLSndMgr::LS_LoadSoundIntoBuffer(ALuint sndBuffer, lg_str szFilename)
{
	CLSndFile sndFile;
	if(!sndFile.Open(szFilename))
		return LG_FALSE;	
	ALvoid* pData=LG_Malloc(sndFile.GetDataSize());
	sndFile.Read(pData, sndFile.GetDataSize());
	ALenum nFormat=GetALFormat(sndFile.GetNumChannels(), sndFile.GetBitsPerSample());
	alBufferData(sndBuffer, nFormat, pData, sndFile.GetDataSize(), sndFile.GetSamplesPerSecond());
	LG_Free(pData);
	alGetError();
	return LG_TRUE;
}
#endif

#if L3D_WITH_OPENAL_AUDIO
ALenum CLSndMgr::GetALFormat(lg_dword nChannels, lg_dword nBPS)
{
	//Only the following sound file formats are supported...
	if(nChannels==1 && nBPS==8)
		return AL_FORMAT_MONO8;
	else if(nChannels==1 && nBPS==16)
		return AL_FORMAT_MONO16;
	else if(nChannels==2 && nBPS==8)
		return AL_FORMAT_STEREO8;
	else if(nChannels==2 && nBPS==16)
		return AL_FORMAT_STEREO16;
	else
		return -1;
}
#endif
