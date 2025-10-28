#include <stdio.h>
#include <string.h>
#include <direct.h>
#include <io.h>
#include <windows.h>
#include "la_sys.h"

#pragma comment(lib, "larchive.lib")


int main(int argc, char* argv[])
{
	char szDir[MAX_A_PATH];
	char* szPath;
	unsigned long nCount=0;

	szPath=argv[1];
	printf("Legacy Archiver v1.0 Copyright (c) 2006, Blaine Myers\n");
	if(argc<3)
	{
		printf("Usage: larchiver <pathtree> <archivefile>\n");
		return 0;
	}

	if(szPath[0]=='\\')
		szPath++;

	_snprintf(szDir, MAX_A_PATH-1, "%s\\%s", _getdcwd(_getdrive(), szDir, MAX_A_PATH-1), szPath);
	printf("Packaging everything in \"%s\"\n", szDir);
	_chdir(szDir);
	sprintf(szDir, "..\\%s", argv[2]);
	Arc_Archive(szDir, ".", NULL);

	return 0;
}