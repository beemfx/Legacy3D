#ifndef __LC_PRIVATE_H__
#define __LC_PRIVATE_H__
#include "lc_sys.h"

#ifdef __cplusplus
extern "C"{
#endif /* __cplusplus */

/* Internal function def values. */
#define CONF_SET      0x00800001l
#define CONF_GET      0x00800002l
#define CONF_LOADCFG  0x00800003l
#define CONF_CLEAR    0x00800004l
#define CONF_ECHO     0x00800005l
#define CONF_DEFINE   0x00800006l
#define CONF_CVARLIST 0x00800007l
#define CONF_REGCVAR  0x00800008l
#define CONF_SAVECFG  0x00800009l
#define CONF_CONDUMP  0x0080000Al
#define CONF_CMDLIST  0x0080000Bl


/*****************************************************
	Definition storage, the following declarations and
	variables allow for the use of definitions in the
	cvarlist.
******************************************************/
typedef void* HLDEFS;

int Defs_CheckDefName(char* szDef, char* szNoFAllow, char* szNoAllow);

typedef struct tagLDef{
	char* name;
	float value;
	struct tagLDef* next;
}LDef, *LPLDef;

typedef struct tagLDefs{
	LDef* list;
}LDefs, *LPLDefs;

HLDEFS Defs_CreateDefs();
int Defs_DeleteDefs(HLDEFS hDefs);

float Defs_Get(HLDEFS hDef, char* szName, int* bGotDef);
int Defs_Add(HLDEFS hDefs, char* szDef, float fValue);
int Defs_ReDef(HLDEFS hDef, char* szName, float fNewValue);



/* Private types. */
typedef struct tagLCENTRY{
	char* lpstrText;
	struct tagLCENTRY * lpNext;
}LCENTRY, *LPLCENTRY;

typedef struct tagLCONSOLE{
	unsigned long dwNumEntries;
	LCENTRY * lpActiveEntry;
	LCENTRY * lpEntryList;
	LPCONCOMMAND CommandFunction;
	unsigned long nMaxStrLen;
	unsigned long nMaxEntries;
	int bProcessInternal;
	HCVARLIST cvarlist;
	HLDEFS commands;
	void* pExtraData;
}LCONSOLE, *LPLCONSOLE;


/* Legacy Console Private Function declarations. */
int Con_AddEntry(HLCONSOLE hConsole, char* szEntry);
int Con_ClearOldestEntry(HLCONSOLE hConsole);
int Con_SimpleParse(char* szCommand,char* szParams,char* szIgnore,char* szLineIn,unsigned long dwMaxLen);

int Con_InternalCommands(unsigned long nCommand, const char* szParams, HLCONSOLE hConsole);
unsigned long Con_CmdNameToValue(HLCONSOLE hConsole, const char* szCmdName);

/* Private functions. */
int L_CheckValueName(char* szDef, char* szNoFirstAllow, char* szNoAllow);




#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __LC_PRIVATE_H__ */