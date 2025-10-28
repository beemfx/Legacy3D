#ifndef __LI_SYS_H__
#define __LI_SYS_H__
#include <dinput.h>
#include "common.h"

typedef enum _LI_COMMAND{
	COMMAND_MOVEFORWARD=0,
	COMMAND_MOVEBACK,
	COMMAND_MOVERIGHT,
	COMMAND_MOVELEFT,
	COMMAND_TURNRIGHT,
	COMMAND_TURNLEFT,
	COMMAND_MOVEUP,        //Jumping, or swimming/flying up.
	COMMAND_MOVEDOWN,      //Ducking or swimming/flying down.
	COMMAND_SPEED,
	COMMAND_PRIMARY_WEAPON,
	
	COMMAND_COUNT,
}LI_COMMAND;

typedef enum _LI_INPUTDEVICE{
	DEV_KB=0,
	DEV_MOUSE,
	DEV_JOYSTICK,
}LI_INPUTDEVICE;

class LG_CACHE_ALIGN CLICommand
{
friend class CLInput;
private:
	LI_COMMAND     m_nCommand;
	LI_INPUTDEVICE m_nDevice;
	lg_byte        m_nButton;
	lg_bool        m_bActive;
	lg_bool        m_bWasActivated;
	lg_bool        m_bWillDeactivate;
public:
	CLICommand():
		m_nCommand(COMMAND_MOVEFORWARD),
		m_nDevice(DEV_KB),
		m_nButton(0), 
		m_bActive(0), 
		m_bWasActivated(0), 
		m_bWillDeactivate(0)
	{
	}
	lg_bool IsActive(){return m_bActive;}
	lg_bool Activated(){return m_bWasActivated;}
	
	void Init(LI_COMMAND cmd, LI_INPUTDEVICE dev, lg_byte nButton)
	{
		m_nCommand=cmd;
		m_nDevice=dev;
		m_nButton=nButton;
	}
};

class CLAxis
{
public:
	CLAxis():nY(0),nX(0){}
	const lg_long nX;
	const lg_long nY;
};

class CElementInput
{
public:
	static CLICommand* s_pCmds;
	static CLAxis*     s_pAxis;
};

class CLInput: public CElementInput
{
private:
	//DirectInput interfaces (we always have a mouse and keyboard)
	IDirectInput8*        m_pDI;
	IDirectInputDevice8*  m_pKB;
	lg_bool               m_bExclusiveKB;
	lg_byte               m_kbData[256]; //Keyboard_data.
	lg_byte               m_nConKey;
	
	lg_float              m_fXSense;  //X axis sensitivity.
	lg_float              m_fYSense;  //y axis "         ".
	
	IDirectInputDevice8*  m_pMouse;
	lg_bool               m_bExclusiveMouse;
	lg_bool               m_bSmoothMouse;
	DIMOUSESTATE2         m_mouseState; //Mouse Data
	LONG                  m_nLastX, m_nLastY;
	
	HWND m_hwnd;
	
	lg_bool               m_bInitialized;
	
	CLICommand m_Commands[COMMAND_COUNT];
	CLAxis     m_Axis; //The axis movment.
	
	//The following is a compact way to store commands,
	//rather than have an individual structure all commands
	//are store inside of flags, see examples for usage.
public:
	lg_dword   m_nCmdsActive[COMMAND_COUNT/32+1];
	lg_dword   m_nCmdsPressed[COMMAND_COUNT/32+1];
	lg_float   m_fAxis[2];

private:
	void InitCommands();
public:
	CLInput();
	~CLInput();
	#define LINPUT_EXCLUSIVEKB    0x00000001
	#define LINPUT_EXCLUSIVEMOUSE 0x00000002
	#define LINPUT_DISABLEWINKEY  0x00000004
	#define LINPUT_SMOOTHMOUSE    0x00000008
	lg_bool LI_Init(HWND hwnd, lg_dword nFlags, lg_byte nConKey);
	void LI_Shutdown();
	#define LIS_OK      0x00000001
	#define LIS_CONSOLE 0x00000002
	#define LIERR_FAIL  0x10000000
	#define LI_UPDATE_MOUSE 0x00000001
	#define LI_UPDATE_KB    0x00000002
	#define LI_UPDATE_GP    0x00000004
	#define LI_UPDATE_ALL   0x0000000F
	lg_result LI_Update(lg_dword Flags); //Update all input device (should be called once per frame before AI is processed).
	
	const lg_byte* GetKBData();
	const DIMOUSESTATE2* GetMouseData();
	
	CLICommand* GetCommand(LI_COMMAND cmd);
	CLICommand* GetCommands();
	CLAxis* GetAxis();
	void LI_ResetStates();
	
private:
	__inline void LI_InitKB(lg_dword Flags);
	__inline void LI_InitMouse(lg_dword Flags);
	
	void UpdateCommands();
};

#endif __LI_SYS_H__