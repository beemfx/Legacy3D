#ifndef __LC_CON_H__
#define __LC_CON_H__

#include "lc_sys2.h"
#include "lc_def.h"

#define LC_MAX_LINE_LEN 127
#define LC_MAX_CMD_ARGS 8

class CConsole
{	
friend void LC_Printf(lg_char* szFormat, ...);
friend void LC_SendCommandf(lg_char* szFormat, ...);
friend lg_dword LC_FUNC LC_GetNumLines();
private:
	enum GET_LINE_MODE{GET_NONE=0, GET_OLDER, GET_NEWER};
	struct LC_LINE{
		lg_char szLine[LC_MAX_LINE_LEN+1];
		LC_LINE* pOlder;
		LC_LINE* pNewer;
	};
	struct LC_ARGS{
		lg_dword nArgCount;
		lg_char szArg[LC_MAX_CMD_ARGS+1][LC_MAX_LINE_LEN+1];
	};
private:
	lg_char  m_szActiveLine[LC_MAX_LINE_LEN+1];
	lg_dword m_nActiveCursorPos;
	
	LC_LINE* m_pNewestLine;
	LC_LINE* m_pOldestLine;
	lg_dword m_nLineCount;
	
	LC_CMD_FUNC m_pfnCmd;
	lg_void*    m_pCmdExtra;
	
	GET_LINE_MODE m_nGetLineMode;
	LC_LINE* m_pGetLine;
	
	CDefs m_Commands;
	
	void AddEntry(lg_char* szText);
	
	static void SimpleParse1(lg_char* szLine);
public:
	CConsole();
	~CConsole();
	void SetCommandFunc(LC_CMD_FUNC pfn, lg_void* pExtra);
	void Clear();
	void Print(lg_char* szText);
	void Printf(lg_char* szFormat, ...);
	void SendCommand(lg_char* szCmd);
	void SendCommandf(lg_char* szFormat, ...);
	
	void OnChar(lg_char c);
	
	const lg_char* GetLine(lg_dword nRef, const lg_bool bStartWithNew);
	const lg_char* GetNewestLine();
	const lg_char* GetOldestLine();
	const lg_char* GetNextLine();
	const lg_char* GetActiveLine();
	
	lg_bool RegisterCommand(const lg_char* szCommand, lg_dword nID, const lg_char* szHelpString);

	void ListCommands();
private:
	static lg_char s_szTemp[LC_MAX_LINE_LEN+1];
	static lg_char s_szTemp2[LC_MAX_LINE_LEN+1];
	//static lg_char s_szArgs[sizeof(lg_tchar)*(LC_MAX_CMD_ARGS+1)+LC_MAX_LINE_LEN+1];
	static LC_ARGS  s_Args;
public:
	static const lg_char* GetArg(lg_dword nParam, lg_void* pParams);
	static lg_bool CheckArg(const lg_char* string, lg_void* args);
};

#endif __LC_CON_H__