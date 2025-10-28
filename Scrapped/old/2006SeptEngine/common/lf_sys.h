/* lf_sys.h Header for teh Legacy file system.
	Copyright (c) 2006, Blaine Myers. */
#ifndef __LF_SYS_H__
#define __LF_SYS_H__

#include "common.h"

#ifdef __cplusplus
extern "C"{
#endif /*__cplusplus*/


#define FS_CHECK_EXIT if(!g_bFSInited){LF_SetError("LF Error: File system not initialized."); return 0;}



#define MAX_F_PATH 260

#define TYPE_DATASTREAM 0x00000001
#define TYPE_FILESTREAM 0x00000002

typedef void* LF_FILE2;

typedef enum _LF_MOVETYPE{
	MOVETYPE_SET=0,
	MOVETYPE_CUR=1,
	MOVETYPE_END=2,
	MOVETYPE_FORCE_DWORD=0xFFFFFFFFl
}LF_MOVETYPE;

/*************************************
	File system initialization stuff.
*************************************/
L_bool LF_Init(char* dir, L_dword Flags);
#define LFINIT_CHECKARCHIVESFIRST 0x00000001
L_bool LF_Shutdown();
L_bool LF_AddArchive(L_lpstr szFilename);
L_bool LF_CloseAllArchives();
L_bool LF_ChangeDir(const char* dirname);
L_bool LF_ShowPaths(const char* szLimit, L_int (*OutputFunc)(const char*, ...));
char* LF_GetDir(char* buffer, int maxlen);
void LF_SetError(char* format, ...);
char* LF_GetLastError();


/***************************************
	File input output functions.
***************************************/

#define LF_ACCESS_READ               0x00000001
#define LF_ACCESS_WRITE              0x00000002
#define LF_ACCESS_MEMORY             0x00000004
#define LF_ACCESS_DONTSEARCHARCHIVES 0x00000008

typedef enum _LFCREATE{
	LFCREATE_UNKNOWN=0,
	LFCREATE_OPEN_EXISTING=1,
	LFCREATE_CREATE_NEW=2,
	LFCREATE_FORCE_DWORD=0xFFFFFFFF}LFCREATE;

LF_FILE2 File_Open(L_lpstr szFilename, L_dword dwSize, L_dword dwAccess, LFCREATE nCreate);
L_bool File_Close(LF_FILE2 lpFile2);
L_dword File_Read(LF_FILE2 lpFile2, L_dword dwByesToRead, L_void* lpBuffer);
L_dword File_Write(LF_FILE2 lpFile2, L_dword dwBytesToWrite, L_void* lpBuffer);
L_dword File_Seek(LF_FILE2 lpFile2, L_long nMoveDist, LF_MOVETYPE nMoveType);
L_dword File_Tell(LF_FILE2 lpFile2);
L_dword File_GetSize(LF_FILE2 lpFile2);
void* File_GetMemPointer(LF_FILE2 lpFile2, L_dword* lpSize);


/****************************************
	Archiving functions, reading and
	writing.
****************************************/


/* The _SEARCHPATHS structure is used to hold information
	about files that are contained within Legacy Archive (lpk)
	files.  It is sort of an indexing system to quickly load
	files from within the lpks.*/
typedef struct _SEARCHPATHS{
	char m_szFilename[MAX_F_PATH];
	char m_szLibrary[MAX_F_PATH];
	char m_szNameInLibrary[MAX_F_PATH];
	struct _SEARCHPATHS* m_next;
}SEARCHPATHS, *PSEARCHPATHS;


#define ARC_VERSION    (101)
#define ARC_VERSIONTXT ("1.01")

#define LARCHIVE_TYPE (*(unsigned long*)"LPK1")

typedef struct _ARCHEADER{
	unsigned long m_nType;
	unsigned long m_nVersion;
	unsigned long m_nCount;
}ARCHEADER;

#define ARCHEADER_SIZE 12

typedef struct _ARCENTRY2{
	char                  m_szFilename[MAX_F_PATH];
	unsigned long         m_nType;
	unsigned long         m_nOffset;
	unsigned long         m_nInfoOffset;
	unsigned long         m_nCmpSize;
	unsigned long         m_nUncmpSize;
	struct _ARCENTRY2*    m_next;
}ARCENTRY2, *PARCENTRY2;

#define ARCHTYPE_DEFAULT 0x00000000
#define ARCHTYPE_ZLIBC   0x00000001

#define ARCENTRY2_SIZE (MAX_F_PATH+20)

typedef void* HLARCHIVE;

HLARCHIVE Arc_Open(char* szFilename);
int Arc_Close(HLARCHIVE hArchive);
unsigned long Arc_Extract(HLARCHIVE hArchive, char* szFile, void* lpBuffer);
unsigned long Arc_GetNumFiles(HLARCHIVE hArchive);
const char* Arc_GetFilename(HLARCHIVE hArchive, unsigned long nFile);
unsigned long Arc_GetFileSize(HLARCHIVE hArchive, char* szFile);

typedef int (*PERCENTAGEFUNC)(void*, unsigned int, char* szOutput);

int Arc_Archive(
	char* szArchive, 
	char* szPath, 
	unsigned long dwFlags, 
	PERCENTAGEFUNC pPercentageFunc);

#define ARCHIVE_COMPRESS 0x00000010


#ifdef __cplusplus
}
#endif /*__cplusplus*/
#endif /* __LF_SYS_H__*/