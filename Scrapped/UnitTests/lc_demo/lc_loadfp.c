#include <windows.h>
#include "..\..\lc_sys\src\lc_sys.h"

/* Legacy Console Function Pointers. */

LPCON_CREATE pCon_Create=NULL;
LPCON_DELETE pCon_Delete=NULL;
LPCON_SENDMESSAGE pCon_SendMessage=NULL;
LPCON_SENDERRORMSG pCon_SendErrorMsg=NULL;
LPCON_SENDCOMMAND pCon_SendCommand=NULL;
LPCON_SETCURRENT pCon_SetCurrent=NULL;
LPCON_ERASECURRENT pCon_EraseCurrent=NULL;
LPCON_CLEAR pCon_Clear=NULL;
LPCON_GETNUMENTRIES pCon_GetNumEntries=NULL;
LPCON_GETENTRY pCon_GetEntry=NULL;
LPCON_REGISTERCMD pCon_RegisterCmd=NULL;
LPCON_ONCHAR pCon_OnChar=NULL;
LPCON_ATTACHCVARLIST pCon_AttachCVarList=NULL;
LPCON_CREATEDLGBOX pCon_CreateDlgBox=NULL;

LPCCPARSE_GETPARAM pCCParse_GetParam=NULL;
LPCCPARSE_GETFLOAT pCCParse_GetFloat=NULL;
LPCCPARSE_GETINT pCCParse_GetInt=NULL;
LPCCPARSE_CHECKPARAM pCCParse_CheckParam=NULL;

LPCVAR_CREATELIST pCVar_CreateList=NULL;
LPCVAR_DELETELIST pCVar_DeleteList=NULL;
LPCVAR_SET pCVar_Set=NULL;
LPCVAR_SETVALUE pCVar_SetValue=NULL;
LPCVAR_GET pCVar_Get=NULL;
LPCVAR_GETVALUE pCVar_GetValue=NULL;
LPCVAR_GETCVAR pCVar_GetCVar=NULL;
LPCVAR_GETFIRSTCVAR pCVar_GetFirstCVar=NULL;
LPCVAR_ADDDEF pCVar_AddDef=NULL;
LPCVAR_GETDEF pCVar_GetDef=NULL;

int ObtainFunctions(HMODULE hDllFile)
{
	/* If we fail to get one function, the function will return false. */
	#define GET_FUNC(name) {p##name=(void*)GetProcAddress(hDllFile, #name); \
		if(p##name==NULL)return 0;}

	GET_FUNC(Con_Create);
	GET_FUNC(Con_Delete);
	GET_FUNC(Con_SendMessage);
	GET_FUNC(Con_SendErrorMsg);
	GET_FUNC(Con_SendCommand);
	GET_FUNC(Con_SetCurrent);
	GET_FUNC(Con_EraseCurrent);
	GET_FUNC(Con_Clear);
	GET_FUNC(Con_GetNumEntries);
	GET_FUNC(Con_GetEntry);
	GET_FUNC(Con_RegisterCmd);
	GET_FUNC(Con_OnChar);
	GET_FUNC(Con_AttachCVarList);
	GET_FUNC(Con_CreateDlgBox);

	GET_FUNC(CCParse_GetParam);
	GET_FUNC(CCParse_GetFloat);
	GET_FUNC(CCParse_GetInt);
	GET_FUNC(CCParse_CheckParam);

	GET_FUNC(CVar_CreateList);
	GET_FUNC(CVar_DeleteList);
	GET_FUNC(CVar_Set);
	GET_FUNC(CVar_SetValue);
	GET_FUNC(CVar_Get);
	GET_FUNC(CVar_GetValue);
	GET_FUNC(CVar_GetCVar);
	GET_FUNC(CVar_GetFirstCVar);
	GET_FUNC(CVar_AddDef);
	GET_FUNC(CVar_GetDef);

	/* We return true if we got all the functions. */
	return 1;
}


