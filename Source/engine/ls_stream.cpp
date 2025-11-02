#include "ls_stream.h"
#include "lg_err.h"
#include "ls_sys.h"
#include "lg_malloc.h"


/*	lg_dword  CLSndStream::m_nLastUpdate;
	ALuint    CLSndStream::m_Buffers[2];
	ALuint    CLSndStream::m_Source;
	ALenum    CLSndStream::m_nFormat;
	CLSndFile CLSndStream::m_cSndFile;*/
	
lg_bool CLSndStream::CheckError()
{
#if L3D_WITH_OPENAL_AUDIO
	ALenum nError=alGetError();
	if(nError==AL_NO_ERROR)
		return LG_FALSE;
	
	Err_Printf("Sound Stream ERROR %u: %s", nError, "ERROR MESSAGE GOES HERE");
	return LG_TRUE;
#else
	return LG_TRUE;
#endif
}
	
CLSndStream::CLSndStream():
	m_bOpen(LG_FALSE),
	m_pTempBuffer(LG_NULL),
	m_bLoop(LG_FALSE),
	m_bPlaying(LG_FALSE),
	m_bPaused(LG_TRUE)
{

}
CLSndStream::~CLSndStream()
{
	Close();
}

void CLSndStream::SetVolume(lg_int nVolume)
{
	if(!m_bOpen)
		return;
	nVolume=LG_Clamp(nVolume, 0, 100);	
#if L3D_WITH_OPENAL_AUDIO
	alSourcef(m_Source, AL_GAIN, nVolume/100.0f);
#endif
}
lg_bool CLSndStream::Load(lg_str szPath)
{
#if L3D_WITH_OPENAL_AUDIO
	Close();
	Err_Printf("Loading \"%s\" as a streaming sound...", szPath);
	if(!m_cSndFile.Open(szPath))
	{
		Err_Printf("   ERROR: Could not open sound file!", szPath);
		return LG_FALSE;
	}
	
	m_nFormat=CLSndMgr::GetALFormat(m_cSndFile.GetNumChannels(), m_cSndFile.GetBitsPerSample());
	if(m_nFormat==-1)
	{
		Err_Printf("   ERROR: Sound is not a supported audio format.");
		m_cSndFile.Close();
		return LG_FALSE;
	}
	
	//Setup the temp buffer...
	m_nTempBufferSize=m_cSndFile.GetBitsPerSample()*m_cSndFile.GetSamplesPerSecond()/8;
	m_pTempBuffer=LG_Malloc(m_nTempBufferSize);
	if(!m_pTempBuffer)
	{
		Err_Printf("   ERROR: Could not allocate memory for temp buffer.");
		m_cSndFile.Close();
		return LG_FALSE;
	}
	
	CheckError();
	alGenBuffers(2, m_Buffers);
	if(CheckError())
	{
		Err_Printf("   ERROR: Could not create buffers.");
		m_cSndFile.Close();
		LG_Free(m_pTempBuffer);
		return LG_FALSE;
	}
	alGenSources(1, &m_Source);
	if(CheckError())
	{
		Err_Printf("   ERROR: Could not create source.");
		alDeleteBuffers(2, m_Buffers);
		m_cSndFile.Close();
		LG_Free(m_pTempBuffer);
		return LG_FALSE;
	}
	
	/* Set source properties for background music, may want this changeable, in
		case an object is giving a dialog or something. */
	alSource3f(m_Source, AL_POSITION, 0.0f, 0.0f, 0.0f);
   alSource3f(m_Source, AL_VELOCITY, 0.0f, 0.0f, 0.0f);
   alSource3f(m_Source, AL_DIRECTION, 0.0f, 0.0f, 0.0f);
   alSourcef(m_Source, AL_ROLLOFF_FACTOR, 0.0f);
   alSourcei(m_Source, AL_SOURCE_RELATIVE, AL_TRUE);
    
	m_bOpen=LG_TRUE;
	m_bPlaying=LG_FALSE;
	m_bLoop=LG_FALSE;
	SetVolume(100);
	return LG_TRUE;
#else
	return LG_TRUE;
#endif
}
void CLSndStream::Close()
{
#if L3D_WITH_OPENAL_AUDIO
	if(!m_bOpen)
		return;
	LG_Free(m_pTempBuffer);
	m_pTempBuffer=LG_NULL;
	CheckError();
	alDeleteSources(1, &m_Source);
	CheckError();
	alDeleteBuffers(2, m_Buffers);	
	CheckError();
	m_cSndFile.Close();
	m_bOpen=LG_FALSE;
	m_bPlaying=LG_FALSE;
	m_bPaused=LG_FALSE;
	m_bLoop=LG_FALSE;
#endif
}

