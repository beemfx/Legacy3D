#include "lg_func.h"
#include <string.h>
#include <stdlib.h>

lg_char* LG_GetShortNameFromPathA(lg_char* szName, const lg_char* szFilename)
{
	lg_dword dwLen=strlen(szFilename);
	lg_dword i=0, j=0;
	
	for(i=dwLen; i>0; i--)
	{
		if(szFilename[i]=='\\' || szFilename[i]=='/')
		{
			i++;
			break;
		}
	}
	for(j=0 ;i<dwLen+1; i++, j++)
	{
		szName[j]=szFilename[i];
		if(szName[j]=='.')
		{
			szName[j]=0;
			break;
		}
	}
	return szName;
}

/* LG_HashFilename: See remarks in lg_func.h*/
lg_dword LG_HashFilename(lg_char* szPath)
{
	lg_dword nLen=strlen(szPath);
	
	//This is basically Bob Jenkins' One-At-A-Time-Hash.
	//http://www.burtleburtle.net/bob/hash/doobs.html
	
	lg_dword nHash=0;
	lg_dword i=0;
	
	for(i=nLen; (i>=0) && (szPath[i]!='\\') && (szPath[i]!='/'); i--)
	{
		nHash+=szPath[i];
		nHash+=(nHash<<10);
		nHash^=(nHash>>6);
	}
	nHash+=(nHash<<3);
	nHash^=(nHash>>11);
	nHash+=(nHash<<15);

	return nHash;
}

/*LG_RandomSeed: See remarks in lg_func.h*/
void LG_RandomSeed(lg_uint Seed)
{
	srand(Seed);
}
/*LG_RandomFloat: See remarks in lg_func.h*/
lg_float LG_RandomFloat(lg_float fMin, lg_float fMax)
{
	return (lg_float)rand() / (RAND_MAX + 1) * (fMax - fMin) + fMin;
}
/*LG_RandomLong: See remarks in lg_func.h*/
lg_long LG_RandomLong(lg_long nMin, lg_long nMax)
{
	return rand()%(nMax-nMin+1) + nMin;
}

/* LG_strncpy: See remarks in lg_func.h */
lg_char* LG_strncpy(lg_char* strDest, const lg_char* strSrc, lg_size_t count)
{
	#if 1
	strncpy(strDest, strSrc, count);
	strDest[count]=0;
	return strDest;
	#else
	unsigned long i=0;

	if((strDest==LG_NULL) || (strSrc==LG_NULL))
		return 0;

	for(i=0; strSrc[i]!=0, i<count; i++)
	{
		strDest[i]=strSrc[i];
	}
	if(i<count)
		strDest[i]=0;
	else
		strDest[count-1]=0;
	return i;
	#endif
}

/* LG_wcsncpy: See remarks in lg_func.h */
lg_wchar_t* LG_wcsncpy(lg_wchar_t* strDest, const lg_wchar_t* strSrc, lg_size_t count)
{
	wcsncpy(strDest, strSrc, count);
	strDest[count]=0;
	return strDest;
}