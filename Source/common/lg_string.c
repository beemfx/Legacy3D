#include <string.h>
#include "lg_string.h"

lg_int LG_StrNcCmpA(const lg_char* s1, const lg_char* s2, lg_long n)
{
	lg_char c1, c2;
	
	#if 1
	if(!s1)
	{
		return s2?-1:0;
	}
	else if(!s2)
		return 1;
	#endif

	do
	{
		c1=*s1++;
		c2=*s2++;
		
		if(!n--)
			return 0;
		
		if(c1!=c2)
		{
			if((c1>='a') && (c1<='z'))
				c1-=('a'-'A');
				
			if((c2>='a') && (c2<='z'))
				c2-=('a'-'A');
				
			if(c1!=c2)
				return c1>c2?1:-1;
		}
	}while(c1);
	
	return 0;
}
lg_int LG_StrNcCmpW(const lg_wchar* s1, const lg_wchar* s2, lg_long n)
{
	lg_wchar c1, c2;
	
	#if 1
	if(!s1)
	{
		return s2?-1:0;
	}
	else if(!s2)
		return 1;
	#endif

	do
	{
		c1=*s1++;
		c2=*s2++;
		
		if(!n--)
			return 0;
		
		if(c1!=c2)
		{
			if((c1>='a') && (c1<='z'))
				c1-=('a'-'A');
				
			if((c2>='a') && (c2<='z'))
				c2-=('a'-'A');
				
			if(c1!=c2)
				return c1>c2?1:-1;
		}
	}while(c1);
	
	return 0;
}

lg_dword LG_StrLenA(const lg_char* s)
{
	lg_dword count=0;
	while(s[count])
		count++;
	return count;
}
lg_dword LG_StrLenW(const lg_wchar* s)
{
	lg_dword count=0;
	while(s[count])
		count++;
	return count;
}

lg_char* LG_StrCopySafeA(lg_char* dest, const lg_char* src, lg_dword destsize)
{
	strncpy(dest,src, destsize-1);
	dest[destsize-1] = 0;
	return dest;
}

lg_wchar* LG_StrCopySafeW(lg_wchar* dest, const lg_wchar* src, lg_dword destsize)
{
	wcsncpy(dest,src, destsize-1);
	dest[destsize-1] = 0;
	return dest;
}