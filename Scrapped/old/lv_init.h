/* lv_init.h - Header for video initialization functions. */
#ifdef __LVINIT_H__
#define __LVINIT_H__

#include "lg_sys.h"

#ifdef __cplusplus
extern "C"{
#endif /*__cplusplus*/

L_bool LV_PrintPP(D3DPRESENT_PARAMETERS* pp);

int LV_SetPPFromCVars(
	L3DGame* lpGame, 
	D3DPRESENT_PARAMETERS* pp);

L_bool LV_SetStates(L3DGame* lpGame);
L_bool LV_SetFilterMode(L3DGame* lpGame, L_int nFilterMode, L_dword dwStage);

L_result LV_Init(L3DGame* lpGame);
L_result LV_Shutdown(L3DGame* lpGame);

L_bool LV_SupportedTexFormats(
	HCVARLIST cvars, 
	IDirect3D9* lpD3D, 
	UINT nAdapterID, 
	UINT nDeviceType, 
	D3DFORMAT dfmt);



/* Some Definitions to help make sure everything works.*/

/* These ones are for the texture filter mode. */
#define FILTER_MODE          "v_TextureFilter"
typedef enum _LV_TEXFILTER_MODE{
	FILTER_UNKNOWN     = 0,
	FILTER_NONE        = 0,
	FILTER_POINT       = 1,
	FILTER_LINEAR      = 2,
	FILTER_BILINEAR    = 2,
	FILTER_TRILINEAR   = 3,
	FILTER_ANISOTROPIC = 4,
	FILTER_FORCE_DWORD = 0xFFFFFFFF
}LV_TEXFILTER_MODE;

#define FILTER_MAXANISOTROPY "v_MaxAnisotropy"


#ifdef __cplusplus
}
#endif /*__cplusplus*/
#endif /*__LIVINIT_H__*/