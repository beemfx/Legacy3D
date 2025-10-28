/*****************************************************************************
	li_sys.cpp - Legacy Engine Input.  The CLInput class is used to get all
	"game input" (not console input), note that when this class detects that
	the console key was pressed it notifies the game that it needs to get
	input from windows.  This class uses DirectInput8, and a keyboard and
	mouse are always supported.  Note that this class is by no means error
	proof and as it stands right now if there is no mouse installed in the
	system, the program will crash.
*****************************************************************************/

#include "li_sys.h"
#include "lg_err.h"
#include "lg_func.h"

CLInput::CLInput():
	m_pDI(LG_NULL),
	m_pKB(LG_NULL),
	m_pMouse(LG_NULL),
	m_bExclusiveKB(LG_FALSE),
	m_bExclusiveMouse(LG_FALSE),
	m_nConKey(0),
	m_bInitialized(LG_FALSE),
	m_hwnd(LG_NULL),
	m_nLastX(0),
	m_nLastY(0),
	m_bSmoothMouse(LG_FALSE),
	m_fXSense(0.01f),
	m_fYSense(0.01f)
{
	memset(m_kbData, 0, sizeof(m_kbData));
	memset(&m_mouseState, 0, sizeof(m_mouseState));
	
	s_pCmds=m_Commands;
	s_pAxis=&m_Axis;
	
	m_nCmdsActive[0]=0;
	m_nCmdsPressed[0]=0;
	m_fAxis[0]=0.0f;
	m_fAxis[1]=0.0f;
}

CLInput::~CLInput()
{
	if(m_bInitialized)
	{
		Err_Printf("Calling LI_Shutdown...");
		LI_Shutdown();
	}
}

const lg_byte* CLInput::GetKBData()
{
	return m_kbData;
}
const DIMOUSESTATE2* CLInput::GetMouseData()
{
	return &m_mouseState;
}

void CLInput::InitCommands()
{
	m_Commands[COMMAND_MOVEFORWARD].Init(
		COMMAND_MOVEFORWARD,
		DEV_KB,
		DIK_W);
		
	m_Commands[COMMAND_MOVEBACK].Init(
		COMMAND_MOVEBACK,
		DEV_KB,
		DIK_S);
		
	m_Commands[COMMAND_MOVERIGHT].Init(
		COMMAND_MOVERIGHT,
		DEV_KB,
		DIK_D);
		
	m_Commands[COMMAND_MOVELEFT].Init(
		COMMAND_MOVELEFT,
		DEV_KB,
		DIK_A);
		
	m_Commands[COMMAND_MOVEUP].Init(
		COMMAND_MOVEUP,
		DEV_KB,
		DIK_SPACE);
		
	m_Commands[COMMAND_MOVEDOWN].Init(
		COMMAND_MOVEUP,
		DEV_KB,
		DIK_LCONTROL);
		
	m_Commands[COMMAND_SPEED].Init(
		COMMAND_SPEED,
		DEV_KB,
		DIK_LSHIFT);
		
	m_Commands[COMMAND_TURNRIGHT].Init(
		COMMAND_TURNRIGHT,
		DEV_KB,
		DIK_RIGHT);
		
	m_Commands[COMMAND_TURNLEFT].Init(
		COMMAND_TURNLEFT,
		DEV_KB,
		DIK_LEFT);
		
	m_Commands[COMMAND_PRIMARY_WEAPON].Init(
		COMMAND_PRIMARY_WEAPON,
		DEV_MOUSE,
		0);
}

CLICommand* CLInput::GetCommand(LI_COMMAND cmd)
{
	return &m_Commands[cmd];
}

CLICommand* CLInput::GetCommands()
{
	return &m_Commands[0];
}

CLAxis* CLInput::GetAxis()
{
	return &m_Axis;
}

void CLInput::UpdateCommands()
{
	m_nCmdsActive[0]=0;
	m_nCmdsPressed[0]=0;
	for(lg_dword i=0; i<COMMAND_COUNT; i++)
	{
		lg_byte nState;
		
		switch(m_Commands[i].m_nDevice)
		{
		case DEV_KB:
			nState=m_kbData[m_Commands[i].m_nButton]&0x80;
			break;
		case DEV_MOUSE:
			nState=m_mouseState.rgbButtons[m_Commands[i].m_nButton]&0x80;
			break;
		case DEV_JOYSTICK:
			break;
		}
		
		if(nState)
		{
			if(m_Commands[i].m_bActive)
				m_Commands[i].m_bWasActivated=LG_FALSE;
			else
				m_Commands[i].m_bWasActivated=LG_TRUE;
			m_Commands[i].m_bActive=LG_TRUE;	
			m_Commands[i].m_bWillDeactivate=LG_TRUE;
		}
		else
		{
			m_Commands[i].m_bActive=LG_FALSE;
			m_Commands[i].m_bWasActivated=LG_FALSE;
			m_Commands[i].m_bWillDeactivate=LG_FALSE;
		}
		
		if(m_Commands[i].m_bActive)
		{
			LG_SetFlag(m_nCmdsActive[i/32], (1<<((i/32)+i))); 
		}
		
		if(m_Commands[i].m_bWasActivated)
		{
			LG_SetFlag(m_nCmdsPressed[i/32], (1<<((i/32)+i)));
		}
	}
	
	//The axis should be able to come from either the keyboard or the mouse...
	//Ignor the const qualifier when updating the axis.
	(lg_long)(m_Axis.nX)=m_mouseState.lX;
	(lg_long)(m_Axis.nY)=m_mouseState.lY;
	
	m_fAxis[0]=m_Axis.nX*m_fXSense;
	m_fAxis[1]=m_Axis.nY*m_fYSense;
}

