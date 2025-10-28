/* ls_init.h - Header for audio initialization. */
#ifndef __LS_INIT_H__
#define __LS_INIT_H__
#include "lg_sys.h"
#ifdef __cplusplus
extern "C"{
#endif /* __cplusplus */

L_result LS_Init(L3DGame* lpGame);
L_result LS_Shutdown(L3DGame* lpGame);

#ifdef __cplusplus
}
#endif /* __cplusplus */
#endif /* __LS_INIT_H__ */