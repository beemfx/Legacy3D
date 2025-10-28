#include "common.h"
#include <memory.h>
#include <stdio.h>
#include <malloc.h>
#include "lf_sys.h"

typedef struct _LF_FILE_S{
	L_dword dwType;
	L_dword FileSize;
	L_dword FilePointer;
	L_dword dwAccess;
	char szFilename[MAX_F_PATH];
	void* pFileData;
	void* lpFile;
}LF_FILE_S, *LPLF_FILE_S;

extern L_bool g_bFSInited;
extern char g_szCurDir[MAX_F_PATH];

extern L_bool g_bArchivesFirst;

extern SEARCHPATHS* g_SearchPaths;

/*********************************************************
	The File_* functions are used to open files and read
	them, there is no writing ability implimented, yet.
**********************************************************/

/********************************************************************
	File_OpenFromData()

	Opens a file from data.  This function creates its own buffer
	to use to read from, so the data passed to the first parameter
	should be freed if it is no long going to be used.
*********************************************************************/
LF_FILE2 File_OpenFromData(void* lpBuffer, char* szFilename, L_dword nSize, L_dword dwAccess)
{
	LF_FILE_S* lpNewFile=L_null;

	if(!lpBuffer)
		return L_null;

	lpNewFile=malloc(sizeof(LF_FILE_S));
	
	if(!lpNewFile)
	{
		free(lpBuffer);
		return L_null;
	}

	lpNewFile->pFileData=malloc(nSize);
	if(!lpNewFile)
	{
		free(lpNewFile);
		return L_null;
	}
	memcpy(lpNewFile->pFileData, lpBuffer, nSize);
	lpNewFile->dwAccess=dwAccess;
	lpNewFile->dwType=TYPE_DATASTREAM;
	lpNewFile->FilePointer=0;
	lpNewFile->FileSize=nSize;
	L_strncpy(lpNewFile->szFilename, szFilename, MAX_F_PATH);
	//lpNewFile->pFileData=lpBuffer;
	return (LF_FILE2)lpNewFile;
}

/************************************************************
	File_OpenFromArchive()

	Goes through the archive index, if it finds a matching
	name it will open that file from the archive.  The file
	can then be used as normal.
************************************************************/
LF_FILE2 File_OpenFromArchive(char* szFilename)
{
	SEARCHPATHS* lpPath=L_null;
	HLARCHIVE hArchive=L_null;
	LF_FILE_S* lpNewFile=L_null;
	void* lpBuffer=L_null;
	L_dword nSize=0;

	/* Step 1: Go through the seach paths until the file is found. */
	for(lpPath=g_SearchPaths; lpPath; lpPath=lpPath->m_next)
	{
		if(L_strnicmp(lpPath->m_szFilename, szFilename, 0))
		{
			hArchive=Arc_Open(lpPath->m_szLibrary);
			break;
		}
	}

	/* If the file wasn't found the archive won't have been opened. */
	if(!hArchive)
		return L_null;
	/* Step 2: Allocate memory for the file. */
	lpBuffer=malloc(Arc_GetFileSize(hArchive, lpPath->m_szNameInLibrary));
	if(!lpBuffer)
	{
		Arc_Close(hArchive);
		return L_null;
	}
	/* Step 3: Extract the file into the allocated memory. Close
		the archive. */
	nSize=Arc_Extract(hArchive, lpPath->m_szNameInLibrary, lpBuffer);
	Arc_Close(hArchive);
	/* Step 4: Call File_Open from data, then free the allocated memory. */
	lpNewFile=(LF_FILE_S*)File_OpenFromData(
		lpBuffer, 
		lpPath->m_szFilename, 
		nSize, 
		LF_ACCESS_READ|LF_ACCESS_MEMORY);

	free(lpBuffer);
	return (LF_FILE2)lpNewFile;
}


