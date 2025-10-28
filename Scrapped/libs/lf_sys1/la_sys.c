#include <stdio.h>
#include <malloc.h>
#include <memory.h>

#ifdef _DEBUG
#include <windows.h>
#include <stdlib.h>
#include <crtdbg.h>
#endif /*_DEBUG*/

#include <common.h>
#include "lf_sys.h"
#include "zlib\zlib.h"


typedef struct _ARCHIVE{
	ARCHEADER       m_Header;
	char            m_szFilename[MAX_F_PATH];
	ARCENTRY2*      m_lpFilelist;
	FILE*           m_fArchive;
}ARCHIVE, *PARCHIVE;


/**************************************
	The archive extraction utility.
***************************************/

HLARCHIVE Arc_Open(char* szFilename)
{
	ARCHIVE* lpNewArchive=NULL;
	unsigned long i=0;


	lpNewArchive=malloc(sizeof(ARCHIVE));
	if(!lpNewArchive)
		return L_null;
	memset(lpNewArchive, 0, sizeof(ARCHIVE));
	lpNewArchive->m_fArchive=L_fopen(szFilename, LOPEN_OPEN_EXISTING, LOPEN_READ);//"rb");
	if(!lpNewArchive->m_fArchive)
	{
		free(lpNewArchive);
		return L_null;
	}

	L_strncpy(lpNewArchive->m_szFilename, szFilename, MAX_F_PATH);

	/* Read the file type. */
	L_fread(&lpNewArchive->m_Header.m_nType, 1, 4, lpNewArchive->m_fArchive);
	if(lpNewArchive->m_Header.m_nType!=LARCHIVE_TYPE)
	{
		L_fclose(lpNewArchive->m_fArchive);
		free(lpNewArchive);
		return L_null;
	}
	/* Get the version. */
	L_fread(&lpNewArchive->m_Header.m_nVersion, 1, 4, lpNewArchive->m_fArchive);
	if(lpNewArchive->m_Header.m_nVersion!=ARC_VERSION)
	{
		L_fclose(lpNewArchive->m_fArchive);
		free(lpNewArchive);
		return L_null;
	}
	/* Read the number of files. */
	L_fread(&lpNewArchive->m_Header.m_nCount, 1, 4, lpNewArchive->m_fArchive);
	if(lpNewArchive->m_Header.m_nCount<1)
	{
		L_fclose(lpNewArchive->m_fArchive);
		free(lpNewArchive);
		return L_null;
	}

	/* Allocate memory for the file information and read it. */
	lpNewArchive->m_lpFilelist=malloc(lpNewArchive->m_Header.m_nCount*sizeof(ARCENTRY2));
	if(!lpNewArchive->m_lpFilelist)
	{
		L_fclose(lpNewArchive->m_fArchive);
		free(lpNewArchive);
		return L_null;
	}


	for(i=0; i<lpNewArchive->m_Header.m_nCount; i++)
	{
		size_t nRead=0;
		L_bool bFailed=L_false;

		/*
		#define READ_COMPONENT(comp, size) {nRead=L_fread(&lpNewArchive->m_lpFilelist[i].comp, \
																1, size, lpNewArchive->m_fArchive);\
																if(nRead<size)bFailed=L_true;}
		*/

		#define READ_COMPONENT(comp, size) if(L_fread(&lpNewArchive->m_lpFilelist[i].comp, 1, size, lpNewArchive->m_fArchive)<size)bFailed=L_true

		nRead=L_fread(
			&lpNewArchive->m_lpFilelist[i].m_szFilename, 
			1, 
			MAX_F_PATH, 
			lpNewArchive->m_fArchive);

		if(nRead<MAX_F_PATH)
			bFailed=L_true;

		READ_COMPONENT(m_nType, 4);
		READ_COMPONENT(m_nOffset, 4);
		READ_COMPONENT(m_nInfoOffset, 4);
		READ_COMPONENT(m_nCmpSize, 4);
		READ_COMPONENT(m_nUncmpSize, 4);

		L_fseek(lpNewArchive->m_fArchive, ARCHEADER_SIZE+ARCENTRY2_SIZE*(i+1), SEEK_SET);

		if(bFailed)
		{
			L_fclose(lpNewArchive->m_fArchive);
			free(lpNewArchive->m_lpFilelist);
			free(lpNewArchive);
			return L_null;
		}
	}

	L_fseek(lpNewArchive->m_fArchive, 0, SEEK_SET);

	return lpNewArchive;
}

