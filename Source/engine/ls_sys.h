#ifndef __LS_SYS_H__
#define __LS_SYS_H__

#include <al/al.h>
#include <al/alc.h>

#include "lg_types.h"
#include "ls_stream.h"


class CLSndMgr
{
private:
	lg_bool              m_bSndAvailable;
	
	//OpenAL Interfaces
	ALCcontext* m_pAudioContext;
	ALCdevice* m_pAudioDevice;
	//The test sound.
	
	//ALuint m_TestSnd;
	//ALuint m_TestSndBuffer;
	//CLSndStream m_TestSnd2;
	
	//The background music track
	CLSndStream m_MusicTrack;
public:	
	static ALenum GetALFormat(lg_dword nChannels, lg_dword nBitsPerSample);
public:
	CLSndMgr();
	~CLSndMgr();
	
	//Sound initialziation stuff:
	lg_bool LS_Init();
	void LS_Shutdown();
	
	void Update();
	
	void Music_Start(lg_str szPath);
	void Music_Pause();
	void Music_Stop();
	void Music_Resume();
	void Music_UpdateVolume();
	
	static lg_bool LS_LoadSoundIntoBuffer(ALuint sndBuffer, lg_str szFilename);
};

#endif __LS_SYS_H__