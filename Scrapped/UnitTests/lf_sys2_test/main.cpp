//#define UNICODE
//#define _UNICODE
#include <conio.h>
#include <direct.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <tchar.h>
#include <time.h>
#include "lf_sys2.h"

FILE* fout=NULL;

void __cdecl Debug_PrintfW(lf_cwstr szFormat)
{	
	lf_char szString[512];
	wcstombs(szString, szFormat, 511);
	printf("FS: %s\n", szString);
	
	if(fout)
		fprintf(fout, "%s\r\n", szString);
	
}

void CreateAlwaysTest()
{
	char szString[1024];
	time_t tm;
	LF_FILE3 File=LF_NULL;
	
	time(&tm);
	sprintf(szString, "This file is always created. %s.\r\n", ctime(&tm));
	File=LF_Open(_T("/Create Always.txt"), LF_ACCESS_READ|LF_ACCESS_WRITE, LF_CREATE_ALWAYS);
	if(!File)
		return;
	LF_Write(File, szString, strlen(szString));
	LF_Close(File);
}
void CreateNewTest()
{
	char szString[1024];
	LF_FILE3 File=LF_NULL;
	time_t tm;
	time(&tm);
	sprintf(szString, "This file is new created. %s.\r\n", ctime(&tm));
	File=LF_Open(_T("/Create New.txt"), LF_ACCESS_READ|LF_ACCESS_WRITE, LF_CREATE_NEW);
	if(!File)
		return;
	LF_Write(File, szString, strlen(szString));
	LF_Close(File);
}

void OpenExistingTest()
{
	LF_FILE3 Existing, Out;
	
	Existing=LF_Open(_T("/Existing File.txt"), LF_ACCESS_READ, LF_OPEN_EXISTING);
	if(!Existing)
		return;
	
	Out=LF_Open(_T("/Create Always Out.txt"), LF_ACCESS_WRITE, LF_CREATE_ALWAYS);
	if(!Out)
	{
		LF_Close(Existing);
		return;
	}
	
	while(!LF_IsEOF(Existing))
	{
		lf_byte data[12];
		lf_dword nRead=LF_Read(Existing, data, 12);
		LF_Write(Out, data, nRead);
	}
	
	LF_Close(Existing);
	LF_Close(Out);
}

void OpenArchiveFilesTest()
{
	LF_FILE3 Cmp, UnCmp;
	
	UnCmp=LF_Open(_T("/Existing File Archived Uncompressed.txt"), LF_ACCESS_READ, LF_OPEN_EXISTING);
	if(!UnCmp)
		return;
		
	Cmp=LF_Open(_T("/Existing File Archived Compressed.txt"), LF_ACCESS_READ, LF_OPEN_EXISTING);
	if(!Cmp)
	{
		LF_Close(UnCmp);
		return;
	}
	
	printf("Uncompressed Contents:\n\n");
	while(!LF_IsEOF(UnCmp))
	{
		char szData[41];
		//pFile->Read(szSample, 20);
		szData[LF_Read(UnCmp, szData, 40)]=0;
		printf("%s", szData);
	}
	
	printf("Compressed Contents:\n\n");
	while(!LF_IsEOF(Cmp))
	{
		char szData[41];
		//pFile->Read(szSample, 20);
		szData[LF_Read(Cmp, szData, 40)]=0;
		printf("%s", szData);
	}
	printf("\n\n");
	
	LF_Close(UnCmp);
	LF_Close(Cmp);
}

int _tmain(int argc, TCHAR *argv[], TCHAR *envp[])
{
	fout=fopen("out.txt", "w");
	//LF_SetErrLevel(ERR_LEVEL_NOTICE);
	LF_SetErrPrintFn(Debug_PrintfW);
	FS_Initialze();
	FS_MountBase(_T(".\\"));
	FS_MountLPK(_T("/Test.lpk"), 0);
	FS_Mount(_T("base"), _T("/base1"), MOUNT_MOUNT_SUBDIRS);
	FS_MountLPK(_T("/base1/pak00.lpk"), MOUNT_FILE_OVERWRITELPKONLY);
	FS_MountLPK(_T("/base1/pak01.lpk"), MOUNT_FILE_OVERWRITELPKONLY);
	
	FS_PrintMountInfo();
	
	//CreateAlwaysTest();
	//CreateNewTest();
	//OpenExistingTest();
	//OpenArchiveFilesTest();
	
	FS_Shutdown();
	
	fclose(fout);
	_getch();
	return 0;
}
