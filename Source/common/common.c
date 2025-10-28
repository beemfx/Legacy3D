/***************************************************************
	File: common.c
	Copyright (c) 2006, Blaine Myers

	Purpose: All of the types and functions that are universal
	are contained in common.c and common.h.  These two files
	are used not just for Legacy Engine, but for lc_sys.dll and
	la_sys.dll, as well as others.
***************************************************************/
#include "common.h"
//#include <direct.h>

#include <stdlib.h>

#ifdef _DEBUG
#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
//void __stdcall OutputDebugStringA(const char* lpOutputString);
#ifdef _UNICODE
//void __stdcall OutputDebugStringW(const char* lpOutputString);
#endif _UNICODE
#endif /*_DEBUG*/



lg_char* L_GetNameFromPath(lg_char* szName, const char* szFilename)
{
	lg_dword dwLen=L_strlenA(szFilename);
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
	}
	return szName;
}

lg_char* L_GetShortNameFromPath(lg_char* szName, const char* szFilename)
{
	lg_dword dwLen=L_strlenA(szFilename);
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

char* L_GetPathFromPath(char* szDir, const char* szFilename)
{
	lg_dword dwLen=L_strlenA(szFilename);
	lg_long i=0;
	for(i=(lg_long)dwLen-1; i>=0; i--)
	{
		if(szFilename[i]=='\\' || szFilename[i]=='/')
			break;
	}
	L_strncpy(szDir, szFilename, i+2);
	return szDir;
}

char* L_strncat(char* szDest, const char* szSrc, unsigned long nMax)
{
	lg_dword dwLen=L_strlenA(szDest);
	L_strncpy(&szDest[dwLen], szSrc, nMax-dwLen);
	return szDest;
}

/******************************************************************* 
	L_CopyString()
	 
	Copies a string, up till a 0 is reached or
	the max is reached whichever comes first. Appends a null
	terminating character at the end of the string. Returns the
	number of characters copied.
*******************************************************************/
unsigned long L_strncpy(char* szOut, const char* szIn, unsigned long nMax)
{
	unsigned long i=0;

	if((szOut==LG_NULL) || (szIn==LG_NULL))
		return 0;

	for(i=0; szIn[i]!=0, i<nMax; i++)
	{
		szOut[i]=szIn[i];
	}
	if(i<nMax)
		szOut[i]=0;
	else
		szOut[nMax-1]=0;
	return i;
}

/**********************************************
	lg_strnlen()

	Returns the length of the specified string.
**********************************************/
lg_dword L_strlenA(const lg_char* string)
{
	lg_dword count=0;
	while (string[count])
		count++;
	return count;
}

lg_dword L_strlenW(const lg_wchar* string)
{
	lg_dword count=0;
	while (string[count])
		count++;
	return count;
}

/************************************************************ 
	L_strnicmp()
 
	Compare two strings without regard to case of a certain
	number of characters long.  Returns 0 if not the same, 
	otherwize returns nonzero.  This function is not used in
	exactly the same way that the _strnicmp stdlib function 
	is used.
************************************************************/
int L_strnicmp(const char* szOne, const char* szTwo, unsigned long nNum)
{
	unsigned long i=0;
	unsigned long nLen1=0, nLen2=0;
	char c1=0, c2=0;

	if((szOne==LG_NULL) || (szTwo==LG_NULL))
		return 0;

	nLen1=L_strlenA(szOne);
	nLen2=L_strlenA(szTwo);

	if(nNum>0)
	{
		if(nLen1>nNum)
			nLen1=nNum;
		if(nLen2>nNum)
			nLen2=nNum;
	}

	if(nLen1!=nLen2)
		return 0;

	for(i=0; i<nLen1; i++)
	{
		if(szOne[i]!=szTwo[i])
		{
			c1=szOne[i];
			c2=szTwo[i];
			
			if( (c1>='a') && (c1<='z') )
				c1 -= ('a'-'A');
			if( (c2>='a') && (c2<='z') )
				c2 -= ('a'-'A');
			if(c1==c2)
				continue;
			
			return 0;
		}
	}
	return 1;
}

/***************************************************** 
	L_axtol() 

	Converts a hexidecimal string to a unsigned long. 
*****************************************************/
unsigned long L_axtol(char* string)
{
	int i=0;
	char c=0;
	unsigned long nHexValue=0;
	/*int sign=1;*/

	/*sign=(string[0]=='-')?-1:1;*/

	/* Make sure it is actually a hex string. */
	if(!L_strnicmp("0X", string, 2)/* && !L_StringICompare("-0X", string, 3)*/)
		return 0x00000000l;

	/* The idea behind this is keep adding to value, until
		we reach an invalid character, or the end of the string. */
	for(i=2/*(sign>0)?2:3*/, nHexValue=0x00000000; string[i]!=0; i++)
	{
		c=string[i];
		if(c>='0' && c<='9')
			nHexValue=(nHexValue<<4)|(c-'0');
		else if(c>='a' && c<='f')
			nHexValue=(nHexValue<<4)|(c-'a'+10);
		else if(c>='A' && c<='F')
			nHexValue=(nHexValue<<4)|(c-'A'+10);
		else
			break;
	}
	return /*sign**/nHexValue;
}

/*******************************************
	L_atof()
	 
	Converts a string to a floating point. 
*******************************************/
float L_atof(char* string)
{	/* Check for a hexidecimal value. */
	int sign=1;
	int i=0;
	float fValue=0.0f;
	signed long total=0, decimal=0;
	double value=0.0f;
	char c=0;

	/* Check for sign. */
	sign=(string[0]=='-')?-1:1;

	for(i=(sign>0)?0:1, total=0, value=0.0f, decimal=-1; ;i++)
	{
		c=string[i];
		if(c=='.')
		{
			decimal=total;
			continue;
		}
		if(c<'0' || c>'9')
			break;

		value=value*10+(c-'0');
		total++;
	}
	if (decimal==-1)
		return (float)(value*sign);
	while(total>decimal)
	{
		value /= 10;
		total--;
	}
	return(float)(value*sign);
}

/*************************************************************
	L_atovalue()
	 
	Converts a string to a floating point, but checks to see
	if the value is a hexidecimal. 
*************************************************************/
float L_atovalue(char* string)
{
	/* Check to see if it is a hex. */
	if((string[0]=='0') && (string[1]=='x' || string[1]=='X'))
		return (float)L_axtol(string);

	/* Figure its a float*/
	return (float)L_atof(string);
}

/**************************************** 
	L_atol() 
	
	Converts a string to a long. 
*****************************************/
signed long L_atol(char* string)
{
	signed long lValue=0;
	char c=0;
	int i=0;
	int sign=1;

	sign=(string[0]=='-')?-1:1;

	for(i=(sign>0)?0:1, lValue=0; ;i++)
	{
		c=string[i];
		if(c<'0' || c>'9')
			break;

		lValue=lValue*10+c-'0';
	}

	return sign*lValue;
}

/*************************************
	lg_strtok
	
	Replacement for stdc strtok
*************************************/
char* L_strtokA(char* strToken, char* strDelimit, char cIgIns)
{
	static char* szString=LG_NULL;
	static char* szDelimit=LG_NULL;
	static lg_dword nPosition=0;
	static char cIgnore=0;
	
	lg_bool bIgnoring=LG_FALSE;
	
	lg_bool bFoundFirst=LG_FALSE;
	lg_bool bIgnoreIsDel=LG_FALSE;
	
	lg_dword i=0, j=0;
	lg_dword nStrLen=0, nDelLen=0;
	
	cIgnore=(cIgIns==' ' || cIgIns==0)?cIgnore:cIgIns;
	
	
	if(strToken)
	{
		szString=strToken;
		nPosition=0;
	}
	
	if(strDelimit)
	{
		szDelimit=strDelimit;
	}
	
	if(!szString || !szDelimit)
		return LG_NULL;
		
	szString=&szString[nPosition];
	
	nDelLen=L_strlenA(szDelimit);
	nStrLen=L_strlenA(szString);
	
	for(i=0; i<nDelLen; i++)
	{
		if(cIgnore==szDelimit[i])
			bIgnoreIsDel=LG_TRUE;
	}
		
	for(i=0; i<nStrLen ;i++)
	{
		if(szString[i]==cIgnore)
		{
			bIgnoring=!bIgnoring;
			if(!bFoundFirst)
			{
				if(bIgnoreIsDel)
					i++;
				szString=&szString[i];
				i=0;
				nStrLen=L_strlenA(szString);
				bFoundFirst=LG_TRUE;
			}
		}
		
		if(bIgnoring)
			continue;
			
		if(bFoundFirst)
		{
			for(j=0; j<nDelLen; j++)
			{
				if(bFoundFirst)
				{
					if(szString[i]==szDelimit[j])
					{
						szString[i]=0;
						nPosition=i+1;
						return szString;
					}
				}
			}
		}
		else
		{
			for(j=0; j<nDelLen; j++)
			{
				if(szString[i]==szDelimit[j])
				{
					bFoundFirst=LG_TRUE;
				}
			}
			if(bFoundFirst)
				bFoundFirst=LG_FALSE;
			else
			{
				szString=&szString[i];
				i=0;
				nStrLen=L_strlenA(szString);
				bFoundFirst=LG_TRUE;
			}
		}
	}
	//If we got to the end of the string...
	nPosition=nStrLen;
	return szString;
}

lg_wchar* L_strtokW(lg_wchar* strToken, lg_wchar* strDelimit, lg_wchar cIgIns)
{
	static lg_wchar* szString=LG_NULL;
	static lg_wchar* szDelimit=LG_NULL;
	static lg_dword nPosition=0;
	static lg_wchar cIgnore=0;
	
	lg_bool bIgnoring=LG_FALSE;
	
	lg_bool bFoundFirst=LG_FALSE;
	lg_bool bIgnoreIsDel=LG_FALSE;
	
	lg_dword i=0, j=0;
	lg_dword nStrLen=0, nDelLen=0;
	
	cIgnore=(cIgIns==' ' || cIgIns==0)?cIgnore:cIgIns;
	
	
	if(strToken)
	{
		szString=strToken;
		nPosition=0;
	}
	
	if(strDelimit)
	{
		szDelimit=strDelimit;
	}
	
	if(!szString || !szDelimit)
		return LG_NULL;
		
	szString=&szString[nPosition];
	
	nDelLen=L_strlenW(szDelimit);
	nStrLen=L_strlenW(szString);
	
	for(i=0; i<nDelLen; i++)
	{
		if(cIgnore==szDelimit[i])
			bIgnoreIsDel=LG_TRUE;
	}
		
	for(i=0; i<nStrLen ;i++)
	{
		if(szString[i]==cIgnore)
		{
			bIgnoring=!bIgnoring;
			if(!bFoundFirst)
			{
				if(bIgnoreIsDel)
					i++;
				szString=&szString[i];
				i=0;
				nStrLen=L_strlenW(szString);
				bFoundFirst=LG_TRUE;
			}
		}
		
		if(bIgnoring)
			continue;
			
		if(bFoundFirst)
		{
			for(j=0; j<nDelLen; j++)
			{
				if(bFoundFirst)
				{
					if(szString[i]==szDelimit[j])
					{
						szString[i]=0;
						nPosition=i+1;
						return szString;
					}
				}
			}
		}
		else
		{
			for(j=0; j<nDelLen; j++)
			{
				if(szString[i]==szDelimit[j])
				{
					bFoundFirst=LG_TRUE;
				}
			}
			if(bFoundFirst)
				bFoundFirst=LG_FALSE;
			else
			{
				szString=&szString[i];
				i=0;
				nStrLen=L_strlenW(szString);
				bFoundFirst=LG_TRUE;
			}
		}
	}
	//If we got to the end of the string...
	nPosition=nStrLen;
	return szString;
}


/*********************************************
	Debug_printf()

	In debug mode with print a string to the
	debugger window.
*********************************************/
void Debug_printfA(char* format, ...)
{
	#ifdef _DEBUG
	char szOutput[1024];
	va_list arglist=LG_NULL;
	/* Allocate memory for output, we free when we exit the function. */

	if(!format)
		return;

	/* We use _vsnprintf so we can print arguments, and also so we can limit the
		number of characters, put to the output buffer by the max string length
		allowed in the console. */
	va_start(arglist, format);
	_vsnprintf(szOutput, 1023, format, arglist);
	va_end(arglist);
	OutputDebugStringA(szOutput);
	#endif /* _DEBUG */
}

void Debug_printfW(lg_wchar_t* format, ...)
{
	#ifdef _DEBUG
	wchar_t szOutput[1024];
	va_list arglist=LG_NULL;
	/* Allocate memory for output, we free when we exit the function. */

	if(!format)
		return;

	/* We use _vsnprintf so we can print arguments, and also so we can limit the
		number of characters, put to the output buffer by the max string length
		allowed in the console. */
	va_start(arglist, format);
	_vsnwprintf(szOutput, 1023, format, arglist);
	va_end(arglist);
	OutputDebugStringW(szOutput);
	#endif /* _DEBUG */
}
