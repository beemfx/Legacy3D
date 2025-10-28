#ifndef __LC_CVAR_H__
#define __LC_CVAR_H__

#include "lc_def.h"
#include "lg_types.h"
#include "lc_sys2.h"


class CCvarList
{
friend void LC_FUNC CV_Set(lg_cvar* cvar, const lg_char* szValue);
friend void LC_FUNC LC_ListCvars(const lg_char* szLimit);
private:
	lg_cvar* m_pHashList;
	CDefs    m_Defs;
	
	lg_dword m_nLastGotten;
	lg_cvar* m_pLastGotten;
	
	lg_dword GenHash(const lg_char* szName);
	lg_dword DefToUnsigned(const lg_char* szDef);
	lg_long  DefToLong(const lg_char* szDef);
	lg_float DefToFloat(const lg_char* szDef);
public:
	CCvarList();
	~CCvarList();
	
	lg_cvar* Register(const lg_char* szName, const lg_char* szValue, lg_dword Flags);
	
	lg_cvar* Get(const lg_char* szName);
	
	lg_cvar* GetFirst();
	lg_cvar* GetNext();
	
	lg_bool Define(lg_char* szDef, lg_float fValue);
	lg_bool Define(lg_char* szDef, lg_long  nValue);
	lg_bool Define(lg_char* szDef, lg_dword nValue);
	
	static void Set(lg_cvar* cvar, const lg_char* szValue, CDefs* pDefs);
	static void Set(lg_cvar* cvar, lg_float fValue);
	static void Set(lg_cvar* cvar, lg_dword nValue);
	static void Set(lg_cvar* cvar, lg_long nValue);
};

#endif __LC_CVAR_H__