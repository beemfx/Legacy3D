#pragma once

#include "lg_types.h"
#include "lv_img2d.h"
#include "lv_sys.h"

namespace wnd_sys{

class CWindow: private CElementD3D{
private:
	lg_long m_nX, m_nY; //The position.
	lg_long m_nWidth, m_nHeight; //Width and height of window.
	
	lg_dword m_nFlags;
	
	CLImg2D m_Img;
public: //FLAGS
	static const lg_dword WND_CREATED=0x00000001;
public:
	CWindow();
	~CWindow();

	void Create(lg_long nX, lg_long nY, lg_long nWidth, lg_long nHeight);
	void Destroy();
	
	void Render();
};

} //namespace wnd_sys