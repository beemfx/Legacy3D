/* la_sys.h - the legacy archive file format. */
#ifndef __LA_SYS_H__
#define __LA_SYS_H__

#ifdef __cplusplus
extern "C"{
#endif /* __cplusplus */


#define MAX_A_PATH (260)
#define LARCHIVE_TYPE (*(unsigned long*)"LPK1")



typedef void* HLARCHIVE;

HLARCHIVE Arc_Open(char* szFilename);
int Arc_Close(HLARCHIVE hArchive);
unsigned long Arc_Extract(HLARCHIVE hArchive, char* szFile, void** lpBuffer);
unsigned long Arc_GetNumFiles(HLARCHIVE hArchive);
const char* Arc_GetFilename(HLARCHIVE hArchive, unsigned long nFile);
unsigned long Arc_GetFileSize(HLARCHIVE hArchive, char* szFile);

int Arc_Archive(char* szArchive, char* szPath, char* szPersist);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __LA_SYS_H__ */