int Arc_Close(HLARCHIVE hArchive)
{
	ARCHIVE* lpArchive=hArchive;

	if(!lpArchive)
		return 0;
	
	L_fclose(lpArchive->m_fArchive);
	free(lpArchive->m_lpFilelist);
	free(lpArchive);
	return 1;
}

unsigned long Arc_GetFileSize(HLARCHIVE hArchive, char* szFile)
{
	ARCHIVE* lpArchive=hArchive;
	ARCENTRY2* lpEntry=NULL;
	unsigned long i=0;

	if(!lpArchive || !szFile)
		return 0;

	for(i=0, lpEntry=NULL; i<lpArchive->m_Header.m_nCount; i++)
	{
		if(L_strnicmp(szFile, lpArchive->m_lpFilelist[i].m_szFilename, 0))
			return lpArchive->m_lpFilelist[i].m_nUncmpSize;
	}
	return 0;
}

unsigned long Arc_ExtractZLIBC(FILE* fin, ARCENTRY2* lpEntry, void* lpBuffer)
{
	void* lpTempBuffer=L_null;
	L_dword nRead=0;
	L_dword nUncmp=0;

	lpTempBuffer=malloc(lpEntry->m_nCmpSize);
	if(!lpTempBuffer)
		return 0;

	L_fseek(fin, lpEntry->m_nOffset, SEEK_SET);
	nRead=L_fread(lpTempBuffer, 1, lpEntry->m_nCmpSize, fin);
	nUncmp=lpEntry->m_nUncmpSize;
	if(uncompress(lpBuffer, &nUncmp, lpTempBuffer, nRead)!=Z_OK)
	{
		free(lpTempBuffer);
		return 0;
	}
	free(lpTempBuffer);
	return nUncmp;
}

unsigned long Arc_ExtractDefault(FILE* fin, ARCENTRY2* lpEntry, void* lpBuffer)
{
	if(L_fseek(fin, lpEntry->m_nOffset, SEEK_SET))
	{
		return 0;
	}
	return L_fread(lpBuffer, 1, lpEntry->m_nUncmpSize, fin);
}

unsigned long Arc_Extract(HLARCHIVE hArchive, char* szFile, void* lpBuffer)
{
	ARCHIVE* lpArchive=hArchive;
	ARCENTRY2* lpEntry=NULL;

	unsigned long i=0;

	if(!lpArchive || !szFile || !lpBuffer)
		return 0;

	for(i=0, lpEntry=NULL; i<lpArchive->m_Header.m_nCount; i++)
	{
		if(L_strnicmp(szFile, lpArchive->m_lpFilelist[i].m_szFilename, 0))
		{
			lpEntry=&lpArchive->m_lpFilelist[i];
			break;
		}
	}
	if(!lpEntry)
		return 0;

	switch(lpEntry->m_nType)
	{
	default:
	case ARCHTYPE_DEFAULT:
		return Arc_ExtractDefault(lpArchive->m_fArchive, lpEntry, lpBuffer);
	case ARCHTYPE_ZLIBC:
		return Arc_ExtractZLIBC(lpArchive->m_fArchive, lpEntry, lpBuffer);
	}
}

unsigned long Arc_GetNumFiles(HLARCHIVE hArchive)
{
	ARCHIVE* lpArchive=hArchive;

	if(!lpArchive)
		return 0;

	return lpArchive->m_Header.m_nCount;
}

const char* Arc_GetFilename(HLARCHIVE hArchive, unsigned long nFile)
{
	ARCHIVE* lpArchive=hArchive;

	if(!lpArchive)
		return NULL;

	if(nFile<1 || nFile>lpArchive->m_Header.m_nCount)
		return NULL;

	return lpArchive->m_lpFilelist[nFile-1].m_szFilename;
}


#ifdef _DEBUG
extern int g_nNumFilesOpen;
BOOL WINAPI DllMain(HINSTANCE hInst, DWORD fdwReason, LPVOID lpReserved)
{
	static _CrtMemState s1, s2, s3;
	char szTemp[1024];
	_snprintf(szTemp, 1023, "There are %d files open!\n", g_nNumFilesOpen);
	OutputDebugStringA(szTemp);
	switch(fdwReason)
	{
	//case DLL_THREAD_ATTACH:
	case DLL_PROCESS_ATTACH:
		_CrtMemCheckpoint(&s1);
		break;
	//case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		OutputDebugString("MEMORY USAGE FOR \"lf_sys.dll\"\"\n");
		_CrtMemCheckpoint(&s2);
		_CrtMemDifference(&s3, &s1, &s2);
		_CrtMemDumpStatistics(&s3);
		_CrtDumpMemoryLeaks();
		break;
	}
	return TRUE;
}
#endif /*_DEBUG*/
