#if 0
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "lc_sys2.h"
#include "lc_con.h"
#include "common.h"


class CConsoleU
{	
friend void LC_PrintfW(lg_wchar* szFormat, ...);
friend void LC_SendCommandfW(lg_wchar* szFormat, ...);
private:
	enum GET_LINE_MODE{GET_NONE=0, GET_OLDER, GET_NEWER};
	struct LC_LINE{
		lg_wchar szLine[LC_MAX_LINE_LEN+1];
		LC_LINE* pOlder;
		LC_LINE* pNewer;
	};
	struct LC_ARGS{
		lg_dword nArgCount;
		lg_wchar szArg[LC_MAX_CMD_ARGS+1][LC_MAX_LINE_LEN+1];
	};
private:
	LC_LINE* m_pNewestLine;
	LC_LINE* m_pOldestLine;
	lg_dword m_nLineCount;
	
	LC_CMD_FUNC m_pfnCmd;
	lg_void*    m_pCmdExtra;
	
	GET_LINE_MODE m_nGetLineMode;
	LC_LINE* m_pGetLine;
	
	CDefsU m_Commands;
	
	void AddEntry(lg_wchar* szText);
	
	static void SimpleParse1(lg_wchar* szLine);
public:
	CConsoleU();
	~CConsoleU();
	void SetCommandFunc(LC_CMD_FUNC pfn, lg_void* pExtra);
	void Clear();
	void Print(lg_wchar* szText);
	void Printf(lg_wchar* szFormat, ...);
	void SendCommand(lg_wchar* szCmd);
	void SendCommandf(lg_wchar* szFormat, ...);
	
	const lg_wchar* GetLine(lg_dword nRef, const lg_bool bStartWithNew);
	const lg_wchar* GetNewestLine();
	const lg_wchar* GetOldestLine();
	const lg_wchar* GetNextLine();
	
	lg_bool RegisterCommand(const lg_wchar* szCommand, lg_dword nID, const lg_wchar* szHelpString);

	void ListCommands();
private:
	static lg_wchar s_szTemp[LC_MAX_LINE_LEN+1];
	static lg_wchar s_szTemp2[LC_MAX_LINE_LEN+1];
	//static lg_wchar s_szArgs[sizeof(lg_wchar)*(LC_MAX_CMD_ARGS+1)+LC_MAX_LINE_LEN+1];
	static LC_ARGS  s_Args;
public:
	static const lg_wchar* GetArg(lg_dword nParam, lg_void* pParams);
};

lg_wchar CConsoleU::s_szTemp[LC_MAX_LINE_LEN+1];
lg_wchar CConsoleU::s_szTemp2[LC_MAX_LINE_LEN+1];
//lg_wchar CConsoleU::s_szArgs[sizeof(lg_wchar)*(LC_MAX_CMD_ARGS+1)+LC_MAX_LINE_LEN+1];
CConsoleU::LC_ARGS  CConsoleU::s_Args;

CConsoleU::CConsoleU():
	m_pNewestLine(LG_NULL),
	m_pOldestLine(LG_NULL),
	m_nLineCount(0),
	m_nGetLineMode(GET_NONE),
	m_pGetLine(LG_NULL),
	m_pCmdExtra(LG_NULL),
	m_pfnCmd(LG_NULL)
{

}

CConsoleU::~CConsoleU()
{
	Clear();
}

void CConsoleU::ListCommands()
{
	Print((lg_wchar*)L"Registered commands:");
	for(lg_dword i=0; i<LC_DEF_HASH_SIZE; i++)
	{
		if(!m_Commands.m_pHashList[i].bDefined)
			continue;
			
		CDefsU::DEF* pDef=&m_Commands.m_pHashList[i];
		while(pDef)
		{
			Printf((lg_wchar*)L"   %s", pDef->szDef);
			pDef=pDef->pHashNext;
		}
	}
}

lg_bool CConsoleU::RegisterCommand(const lg_wchar* szCommand, lg_dword nID, const lg_wchar* szHelpString)
{
	if(!m_Commands.AddDef(szCommand, nID, LC_DEF_NOREDEFINE))
	{
		Printf((lg_wchar*)L"LC WARNING: \"%s\" is already a registered command.", szCommand);
		return LG_FALSE;
	}
	return LG_TRUE;	
}

void CConsoleU::SetCommandFunc(LC_CMD_FUNC pfn, lg_void* pExtra)
{
	m_pfnCmd=pfn;
	m_pCmdExtra=pExtra;
}

