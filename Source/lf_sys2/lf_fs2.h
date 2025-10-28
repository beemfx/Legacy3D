#ifndef __LF_FS2_H__
#define __LF_FS2_H__

#include "lf_sys2.h"
#include "lf_file.h"
#include "lf_list_stack.h"


class LF_SYS2_EXPORTS CLFileSystem
{
private:
	//Data containing information about a mounted file:
	struct MOUNT_FILE{
		lf_pathW szMountFileW;
		lf_pathW szOSFileW;
		lf_dword Flags;
		lf_dword nSize;
		lf_dword nCmpSize;
		lf_dword nOffset;
		static const lf_dword MOUNT_FILE_ARCHIVED=0x00000001; //This file is archived.
		static const lf_dword MOUNT_FILE_READONLY=0x00000002; //The file is read only.
		static const lf_dword MOUNT_FILE_ZLIBCMP=0x00000004;  //The file is compressed with zlib compression.
		static const lf_dword MOUNT_FILE_DIRECTORY=0x00000008; //The mount file is a directory.
		static const lf_dword MOUNT_FILE_EMPTY=0x00000010;    //This partition table entry is empty and can be mounted over.
	};

	//The partition table is used to store all the mounted file information.
	//It stores an array of MOUNT_FILEs in a hash table and uses hash values
	//to lookup information.
	class LF_SYS2_EXPORTS CPartitionTable
	{
	private:
		struct MOUNT_FILE_EX: public MOUNT_FILE, CLfListStack::LSItem{
			lf_dword nHashValue;
			MOUNT_FILE_EX* pNext;
		};
		
		MOUNT_FILE_EX** m_pHashList;
		
		const lf_dword m_nMaxFiles;
		MOUNT_FILE_EX* m_pMasterList;
		CLfListStack   m_UnusedFiles;
		
		
	public:
		CPartitionTable(lf_dword nMaxFiles);
		~CPartitionTable();
		
		void Clear();
		
		//The MountFile function mounts the specified MOUNT_FILE data.
		//The only flag that is meaningful here is MOUNT_FILE_OVERWRITE
		//and MOUNT_FILE_OVERWRITELPKONLY.
		//which, when specified, will cause a duplicate mount file to
		//overwrite a previous mount file.  If not specifed the duplicate
		//will not be overwritten.  The LPK only will only overwrite a file
		//if the previous file is archived.  If the file is a disk file it
		//will not be overwritten.  This is useful so that new lpk files
		//will take priority over old ones, but disk files still have
		//the highest priority.
		lf_bool MountFile(MOUNT_FILE* pFile, lf_dword Flags);
		
		const CLFileSystem::MOUNT_FILE* FindFile(lf_cwstr szFile);
		
		void PrintMountInfo();
		static void PrintFileInfo(MOUNT_FILE_EX* pFile);
	private:
		static lf_dword GetHashValue1024(lf_cwstr szString);
		static lf_dword GetHashValue1024_Short(lf_cwstr szString);
	};	
	
private:
	const lf_dword m_nMaxFiles;
	CPartitionTable m_PrtTable;
	lf_bool m_bBaseMounted;
	lf_pathW m_szBasePath;
	
	lf_dword MountDirW(lf_cwstr szOSPath, lf_cwstr szMountPath, lf_dword Flags);
	lf_dword MountDirA(lf_cstr szOSPath, lf_cstr szMountPath, lf_dword Flags);
	lf_bool MountFile(MOUNT_FILE* pFile, lf_dword Flags);
	
	const MOUNT_FILE* GetFileInfoW(lf_cwstr szMountPath);
	const MOUNT_FILE* GetFileInfoA(lf_cstr szMountPath);
	
	CLFile* OpenExistingFile(const MOUNT_FILE* pMountInfo, BDIO_FILE File, lf_dword Access, lf_bool bNew);	
public:
	CLFileSystem(lf_dword nMaxFiles);
	~CLFileSystem();
	lf_dword MountBaseA(lf_cstr szOSPath);
	lf_dword MountBaseW(lf_cwstr szOSPath);
	
	lf_dword MountA(lf_cstr szOSPath, lf_cstr szMountPath, lf_dword Flags);
	lf_dword MountW(lf_cwstr szOSPath, lf_cwstr szMountPath, lf_dword Flags);
	
	lf_dword MountLPKA(lf_cstr szMountedFile, lf_dword Flags);
	lf_dword MountLPKW(lf_cwstr szMountedFile, lf_dword Flags);
	
	lf_bool UnMountAll();
	
	CLFile* OpenFileA(lf_cstr szFilename, lf_dword Access, lf_dword CreateMode);
	CLFile* OpenFileW(lf_cwstr szFilename, lf_dword Access, lf_dword CreateMode);
	
	lf_bool CloseFile(CLFile* pFile);
	
	#ifdef UNICODE
	#define MountBase MountBaseW
	#define Mount MountW
	#define MountLPK MountLPKW
	#define OpenFile OpenFileW
	#else !UNICODE
	#define MountBase MountBaseA
	#define Mount MountA
	#define MountLPK MountLPKA
	#define OpenFile OpenFileA
	#endif UNICODE
	
	void PrintMountInfo();
};

#endif __LF_FS2_H__