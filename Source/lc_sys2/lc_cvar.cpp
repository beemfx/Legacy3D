#include <string.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "lc_cvar.h"
#include "lg_string.h"
#include "common.h"

CCvarList::CCvarList():
	m_pHashList(LG_NULL)
{
	m_pHashList=new lg_cvar[LC_CVAR_HASH_SIZE];
	if(!m_pHashList)
		return;
		
	for(lg_dword i=0; i<LC_CVAR_HASH_SIZE; i++)
	{
		m_pHashList[i].Flags=CVAR_EMPTY;
		m_pHashList[i].pHashNext=LG_NULL;
	}
}

CCvarList::~CCvarList()
{
	if(!m_pHashList)
		return;
		
	for(lg_dword i=0; i<LC_CVAR_HASH_SIZE; i++)
	{
		lg_cvar* pCvar=m_pHashList[i].pHashNext;
		while(pCvar)
		{
			lg_cvar* pt=pCvar->pHashNext;
			delete pCvar;
			pCvar=pt;
		}
	}
	
	delete[]m_pHashList;
}

lg_cvar* CCvarList::GetFirst()
{
	for(m_nLastGotten=0; m_nLastGotten<LC_CVAR_HASH_SIZE; m_nLastGotten++)
	{
		if(m_pHashList[m_nLastGotten].Flags!=CVAR_EMPTY)
		{
			m_pLastGotten=&m_pHashList[m_nLastGotten];
			return m_pLastGotten;
		}
	}
	return LG_NULL;
}
lg_cvar* CCvarList::GetNext()
{
	m_pLastGotten=m_pLastGotten->pHashNext;
	if(m_pLastGotten)
		return m_pLastGotten;
		
	for(++m_nLastGotten; m_nLastGotten<LC_CVAR_HASH_SIZE; m_nLastGotten++)
	{
		if(m_pHashList[m_nLastGotten].Flags!=CVAR_EMPTY)
		{
			m_pLastGotten=&m_pHashList[m_nLastGotten];
			return m_pLastGotten;
		}
	}
	return LG_NULL;
}

lg_dword CCvarList::DefToUnsigned(const lg_char* szDef)
{
	CDefs::DEF_VALUE*v=m_Defs.GetDef(szDef);
	if(!v)
		return (lg_dword)atol(szDef);
	return (lg_dword)v->u;
}
lg_long  CCvarList::DefToLong(const lg_char* szDef)
{
	CDefs::DEF_VALUE*v=m_Defs.GetDef(szDef);
	if(!v)
		return (lg_long)atol(szDef);
	return (lg_long)v->u;
}
lg_float CCvarList::DefToFloat(const lg_char* szDef)
{
	CDefs::DEF_VALUE*v=m_Defs.GetDef(szDef);
	if(!v)
		return (lg_float)atof(szDef);
	return (lg_float)v->u;
}

lg_dword CCvarList::GenHash(const lg_char* szName)
{
	lg_dword nLen=LG_Min(LG_StrLen(szName), LC_CVAR_MAX_LEN);
	lg_dword nHash=0;
	lg_dword i=0;
	lg_tchar c;
	
	for(i=0; i<nLen; i++)
	{
		c=szName[i];
		//Insure that the hash value is case insensitive
		//by capitalizing lowercase letters.
		if((c>='a') && (c<='z'))
			c-='a'-'A';
			
		nHash+=c;
		nHash+=(nHash<<10);
		nHash^=(nHash>>6);
	}
	nHash+=(nHash<<3);
	nHash^=(nHash>>11);
	nHash+=(nHash<<15);
	return nHash;
}
lg_cvar* CCvarList::Get(const lg_char* szName)
{
	lg_dword nHash=GenHash(szName)%LC_CVAR_HASH_SIZE;
	
	lg_cvar* p=&m_pHashList[nHash];
	
	if(L_CHECK_FLAG(p->Flags, CVAR_EMPTY))
		return LG_NULL;
		
	while(p)
	{
		if(LG_StrNcCmpA(p->szName, szName, -1)==0)
			return p;
			
		p=p->pHashNext;
	}
	return LG_NULL;
}

