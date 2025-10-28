#include "lf_sys2.h"
#include "lf_bdio.h"

class LF_SYS2_EXPORTS CLFile
{
friend class CLFileSystem;
//Private types, enums, and constants.
private:
	//static const lf_dword LF_TYPE_FILE=    0x00000001;
	//static const lf_dword LF_TYPE_MEMORY=  0x00000002;
	//static const lf_dword LF_TYPE_ZLIB_CMP=0x00000004;
	//static const lf_dword LF_TYPE_ARCHIVED=0x00000008;
	
//Private members.
private:
	//lf_dword m_nTypeFlags;
	lf_dword m_nAccess;
	lf_dword m_nSize;
	//lf_dword m_nSizeCmp;
	lf_dword m_nFilePointer;
	//lf_pathA m_szPathA;
	lf_pathW m_szPathW;
	lf_pbyte m_pData;
	BDIO_FILE m_BaseFile;
	lf_dword m_nBaseFileBegin; //Where the CLFile starts in the base file (0 if the file is being accessed directly from the disk).

	//lf_bool m_bIsOpen;
	lf_bool m_bEOF;
	
//Types, enums, and constants used with the CLFile class.
public:
	
//Public methods.
public:
	CLFile();
	~CLFile();
	/*
	lf_bool OpenA(lf_cstr szFilename, lf_dword nAccess=ACCESS_READ|ACCESS_WRITE, CREATE_MODE nMode=OPEN_EXISTING);
	lf_bool OpenW(lf_cwstr szFilename, lf_dword nAccess=ACCESS_READ|ACCESS_WRITE, CREATE_MODE nMode=OPEN_EXISTING);
	lf_bool Close();
	*/
	lf_dword Read(lf_void* pOutBuffer, lf_dword nSize);
	lf_dword Write(lf_void* pInBuffer, lf_dword nSize);
	lf_dword Tell();
	lf_dword Seek(LF_SEEK_TYPE nMode, lf_long nOffset);
	lf_dword GetSize();
	const lf_pvoid GetMemPointer();
	lf_bool IsEOF();
};