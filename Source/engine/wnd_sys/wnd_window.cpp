#include "wnd_window.h"
#include "lg_func.h"

namespace wnd_sys{

CWindow::CWindow()
: m_nFlags(0)
{
}

CWindow::~CWindow()
{
}

void CWindow::Create(lg_long nX, lg_long nY, lg_long nWidth, lg_long nHeight)
{
	m_nX=nX;
	m_nY=nY;
	m_nWidth=nWidth;
	m_nHeight=nHeight;
	
	//m_Img.CreateFromColor(s_pDevice, m_nWidth, m_nHeight, D3DCOLOR_XRGB(LG_RandomLong(0, 100), 0, 0));
	m_Img.CreateFromFile(s_pDevice, "/dbase/textures/WindowBG01.tga", NULL, m_nWidth, m_nHeight, 0);
	m_nFlags=WND_CREATED;
}

void CWindow::Destroy()
{
	m_Img.Delete();
}

void CWindow::Render()
{
	CLImg2D::StartStopDrawing(s_pDevice, 400, 400, LG_TRUE);

	m_Img.Render(m_nX, m_nY);
	
	CLImg2D::StartStopDrawing(s_pDevice, 0, 0, LG_FALSE);
}

}//namespace wnd_sys