LF_FILE2 File_CreateNew(szFilename, nSize, dwAccess)
{
	LF_FILE_S* lpNewFile=L_null;
	void* lpBuffer=L_null;
	char szFilenameLong[MAX_F_PATH];
	_snprintf(szFilenameLong, MAX_F_PATH-1, "%s\\%s", LF_GetDir(L_null, 0), szFilename);
	if(L_CHECK_FLAG(dwAccess, LF_ACCESS_MEMORY))
	{
		lpBuffer=malloc(nSize);
		if(!lpBuffer)
		{
			LF_SetError("File_Open: Could not allocate memory for \"%s\".", szFilename);
			return L_null;
		}
		memset(lpBuffer, 0, nSize);
		lpNewFile=(LF_FILE_S*)File_OpenFromData(lpBuffer, szFilenameLong, nSize, dwAccess);
		free(lpBuffer);
	}
	else
	{
		/* Initialize everything to 0. */
		void* fin=L_fopen(szFilenameLong, "wb");
		if(!fin)
		{
			LF_SetError("File_Open: Could not open \"%s\" for writing.", szFilename);
			return L_null;
		}

		lpNewFile=malloc(sizeof(LF_FILE_S));
		if(!lpNewFile)
		{
			L_fclose(fin);
			LF_SetError("File_Open: Could not allocate memory for \"%s\".", szFilename);
			return L_null;
		}
		memset(lpNewFile, 0, sizeof(LF_FILE_S));
		lpNewFile->dwAccess=dwAccess;
		lpNewFile->dwType=TYPE_FILESTREAM;
		lpNewFile->FilePointer=0;
		lpNewFile->FileSize=nSize;
		lpNewFile->lpFile=fin;
		lpNewFile->pFileData=L_null;
		L_strncpy(lpNewFile->szFilename, szFilenameLong, MAX_F_PATH);
	}
	return (LF_FILE2)lpNewFile;
}

LF_FILE2 File_OpenExisting(L_lpstr szFilename, L_dword dwAccess)
{
	LF_FILE_S* lpNewFile=L_null;
	void* lpBuffer=L_null;
	L_dword dwSize=0;
	void* fin=0;
	L_dword dwRead=0;

	L_bool bArchiveFirst=g_bArchivesFirst;

	//Attempt to open from disk first, if that fails, open
	//from archive.

	if(bArchiveFirst)
	{
		if(!L_CHECK_FLAG(dwAccess, LF_ACCESS_DONTSEARCHARCHIVES))
		{
			lpNewFile=(LF_FILE_S*)File_OpenFromArchive(szFilename);

			if(lpNewFile)
				return (LF_FILE2)lpNewFile;
		}
	}
	
	//Attempt ot open from the disk.
	fin=L_fopen(szFilename, "rb+");
	if(!fin)
	{
		if(!L_CHECK_FLAG(dwAccess, LF_ACCESS_DONTSEARCHARCHIVES && !bArchiveFirst))
		{
			lpNewFile=(LF_FILE_S*)File_OpenFromArchive(szFilename);

			if(lpNewFile)
				return (LF_FILE2)lpNewFile;
		}
		LF_SetError("File_Open Error: Could not open \"%s\".", szFilename);
		return L_null;
	}

	/*
	if(!fin)
	{
		LF_SetError("File_Open Error: Could not open \"%s\".", szFilename);
		return L_null;
	}
	*/
	L_fseek(fin, 0, SEEK_END);
	dwSize=L_ftell(fin);
	L_fseek(fin, 0, SEEK_SET);

	if(dwSize<1)
	{
		LF_SetError("File_Open Error: No data found in \"%s\" file.", szFilename);
		L_fclose(fin);
		return L_null;
	}

	/* If we want to create a file in memory, we read the file,
		and call File_OpenFromData. Otherwise we open an actual file.*/
	if(L_CHECK_FLAG(dwAccess, LF_ACCESS_MEMORY))
	{
		lpBuffer=malloc(dwSize);
		dwRead=L_fread(lpBuffer, 1, dwSize, fin);
		if(!lpBuffer)
		{
			L_fclose(fin);
			return L_null;
		}
		lpNewFile=(LF_FILE_S*)File_OpenFromData(lpBuffer, szFilename, dwSize, dwAccess);
		L_fclose(fin);
		free(lpBuffer);
		return (LF_FILE2)lpNewFile;
	}
	else
	{
		lpNewFile=malloc(sizeof(LF_FILE_S));
		if(!lpNewFile)
		{
			LF_SetError("File_Open Error: Could not allocate memory for \"%s\" file.", szFilename);
			L_fclose(fin);
			return L_null;
		}

		/* Initialize everything to 0. */
		memset(lpNewFile, 0, sizeof(LF_FILE_S));
		lpNewFile->dwAccess=dwAccess;
		lpNewFile->dwType=TYPE_FILESTREAM;
		lpNewFile->FilePointer=0;
		lpNewFile->FileSize=dwSize;
		lpNewFile->lpFile=fin;
		lpNewFile->pFileData=L_null;
		L_strncpy(lpNewFile->szFilename, szFilename, MAX_F_PATH);

		return (LF_FILE2)lpNewFile;
	}
}

