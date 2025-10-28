#ifndef __LC_SYS2_H__
#define __LC_SYS2_H__

/********************************
	Disable some of VCs warnings
*********************************/
#pragma warning(disable: 4267)
#pragma warning(disable: 4996)

#include "lg_types.h"
#include "lc_def.h"



#define LC_CVAR_MAX_LEN   127
#define LC_CVAR_HASH_SIZE 128
typedef struct _lg_cvar{
	lg_char  szName[LC_CVAR_MAX_LEN+1];
	
	lg_char  szValue[LC_CVAR_MAX_LEN+1];
	lg_float fValue; //FP value  atof
	lg_long  nValue; //Int value atoi (signed and unsigned).
	
	lg_dword Flags;
	
	struct _lg_cvar* pHashNext;
}lg_cvar;

//cvar flags...
#define CVAR_EMPTY  0x00000001 //This is a blank cvar, whose hash value hasn't come up.
#define CVAR_ROM    0x00000002 //Cvar can't be changed.
#define CVAR_SAVE   0x00000004 //Cvar should be saved upon exit.
#define CVAR_UPDATE 0x00000008 //The game should change something immediated after this cvar is updated.


#define LC_EXP __declspec(dllexport)
#define LC_FUNC __cdecl

typedef lg_dword LC_CMD;
typedef lg_bool (LC_FUNC*LC_CMD_FUNC)(LC_CMD nCmd, lg_void* args, lg_void* pExtra);

#ifdef __cplusplus
extern "C"{
#endif __cplusplus

LC_EXP lg_bool LC_FUNC LC_Init();
LC_EXP void    LC_FUNC LC_Shutdown();

LC_EXP void LC_FUNC LC_Clear();

LC_EXP void LC_FUNC LC_SetCommandFunc(LC_CMD_FUNC pfn, lg_void* pExtra);

LC_EXP void LC_FUNC LC_Print(lg_char* szText);
LC_EXP void LC_FUNC LC_Printf(lg_char* szFormat, ...);

LC_EXP void LC_FUNC LC_SendCommand(const lg_char* szCmd);
LC_EXP void LC_FUNC LC_SendCommandf(lg_char* szFormat, ...);

LC_EXP lg_bool LC_FUNC LC_RegisterCommand(lg_char* szCmd, lg_dword nID, lg_char* szHelpString);
LC_EXP void LC_FUNC LC_ListCommands();

LC_EXP const lg_char* LC_FUNC LC_GetLine(lg_dword nRef, const lg_bool bStartWithNew);
LC_EXP const lg_char* LC_FUNC LC_GetOldestLine();
LC_EXP const lg_char* LC_FUNC LC_GetNewestLine();
LC_EXP const lg_char* LC_FUNC LC_GetNextLine();

LC_EXP const lg_char* LC_FUNC LC_GetActiveLine();
LC_EXP void LC_FUNC LC_OnChar(lg_char c);


LC_EXP lg_dword LC_FUNC LC_GetNumLines();

LC_EXP const lg_char* LC_FUNC LC_GetArg(lg_dword nParam, lg_void* args);
LC_EXP lg_bool LC_FUNC LC_CheckArg(const lg_char* string, lg_void* args);

//Cvar functions
LC_EXP lg_cvar* LC_FUNC CV_Register(const lg_char* szName, const lg_char* szValue, lg_dword Flags);
LC_EXP lg_bool  LC_FUNC CV_Define_l(lg_char* szDef, lg_long nValue);
LC_EXP lg_bool  LC_FUNC CV_Define_f(const lg_char* szDef, lg_float fValue);
LC_EXP lg_cvar* LC_FUNC CV_Get(const lg_char* szName);
LC_EXP void     LC_FUNC CV_Set(lg_cvar* cvar, const lg_char* szValue);
LC_EXP void     LC_FUNC CV_Set_l(lg_cvar* cvar, lg_long nValue);
LC_EXP void     LC_FUNC CV_Set_f(lg_cvar* cvar, lg_float fValue);
LC_EXP lg_cvar* LC_FUNC CV_GetFirst();
LC_EXP lg_cvar* LC_FUNC CV_GetNext();

LC_EXP void     LC_FUNC LC_ListCvars(const lg_char* szLimit);

#ifdef __cplusplus
}
#endif __cplusplus

#endif __LC_SYS2_H__