#if L3D_WITH_OPENAL_AUDIO
lg_bool CLSndStream::UpdateBuffer(ALuint buffer)
{
	if(!m_bPlaying)
		return LG_FALSE;
		
	lg_dword nRead=0;
	while(nRead<m_nTempBufferSize)
	{
		nRead+=m_cSndFile.Read((lg_byte*)m_pTempBuffer+nRead, m_nTempBufferSize-nRead);
		if(m_cSndFile.IsEOF())
		{
			if(m_bLoop)
			{
				m_cSndFile.Reset();
				continue;
			}
				
			break;
		}
	}
	alBufferData(buffer, m_nFormat, m_pTempBuffer, nRead, m_cSndFile.GetSamplesPerSecond());
	CheckError();
	return nRead==m_nTempBufferSize?LG_TRUE:LG_FALSE;
}
#endif

void CLSndStream::Play(lg_bool bLoop)
{
#if L3D_WITH_OPENAL_AUDIO
	if(!m_bOpen)
		return;
		
	m_bLoop=bLoop;
	if(m_bPlaying)
	{
		if(m_bPaused)
		{
			alSourcePlay(m_Source);
			m_bPaused=LG_FALSE;
		}
		return;
	}
	m_bPlaying=LG_TRUE;
	m_bPaused=LG_FALSE;
	if(!UpdateBuffer(m_Buffers[0]) || !UpdateBuffer(m_Buffers[1]))
	{
		m_bPlaying=LG_FALSE;
		return;
	}
	alSourceQueueBuffers(m_Source, 2, m_Buffers);
	alSourcePlay(m_Source);
#endif
}
void CLSndStream::Stop()
{
#if L3D_WITH_OPENAL_AUDIO
	if(!m_bOpen)
		return;
	alSourceStop(m_Source);
	alSourceUnqueueBuffers(m_Source, 2, m_Buffers);
	m_cSndFile.Reset();
	m_bPlaying=LG_FALSE;
	m_bPaused=LG_FALSE;
#endif
}
void CLSndStream::Pause()
{
#if L3D_WITH_OPENAL_AUDIO
	if(!m_bOpen)
		return;
	alSourcePause(m_Source);
	m_bPaused=LG_TRUE;
#endif
}
void CLSndStream::Update()
{
#if L3D_WITH_OPENAL_AUDIO
	if(!m_bPlaying || m_bPaused || !m_bOpen)
		return;
		
	ALint nProcessed;
	 
	alGetSourcei(m_Source, AL_BUFFERS_PROCESSED, &nProcessed);
	while(nProcessed--)
	{
		ALuint buffer;
		        
		alSourceUnqueueBuffers(m_Source, 1, &buffer);
		CheckError();
		 
		lg_bool bResult=UpdateBuffer(buffer); 
		alSourceQueueBuffers(m_Source, 1, &buffer);
		CheckError();
		if(!bResult)
		{
			m_bPlaying=LG_FALSE;
			break;
		}
	}
	ALint nState;
	alGetSourcei(m_Source, AL_SOURCE_STATE, &nState);
	if(nState!=AL_PLAYING)
		alSourcePlay(m_Source);
#endif
}	
