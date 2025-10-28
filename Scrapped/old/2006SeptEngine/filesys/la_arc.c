/* la_arc.c - Functions for archiving Legacy Archive files.
	Copyright (c) 2006, Blaine Myers */

/* Compression by "zlib" version 1.2.3, July 18th, 2005
	Copyright (C) 1995-2005 Jean-loup Gailly and Mark Adler */
#include <stdio.h>
#include <memory.h>
#include <malloc.h>
#include <io.h>

#ifdef _DEBUG
#include <stdlib.h>
#include <crtdbg.h>
#endif /*_DEBUG*/

#include <common.h>
#include "lf_sys.h"
#include "zlib\zlib.h"


ARCENTRY2* Arc_BuildTreeFiles(char* szPath, unsigned long* pNumFiles);
int Arc_DeleteDirTree(ARCENTRY2* list);
unsigned long Arc_CreateDirTree(ARCENTRY2** list, char* szDirectory);
int Arc_WriteZLIBData(void* uncmpdata, ARCENTRY2* entry, FILE* fout);

int Arc_BuildArchiveFile(
	ARCENTRY2* lpList, 
	char* szOut, 
	unsigned long nCount,
	int bCompress, 
	PERCENTAGEFUNC pPercentageFunc);

/************************************
	The archvie compression utility.
*************************************/

/* Arc_Archive - The export function, takes an output filename, a tree path, and
	a Percentage function pointer, to build the archive. */
int Arc_Archive(
	char* szArchive, 
	char* szPath, 
	unsigned long dwFlags, 
	PERCENTAGEFUNC pPercentageFunc)
{
	ARCENTRY2* lpTree=NULL;
	ARCENTRY2* lpTest=NULL;

	unsigned long nNumFiles=0;
	int bResult=0, bCompress=0;

	lpTree=Arc_BuildTreeFiles(szPath, &nNumFiles);
	
	/* nNumFiles=Arc_CreateDirTree(&lpTree, szPath); */

	if(!lpTree)
		return 0;

	if(pPercentageFunc)
		pPercentageFunc(0, 5, "");


	if(nNumFiles<1)
	{
		Arc_DeleteDirTree(lpTree);
		return 0;
	}

	if(L_CHECK_FLAG(dwFlags, ARCHIVE_COMPRESS))
		bCompress=1;
	else
		bCompress=0;
	
	bResult=Arc_BuildArchiveFile(lpTree, szArchive, nNumFiles, bCompress, pPercentageFunc);

	Arc_DeleteDirTree(lpTree);
	return bResult;
}


/* Arc_BuildArchiveFile - uses the data in the ARCENTRY22 linked
	list and takes all those filenames and puts all those files
	in the specified archive.*/
int Arc_BuildArchiveFile(
	ARCENTRY2* lpList, 
	char* szOut, 
	unsigned long nCount,
	int bCompress, 
	PERCENTAGEFUNC pPercentageFunc)
{
	ARCENTRY2* entry=NULL;
	ARCHEADER Header={LARCHIVE_TYPE, ARC_VERSION, nCount};
	//const unsigned long nType=LARCHIVE_TYPE;
	//const unsigned long nVersion=101;
	FILE* fout=NULL;
	FILE* fin=NULL;
	void* data=NULL;
	unsigned long nOffset=0;
	unsigned long nPercentage=0;
	unsigned long nNumArchived=0;
	unsigned long nWritePos=0;

	fout=L_fopen(szOut, "wb+");
	if(!fout)
		return 0;

	L_fwrite(&Header.m_nType, 1, 4, fout);
	L_fwrite(&Header.m_nVersion, 1, 4, fout);
	L_fwrite(&Header.m_nCount, 1, 4, fout);

	nOffset=ARCHEADER_SIZE+(ARCENTRY2_SIZE)*nCount;

	for(entry=lpList; entry; entry=entry->m_next)
	{
		entry->m_nOffset=nOffset;
		nOffset+=entry->m_nUncmpSize;
		entry->m_nInfoOffset=L_ftell(fout);
		entry->m_nCmpSize=entry->m_nUncmpSize;

		if(bCompress)
			entry->m_nType=ARCHTYPE_ZLIBC;
		else
			entry->m_nType=ARCHTYPE_DEFAULT;

		L_fwrite(&entry->m_szFilename, 1, MAX_F_PATH, fout);
		L_fwrite(&entry->m_nType, 1, 4, fout);
		L_fwrite(&entry->m_nOffset, 1, 4, fout);
		L_fwrite(&entry->m_nInfoOffset, 1, 4, fout);
		L_fwrite(&entry->m_nCmpSize, 1, 4, fout);
		L_fwrite(&entry->m_nUncmpSize, 1, 4, fout);
	}

	for(entry=lpList; entry; entry=entry->m_next)
	{
		printf(
			"encrypting %s, %i bytes at %i (%i)\n", 
			entry->m_szFilename, 
			entry->m_nUncmpSize, 
			L_ftell(fout),
			entry->m_nOffset);

		nPercentage=(unsigned long)(((float)nNumArchived/(float)nCount)*95.0f+5.0f);
		if(pPercentageFunc)
			pPercentageFunc(0, nPercentage, entry->m_szFilename);	

		fin=L_fopen(entry->m_szFilename, "rb");
		if(!fin)
		{
			printf(
				"Could not open \"%s\" possible access violation.\n",
				entry->m_szFilename);
			if(L_fseek(fout, entry->m_nUncmpSize, SEEK_CUR))
			{
				printf("Could not write zeroes for empty file, archive is corrupt!\n");
				L_fclose(fout);
				return 0;
			}
			continue;
		}
		data=malloc(entry->m_nUncmpSize);
		if(!data)
		{
			printf("Could not allocate memory for archiving \"%s\"\n", entry->m_szFilename);
			if(L_fseek(fout, entry->m_nUncmpSize, SEEK_CUR))
			{
				printf("Could not write zeroes for empty file, archive is corrupt!\n");
				L_fclose(fout);
				L_fclose(fin);
				return 0;
			}
			L_fclose(fin);
			continue;
		}
		/* Read the data into the buffer. */
		L_fread(data, 1, entry->m_nUncmpSize, fin);
		L_fclose(fin);
		
		/* Save the position we are writing too. */
		entry->m_nOffset=L_ftell(fout);

		switch(entry->m_nType)
		{
		case ARCHTYPE_ZLIBC:
		{
			if(Arc_WriteZLIBData(data, entry, fout))
				break;
			else
				entry->m_nType=ARCHTYPE_DEFAULT;
		}
		default:
		case ARCHTYPE_DEFAULT:
			L_fwrite(data, 1, entry->m_nUncmpSize, fout);
			break;
		}
		/* Free the data. */
		free(data);
		/* Now we need to go back to the data and adjust
			the file offset position. */
		
		/* We need to change some of the data that was saved in the directory tree. */
		L_fseek(fout, entry->m_nInfoOffset+MAX_F_PATH, SEEK_SET);
		L_fwrite(&entry->m_nType, 1, 4, fout);
		L_fwrite(&entry->m_nOffset, 1, 4, fout);
		L_fwrite(&entry->m_nInfoOffset, 1, 4, fout);
		L_fwrite(&entry->m_nCmpSize, 1, 4, fout);
		L_fwrite(&entry->m_nUncmpSize, 1, 4, fout);
		L_fseek(fout, 0, SEEK_END);


		nNumArchived++;
		nPercentage=(unsigned long)(((float)nNumArchived/(float)nCount)*95.0f+5.0f);
		if(pPercentageFunc)
			pPercentageFunc(0, nPercentage, entry->m_szFilename);	
	}


	L_fclose(fout);
	return 1;
}

