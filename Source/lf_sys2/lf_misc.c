#include <string.h>
#include "lf_sys2.h"

/***************************************************
	Pathname getting functions, retrieve filename
	path, or short filename from a full path
***************************************************/
lf_str LF_GetFileNameFromPathA(lf_str szFileName, lf_cstr szFullPath)
{
	lf_dword dwLen=strlen(szFullPath);
	lf_dword i=0, j=0;
	
	for(i=dwLen; i>0; i--)
	{
		if(szFullPath[i]=='\\' || szFullPath[i]=='/')
		{
			i++;
			break;
		}
	}
	for(j=0 ;i<dwLen+1; i++, j++)
	{
		szFileName[j]=szFullPath[i];
	}
	return szFileName;
}

lf_wstr LF_GetFileNameFromPathW(lf_wstr szFileName, lf_cwstr szFullPath)
{
	lf_dword dwLen=wcslen(szFullPath);
	lf_dword i=0, j=0;
	
	for(i=dwLen; i>0; i--)
	{
		if(szFullPath[i]=='\\' || szFullPath[i]=='/')
		{
			i++;
			break;
		}
	}
	for(j=0 ;i<dwLen+1; i++, j++)
	{
		szFileName[j]=szFullPath[i];
	}
	return szFileName;
}

lf_str LF_GetShortNameFromPathA(lf_str szName, lf_cstr szFullPath)
{
	lf_dword dwLen=strlen(szFullPath);
	lf_dword i=0, j=0;
	
	for(i=dwLen; i>0; i--)
	{
		if(szFullPath[i]=='\\' || szFullPath[i]=='/')
		{
			i++;
			break;
		}
	}
	for(j=0 ;i<dwLen+1; i++, j++)
	{
		szName[j]=szFullPath[i];
		if(szName[j]=='.')
		{
			szName[j]=0;
			break;
		}
	}
	return szName;
}

lf_wstr LF_GetShortNameFromPathW(lf_wstr szName, lf_cwstr szFullPath)
{
	lf_dword dwLen=wcslen(szFullPath);
	lf_dword i=0, j=0;
	
	for(i=dwLen; i>0; i--)
	{
		if(szFullPath[i]=='\\' || szFullPath[i]=='/')
		{
			i++;
			break;
		}
	}
	for(j=0 ;i<dwLen+1; i++, j++)
	{
		szName[j]=szFullPath[i];
		if(szName[j]=='.')
		{
			szName[j]=0;
			break;
		}
	}
	return szName;
}

lf_str LF_GetDirFromPathA(lf_str szDir, lf_cstr szFullPath)
{
	lf_dword dwLen=strlen(szFullPath);
	lf_long i=0;
	for(i=(lf_long)dwLen-1; i>=0; i--)
	{
		if(szFullPath[i]=='\\' || szFullPath[i]=='/')
			break;
	}
	strncpy(szDir, szFullPath, i+1);
	szDir[i+1]=0;
	return szDir;
}

lf_wstr LF_GetDirFromPathW(lf_wstr szDir, lf_cwstr szFullPath)
{
	lf_dword dwLen=wcslen(szFullPath);
	lf_long i=0;
	for(i=(lf_long)dwLen-1; i>=0; i--)
	{
		if(szFullPath[i]=='\\' || szFullPath[i]=='/')
			break;
	}
	wcsncpy(szDir, szFullPath, i+1);
	szDir[i+1]=0;
	return szDir;
}

