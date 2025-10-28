#ifndef __LS_STREAM_H__
#define __LS_STREAM_H__

#include <al/al.h>
#include "ls_sndfile.h"
#include "lt_sys.h"

//The sound stream class is primarily used for background
//music, but can also be used for long portions of audio such
//as dialogs, etc...

class CLSndStream
{
private:
	ALuint    m_Buffers[2];
	ALuint    m_Source;
	ALenum    m_nFormat;
	CLSndFile m_cSndFile;
	lg_void*  m_pTempBuffer; //Buffer to hold a few seconds worth of data
	lg_dword  m_nTempBufferSize;
	
	lg_bool   m_bOpen;
	
	lg_bool   m_bLoop;
	lg_bool   m_bPlaying;
	lg_bool   m_bPaused;
	
	__inline lg_bool CheckError();
	lg_bool UpdateBuffer(ALuint buffer);
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