int Arc_WriteZLIBData(void* uncmpdata, ARCENTRY2* entry, FILE* fout)
{
	void* cmpdata=NULL;

	cmpdata=malloc(entry->m_nUncmpSize);

	if(!cmpdata)
		return 0;

	entry->m_nCmpSize=entry->m_nUncmpSize;
	if(compress(cmpdata, &entry->m_nCmpSize, uncmpdata, entry->m_nUncmpSize)!=Z_OK)
	{
		free(cmpdata);
		return 0;
	}
	
	L_fwrite(cmpdata, 1, entry->m_nCmpSize, fout);
	free(cmpdata);
	return 1;
}

/* Arc_CreateDirTree - Builds a ARCENTRY22 with all the files
	it can find in the specified directory.  Including files in
	subdirectories. */
unsigned long Arc_CreateDirTree(ARCENTRY2** list, char* szDirectory)
{
	long Handle=0;
	char szPath[MAX_F_PATH];
	struct _finddata_t data;
	unsigned long nNumArchived=0;
	ARCENTRY2 * tempentry=NULL;

	_snprintf(szPath, MAX_F_PATH-1, "%s\\*.*", szDirectory);

	memset(&data, 0, sizeof(data));
	Handle=_findfirst(szPath, &data);
	if(Handle==-1)
	{
		return 0;
	}
	do
	{
		/* If the found item is a subdirectory we archive all the files in
			that directory, unless it is the ..\ or .\ directories. */
		if(L_CHECK_FLAG(data.attrib, _A_SUBDIR))
		{
			if(L_strnicmp("..", data.name, 0) || L_strnicmp(".", data.name, 0))
				continue;
			_snprintf(szPath, MAX_F_PATH-1, "%s\\%s", szDirectory, data.name);
			nNumArchived+=Arc_CreateDirTree(list, szPath);
		}
		else		
		{
			_snprintf(szPath, MAX_F_PATH-1, "%s\\%s", szDirectory, data.name);
			tempentry=malloc(sizeof(ARCENTRY2));
			if(!tempentry)
				return nNumArchived;

			L_strncpy(tempentry->m_szFilename, szPath, MAX_F_PATH);
			tempentry->m_nUncmpSize=data.size;
			tempentry->m_nCmpSize=data.size;
			tempentry->m_nOffset=0;
			tempentry->m_nInfoOffset=0;
			tempentry->m_next=*list;
			*list=tempentry;
			nNumArchived++;
		}
		
	}
	while(_findnext(Handle, &data)!=-1);

	_findclose(Handle);
	return nNumArchived;
}

/* Calls Arc_CreateDirTree to create ARCENTRY22. */
ARCENTRY2* Arc_BuildTreeFiles(char* szPath, unsigned long* pNumFiles)
{
	ARCENTRY2* lpEntry=NULL;
	*pNumFiles=Arc_CreateDirTree(&lpEntry, szPath);
	return lpEntry;
}

/* Deletes the dir tree created with Arc_CreateDirTree*/
int Arc_DeleteDirTree(ARCENTRY2* list)
{
	ARCENTRY2* temp=NULL;
	ARCENTRY2* entry=NULL;

	entry=list;
	while(entry)
	{
		temp=entry->m_next;
		free(entry);
		entry=temp;
	}
	return 1;
}