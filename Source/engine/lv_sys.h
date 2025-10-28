#ifndef __LV_SYS_H__
#define __LV_SYS_H__

#include <d3d9.h>
#include "lg_types.h"

class CElementD3D
{
protected:
	static IDirect3D9*        s_pD3D;
	static IDirect3DDevice9*  s_pDevice;
	static IDirect3DSurface9* s_pBackSurface;
	static lg_uint            s_nAdapterID;
	static D3DDEVTYPE         s_nDeviceType;
	static D3DFORMAT          s_nBackBufferFormat;
	static lg_dword           s_nDisplayWidth;
	static lg_dword           s_nDisplayHeight;
};

typedef enum _TEXFILTER_MODE{
	FILTER_UNKNOWN     = 0,
	FILTER_NONE        = 0,
	FILTER_POINT       = 1,
	FILTER_LINEAR      = 2,
	FILTER_BILINEAR    = 2,
	FILTER_TRILINEAR   = 3,
	FILTER_ANISOTROPIC = 4,
	FILTER_FORCE_DWORD = 0xFFFFFFFF
}TEXFILTER_MODE;
//#endif FILTER_MODE

class CLVideo: private CElementD3D
{
friend class CLGame; //Will remove this friend when completely implemented.
private:
	HWND      m_hwnd;
public:
	CLVideo();
	~CLVideo();
	
	lg_bool Init(HWND hwnd);
	void Shutdown();
	
	lg_bool SetStates();
	lg_bool SetFilterMode(lg_int nFilterMode, lg_dword dwStage);
private:
	lg_bool SetPPFromCVars(D3DPRESENT_PARAMETERS* pp);

	//void LV_SetTexLoadModes();

	//static lg_bool SupportedTexFormats();
	static lg_bool CorrectWindowSize(HWND hWnd, DWORD nWidth, DWORD nHeight);
	static void PrintPP(D3DPRESENT_PARAMETERS* pp);
	static lg_dword D3DFMTToBitDepth(D3DFORMAT fmt);
	static lg_dword StringToD3DFMT(lg_cstr szfmt);
	static lg_cstr  D3DFMTToString(D3DFORMAT fmt);
	//Video reset stuff:
	//lg_result LV_ValidateDevice();
	//lg_result LV_ValidateGraphics();
	//lg_result LV_InvalidateGraphics();
	lg_bool Restart();
};

#endif __LV_SYS_H__