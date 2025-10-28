#include <stdio.h>
#include <string.h>
#include "lc_def.h"
#include "common.h"


lg_bool LC_CheckName(const lg_char* szName)
{
	lg_dword nLen=strlen(szName);
	/* The idea behind the default checks, is that the first
		number cannot be a number, and no special characters are
		allowed in the name, this is similar to what c language
		allows. */
	lg_char szNoFirst[]="0123456789";
	lg_char szNoAllow[]="{}[]\'\";:.,<>?/\\`~=+)(*&$^%#@!~ ";
	
	/* Check to see if the first character is a no allow. */
	for(lg_dword i=0, lenallow=strlen(szNoFirst); i<lenallow; i++)
	{
		if(szName[0]==szNoFirst[i])
			return LG_FALSE;
	}

	/* Now make sure no invalid characters are in the string. */
	for(lg_dword i=0, lenallow=strlen(szNoAllow); i<lenallow; i++)
	{
		for(lg_dword j=0; j<nLen; j++)
		{
			if(szName[j]==szNoAllow[i])
				return LG_FALSE;;
		}
	}
	
	return LG_TRUE;
}

CDefs::CDefs():
	m_pHashList(LG_NULL),
	m_nDefCount(0)
{
	m_pHashList=new DEF[LC_DEF_HASH_SIZE];
	for(lg_dword i=0; i<LC_DEF_HASH_SIZE; i++)
	{
		m_pHashList[i].bDefined=LG_FALSE;
	}
}


CDefs::~CDefs()
{
	Clear();
	
	if(m_pHashList)
	{
		delete[]m_pHashList;
	}
	//printf("%d definitions.\n", m_nDefCount);
}

CDefs::DEF_VALUE* CDefs::GetDef(const lg_char* szDef)
{
	lg_dword nHash=GenDefHash(szDef)%LC_DEF_HASH_SIZE;
	if(!m_pHashList[nHash].bDefined)
		return LG_NULL;
		
	DEF* pDef=&m_pHashList[nHash];
	while(pDef)
	{
		if(strnicmp(pDef->szDef, szDef, LC_MAX_DEF_LEN)==0)
			return &pDef->nValue;
		pDef=pDef->pHashNext;
	}
	return LG_NULL;
}

lg_dword CDefs::GetDefUnsigned(const lg_char* def, lg_bool* bGot)
{
	DEF_VALUE* pValue=GetDef(def);
	
	if(bGot)
		*bGot=pValue?LG_TRUE:LG_FALSE;
		
	if(pValue)
		return (lg_dword)pValue->u;
	return 0;
}
lg_long CDefs::GetDefSigned(const lg_char* def, lg_bool* bGot)
{
	DEF_VALUE* pValue=GetDef(def);
	
	if(bGot)
		*bGot=pValue?LG_TRUE:LG_FALSE;
		
	if(pValue)
		return (lg_long)pValue->u;
	return 0;
}

lg_float CDefs::GetDefFloat(const lg_char* def, lg_bool* bGot)
{
	DEF_VALUE* pValue=GetDef(def);
	
	if(bGot)
		*bGot=pValue?LG_TRUE:LG_FALSE;
		
	if(pValue)
		return (lg_float)pValue->f;
	return 0.0f;
}

lg_bool CDefs::AddDef(const lg_char* szDef, DEF_VALUE* pValue, lg_dword Flags)
{
	if(!LC_CheckName(szDef))
		return LG_FALSE;
		
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
		strncpy(m_pHashList[nHash].szDef, szDef, LC_MAX_DEF_LEN);
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
		strncpy(pNew->szDef, szDef, LC_MAX_DEF_LEN);
		
		pNew->pHashNext=m_pHashList[nHash].pHashNext;
		m_pHashList[nHash].pHashNext=pNew;
		
	}
	m_nDefCount++;
	return LG_TRUE;
}

lg_bool CDefs::AddDef(const lg_char *szDef, lg_dword nValue, lg_dword Flags)
{
	DEF_VALUE v={(lg_float)nValue, nValue};
	return AddDef(szDef, &v, Flags);
}

lg_bool CDefs::AddDef(const lg_char *szDef, lg_long nValue, lg_dword Flags)
{
	DEF_VALUE v={(lg_float)nValue, nValue};
	return AddDef(szDef, &v, Flags);
}

lg_bool CDefs::AddDef(const lg_char *szDef, lg_float fValue, lg_dword Flags)
{
	DEF_VALUE v={fValue, (lg_long)fValue};
	return AddDef(szDef, &v, Flags);
}

void CDefs::Clear()
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

lg_dword CDefs::GenDefHash(const lg_char* def)
{
	lg_dword nLen=LG_Min(strlen(def), LC_MAX_DEF_LEN);
	lg_dword nHash=0;
	lg_dword i=0;
	lg_char c;
	
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