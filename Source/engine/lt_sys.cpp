#include "lt_sys.h"
#include "lg_err_ex.h"
#include <windows.h>

//LT_GetTimeMS returns the number of milliseconds since the game was started.
lg_dword LT_GetTimeMS()
{
	static lg_bool bInit=LG_FALSE;
	static lg_dword dwBaseTime;
	if(!bInit)
	{
		dwBaseTime=timeGetTime();
		bInit=LG_TRUE;
	}
	return timeGetTime()-dwBaseTime;
}

lg_dword LT_GetHPTimeMS()
{
	static lg_bool bInit=LG_FALSE;
	static UINT64 nAdjFreq;
	static UINT64 nBaseTime;
	
	if(!bInit)
	{
		if(!QueryPerformanceFrequency((LARGE_INTEGER*)&nAdjFreq) || nAdjFreq==0)
			throw CLError(LG_ERR_DEFAULT, __FILE__, __LINE__, "Could not initialize high performance timer!");
			
		//Adjust the freqency to milliseconds (we'll lose precision here).
		nAdjFreq/=1000;
		QueryPerformanceCounter((LARGE_INTEGER*)&nBaseTime);
		bInit=LG_TRUE;
	}
	
	UINT64 nTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&nTime);
	
	return (lg_dword)((nTime-nBaseTime)/(nAdjFreq));
}


//CLTimer class methods...
CLTimer::CLTimer():
	m_dwInitialTime(0),
	m_dwTime(0),
	m_dwLastTime(0),
	m_dwElapsed(0),
	m_fTime(0.0f),
	m_fLastTime(0.0f),
	m_fElapsed(0.0f)
{

}

void CLTimer::Initialize()
{
	m_dwInitialTime=LT_GetTimeMS();
	m_dwLastTime=m_dwTime=0;//m_dwInitialTime-m_dwInitialTime/*(=0)*/;
	m_fTime=m_fLastTime=(lg_float)m_dwInitialTime;
}
void CLTimer::Update()
{
	m_dwLastTime=m_dwTime;
	m_dwTime=LT_GetTimeMS()-m_dwInitialTime;
	m_dwElapsed=m_dwTime-m_dwLastTime;
	m_fLastTime=m_dwLastTime*0.001f;
	m_fTime=m_dwTime*0.001f;
	m_fElapsed=m_dwElapsed*0.001f;
}
lg_dword CLTimer::GetTime()
{
	return m_dwTime;
}

////////////////////////////////////////////////
//	CLTimer::GetElapsedTime returns the amount
//	of milliseconds since the last call to Update
//	so in effect this will report how much time
//	has passed between two frames.
////////////////////////////////////////////////
lg_dword CLTimer::GetElapsed()
{
	return m_dwElapsed;
}

//Note that the GetF methods return a floating
//point time that represents a decimal number of
//the seconds as opposed to milliseconds for the
//Get methods.

lg_float CLTimer::GetFTime()
{
	return m_fTime;
}

lg_float CLTimer::GetFElapsed()
{
	return m_fElapsed;
}

