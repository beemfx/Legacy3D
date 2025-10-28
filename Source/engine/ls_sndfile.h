#ifndef __LS_SNDFILE_H__
#define __LS_SNDFILE_H__

#include <windows.h>
#include "common.h"

class CLSndFile
{
private:
	struct SNDCALLBACKS{
		void* (*Snd_CreateFromDisk)(char*);
		/*void* (*Snd_CreateFromData)(void*, lg_dword);*/
		lg_bool (*Snd_Delete)(void*);
		lg_dword (*Snd_Read)(void*, void*, lg_dword);
		lg_bool (*Snd_Reset)(void*);
		WAVEFORMATEX* (*Snd_GetFormat)(void*, WAVEFORMATEX*);
		lg_dword (*Snd_GetSize)(void*);
		lg_bool (*Snd_IsEOF)(void*);
	};

	SNDCALLBACKS m_CB;
	lg_void* m_pSnd;
	lg_dword m_nChannels;
	lg_dword m_nBitsPerSample;
	lg_dword m_nSamplesPerSecond;
	lg_dword m_nDataSize;
	
	lg_bool m_bOpen;
	
	void SetupCallbacks(lg_str szType);
public:
	CLSndFile();
	~CLSndFile();
	lg_bool Open(lg_str szFilename);
	void Close();
	lg_dword Read(lg_void* pBuffer, lg_dword nSize);
	lg_bool Reset();
	lg_dword GetNumChannels();
	lg_dword GetBitsPerSample();
	lg_dword GetSamplesPerSecond();
	lg_dword GetDataSize();
	lg_bool  IsEOF();
};

#endif __LS_SNDFILE_H__