#if 0
#include <stdio.h>
#include <string.h>
#include "lc_sys2.h"
#include "common.h"


class CDefsU
{
friend class CConsoleU;
private:
	struct DEF_VALUE{
		lg_float f;
		lg_int   u;
	};
	
	struct DEF{
		lg_wchar       szDef[LC_MAX_DEF_LEN+1];
		DEF_VALUE      nValue;
		DEF*           pHashNext;
		lg_bool        bDefined;
	};
private:
	DEF* m_pHashList;
	lg_dword m_nDefCount;
	
	lg_dword GenDefHash(const lg_wchar* def);
	lg_bool AddDef(const lg_wchar* szDef, DEF_VALUE* pValue, lg_dword Flags=0);
	DEF_VALUE* GetDef(const lg_wchar* szDef);
public:
	CDefsU();
	~CDefsU();
	lg_bool AddDef(const lg_wchar* szDef, lg_float value, lg_dword Flags=0);
	lg_bool AddDef(const lg_wchar* szDef, lg_dword value, lg_dword Flags=0);
	lg_bool AddDef(const lg_wchar* szDef, lg_long value, lg_dword Flags=0);
	void Clear();
	lg_dword GetDefUnsigned(const lg_wchar* def, lg_bool* bGot);
	lg_long  GetDefSigned(const lg_wchar* def, lg_bool* bGot);
	lg_float GetDefFloat(const lg_wchar* def, lg_bool* bGot);
};

CDefsU::CDefsU():
	m_pHashList(LG_NULL),
	m_nDefCount(0)
{
	m_pHashList=new DEF[LC_DEF_HASH_SIZE];
	for(lg_dword i=0; i<LC_DEF_HASH_SIZE; i++)
	{
		m_pHashList[i].bDefined=LG_FALSE;
	}
}


CDefsU::~CDefsU()
{
	Clear();
	
	if(m_pHashList)
	{
		delete[]m_pHashList;
	}
	//printf("%d definitions.\n", m_nDefCount);
}

CDefsU::DEF_VALUE* CDefsU::GetDef(const lg_wchar* szDef)
{
	lg_dword nHash=GenDefHash(szDef)%LC_DEF_HASH_SIZE;
	if(!m_pHashList[nHash].bDefined)
		return LG_NULL;
		
	DEF* pDef=&m_pHashList[nHash];
	while(pDef)
	{
		if(wcsnicmp((wchar_t*)pDef->szDef, (wchar_t*)szDef, LC_MAX_DEF_LEN)==0)
			return &pDef->nValue;
		pDef=pDef->pHashNext;
	}
	return LG_NULL;
}

lg_dword CDefsU::GetDefUnsigned(const lg_wchar* def, lg_bool* bGot)
{
	DEF_VALUE* pValue=GetDef(def);
	
	if(bGot)
		*bGot=pValue?LG_TRUE:LG_FALSE;
		
	if(pValue)
		return (lg_dword)pValue->u;
	return 0;
}
lg_long CDefsU::GetDefSigned(const lg_wchar* def, lg_bool* bGot)
{
	DEF_VALUE* pValue=GetDef(def);
	
	if(bGot)
		*bGot=pValue?LG_TRUE:LG_FALSE;
		
	if(pValue)
		return (lg_long)pValue->u;
	return 0;
}

lg_float CDefsU::GetDefFloat(const lg_wchar* def, lg_bool* bGot)
{
	DEF_VALUE* pValue=GetDef(def);
	
	if(bGot)
		*bGot=pValue?LG_TRUE:LG_FALSE;
		
	if(pValue)
		return (lg_float)pValue->f;
	return 0.0f;
}

lg_bool CDefsU::AddDef(const lg_wchar* szDef, DEF_VALUE* pValue, lg_dword Flags)
{
	//First check to see if the definition has already been defined.
	DEF_VALUE* val=GetDef(szDef);
	if(val)
	{
		if(L_CHECK_FLAG(Flags, LC_DEF_NOREDEFINE))
		{
			//printf("Can't redifine.\n");
			return LG_FALSE;
		}
		else
		{
			//printf("Redefining.\n");
			*val=*pValue;
			return LG_TRUE;
		}
	}
	
	lg_dword nHash=GenDefHash(szDef)%LC_DEF_HASH_SIZE;
	//LC_Printf("The hash for %s is %d", szDef, nHash);
	
	if(!m_pHashList[nHash].bDefined)
	{
		//If there is no definition in the hash slot we just
		//stick the definition in the hash slot.
		m_pHashList[nHash].bDefined=LG_TRUE;
		m_pHashList[nHash].nValue=*pValue;
		m_pHashList[nHash].pHashNext=LG_NULL;
		wcsncpy((wchar_t*)m_pHashList[nHash].szDef, (wchar_t*)szDef, LC_MAX_DEF_LEN);
	}
	else
	{
		//Add the new item (we know it isn't a redefinition because we
		//already checked).
		DEF* pNew=new DEF;
		if(!pNew)
			return LG_FALSE;
			
		pNew->bDefined=LG_TRUE;
		pNew->nValue=*pValue;
		pNew->pHashNext=LG_NULL;
		wcsncpy((wchar_t*)pNew->szDef, (wchar_t*)szDef, LC_MAX_DEF_LEN);
		
		pNew->pHashNext=m_pHashList[nHash].pHashNext;
		m_pHashList[nHash].pHashNext=pNew;
		
	}
	m_nDefCount++;
	return LG_TRUE;
}

lg_bool CDefsU::AddDef(const lg_wchar *szDef, lg_dword nValue, lg_dword Flags)
{
	DEF_VALUE v={(lg_float)nValue, nValue};
	return AddDef(szDef, &v, Flags);
}

lg_bool CDefsU::AddDef(const lg_wchar *szDef, lg_long nValue, lg_dword Flags)
{
	DEF_VALUE v={(lg_float)nValue, nValue};
	return AddDef(szDef, &v, Flags);
}

lg_bool CDefsU::AddDef(const lg_wchar *szDef, lg_float fValue, lg_dword Flags)
{
	DEF_VALUE v={fValue, (lg_long)fValue};
	return AddDef(szDef, &v, Flags);
}

void CDefsU::Clear()
{
	if(!m_pHashList)
		return;
		
	for(lg_dword i=0; i<LC_DEF_HASH_SIZE; i++)
	{
		//If the slot doesn't have a definition
		//then there are no child subsequient definitions
		//with the same hash value, so it is safe to continue
		//down the list.
		if(!m_pHashList[i].bDefined)
			continue;
			
		DEF* pDef=m_pHashList[i].pHashNext;
		while(pDef)
		{
			DEF* pTemp=pDef->pHashNext;
			delete pDef;
			pDef=pTemp;
			m_nDefCount--;
		}
		m_pHashList[i].bDefined=LG_FALSE;
		m_nDefCount--;
	}
}

lg_dword CDefsU::GenDefHash(const lg_wchar* def)
{
	lg_dword nLen=LG_Min(wcslen((wchar_t*)def), LC_MAX_DEF_LEN);
	lg_dword nHash=0;
	lg_dword i=0;
	lg_wchar c;
	
	for(i=0; i<nLen; i++)
	{
		c=def[i];
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
#endif