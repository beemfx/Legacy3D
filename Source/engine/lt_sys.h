//lt_sys.h - The Legacy Timer
//Copyright (c) 2007 Blaine Myers
#ifndef __LT_SYS_H__
#define __LT_SYS_H__

#include "lg_types.h"

lg_dword LT_GetTimeMS(); //Return the time since the game was started in milliseconds.

class CLTimer
{
private:
	lg_dword m_dwInitialTime;
	lg_dword m_dwTime;
	lg_dword m_dwLastTime;
	lg_dword m_dwElapsed;
	lg_float m_fTime;
	lg_float m_fLastTime;
	lg_float m_fElapsed;
public:
	CLTimer();
	void Initialize();     //Should only be called once.
	void Update();         //Update should be called once per frame.
	lg_dword GetTime();     //return time in milliseconds
	lg_dword GetElapsed();  //return time elapsed in milliseconds.
	lg_float GetFTime();    //Return floating time in seconds
	lg_float GetFElapsed(); //return floating time elapsed in seconds
};

class CElementTimer
{
protected:
	static CLTimer s_Timer;
};

#endif __LT_SYS_H__