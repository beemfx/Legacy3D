#include "ls_sndfile.h"
#include "ls_ogg.h"
#include "ls_wav.h"
#include "lg_err.h"

CLSndFile::CLSndFile():
	//m_CB(LG_NULL),
	m_pSnd(LG_NULL),
	m_nChannels(0),
	m_nBitsPerSample(0),
	m_nSamplesPerSecond(0),
	m_nDataSize(0),
	m_bOpen(LG_FALSE)
{
}

CLSndFile::~CLSndFile()
{
	Close();
}

lg_bool CLSndFile::Open(lg_str szFilename)
{
	Close();
	SetupCallbacks(szFilename);
	m_pSnd=m_CB.Snd_CreateFromDisk(szFilename);
	if(!m_pSnd)
	{
		Err_Printf("Sound File Open ERROR: Could not load not create \"%s\"", szFilename);
		return LG_FALSE;
	}
	WAVEFORMATEX* pFormat=m_CB.Snd_GetFormat(m_pSnd, LG_NULL);
	m_nChannels=pFormat->nChannels;
	m_nBitsPerSample=pFormat->wBitsPerSample;
	m_nSamplesPerSecond=pFormat->nSamplesPerSec;
	m_nDataSize=m_CB.Snd_GetSize(m_pSnd);
	
	m_bOpen=LG_TRUE;
	return LG_TRUE;
}

void CLSndFile::Close()
{
	if(!m_bOpen)
		return;
		
	m_CB.Snd_Delete(m_pSnd);
	m_pSnd=LG_NULL;
	m_nChannels=0;
	m_nBitsPerSample=0;
	m_nSamplesPerSecond=0;
	m_nDataSize=0;
	m_bOpen=LG_FALSE;
	return;
}

lg_dword CLSndFile::Read(lg_void* pBuffer, lg_dword nSize)
{
	if(!m_bOpen)
		return 0;
		
	return m_CB.Snd_Read(m_pSnd, pBuffer, nSize);
}

lg_dword CLSndFile::GetNumChannels()
{
	return m_nChannels;
}

lg_dword CLSndFile::GetDataSize()
{
	return m_nDataSize;
}

lg_dword CLSndFile::GetBitsPerSample()
{
	return m_nBitsPerSample;
}

lg_dword CLSndFile::GetSamplesPerSecond()
{
	return m_nSamplesPerSecond;
}

lg_bool CLSndFile::IsEOF()
{
	if(!m_bOpen)
		return LG_TRUE;
		
	return m_CB.Snd_IsEOF(m_pSnd);
}

lg_bool CLSndFile::Reset()
{
	if(!m_bOpen)
		return LG_FALSE;
		
	return m_CB.Snd_Reset(m_pSnd);
}

void CLSndFile::SetupCallbacks(lg_str szFilename)
{
	lg_str szExt=szFilename;
	
	for(lg_dword i=strlen(szFilename); i>0; i--)
	{
		if(szFilename[i]=='.')
		{
			szExt=&szFilename[i+1];
			break;
		}
	}
	
	if(stricmp(szExt, "ogg")==0)
	{
		m_CB.Snd_CreateFromDisk=Ogg_CreateFromDisk;
		m_CB.Snd_Delete=Ogg_Delete;
		m_CB.Snd_GetFormat=Ogg_GetFormat;
		m_CB.Snd_GetSize=Ogg_GetSize;
		m_CB.Snd_IsEOF=Ogg_IsEOF;
		m_CB.Snd_Read=Ogg_Read;
		m_CB.Snd_Reset=Ogg_Reset;
	}
	else
	{
		m_CB.Snd_CreateFromDisk=Wav_CreateFromDisk;
		m_CB.Snd_Delete=Wav_Delete;
		m_CB.Snd_GetFormat=Wav_GetFormat;
		m_CB.Snd_GetSize=Wav_GetSize;
		m_CB.Snd_IsEOF=Wav_IsEOF;
		m_CB.Snd_Read=Wav_Read;
		m_CB.Snd_Reset=Wav_Reset;
	}
}