const lg_wchar* CConsoleU::GetLine(lg_dword nRef, const lg_bool bStartWithNew)
{

	m_pGetLine=bStartWithNew?m_pNewestLine:m_pOldestLine;
	m_nGetLineMode=bStartWithNew?GET_OLDER:GET_NEWER;
	for(lg_dword i=0; m_pGetLine; i++)
	{
		if(i==nRef)
			return m_pGetLine->szLine;
			
		m_pGetLine=bStartWithNew?m_pGetLine->pOlder:m_pGetLine->pNewer;
	}
	
	return LG_NULL;
}
const lg_wchar* CConsoleU::GetNewestLine()
{
	if(!m_pNewestLine)
		return LG_NULL;
		
	m_pGetLine=m_pNewestLine;
	m_nGetLineMode=GET_OLDER;
	return m_pGetLine->szLine;
}
const lg_wchar* CConsoleU::GetOldestLine()
{
	if(!m_pOldestLine)
		return LG_NULL;
		
	m_pGetLine=m_pOldestLine;
	m_nGetLineMode=GET_NEWER;
	return m_pGetLine->szLine;
}
const lg_wchar* CConsoleU::GetNextLine()
{
	if(!m_pGetLine)
		return LG_NULL;
		
	if(m_nGetLineMode==GET_NEWER)
	{
		m_pGetLine=m_pGetLine->pNewer;
		if(m_pGetLine)
			return m_pGetLine->szLine;
		else
		{
			m_nGetLineMode=GET_NONE;
			return LG_NULL;
		}
	}
	else if(m_nGetLineMode==GET_OLDER)
	{
		m_pGetLine=m_pGetLine->pOlder;
		if(m_pGetLine)
			return m_pGetLine->szLine;
		else
		{
			m_nGetLineMode=GET_NONE;
			return LG_NULL;
		}
	}
	
	return LG_NULL;
}

void CConsoleU::AddEntry(lg_wchar* szText)
{
	LC_LINE* pNew=new LC_LINE;
	if(!pNew)
		return;
		
	wcsncpy((wchar_t*)pNew->szLine, (wchar_t*)szText, LC_MAX_LINE_LEN);
	pNew->pOlder=pNew->pNewer=LG_NULL;
	
	if(!m_pNewestLine)
	{
		m_pNewestLine=m_pOldestLine=pNew;
	}
	else
	{
		pNew->pOlder=m_pNewestLine;
		m_pNewestLine->pNewer=pNew;
		m_pNewestLine=pNew;	
	}
	
	m_nLineCount++;
	return;
}

void CConsoleU::Clear()
{
	for(LC_LINE* pLine=m_pNewestLine; pLine; )
	{
		LC_LINE* pTemp=pLine->pOlder;
		delete pLine;
		pLine=pTemp;
	}
	
	m_pNewestLine=m_pOldestLine=LG_NULL;
	m_nLineCount=0;
}

void CConsoleU::Print(lg_wchar* szText)
{
	AddEntry(szText);
}

void CConsoleU::Printf(lg_wchar* szFormat, ...)
{
	va_list arglist;
	va_start(arglist, szFormat);
	_vsnwprintf((wchar_t*)s_szTemp, LC_MAX_LINE_LEN, (wchar_t*)szFormat, arglist);
	va_end(arglist);
	Print(s_szTemp);
}

void CConsoleU::SendCommand(lg_wchar* szCmd)
{
	lg_dword nCurrent=0;
	wcsncpy((wchar_t*)s_szTemp, (wchar_t*)szCmd, LC_MAX_LINE_LEN);
	s_szTemp[LC_MAX_LINE_LEN]=0;
	lg_wchar* szTemp=L_strtokW(s_szTemp, (lg_wchar*)L" \t\r\n", '"');
	while(szTemp[0]!=0)
	{
		wcsncpy((wchar_t*)s_Args.szArg[nCurrent], (wchar_t*)&szTemp[szTemp[0]=='"'?1:0], LC_MAX_LINE_LEN);
		lg_dword nLen=wcslen((wchar_t*)s_Args.szArg[nCurrent]);
		if(s_Args.szArg[nCurrent][nLen-1]=='"')
		{
			s_Args.szArg[nCurrent][nLen-1]=0;
		}
		
		nCurrent++;
		if(nCurrent>LC_MAX_CMD_ARGS)
			break;
			
		szTemp=L_strtokW(0, 0, 0);
	}
	
	s_Args.nArgCount=nCurrent;
	//If no arguments were found then the command
	//wasn't found so we just print a blank
	//line and that's it.
	
	if(s_Args.nArgCount<1)
	{
		Print((lg_wchar*)L"");
		return;
	}
	
	#if 0
	for(lg_dword i=0; i<s_Args.nArgCount; i++)
	{
		printf("Arg[%d]=\"%s\"\n", i, s_Args.szArg[i]);
	}
	#endif
	
	lg_bool bResult;
	lg_dword nCmdID=m_Commands.GetDefUnsigned(s_Args.szArg[0], &bResult);
	if(!bResult)
	{
		Printf((lg_wchar*)L"\"%s\" is not recognized as a valid command.", s_Args.szArg[0]);
		return;
	}
	
	if(m_pfnCmd)
		bResult=m_pfnCmd(nCmdID, (lg_void*)&s_Args, m_pCmdExtra);
	else
		Printf((lg_wchar*)L"No command function specified.");
	
		
}

const lg_wchar* CConsoleU::GetArg(lg_dword nParam, lg_void* pParams)
{
	LC_ARGS* pArgs=(LC_ARGS*)pParams;
	if(nParam>=pArgs->nArgCount)
		return LG_NULL;
	
	return pArgs->szArg[nParam];
}

void CConsoleU::SendCommandf(lg_wchar* szFormat, ...)
{
	va_list arglist;
	va_start(arglist, szFormat);
	_vsnwprintf((wchar_t*)s_szTemp, LC_MAX_LINE_LEN, (wchar_t*)szFormat, arglist);
	va_end(arglist);
	SendCommand(s_szTemp);
}
#endif