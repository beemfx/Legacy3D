#ifndef __L_ERROR_H__
#define __L_ERROR_H__
#include "lc_sys.h"
#include "common.h"
#ifdef __cplusplus
extern "C"{
#endif __cplusplus

#define MAX_ERR_MSG_LEN (1024)

void Err_InitReporting(HLCONSOLE hConsole);
L_int Err_Printf(const char* format, ...);
L_int Err_PrintDX(const char* function, L_result result);
void Err_ErrBox(const char* format, ...);
void Err_PrintMatrix(void* pM);
L_int Err_PrintVersion();

#ifdef __cplusplus
}
#endif __cplusplus
#endif /*__L_ERROR_H__*/