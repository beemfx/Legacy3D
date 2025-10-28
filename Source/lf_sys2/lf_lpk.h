#ifndef __LF_ARC_H__
#define __LF_ARC_H__

#include "lf_sys2.h"
#include "lf_bdio.h"

#define LPK_VERSION 102
#define LPK_TYPE    0x314B504C //(*(lf_dword*)"LPK1")

#define LPK_VERSIONTXT ("1.02")

#define LPK_FILE_TYPE_NORMAL    0x00000001
#define LPK_FILE_TYPE_ZLIBCMP   0x00000002


#define LPK_OPEN_ENABLEWRITE 0x00000001
#define LPK_ADD_ZLIBCMP      0x00000002

typedef struct _LPK_FILE_INFO{
	lf_pathW szFilename;
	lf_dword nType;
	lf_dword nOffset;
	lf_dword nSize;
	lf_dword nCmpSize;
}LPK_FILE_INFO;

#define LPK_FILE_POS_TEMP 0x00000020
#define LPK_FILE_POS_MAIN 0x00000030

typedef struct _LPK_FILE_INFO_EX: public LPK_FILE_INFO{
	lf_dword nInternalPosition;
}LPK_FILE_INFO_EX;

class LF_SYS2_EXPORTS CLArchive
{
private:
	lf_dword m_nType;
	lf_dword m_nVersion;
	lf_dword m_nCount;
	lf_dword m_nInfoOffset;
	LPK_FILE_INFO_EX* m_pFileList;
	
	BDIO_FILE m_File;
	BDIO_FILE m_TempFile;
	
	lf_bool m_bOpen;
	lf_bool m_bWriteable;
	lf_dword m_nMainFileWritePos;
	lf_bool m_bHasChanged;
	lf_dword m_nCmpThreshold;
	lf_bool ReadArcInfo();
	
	lf_dword DoAddFile(BDIO_FILE fin, lf_cwstr szNameInArc, lf_dword dwFlags);
	lf_bool OpenOld();
public:
	CLArchive();
	~CLArchive();
	
	lf_bool CreateNewW(lf_cwstr szFilename);
	lf_bool CreateNewA(lf_cstr szFilename);
	lf_bool OpenW(lf_cwstr szFilename, lf_dword Flags=0);
	lf_bool OpenA(lf_cstr szFilename, lf_dword Flags=0);
	void Close();
	lf_bool Save();
	
	lf_dword AddFileW(lf_cwstr szFilename, lf_cwstr szNameInArc, lf_dword Flags);
	lf_dword AddFileA(lf_cstr szFilename, lf_cstr szNameInArc, lf_dword Flags);
	lf_dword GetNumFiles();
	lf_bool GetFileInfo(lf_dword nRef, LPK_FILE_INFO* pInfo);
	const LPK_FILE_INFO* GetFileInfo(lf_dword nRef);
	lf_bool IsOpen();
	lf_dword GetFileRef(const lf_pathW szName);
	lf_byte* ExtractFile(lf_byte* pOut, lf_dword nRef);
	
	#ifdef UNICODE
	#define CreateNew CreateNewW
	#define Open OpenW
	#define AddFile AddFileW
	#else !UNICODE
	#define CreateNew CreateNewA
	#define Open OpenA
	#define AddFile AddFileA
	#endif UNICODE
};

#endif __LF_ARC_H__