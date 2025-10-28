#include "wnd_manager.h"
#include "lg_err.h"

namespace wnd_sys{

CManager::CManager()
{

}

CManager::~CManager()
{

}

void CManager::Init()
{
	m_TestWnd.Create(20, 20, 100, 100);
	m_TestWnd2.Create(50, 100, 150, 100);
}

void CManager::Shutdown()
{

}

void CManager::Render()
{
	m_TestWnd.Render();
	m_TestWnd2.Render();
}

} //namespace wnd_sys