/****************************************************************
	File_Open()

	Opens a file using the specified flags.  If the LFF_MEMBUF
	flag is specified the file will be opened all the way into
	memory.  If that flag is not specified the file will be
	opened as a FILE*.
****************************************************************/
LF_FILE2 File_Open(L_lpstr szFilename, L_dword dwSize, L_dword dwAccess, LFCREATE nCreate)
{
	LF_FILE_S* lpNewFile=L_null;

	/* We should try to open from an archive, firs then result to opening from
		the actual file system, but the archvie feature hasn't been implimented
		yet so we are just opening from the disk. */
	FS_CHECK_EXIT
	/*
	lpNewFile=malloc(sizeof(LF_FILE));
	if(!lpNewFile)
	{
		LF_SetError("File_Open: Could not allocate memory for new file.");
		return L_null;
	}

	memset(lpNewFile, 0, sizeof(LF_FILE));
	lpNewFile->dwAccess=dwAccess;
	if(szFilename[1]!=':')
		_snprintf(lpNewFile->szFilename, MAX_F_PATH-1, "%s\\%s", LF_GetDir(), szFilename);
	else
		_snprintf(lpNewFile->szFilename, MAX_F_PATH-1, "%s", szFilename);
	if(L_CHECK_FLAG(dwAccess, LF_ACCESS_MEMORY))
	{
		lpNewFile->pFileData=malloc(dwSize);
		if(!lpNewFile->pFileData)
		{
			free(lpNewFile);
			LF_SetError("File_Open: Could not allocate memory.");
			return L_null;
		}
	}
	*/

	/* Try to open the file from archive, if that fails, open it
		from disk. */

	/* We will set a default error message now, and hopefully the
		other functions will change it if they can't open the file.*/
	LF_SetError("File_Open: Succeeded.");

	switch (nCreate)
	{
	case LFCREATE_OPEN_EXISTING:
		return File_OpenExisting(szFilename, dwAccess);
	case LFCREATE_CREATE_NEW:
		return File_CreateNew(szFilename, dwSize, dwAccess);
	default:
		return L_null;
	}
}

/*************************************************
	File_Close()

	Closes the open file and frees any memory.
*************************************************/
L_bool File_Close(LF_FILE2 lpFile2)
{
	LF_FILE_S* lpFile=(LF_FILE_S*)lpFile2;
	FS_CHECK_EXIT

	if(!lpFile)
		return L_false;

	/* If this was a regular file we close the file. */
	if(lpFile->dwType==TYPE_FILESTREAM)
	{
		if(lpFile->lpFile)
			L_fclose(lpFile->lpFile);
	}
	else
	{
		if(L_CHECK_FLAG(lpFile->dwAccess, LF_ACCESS_WRITE))
		{
			void* fout=L_null;

			fout=L_fopen(lpFile->szFilename, "wb");
			if(fout)
			{
				L_fwrite(lpFile->pFileData, 1, lpFile->FileSize, fout);
				L_fclose(fout);
			}
		}
		L_safe_free(lpFile->pFileData);
	}

	/* Just free all the memory we allocated. */
	L_safe_free(lpFile);
	LF_SetError("File_Close: Succeeded.");
	return L_true;
}

/**************************
	File_Write()

	Writes data to file.
**************************/
L_dword File_Write(LF_FILE2 lpFile2, L_dword dwBytesToWrite, L_void* lpBuffer)
{
	LF_FILE_S* lpFile=(LF_FILE_S*)lpFile2;
	FS_CHECK_EXIT

	if(!lpFile || 
		!lpBuffer || 
		!L_CHECK_FLAG(lpFile->dwAccess, LF_ACCESS_WRITE)||
		!dwBytesToWrite)
	{
		LF_SetError("File_Write: Invalid Args.");
		return 0;
	}

	LF_SetError("File_Write: Succeeded.");
	if((lpFile->FileSize<(lpFile->FilePointer+dwBytesToWrite)) && L_CHECK_FLAG(lpFile->dwAccess, LF_ACCESS_MEMORY))
	{
		LF_SetError("File_Write: Could only write %i bytes.", dwBytesToWrite);
	}

	/* Copy the data. */
	if(lpFile->dwType==TYPE_FILESTREAM)
	{
		dwBytesToWrite=L_fwrite(lpBuffer, 1, dwBytesToWrite, lpFile->lpFile);
		lpFile->FilePointer+=dwBytesToWrite;
		return dwBytesToWrite;
	}
	else
	{
		memcpy((void*)((unsigned int)lpFile->pFileData+lpFile->FilePointer), lpBuffer, dwBytesToWrite);
		lpFile->FilePointer+=dwBytesToWrite;
	}
	return dwBytesToWrite;
}