lg_result CLInput::LI_Update(lg_dword Flags)
{
	#ifdef _DEBUG
	lg_result nResult;
	#endif _DEBUG
	//Update the keyboard.
	if(L_CHECK_FLAG(Flags, LI_UPDATE_KB))
	{
		m_pKB->Acquire();
		#ifdef _DEBUG
		nResult=m_pKB->GetDeviceState(sizeof(m_kbData), (lg_void*)&m_kbData);
		if(LG_FAILED(nResult))
			Err_PrintDX("IDirectInputDevice8::GetDeviceState (KB)", nResult);
		#else _DEBUG
		m_pKB->GetDeviceState(sizeof(m_kbData), (lg_void*)&m_kbData);
		#endif _DEBUG
	}
	//Get mouse input...
	if(L_CHECK_FLAG(Flags, LI_UPDATE_MOUSE))
	{
		//m_mouseStateLast=m_mouseStateNow;
		m_nLastX=m_mouseState.lX;
		m_nLastY=m_mouseState.lY;
		m_pMouse->Acquire();
		#ifdef _DEBUG
		nResult=m_pMouse->GetDeviceState(sizeof(DIMOUSESTATE2), (lg_void*)&m_mouseState);
		if(LG_FAILED(nResult))
			Err_PrintDX("IDirectInputDevice8::GetDeviceState (Mouse)", nResult);
		#else _DEBUG
		m_pMouse->GetDeviceState(sizeof(DIMOUSESTATE2), (lg_void*)&m_mouseState);
		#endif _DEBUG
		
		//This is a relatively simply way to cause mouse
		//smoothing.
		if(m_bSmoothMouse)
		{
			m_mouseState.lX=(m_mouseState.lX+m_nLastX)/2;
			m_mouseState.lY=(m_mouseState.lY+m_nLastY)/2;
		}
		
	}
	
	//Update all commands.
	UpdateCommands();
	
	//Check to see if the console key was pressed,
	//if it was we'll unacquire the keyboard and return
	//a message stating that the console should be opened.
	if(m_kbData[m_nConKey]&0x80)
	{
		m_pKB->Unacquire();
		memset(m_kbData, 0, sizeof(m_kbData));
		memset(&m_mouseState, 0, sizeof(m_mouseState));
		return LIS_CONSOLE;
	}
	
	return LIS_OK;
}

void CLInput::LI_ResetStates()
{
	m_pMouse->Unacquire();
	m_pKB->Unacquire();
	
	m_pMouse->Acquire();
	m_pKB->Acquire();
	
	m_pMouse->GetDeviceState(sizeof(DIMOUSESTATE2), (lg_void*)&m_mouseState);
	m_pKB->GetDeviceState(sizeof(m_kbData), (lg_void*)&m_kbData);
	
}


///////////////////////////////////
/// LI_Init() and LI_Shutdown() ///
/// Create and destory input    ///
///////////////////////////////////
lg_bool CLInput::LI_Init(HWND hwnd, lg_dword nFlags, lg_byte nConKey)
{
	//If we are already initialized we'll just call shutdown,
	//this will in effect reset the input.
	LI_Shutdown();
		
	m_nConKey=nConKey;
	m_hwnd=hwnd;
	lg_result nResult=0;
	lg_dword dwFmt=0;
	nResult=DirectInput8Create(GetModuleHandle(LG_NULL), DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&m_pDI, LG_NULL);	
	if(LG_FAILED(nResult))
	{
		Err_Printf("Could not create IDirectInput8 interface.");
		Err_PrintDX("DirectInput8Create", nResult);
		m_bInitialized=LG_FALSE;
		return LG_FALSE;
	}
	Err_Printf("Created IDirectInput8 interface at 0x%08X.", m_pDI);
	
	//Create keyboard.
	Err_Printf("Initializing keyboard...");
	LI_InitKB(nFlags);
	//Create the mouse.
	Err_Printf("Initializing mouse...");
	LI_InitMouse(nFlags);
	
	//Set commands.
	InitCommands();
	
	m_bInitialized=LG_TRUE;
	return LG_TRUE;
}

