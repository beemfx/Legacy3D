#ifndef __LG_ERR_EX_H__
#define __LG_ERR_EX_H__

#define LG_ERR_MSG_SIZE 1024

#include "lg_types.h"
typedef enum _LG_ERR
{
	LG_ERR_UNKNOWN=0,
	LG_ERR_DEFAULT,
	LG_ERR_OUTOFMEMORY,
	LG_ERR_INVALID_PARAM,
	LG_ERR_UNEEDED_CLASS,
	LG_ERR_ASSERT_FAIL,
	LG_ERR_ERRCOUNT
}LG_ERR;

#define LG_ERROR(type, msg) CLError(type, __FILE__, __LINE__, msg)
#define LG_ASSERT(p, msg) if(!p){throw CLError(LG_ERR_ASSERT_FAIL, __FILE__, __LINE__, msg);}

/*
#define LG_ERR_DEFAULT 0x00000001
#define LG_ERR_OUTOFMEMORY 0x00000002
*/
class CLError
{
private:
	char m_szError[LG_ERR_MSG_SIZE+1];
	LG_ERR m_nErrorCode;
public:
	//CLError();
	CLError(LG_ERR nErrorCode, char* szFilename, lg_dword nLine, char* szMsg=LG_NULL);
	/*
	CLError(char* szError, lg_dword nErrorCode);
	CLError(char* szError, lg_dword nErrorCode, char* szFile, lg_dword nLine);
	*/
	void Catenate(LG_ERR nErrorCode, char* szFile, lg_dword nLine, char* szMsg=LG_NULL);
	//void Catenate(char* szError, lg_dword nErrorCode, char* szFile, lg_dword nLine);
	void Catenate(char* Format, ...);
	const char* GetMsg();
};

#define ERROR_CODE(code, msg) CLError(code, __FILE__, __LINE__, msg)

#endif __LG_ERR_EX_H__