/* File: wnd_mgr.h - The window manager.
	Description: The window manager manages in game windows.
	It is currently incomplete.
	
	(c) 2009 Blaine Myers
*/
#pragma once

#include "wnd_window.h"

namespace wnd_sys{

class CManager{
private:
	CWindow m_TestWnd;
	CWindow m_TestWnd2;
public:
	CManager();
	~CManager();
	
	void Init();
	void Shutdown();
	
	void Render();
};

} //namespace wnd_sys