void CLInput::LI_InitKB(lg_dword Flags)
{
	lg_result nResult=m_pDI->CreateDevice(GUID_SysKeyboard, &m_pKB, NULL);
	if(LG_FAILED(nResult))
	{
		Err_Printf("Could not create keyboard.");
		Err_PrintDX("IDirectInput8::CreateDevice", nResult);
		m_pKB=LG_NULL;
		return;
	}
	else
		Err_Printf("Created IDirectInputDevice9 keyboard at 0x%08X.", m_pKB);
		
	Err_Printf("Setting up keyboard input...");
	nResult=m_pKB->SetDataFormat(&c_dfDIKeyboard);
	Err_PrintDX("IDirectInputDevice8::SetDataFormat", nResult);
	lg_dword dwFmt=DISCL_FOREGROUND;
	if(L_CHECK_FLAG(Flags, LINPUT_EXCLUSIVEKB))
	{
		Err_Printf("KB input EXCLUSIVE.");
		dwFmt|=DISCL_EXCLUSIVE;
		m_bExclusiveKB=LG_TRUE;
	}
	else
	{
		Err_Printf("KB input NONEXCLUSIVE.");
		dwFmt|=DISCL_NONEXCLUSIVE;
		m_bExclusiveKB=LG_FALSE;
	}
	if(L_CHECK_FLAG(Flags, LINPUT_DISABLEWINKEY))
	{
		Err_Printf("KB input NOWINKEY.");
		dwFmt|=DISCL_NOWINKEY;
	}	
	nResult=m_pKB->SetCooperativeLevel(m_hwnd, dwFmt);
	Err_PrintDX("IDirectInputDevice8::SetCooperativeLevel", nResult);
}

void CLInput::LI_InitMouse(lg_dword Flags)
{
	lg_result nResult=m_pDI->CreateDevice(GUID_SysMouse, &m_pMouse, NULL);
	if(LG_FAILED(nResult))
	{
		Err_Printf("Could not create mouse.");
		Err_PrintDX("IDirectInput8::CreateDevice", nResult);
		m_pMouse=LG_NULL;
		return;
	}
	else
		Err_Printf("Created IDirectInputDevice9 mouse at 0x%08X.", m_pKB);
		
	Err_Printf("Setting up mouse input...");
	nResult=m_pMouse->SetDataFormat(&c_dfDIMouse2);
	Err_PrintDX("IDirectInputDevice8::SetDataFormat", nResult);
	lg_dword dwFmt=DISCL_FOREGROUND;
	if(L_CHECK_FLAG(Flags, LINPUT_EXCLUSIVEMOUSE))
	{
		Err_Printf("Mouse input EXCLUSIVE.");
		dwFmt|=DISCL_EXCLUSIVE;
		m_bExclusiveMouse=LG_TRUE;
	}
	else
	{
		Err_Printf("Mouse input NONEXCLUSIVE (not recommended).");
		dwFmt|=DISCL_NONEXCLUSIVE;
		m_bExclusiveMouse=LG_FALSE;
	}
	
	nResult=m_pMouse->SetCooperativeLevel(m_hwnd, dwFmt);
	Err_PrintDX("IDirectInputDevice8::SetCooperativeLevel", nResult);
	
	m_bSmoothMouse=L_CHECK_FLAG(Flags, LINPUT_SMOOTHMOUSE);
	if(m_bSmoothMouse)
		Err_Printf("Mouse input SMOOTH.");
}
void CLInput::LI_Shutdown()
{
	if(!m_bInitialized)
		return;
		
	lg_ulong  nNumLeft=0;
	Err_Printf("Releasing IDirectInputDevice8 mouse interface...");
	if(m_pMouse)
	{
		m_pMouse->Unacquire();
		nNumLeft=m_pMouse->Release();
		Err_Printf("Released IDirectInputDevice8 mouse interface at 0x%08X with %d references left.", m_pKB, nNumLeft);
		m_pMouse=LG_NULL;
	}
	
	Err_Printf("Releasing IDirectInputDevice8 keyboard interface...");
	if(m_pKB)
	{
		m_pKB->Unacquire();
		nNumLeft=m_pKB->Release();
		Err_Printf("Released IDirectInputDevice8 keyboard interface at 0x%08X with %d references left.", m_pKB, nNumLeft);
		m_pKB=LG_NULL;
	}
	Err_Printf("Releasing IDirectInput8 interface...");
	if(m_pDI)
	{
		nNumLeft=m_pDI->Release();
		Err_Printf("Released IDirectInput8 interface at 0x%08X with %d references left.", m_pDI, nNumLeft);
		m_pDI=LG_NULL;
	}
	m_bInitialized=LG_FALSE;
}