#ifndef __LS_STREAM_H__
#define __LS_STREAM_H__

#if L3D_WITH_OPENAL_AUDIO
#include <al/al.h>
#endif
#include "ls_sndfile.h"
#include "lt_sys.h"

//The sound stream class is primarily used for background
//music, but can also be used for long portions of audio such
//as dialogs, etc...

class CLSndStream
{
private:
#if L3D_WITH_OPENAL_AUDIO
	ALuint    m_Buffers[2];
	ALuint    m_Source;
	ALenum    m_nFormat;
#endif
	CLSndFile m_cSndFile;
	lg_void*  m_pTempBuffer; //Buffer to hold a few seconds worth of data
	lg_dword  m_nTempBufferSize;
	
	lg_bool   m_bOpen;
	
	lg_bool   m_bLoop;
	lg_bool   m_bPlaying;
	lg_bool   m_bPaused;
	
	__inline lg_bool CheckError();
#if L3D_WITH_OPENAL_AUDIO
	lg_bool UpdateBuffer(ALuint buffer);
#endif
public:
	CLSndStream();
	~CLSndStream();
	lg_bool Load(lg_str szPath);
	void Close();
	void Play(lg_bool bLoop=LG_TRUE);
	void Stop();
	void Pause();
	void Update();	
	void SetVolume(lg_int nVolume);
};

#endif __LS_STREAM_H__