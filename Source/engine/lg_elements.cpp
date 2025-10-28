//lg_elements.cpp Copyright (c) 2007 Blaine Myers
//This is more or less where all the global elements
//of the legacy engine are stored...
//However instead of simply accessing the globabls by
//using an extern a class that uses a global needs
//to inhererit the element as a child class,
//so if a class needs to use the IDirect3DDevice9 interface
//then that class needs to inherit CElementD3D, and if a
//class needs to use the global timer then that class
//needs to inherit CElementTimer, etc...
//The actual declarations for these class are in the header file
//directly above the elements.

#include "lv_sys.h"
IDirect3D9*        CElementD3D::s_pD3D=0;
IDirect3DDevice9*  CElementD3D::s_pDevice=0;
IDirect3DSurface9* CElementD3D::s_pBackSurface=0;
lg_uint            CElementD3D::s_nAdapterID=0;
D3DDEVTYPE         CElementD3D::s_nDeviceType=D3DDEVTYPE_HAL;
D3DFORMAT          CElementD3D::s_nBackBufferFormat=D3DFMT_UNKNOWN;
lg_dword           CElementD3D::s_nDisplayWidth=0;
lg_dword           CElementD3D::s_nDisplayHeight=0;

#include "lt_sys.h"
CLTimer CElementTimer::s_Timer;

#include "li_sys.h"
CLICommand* CElementInput::s_pCmds=0;
CLAxis*     CElementInput::s_pAxis=0;

