/***************************************************************
	File: common.c
	Copyright (c) 2006, Blaine Myers

	Purpose: All of the types and functions that are universal
	are contained in common.c and common.h.  These two files
	are used not just for Legacy 3D, but for lc_sys.dll and
	la_sys.dll.
***************************************************************/
#include "common.h"
#include <direct.h>
#include <stdio.h>

#ifdef _DEBUG
#include <stdio.h>
#include <stdarg.h>
void __stdcall OutputDebugStringA(const char* lpOutputString);
#endif /*_DEBUG*/

/****************************************
	Legacy Engine replacement functions.
****************************************/

void* L_fopen(const char* filename, const char* mode)
{
	return fopen(filename, mode);
}

int L_fclose(void* stream)
{
	return fclose(stream);
}

L_uint L_fread(void* buffer, L_uint size, L_uint count, void* stream)
{
	return fread(buffer, size, count, stream);
}

L_uint L_fwrite(const void* buffer, L_uint size, L_uint count, void* stream)
{
	return fwrite(buffer, size, count, stream);
}

L_long L_ftell(void* stream)
{
	return ftell(stream);
}

int L_fseek(void* stream, long offset, int origin)
{
	return fseek(stream, offset, origin);
}



/**********************************************
	L_mkdir()

	Makes a directory, including subdirectory.
**********************************************/
int L_mkdir(const char* szDir)
{
	L_dword dwLen=L_strlen(szDir);
	L_dword i=0;
	/* We use a cheat here so we can temporarily change the string. */
	char* szTemp=(char*)szDir;
	char c=0;

	if(!szDir)
		return L_false;

	/* The idea here is to go through the string, find each
		subdirectory, then create a directory till we get to
		the final folder. */
	for(i=0; i<=dwLen; i++)
	{
		c=szDir[i];

		if(c=='\\' || c=='/')
		{
			szTemp[i]=0;
			_mkdir(szDir);
			szTemp[i]=c;
		}
	}
	return L_true;
}

char* L_getfilepath(char* szDir, const char* szFilename)
{
	L_dword dwLen=L_strlen(szFilename);
	L_dword i=0;
	for(i=dwLen-1; i>=0; i--)
	{
		if(szFilename[i]=='\\' || szFilename[i]=='/')
			break;
	}
	L_strncpy(szDir, szFilename, i+2);
	return szDir;
}

char* L_strncat(char* szDest, const char* szSrc, unsigned long nMax)
{
	L_dword dwLen=L_strlen(szDest);
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

	if((szOut==L_null) || (szIn==L_null))
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
	L_strnlen()

	Returns the length of the specified string.
**********************************************/
unsigned long L_strlen(const char* string)
{
	unsigned long count=0;
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
int L_strnicmp(char* szOne, char* szTwo, unsigned long nNum)
{
	unsigned long i=0;
	unsigned long nLen1=0, nLen2=0;
	char c1=0, c2=0;

	if((szOne==L_null) || (szTwo==L_null))
		return 0;

	nLen1=L_strlen(szOne);
	nLen2=L_strlen(szTwo);

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


/*********************************************
	Debug_printf()

	In debug mode with print a string to the
	debugger window.
*********************************************/
void Debug_printf(char* format, ...)
{
	#ifdef _DEBUG
	char szOutput[1024];
	va_list arglist=L_null;
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

