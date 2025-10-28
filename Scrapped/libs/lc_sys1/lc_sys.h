/* lc_sys.h - Legacy Console copyright (c) 2006, Blaine Myers */
#ifndef __LC_SYS_H__
#define __LC_SYS_H__

#include "common.h"

#ifdef __cplusplus
extern "C"{
#endif /* __cplusplus */


/***************************************************************************************** 
	The legacy console library, this library is object oriented, it includes a console,
	which basically holds lines of text, and it will also send commands typed into the
	console to an external function.  The library also includes a cvarlist object oriented
	type that store and retrieves variables from a database.  The console has some built
	in functions and if a cvarlist is attached to it, it will process those functions. The
	console also will store a pointer to some private data that is passed to the command
	function.
******************************************************************************************/
typedef void* HLCONSOLE;
/* Declaration for the command function. */
typedef int ( * LPCONCOMMAND)(
	unsigned long nCommand, 
	const char* szParams, 
	HLCONSOLE hConsole,
	void* pExtraData);

#define CONF_CVARUPDATE  0x0080000Fl

/* Legacy Console Function Prototypes. */
HLCONSOLE Con_Create(
	LPCONCOMMAND lpfnCommandFunc, 
	unsigned long nMaxStrLen, 
	unsigned long nMaxEntries, 
	unsigned long dwFlags,
	void* pExtraData);
int Con_Delete(HLCONSOLE hConsole);
int Con_SendMessage(HLCONSOLE hConsole, char* szMessage);
int Con_SendErrorMsg(HLCONSOLE hConsole, char* format, ...);
int Con_SendCommand(HLCONSOLE hConsole, char* szCmdLine);
int Con_SetCurrent(HLCONSOLE hConsole, char* szText);
int Con_EraseCurrent(HLCONSOLE hConsole);
int Con_Clear(HLCONSOLE hConsole);
unsigned long Con_GetNumEntries(HLCONSOLE hConsole);
char* Con_GetEntry(HLCONSOLE hConsole, unsigned long nEntry, char* szOut);
int Con_RegisterCmd(HLCONSOLE hConsole, char* szCmdName, unsigned long nValue);
int Con_OnChar(HLCONSOLE hConsole, unsigned short cChar);
int Con_AttachCVarList(HLCONSOLE hConsole, void* cvarlist);
//void* Con_GetCVar(HLCONSOLE hConsole, char* name);
//void* Con_GetCVarList(HLCONSOLE hConsole);

/* Creation flags. */
#define CONCREATE_USEINTERNAL (0x01000001)
//#define CONCREATE_USECVARLIST (0x01000002)

/* Legacy Console Function Pointers. */
typedef HLCONSOLE (*LPCON_CREATE)(void*, unsigned long, unsigned long, unsigned long, void*);
typedef int (*LPCON_DELETE)(HLCONSOLE);
typedef int (*LPCON_SENDMESSAGE)(HLCONSOLE, char*);
typedef int (*LPCON_SENDERRORMSG)(HLCONSOLE, char*, ...);
typedef int (*LPCON_SENDCOMMAND)(HLCONSOLE, char*);
typedef int (*LPCON_SETCURRENT)(HLCONSOLE, char*);
typedef int (*LPCON_ERASECURRENT)(HLCONSOLE);
typedef int (*LPCON_CLEAR)(HLCONSOLE);
typedef unsigned long (*LPCON_GETNUMENTRIES)(HLCONSOLE);
typedef char* (*LPCON_GETENTRY)(HLCONSOLE, unsigned long, char*);
typedef int (*LPCON_REGISTERCMD)(HLCONSOLE, char*, unsigned long);
typedef int (*LPCON_ONCHAR)(HLCONSOLE, unsigned short);
typedef int (*LPCON_ATTACHCVARLIST)(HLCONSOLE, void*);
//typedef void* (*LPCON_GETCVAR)(HLCONSOLE, char*);
//typedef void* (*LPCON_GETCVARLIST)(HLCONSOLE);

/* Some helper functions. */
int CCParse_GetParam(
	char* szParamOut, 
	const char* szParams, 
	unsigned long nParam);
float CCParse_GetFloat(
	char* szParams,
	unsigned short wParam);
signed long CCParse_GetInt(
	char* szParams,
	unsigned short wParam);
int CCParse_CheckParam(
	const char* szParams,
	const char* szAttrib,
	int nStart);
	
int CCParse_CheckParam(
	const char* szParams,
	const char* szAttrib,
	int nStart);
/* The types for loading teh CCParse functions. */
typedef int (*LPCCPARSE_GETPARAM)(char*, const char*, unsigned long);
typedef float (*LPCCPARSE_GETFLOAT)(char*, unsigned short);
typedef signed long (*LPCCPARSE_GETINT)(char*, unsigned short);
typedef int (*LPCCPARSE_CHECKPARAM)(char*, char*, int);


/* The following functions are to help out with the built in window console.
	Con_CreateDlgBox will return a handle to a window that needs to be included
	in the main window loop for propper processing, the defined messages will 
	allow the user to attach the console to the dialog box.  And also to force
	the dialog box to update. */
/* Some functions to activate the windows dialog. */
void* Con_CreateDlgBox(); /* HWND Con_CreateDlgBox(); */
typedef void* (*LPCON_CREATEDLGBOX)();
/* Definition to attach a console to the windows dialog, and to update that dialog. */
#define WM_USER_UPDATE (WM_USER+1)
#define WM_USER_INSTALLCONSOLE (WM_USER+2)


/***************************************************************** 
	The following declarations and variables apply to the cvarlist 
	object oriented thingy.
******************************************************************/


typedef void* HCVARLIST;

/* The cvar structure. */
/* Note that these values should not be changed, except using the
	CVar_ functions, the only reason the CVar structure is visible,
	is so that value and stringvalue can be accessed, without having
	to go through the linked list every time. */
typedef struct tagLCvar{
	char* name; /* The name of the cvar. */
	char* string; /* The value of the cvar in string format. */
	float value; /* The value of the cvar as a float. */
	unsigned long flags; /* Flags that were set during creaion.*/
	struct tagLCvar* next; /* pointer to the next cvar.  Do not mess with this value, only let the functions manipulate it. */
}CVar, *LPCVar;

HCVARLIST CVar_CreateList(void* hConsole);
int CVar_DeleteList(HCVARLIST hList);
CVar* CVar_Register(HCVARLIST hList, const char* szName, const char* szValue, unsigned long dwFlags);
int CVar_Set(HCVARLIST hList, const char* szName, const char* szValue);
int CVar_SetValue(HCVARLIST hList, const char* szName, const float fValue);
char* CVar_Get(HCVARLIST hList, const char* szName, char* szOutput);
float CVar_GetValue(HCVARLIST hList, const char* szName, int* bGotValue);
CVar* CVar_GetCVar(HCVARLIST hList, const char* szName);
CVar* CVar_GetFirstCVar(HCVARLIST hList);
int CVar_AddDef(HCVARLIST hList, const char* szDef, const float fValue);
float CVar_GetDef(HCVARLIST hList, const char* szDef, int* bGotDef);

typedef void* (*LPCVAR_CREATELIST)(void*);
typedef int (*LPCVAR_DELETELIST)(HCVARLIST);
typedef int (*LPCVAR_SET)(HCVARLIST, const char*, const char*);
typedef int (*LPCVAR_SETVALUE)(HCVARLIST, const char*, const float);
typedef char* (*LPCVAR_GET)(HCVARLIST, const char*, char*);
typedef float (*LPCVAR_GETVALUE)(HCVARLIST, const char*, int*);
typedef CVar* (*LPCVAR_GETCVAR)(HCVARLIST, const char*);
typedef CVar* (*LPCVAR_GETFIRSTCVAR)(HCVARLIST);
typedef int (*LPCVAR_ADDDEF)(HCVARLIST, const char*, float);
typedef float (*LPCVAR_GETDEF)(HCVARLIST, const char*, int*);

/* CVar_Register creation flags. */
#define CVAREG_UPDATE        (0x00000001)
#define CVAREG_SAVE          (0x00000002)
#define CVAREG_SETWONTCHANGE (0x00000004)



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __LC_SYS_H__ */