lg_cvar* CCvarList::Register(const lg_char* szName, const lg_char* szValue, lg_dword Flags)
{
	if(!LC_CheckName(szName))
		return LG_NULL;
		
	if(Get(szName))
	{
		//Cvar is already registered.
		return LG_NULL;
	}
	lg_dword nHash=GenHash(szName)%LC_CVAR_HASH_SIZE;
	//Add the cvar.
	if(m_pHashList[nHash].Flags==CVAR_EMPTY)
	{
		//Add the cvar...
		LG_StrCopySafeA(m_pHashList[nHash].szName, szName, LC_CVAR_MAX_LEN);
		m_pHashList[nHash].Flags=0;
		Set(&m_pHashList[nHash], szValue, &m_Defs);
		m_pHashList[nHash].pHashNext=LG_NULL;
		m_pHashList[nHash].Flags=Flags;
		return &m_pHashList[nHash];
	}
	else
	{
		lg_cvar* pNew=new lg_cvar;
		if(!pNew)
			return LG_NULL;
			
		LG_StrCopySafeA(pNew->szName, szName, LC_CVAR_MAX_LEN);
		pNew->Flags=0;
		Set(pNew, szValue, &m_Defs);
		pNew->Flags=Flags;
		pNew->pHashNext=m_pHashList[nHash].pHashNext;
		m_pHashList[nHash].pHashNext=pNew;
		return pNew;
	}
}

lg_bool CCvarList::Define(lg_char* szDef, lg_float fValue)
{
	return m_Defs.AddDef(szDef, fValue);
}
lg_bool CCvarList::Define(lg_char* szDef, lg_long  nValue)
{
	return m_Defs.AddDef(szDef, nValue);
}
lg_bool CCvarList::Define(lg_char* szDef, lg_dword nValue)
{
	return m_Defs.AddDef(szDef, nValue);
}


void CCvarList::Set(lg_cvar* cvar, const lg_char* szValue, CDefs* pDefs)
{
	if(L_CHECK_FLAG(cvar->Flags, CVAR_ROM))
		return;
		
	LG_StrCopySafeA(cvar->szValue, szValue, LC_CVAR_MAX_LEN);
	if(cvar->szValue[0]=='0' && (cvar->szValue[1]=='x' || cvar->szValue[1]=='X'))
	{
		cvar->nValue=L_axtol((lg_char*)szValue);
		cvar->fValue=(lg_float)(lg_dword)cvar->nValue;
		return;
	}
	
	CDefs::DEF_VALUE*v=pDefs->GetDef(szValue);
	if(!v)
	{
		cvar->nValue=atoi(szValue);
		cvar->fValue=(lg_float)atof(szValue);
	}
	else
	{
		cvar->nValue=v->u;
		cvar->fValue=v->f;
	}
}


void CCvarList::Set(lg_cvar* cvar, lg_float fValue)
{
	if(L_CHECK_FLAG(cvar->Flags, CVAR_ROM))
		return;
		
	cvar->fValue=fValue;
	cvar->nValue=(lg_long)fValue;
	_snprintf(cvar->szValue, LC_CVAR_MAX_LEN, "%f", fValue);
}

void CCvarList::Set(lg_cvar* cvar, lg_dword nValue)
{
	if(L_CHECK_FLAG(cvar->Flags, CVAR_ROM))
		return;
		
	cvar->nValue=nValue;
	cvar->fValue=(lg_float)nValue;
	_snprintf(cvar->szValue, LC_CVAR_MAX_LEN, "0x%08X", nValue);
}

void CCvarList::Set(lg_cvar* cvar, lg_long nValue)
{
	if(L_CHECK_FLAG(cvar->Flags, CVAR_ROM))
		return;
		
	cvar->nValue=nValue;
	cvar->fValue=(lg_float)nValue;
	_snprintf(cvar->szValue, LC_CVAR_MAX_LEN, "%d", nValue);
}

