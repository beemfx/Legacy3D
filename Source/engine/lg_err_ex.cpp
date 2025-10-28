#include <stdio.h>
#include <stdarg.h>
#include "lg_err_ex.h"
#include "lg_func.h"
#include "common.h"

typedef char ERR_TEXT[32];

ERR_TEXT g_szErr[LG_ERR_ERRCOUNT]={
	"UNKNOWN ERROR",
	"DEFAULT ERROR",
	"OUT OF MEMORY",
	"ASSERTION ERROR"};

#if 0
CLError::CLError():
	m_nErrorCode(LG_ERR_UNKNOWN)
{
	m_szError[0]=0;
}
#endif

CLError::CLError(LG_ERR nErrorCode, char* szFile, lg_dword nLine, char* szMsg):
	m_nErrorCode(nErrorCode)
{
	_snprintf(m_szError, LG_ERR_MSG_SIZE, "ERROR 0x%08X (%s) %s (LINE %d)", nErrorCode, g_szErr[nErrorCode], szFile, nLine);
	if(szMsg)
		L_strncat(m_szError, szMsg, LG_ERR_MSG_SIZE);
}
/*
CLError::CLError(char *szError, lg_dword nErrorCode):
	m_nErrorCode(nErrorCode)
{
	LG_strncpy(this->m_szError, szError, LG_ERR_MSG_SIZE);
	this->m_nErrorCode=nErrorCode;
}

CLError::CLError(lg_str szError, lg_dword nErrorCode, lg_str szFile, lg_dword nLine)
{
	_snprintf(m_szError, LG_ERR_MSG_SIZE, "ERROR 0x%08X: %s %s (LINE %d)", nErrorCode, szError, szFile, nLine);
}
*/
void CLError::Catenate(LG_ERR nErrorCode, char* szFile, lg_dword nLine, char* szMsg)
{
	lg_char szTemp[LG_ERR_MSG_SIZE+1];
	LG_strncpy(szTemp, m_szError, LG_ERR_MSG_SIZE);
	_snprintf(m_szError, LG_ERR_MSG_SIZE, "%s >>\nERROR 0x%08X: %s %s (LINE %d)", szTemp, nErrorCode, g_szErr[nErrorCode], szFile, nLine);
	if(szMsg)
		L_strncat(m_szError, szMsg, LG_ERR_MSG_SIZE);
}

void CLError::Catenate(lg_str format, ...)
{
	char szTemp[LG_ERR_MSG_SIZE+1];
	LG_strncpy(szTemp, m_szError, LG_ERR_MSG_SIZE);
	
	va_list arglist=LG_NULL;
	va_start(arglist, format);
	_vsnprintf(m_szError, LG_ERR_MSG_SIZE, format, arglist);
	L_strncat(m_szError, " -->\n", LG_ERR_MSG_SIZE);
	L_strncat(m_szError, szTemp, LG_ERR_MSG_SIZE);
	va_end(arglist);
}

lg_cstr CLError::GetMsg()
{
	return m_szError;
}
