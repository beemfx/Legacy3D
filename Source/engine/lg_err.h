#ifndef __L_ERROR_H__
#define __L_ERROR_H__
#include "lg_types.h"

#ifdef __cplusplus
extern "C"{
#endif __cplusplus

#define MAX_ERR_MSG_LEN (1024)

void Err_Printf(const char* format, ...);
void Err_IncTab();
void Err_DecTab();
void Err_MsgPrintf(const char* format, ...);
void Err_PrintfDebug(const char* format, ...);
void Err_PrintDX(const char* function, lg_result result);
void Err_ErrBox(const char* format, ...);
void Err_PrintMatrix(void* pM);
void Err_PrintVersion();
void __cdecl Err_FSCallback(lg_cwstr szString);

#ifdef __cplusplus
}
#endif __cplusplus
#endif /*__L_ERROR_H__*/