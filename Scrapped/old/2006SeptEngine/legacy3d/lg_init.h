#ifndef __L3DINIT_H__
#define __L3DINIT_H__

#include "lg_sys.h"
#include <lc_sys.h>
#include "common.h"

#ifdef __cplusplus
extern "C"{
#endif /* __cplusplus */

L3DGame* LG_GameInit(char* szGameDir, HWND hwnd);
L_bool LG_GameShutdown(L3DGame* lpGame);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __L3DINIT_H__ */