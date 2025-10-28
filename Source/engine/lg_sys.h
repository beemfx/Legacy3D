#ifndef __LG_SYS2_H__
#define __LG_SYS2_H__
#include "lv_con.h"
#include "ls_sys.h"
//#include "lw_sys.h"

#include "lv_sys.h"
#include "lt_sys.h"
#include "li_sys.h"

#include "lw_server.h"
#include "lw_client.h"
#include "lg_mgr.h"


#include "lc_sys2.h"
#include "wnd_sys/wnd_manager.h"

#define LGAME_NAME    "Legacy Game Engine"
#define LGAME_VERSION "1.00"

const char g_GameString[]=LGAME_NAME " BUILD " LGAME_VERSION " (" __DATE__ " " __TIME__ ")";

class CLGame: private CElementD3D, private CElementTimer, private CLMgrs
{
friend void LGC_DisplayDeviceCaps(CLGame* pGame);
private:
	//Legacy Game Error Codes...
	#define LG_FAIL            0x80000001
	#define LG_SHUTDOWN        0x80000010
	#define LG_V_OR_S_DISABLED 0x80000020
	#define LG_OK              0x00000001
	
	#define LVERR_NODEVICE    0x80000010
	#define LVERR_DEVICELOST  0x80000000
	#define LVERR_CANTRECOVER 0x80000001
	#define LVERR_DEVICERESET 0x80000002

public:
	enum LGSTATE{
		LGSTATE_UNKNOWN=0,
		LGSTATE_NOTSTARTED=1,
		LGSTATE_RUNNING=2,
		LGSTATE_SHUTDOWN=3,
		LGSTATE_FORCE_DWORD=0xFFFFFFFF
	};
	
	enum LGTYPE{
		LGTYPE_CLIENT=0,
		LGTYPE_SERVER_CLIENT=1,
		LGTYPE_SERVER=2
	};
private:
	LGSTATE   m_nGameState;
	LGTYPE    m_nGameType;

	HWND      m_hwnd;
public:		
	//The video manager
	CLVideo m_Video;
private:	
	//The graphic console.
	CLVCon m_VCon;
	
	//The sound manager
	CLSndMgr m_SndMgr;	
	//Input manger
	CLInput m_Input;
	
	//The window manager
	wnd_sys::CManager m_WndMgr;
private:
	//Some CVarst that we keep track of every frame.	
	lg_cvar*  cv_l_ShouldQuit;
	lg_cvar*  cv_l_AverageFPS;
	
	//Game management variables.
public: 
	//Base Game Initialization:
	lg_bool LG_GameInit(lg_str szGameDir, HWND hwnd);
	void LG_GameShutdown();
	lg_bool LG_GameLoop();
	void LG_SendCommand(lg_cstr szCommand);
	
private:
	void LG_RegisterCVars();
	//Command processing methods:
	static void LGC_RegConCmds();
		
	static lg_bool LCG_ConCommand(LC_CMD nCmd, lg_void* args, lg_void* pExtra);
	static void CLGame::LGC_CvarUpdate(CLGame* pGame, const lg_char* szCvar);
	
	//Direct3D Display Initialization:
	
	//void LV_InitTexLoad();
	
	//Video reset stuff:
	lg_result LV_ValidateDevice();
	void LV_ValidateGraphics();
	void LV_InvalidateGraphics();
	lg_result LV_Restart();
	
	//Functions for setting render states
	void SetWorldRS();
	void SetWindowRS();
	void SetConsoleRS();
private:
	//The game server (to replace CLWorld).
	CLWorldSrv  m_WorldSrv;
	CLWorldClnt m_WorldCln;
public:
	//Constructor
	CLGame();
	//Destructor
	~CLGame();
	
public:
	//Game processing methods.
	void   LG_ProcessGame();
	lg_result LG_RenderGraphics();
	
	void LG_OnChar(char c); //OnChar for console input.
};

#endif __LG_SYS2_H__