#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "lc_con.h"
#include "common.h"
#include "lg_string.h"

lg_char CConsole::s_szTemp[LC_MAX_LINE_LEN+1];
lg_char CConsole::s_szTemp2[LC_MAX_LINE_LEN+1];
//lg_char CConsole::s_szArgs[sizeof(lg_char)*(LC_MAX_CMD_ARGS+1)+LC_MAX_LINE_LEN+1];
CConsole::LC_ARGS  CConsole::s_Args;

CConsole::CConsole():
	m_pNewestLine(LG_NULL),
	m_pOldestLine(LG_NULL),
	m_nLineCount(0),
	m_nGetLineMode(GET_NONE),
	m_pGetLine(LG_NULL),
	m_pCmdExtra(LG_NULL),
	m_pfnCmd(LG_NULL),
	m_nActiveCursorPos(0)
{
	m_szActiveLine[0]=0;
}

CConsole::~CConsole()
{
	Clear();
}

const lg_char* CConsole::GetActiveLine()
{
	return m_szActiveLine;
}

void CConsole::OnChar(lg_char c)
{
	switch(c)
	{
	case '\r':
		SendCommand(m_szActiveLine);
		m_szActiveLine[0]=0;
		m_nActiveCursorPos=0;
		break;
	case '\b':
		if(m_nActiveCursorPos==0)
			break;	
		m_szActiveLine[--m_nActiveCursorPos]=0;
		break;
	case '\n':
		break;
	case '\t':
		OnChar(' ');
		OnChar(' ');
		OnChar(' ');
		break;
	default:
		m_szActiveLine[m_nActiveCursorPos++]=c;
		if(m_nActiveCursorPos>=LC_MAX_LINE_LEN)
			m_nActiveCursorPos=LC_MAX_LINE_LEN-1;
			
		m_szActiveLine[m_nActiveCursorPos]=0;
		break;
	}
}

void CConsole::ListCommands()
{
	Print("Registered commands:");
	for(lg_dword i=0; i<LC_DEF_HASH_SIZE; i++)
	{
		if(!m_Commands.m_pHashList[i].bDefined)
			continue;
			
		CDefs::DEF* pDef=&m_Commands.m_pHashList[i];
		while(pDef)
		{
			Printf("   %s", pDef->szDef);
			pDef=pDef->pHashNext;
		}
	}
}

lg_bool CConsole::RegisterCommand(const lg_char* szCommand, lg_dword nID, const lg_char* szHelpString)
{
	if(!m_Commands.AddDef(szCommand, nID, LC_DEF_NOREDEFINE))
	{
		Printf("LC WARNING: \"%s\" is already a registered command.", szCommand);
		return LG_FALSE;
	}
	return LG_TRUE;	
}

void CConsole::SetCommandFunc(LC_CMD_FUNC pfn, lg_void* pExtra)
{
	m_pfnCmd=pfn;
	m_pCmdExtra=pExtra;
}

const lg_char* CConsole::GetLine(lg_dword nRef, const lg_bool bStartWithNew)
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
const lg_char* CConsole::GetNewestLine()
{
	if(!m_pNewestLine)
		return LG_NULL;
		
	m_pGetLine=m_pNewestLine;
	m_nGetLineMode=GET_OLDER;
	return m_pGetLine->szLine;
}
const lg_char* CConsole::GetOldestLine()
{
	if(!m_pOldestLine)
		return LG_NULL;
		
	m_pGetLine=m_pOldestLine;
	m_nGetLineMode=GET_NEWER;
	return m_pGetLine->szLine;
}
const lg_char* CConsole::GetNextLine()
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

void CConsole::AddEntry(lg_char* szText)
{
	LC_LINE* pNew=new LC_LINE;
	if(!pNew)
		return;
		
	if(szText)
		strncpy(pNew->szLine, szText, LC_MAX_LINE_LEN);
	else
		strncpy(pNew->szLine, "", LC_MAX_LINE_LEN);
		
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

void CConsole::Clear()
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

void CConsole::Print(lg_char* szText)
{
	if(szText)
		AddEntry(szText);
}

void CConsole::Printf(lg_char* szFormat, ...)
{
	va_list arglist;
	va_start(arglist, szFormat);
	_vsnprintf(s_szTemp, LC_MAX_LINE_LEN, szFormat, arglist);
	va_end(arglist);
	Print(s_szTemp);
}

void CConsole::SendCommand(lg_char* szCmd)
{
	lg_dword nCurrent=0;
	strncpy(s_szTemp, szCmd, LC_MAX_LINE_LEN);
	s_szTemp[LC_MAX_LINE_LEN]=0;
	lg_char* szTemp=L_strtokA(s_szTemp, " \t\r\n", '"');
	while(szTemp[0]!=0)
	{
		strncpy(s_Args.szArg[nCurrent], &szTemp[szTemp[0]=='"'?1:0], LC_MAX_LINE_LEN);
		lg_dword nLen=strlen(s_Args.szArg[nCurrent]);
		if(s_Args.szArg[nCurrent][nLen-1]=='"')
		{
			s_Args.szArg[nCurrent][nLen-1]=0;
		}
		
		nCurrent++;
		if(nCurrent>LC_MAX_CMD_ARGS)
			break;
			
		szTemp=L_strtokA(0, 0, 0);
	}
	
	s_Args.nArgCount=nCurrent;
	//If no arguments were found then the command
	//wasn't found so we just print a blank
	//line and that's it.
	
	if(s_Args.nArgCount<1)
	{
		Print("");
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
		Printf("\"%s\" is not recognized as a valid command.", s_Args.szArg[0]);
		return;
	}
	
	if(m_pfnCmd)
		bResult=m_pfnCmd(nCmdID, (lg_void*)&s_Args, m_pCmdExtra);
	else
		Printf("No command function specified.");		
}

const lg_char* CConsole::GetArg(lg_dword nParam, lg_void* pParams)
{
	LC_ARGS* pArgs=(LC_ARGS*)pParams;
	if(nParam>=pArgs->nArgCount)
		return LG_NULL;
	
	return pArgs->szArg[nParam];
}

lg_bool CConsole::CheckArg(const lg_char* string, lg_void* args)
{
	LC_ARGS* pArgs=(LC_ARGS*)args;
	for(lg_dword i=0; i<pArgs->nArgCount; i++)
	{
		if(LG_StrNcCmpA(string, pArgs->szArg[i], -1)==0)
			return LG_TRUE;
	}
	return LG_FALSE;
}

void CConsole::SendCommandf(lg_char* szFormat, ...)
{
	va_list arglist;
	va_start(arglist, szFormat);
	_vsnprintf(s_szTemp, LC_MAX_LINE_LEN, szFormat, arglist);
	va_end(arglist);
	SendCommand(s_szTemp);
}