/***********************************************************************
	File_Read()

	Reads the specified amount of data from the file, into the buffer.
	Returns the actual number of bytes read.
***********************************************************************/
L_dword File_Read(LF_FILE2 lpFile2, L_dword dwBytesToRead, L_void* lpBuffer)
{
	LF_FILE_S* lpFile=(LF_FILE_S*)lpFile2;
	FS_CHECK_EXIT

	if(!lpFile || !lpBuffer)
	{
		LF_SetError("File_Read: Invalid argument.");
		return 0;
	}
	if(!L_CHECK_FLAG(lpFile->dwAccess, LF_ACCESS_READ))
	{
		LF_SetError("File_Read: No read access.");
		return 0;
	}

	LF_SetError("File_Read: Succeeded.");
		/* If we don't need to read anything we won't. */
	if(!dwBytesToRead)
		return 0;

	/* If the bytes we want to read goes past the end of
		the file we need to adjust the reading size. */
	if(lpFile->FileSize<(lpFile->FilePointer+dwBytesToRead))
	{
		dwBytesToRead=lpFile->FileSize-lpFile->FilePointer;
	}

	/* Copy the data. */
	if(lpFile->dwType==TYPE_FILESTREAM)
	{
		dwBytesToRead=L_fread(lpBuffer, 1, dwBytesToRead, lpFile->lpFile);
		lpFile->FilePointer+=dwBytesToRead;
		return dwBytesToRead;
	}
	else
	{
		memcpy(lpBuffer, (void*)((unsigned int)lpFile->pFileData+lpFile->FilePointer), dwBytesToRead);
		lpFile->FilePointer+=dwBytesToRead;
	}
	return dwBytesToRead;
}

/*********************************************
	File_Tell()

	Tells the location of the data pointer.
*********************************************/

L_dword File_Tell(LF_FILE2 lpFile2)
{
	LF_FILE_S* lpFile=(LF_FILE_S*)lpFile2;
	FS_CHECK_EXIT

	LF_SetError("File_Tell: Succeeded.");
	return lpFile->FilePointer;
}

/*************************************************************************
	File_Seek()

	With change the read/write cursor of the given file to the specified
	position.  Returns the actual new position of the file pointer.
*************************************************************************/
L_dword File_Seek(LF_FILE2 lpFile2, L_long nMoveDist, LF_MOVETYPE nMoveType)
{
	LF_FILE_S* lpFile=(LF_FILE_S*)lpFile2;
	L_dword nNewPointer=lpFile->FilePointer;

	FS_CHECK_EXIT

	if(lpFile->dwType==TYPE_FILESTREAM)
	{
		switch(nMoveType)
		{
		case MOVETYPE_SET:
			L_fseek(lpFile->lpFile, nMoveDist, SEEK_SET);
			break;
		case MOVETYPE_END:
			L_fseek(lpFile->lpFile, nMoveDist, SEEK_END);
			break;	
		case MOVETYPE_CUR:
			L_fseek(lpFile->lpFile, nMoveDist, SEEK_CUR);
			break;
		}

		lpFile->FilePointer=L_ftell(lpFile->lpFile);
		LF_SetError("File_Seek: Succeeded.");
		return lpFile->FilePointer;
	}
	else
	{
		switch(nMoveType)
		{
		case MOVETYPE_SET:
			nNewPointer=nMoveDist;
			break;
		case MOVETYPE_END:
			nNewPointer=lpFile->FileSize+nMoveDist;
			break;	
		case MOVETYPE_CUR:
			nNewPointer+=nMoveDist;
			break;
		}

		if(nNewPointer<0)
			nNewPointer=0;
		if(nNewPointer>lpFile->FileSize)
			nNewPointer=lpFile->FileSize;

		lpFile->FilePointer=nNewPointer;
		LF_SetError("File_Seek: Succeeded.");
		return lpFile->FilePointer;
	}
}

L_dword File_GetSize(LF_FILE2 lpFile2)
{
	LF_FILE_S* lpFile=lpFile2;

	FS_CHECK_EXIT

	LF_SetError("File_GetSize: Succeeded.");
	return lpFile->FileSize;
}

void* File_GetMemPointer(LF_FILE2 lpFile2, L_dword* lpSize)
{
	LF_FILE_S* lpFile=(LF_FILE_S*)lpFile2;
	FS_CHECK_EXIT

	if(lpSize)
		*lpSize=lpFile->FileSize;

	LF_SetError("File_GetMemPointer: Succeeded.");
	return lpFile->pFileData;
}