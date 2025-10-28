#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "lf_fs2.h"
#include "lf_bdio.h"
#include "lf_lpk.h"

//NOTES: The file system is fairly simply, but most of the methods have a
//wide character and multibyte method that way the file system is compatible
//both with Windows 98 and unicode operating systems.  The file system stores
//information about all files in a partition table that uses hash indexes to
//quickly access information about each of the files.

//TODO: Testing on windows 98
//Inusuring complete testing, and better support for the multi-byte methods.


//Typical mounting procedure.
//1) The first thing that needs to be done is to have the base get mounted.
//eg. 
// MountBase(".\"); //This would mount the current directory.
//This will get the base file system mounted.
//2) Mount any subdirectories.
//eg.
// Mount("base", "/base1/", MOUNT_FILE_OVERWRITE|MOUNT_FILE_SUBDIRS);
// Mount("expansionbase", "/base2/", MOUNT_FILE_OVERWRITE|MOUNT_FILE_SUBDIRS);
// This will get the main folders mounted for the application.  Note that
// You could do something as follows:
// Mount("base", "/base1/", MOUNT_FILE_OVERWRITE|MOUNT_FILE_SUBDIRS);
// Mount("expansionbase", "/base1/", MOUNT_FILE_OVERWRITE|MOUNT_FILE_SUBDIRS);
// And that all files in the OS folder expansionbase would take precedence over
// any files in base, but the same mount path would be used no matter what.
//3) Mount any LPK files.
//eg.
// MountLPK("/base1/pak0.lpk", MOUNT_FILE_OVERWRITELPKONLY);
// MountLPK("/base1/pak1.lpk", MOUNT_FILE_OVERWRITELPKONLY);
// MountLPK("/base1/pak2.lpk", MOUNT_FILE_OVERWTITELPKONLY);
// Note that by specifying MOUNT_FILE_OVERWRITELPKONLY any files
// in pak1 would overwrite files in pak0, but not any files that
// are directly on the hard drive.
//4) Proceed to open, close, read, and write files as necessary.

CLFileSystem::CLFileSystem(lf_dword nMaxFiles):
	m_PrtTable(nMaxFiles),
	m_nMaxFiles(nMaxFiles),
	m_bBaseMounted(LF_FALSE)
{
	m_szBasePath[0]=0;
}

CLFileSystem::~CLFileSystem()
{
	UnMountAll();
}

void CLFileSystem::PrintMountInfo()
{
	LF_ErrPrintW(L"Base mounted at \"%s\"", ERR_LEVEL_ALWAYS, this->m_szBasePath);
	LF_ErrPrintW(L"Mounted Files:", ERR_LEVEL_ALWAYS);
	m_PrtTable.PrintMountInfo();
}


//GetFileInfo retrieves information about a mounted file.

const CLFileSystem::MOUNT_FILE* CLFileSystem::GetFileInfoW(lf_cwstr szMountPath)
{
	return m_PrtTable.FindFile(szMountPath);
}

const CLFileSystem::MOUNT_FILE* CLFileSystem::GetFileInfoA(lf_cstr szMountPath)
{
	lf_pathW szPath;
	mbstowcs(szPath, szMountPath, LF_MAX_PATH);
	return m_PrtTable.FindFile(szPath);
}

//Mount base must be called before any other mounting operations are called.
//Mount base basically sets the current directory.  From there on out any 
//calls to mount don't need full path names they only need path names
//relative to the OS path that was mounted at the base.  When MountBase is
//called any files immediately in the base directory are mounted, but not
//subdirectories.  Note that even if a base is mounted, files outside the
//base may still be mounted (for example a CD-ROM drive can be mounted).
//The path to the base is always /.  Note that any files that are created
//are always created relatvie to the base, and not other paths that may
//be mounted.  I want to change that, however, so that a path can be mounted
//to where files should be saved.

lf_dword CLFileSystem::MountBaseW(lf_cwstr szOSPath)
{
	if(m_bBaseMounted)
	{
		LF_ErrPrintW(L"MountBase Error: The base file system is already mounted.  Call UnMountAll before mounting the base.", ERR_LEVEL_ERROR);
		return 0;
	}
	
	if(!BDIO_ChangeDirW(szOSPath))
	{
		LF_ErrPrintW(L"MountBase Error: Could not mount base file system.", ERR_LEVEL_ERROR);
		return 0;
	}
	
	BDIO_GetCurrentDirW(m_szBasePath, LF_MAX_PATH);	
	m_bBaseMounted=LF_TRUE;
	lf_dword nMountCount=MountW(L".", L"/", 0);
	LF_ErrPrintW(L"Mounted \"%s\" to \"/\"", ERR_LEVEL_DETAIL, m_szBasePath);
	return nMountCount;
}

lf_dword CLFileSystem::MountBaseA(lf_cstr szOSPath)
{
	if(m_bBaseMounted)
	{
		LF_ErrPrintW(L"MountBase Error: The base file system is already mounted.  Call UnMountAll before mounting the base.", ERR_LEVEL_ERROR);
		return 0;
	}
	
	if(!BDIO_ChangeDirA(szOSPath))
	{
		LF_ErrPrintW(L"MountBase Error: Could not mount base file system.", ERR_LEVEL_ERROR);
		return 0;
	}
	
	lf_pathA szBasePath;
	BDIO_GetCurrentDirA(szBasePath, LF_MAX_PATH);
	mbstowcs(m_szBasePath, szBasePath, LF_MAX_PATH);
		
	m_bBaseMounted=LF_TRUE;
	lf_dword nMountCount=MountA(".", "/", 0);
	LF_ErrPrintW(L"Mounted \"%s\" to \"/\"", ERR_LEVEL_DETAIL, m_szBasePath);
	return nMountCount;
}

//MountLPK mounts a Legacy PaKage file into the file system.  Note that before
//an LPK can be mounted the actually LPK file must be within the file system.
//Then when MountLPK is called the path to the mounted file (e.g. /base1/pak0.lpk)
//should be specifed as a parameter, and not the os path (e.g. NOT .\base\pak0.lpk).
//This method will expand the archive file into the current directory, so if
//a file was stored in the LPK as Credits.txt it would now be /base1/Credits.txt
//and Textures/Tex1.tga would be /base1/Textures/Tex1.tga.
//Note that for the flags either MOUNT_FILE_OVERWRITE or MOUNT_FILE_OVERWRITELPKONLY
//may be specifed.
//
//MOUNT_FILE_OVERWRITE:  If any file with the same mout path as the new file
//already exists, then the new file will take precedence over the old file
//that is to say that when calls to open a file with the specified name
//are made the file most recently mounted file will be opened (it doens't
//mean that the OS file will be overwritten, because it won't).
//
//MOUNT_FILE_OVERWRITELPKONLY: Is similar to the above flag, but a newer file
//will only take precedence over other archived files.  So fore example if two
//LPK files have a file with the same name stored in them, the most recently
//mounted LPK file will take precedence, but if there was a file directly on
//the hard drive, the hard drive file will take precedence, whether or not
//the archive was mounted first or second.
//
//If neither flag is specified files will never get overwritten and the first
//file to be mounted will always take precidence.

lf_dword CLFileSystem::MountLPKW(lf_cwstr szMountedFile, lf_dword Flags)
{
	const MOUNT_FILE* pMountFile=GetFileInfoW(szMountedFile);
	if(!pMountFile)
	{
		LF_ErrPrintW(L"MountLPK Error: The archive file \"%s\" has not been mounted.", ERR_LEVEL_ERROR, szMountedFile);
		return 0;
	}
	lf_pathW szMountDir;
	LF_GetDirFromPathW(szMountDir, szMountedFile);
	LF_ErrPrintW(L"Expanding \"%s\" to \"%s\"", ERR_LEVEL_NOTICE, szMountedFile, szMountDir);
	
	CLArchive lpkFile;
	if(!lpkFile.OpenW(pMountFile->szOSFileW))
	{
		LF_ErrPrintW(L"Could not mount \"%s\".  Possibly not a valid archive.", ERR_LEVEL_ERROR, szMountedFile);
		return 0;
	}
	
	lf_dword dwMounted=0;
	for(lf_dword i=0; i<lpkFile.GetNumFiles(); i++)
	{
		LPK_FILE_INFO fileInfo;
		if(!lpkFile.GetFileInfo(i, &fileInfo))
			continue;
		
		MOUNT_FILE mountFile;
		swprintf(mountFile.szMountFileW, LF_MAX_PATH, L"%s%s", szMountDir, fileInfo.szFilename);
		swprintf(mountFile.szOSFileW, LF_MAX_PATH, L"%s", pMountFile->szOSFileW);
		mountFile.nSize=fileInfo.nSize;
		mountFile.nCmpSize=fileInfo.nCmpSize;
		mountFile.nOffset=fileInfo.nOffset;	
		mountFile.Flags=MOUNT_FILE::MOUNT_FILE_ARCHIVED|MOUNT_FILE::MOUNT_FILE_READONLY;
		if(fileInfo.nType==LPK_FILE_TYPE_ZLIBCMP)
			mountFile.Flags|=MOUNT_FILE::MOUNT_FILE_ZLIBCMP;
		
		if(MountFile(&mountFile, Flags))
			dwMounted++;
		else
		{
			LF_ErrPrintW(L"Mount Error: Could not mount \"%s\".", ERR_LEVEL_ERROR, mountFile.szOSFileW);
		}
	}
	return dwMounted;
}

lf_dword CLFileSystem::MountLPKA(lf_cstr szMountedFile, lf_dword Flags)
{
	lf_pathW szMountedFileW;
	mbstowcs(szMountedFileW, szMountedFile, LF_MAX_PATH);
	
	const MOUNT_FILE* pMountFile=GetFileInfoA(szMountedFile);
	if(!pMountFile)
	{
		LF_ErrPrintW(L"MountLPK Error: The archive file \"%s\" has not been mounted.", ERR_LEVEL_ERROR, szMountedFileW);
		return 0;
	}
	lf_pathW szMountDir;
	LF_GetDirFromPathW(szMountDir, szMountedFileW);
	LF_ErrPrintW(L"Expanding \"%s\" to \"%s\"", ERR_LEVEL_NOTICE, szMountedFileW, szMountDir);
	
	lf_pathA szOSFileA;
	wcstombs(szOSFileA, pMountFile->szOSFileW, LF_MAX_PATH);
	
	CLArchive lpkFile;
	if(!lpkFile.OpenA(szOSFileA))
	{
		LF_ErrPrintW(L"Could not mount \"%s\".  Possibly not a valid archive.", ERR_LEVEL_ERROR, szMountedFile);
		return 0;
	}
	
	lf_dword dwMounted=0;
	for(lf_dword i=0; i<lpkFile.GetNumFiles(); i++)
	{
		LPK_FILE_INFO fileInfo;
		if(!lpkFile.GetFileInfo(i, &fileInfo))
			continue;
		
		MOUNT_FILE mountFile;
		swprintf(mountFile.szMountFileW, LF_MAX_PATH, L"%s%s", szMountDir, fileInfo.szFilename);
		swprintf(mountFile.szOSFileW, LF_MAX_PATH, L"%s", pMountFile->szOSFileW);
		mountFile.nSize=fileInfo.nSize;
		mountFile.nCmpSize=fileInfo.nCmpSize;
		mountFile.nOffset=fileInfo.nOffset;	
		mountFile.Flags=MOUNT_FILE::MOUNT_FILE_ARCHIVED|MOUNT_FILE::MOUNT_FILE_READONLY;
		if(fileInfo.nType==LPK_FILE_TYPE_ZLIBCMP)
			mountFile.Flags|=MOUNT_FILE::MOUNT_FILE_ZLIBCMP;
		
		if(MountFile(&mountFile, Flags))
			dwMounted++;
		else
		{
			LF_ErrPrintW(L"Mount Error: Could not mount \"%s\".", ERR_LEVEL_ERROR, mountFile.szOSFileW);
		}
	}
	return dwMounted;
}



lf_dword CLFileSystem::MountDirW(
	lf_cwstr szOSPath, 
	lf_cwstr szMount, 
	lf_dword Flags)
{
	lf_dword nMountCount=0;
	lf_bool bRes=0;
	
	lf_pathW szSearchPath;
	_snwprintf(szSearchPath, LF_MAX_PATH, L"%s\\*", szOSPath);
	//Recursivley mount all files...
	BDIO_FIND_DATAW sData;
	BDIO_FIND hFind=BDIO_FindFirstFileW(szSearchPath, &sData);
	if(!hFind)
	{
		LF_ErrPrintW(
			L"Mount Error: Could not mount any files.  \"%s\" may not exist.",
			ERR_LEVEL_ERROR,
			szOSPath);
		return 0;
	}
	//Mount the directory, itself...
	MOUNT_FILE mountInfo;
	mountInfo.nCmpSize=mountInfo.nOffset=mountInfo.nSize=0;
	mountInfo.Flags=MOUNT_FILE::MOUNT_FILE_DIRECTORY;
	wcsncpy(mountInfo.szOSFileW, szOSPath, LF_MAX_PATH);
	wcsncpy(mountInfo.szMountFileW, szMount, LF_MAX_PATH);
	//Directories take the same overwrite priority, so overwritten
	//directories will be used when creating new files.
	bRes=MountFile(&mountInfo, Flags);
	if(!bRes)
	{
		LF_ErrPrintW(L"Mount Error: Could not mount \"%s\".", ERR_LEVEL_ERROR, mountInfo.szOSFileW);
		BDIO_FindClose(hFind);
		return nMountCount;
	}
	
	do
	{
		//Generate the mounted path and OS path..
		MOUNT_FILE sMountFile;
		_snwprintf(sMountFile.szOSFileW, LF_MAX_PATH, L"%s\\%s", szOSPath, sData.szFilename);
		_snwprintf(sMountFile.szMountFileW, LF_MAX_PATH, L"%s%s", szMount, sData.szFilename);
			
		//If a directory was found mount that directory...
		if(sData.bDirectory)
		{
			//Ignore . and .. directories.
			if(sData.szFilename[0]=='.')
				continue;
				
			if(LF_CHECK_FLAG(Flags, MOUNT_MOUNT_SUBDIRS))
			{
				wcsncat(sMountFile.szMountFileW, L"/", LF_min(1, LF_MAX_PATH-wcslen(sMountFile.szMountFileW)));
				nMountCount+=MountDirW(sMountFile.szOSFileW, sMountFile.szMountFileW, Flags);
			}
		}
		else
		{
			if(sData.nFileSize.dwHighPart)
			{
				LF_ErrPrintW(L"\"%s\" is too large to mount!", ERR_LEVEL_ERROR, sMountFile.szOSFileW);
				continue;
			}
			
			sMountFile.nSize=sData.nFileSize.dwLowPart;
			sMountFile.nCmpSize=sMountFile.nSize;
			sMountFile.nOffset=0;
			sMountFile.Flags=0;
			if(sData.bReadOnly)
				sMountFile.Flags|=MOUNT_FILE::MOUNT_FILE_READONLY;
			if(MountFile(&sMountFile, Flags))
				nMountCount++;
			else
				LF_ErrPrintW(L"Mount Error: Could not mount \"%s\".", ERR_LEVEL_ERROR, mountInfo.szOSFileW);
		}
	}while(BDIO_FindNextFileW(hFind, &sData));
	
	BDIO_FindClose(hFind);
	
	return nMountCount;
}

lf_dword CLFileSystem::MountDirA(
	lf_cstr szOSPath, 
	lf_cstr szMount, 
	lf_dword Flags)
{
	lf_dword nMountCount=0;
	lf_bool bRes=0;
	
	lf_pathA szSearchPath;
	_snprintf(szSearchPath, LF_MAX_PATH, "%s\\*", szOSPath);
	//Recursivley mount all files...
	BDIO_FIND_DATAA sData;
	BDIO_FIND hFind=BDIO_FindFirstFileA(szSearchPath, &sData);
	if(!hFind)
	{
		LF_ErrPrintA(
			"Mount Error: Could not mount any files.  \"%s\" may not exist.",
			ERR_LEVEL_ERROR,
			szOSPath);
		return 0;
	}
	
	//Mount the directory, itself...
	MOUNT_FILE mountInfo;
	mountInfo.nCmpSize=mountInfo.nOffset=mountInfo.nSize=0;
	mountInfo.Flags=MOUNT_FILE::MOUNT_FILE_DIRECTORY;
	mbstowcs(mountInfo.szOSFileW, szOSPath, LF_MAX_PATH);
	mbstowcs(mountInfo.szMountFileW, szMount, LF_MAX_PATH);
	//Directories take the same overwrite priority, so overwritten
	//directories will be used when creating new files.
	bRes=MountFile(&mountInfo, Flags);
	if(!bRes)
	{
		LF_ErrPrintW(L"Mount Error: Could not mount \"%s\".", ERR_LEVEL_ERROR, mountInfo.szOSFileW);
		BDIO_FindClose(hFind);
		return nMountCount;
	}
	
	do
	{
		//Generate the mounted path and OS path..
		MOUNT_FILE sMountFile;
		lf_pathA szOSFileA, szMountFileA;
		_snprintf(szOSFileA, LF_MAX_PATH, "%s\\%s", szOSPath, sData.szFilename);
		_snprintf(szMountFileA, LF_MAX_PATH, "%s%s", szMount, sData.szFilename);
		
		mbstowcs(sMountFile.szOSFileW, szOSFileA, LF_MAX_PATH);
		mbstowcs(sMountFile.szMountFileW, szMountFileA, LF_MAX_PATH);
			
		//If a directory was found mount that directory...
		if(sData.bDirectory)
		{
			//Ignore . and .. directories.
			if(sData.szFilename[0]=='.')
				continue;
				
			if(LF_CHECK_FLAG(Flags, MOUNT_MOUNT_SUBDIRS))
			{
				strncat(szMountFileA, "/", LF_min(1, LF_MAX_PATH-strlen(szMountFileA)));
				nMountCount+=MountDirA(szOSFileA, szMountFileA, Flags);
			}
		}
		else
		{
			if(sData.nFileSize.dwHighPart)
			{
				LF_ErrPrintW(L"\"%s\" is too large to mount!", ERR_LEVEL_ERROR, sMountFile.szOSFileW);
				continue;
			}
			
			sMountFile.nSize=sData.nFileSize.dwLowPart;
			sMountFile.nCmpSize=sMountFile.nSize;
			sMountFile.nOffset=0;
			sMountFile.Flags=0;
			if(sData.bReadOnly)
				sMountFile.Flags|=MOUNT_FILE::MOUNT_FILE_READONLY;
			if(MountFile(&sMountFile, Flags))
				nMountCount++;
			else
				LF_ErrPrintW(L"Mount Error: Could not mount \"%s\".", ERR_LEVEL_ERROR, mountInfo.szOSFileW);
		}
	}while(BDIO_FindNextFileA(hFind, &sData));
	
	BDIO_FindClose(hFind);
	
	return nMountCount;
}

//The Mount method mounts a directory to the file
//system.  If the MOUNT_MOUNT_SUBDIRS flag is specifed
//then all subdirectories will be mounted as well.

lf_dword CLFileSystem::MountW(
	lf_cwstr szOSPathToMount, 
	lf_cwstr szMountToPath, 
	lf_dword Flags)
{
	lf_dword nMountCount=0;
	lf_pathW szMount;
	lf_pathW szOSPath;
	
	//Get the full path to the specified directory...
	BDIO_GetFullPathNameW(szOSPath, szOSPathToMount);
	//Insure that the mounting path does not have a / or \.
	size_t nLast=wcslen(szOSPath)-1;
	if(szOSPath[nLast]==L'/' || szOSPath[nLast]==L'\\')
		szOSPath[nLast]=0;
	
	if(szMountToPath[0]!='/')
	{
		LF_ErrPrintW(
			L"The path must be mounted at least to the base.  Try mount \"%s\" \"/%s\" instead.",
			ERR_LEVEL_WARNING,
			szOSPathToMount,
			szMountToPath);
			
		return 0;
	}
	
	//Insure that the mount path has a / at the end of it
	if(szMountToPath[wcslen(szMountToPath)-1]!=L'/')
		_snwprintf(szMount, LF_MAX_PATH, L"%s/", szMountToPath);
	else
		_snwprintf(szMount, LF_MAX_PATH, L"%s", szMountToPath);
		
	LF_ErrPrintW(L"Mounting \"%s\" to \"%s\"...", ERR_LEVEL_NOTICE, szOSPath, szMount);
	
	return MountDirW(szOSPath, szMount, Flags);
}

lf_dword CLFileSystem::MountA(
	lf_cstr szOSPathToMount, 
	lf_cstr szMountToPath, 
	lf_dword Flags)
{
	lf_dword nMountCount=0;
	lf_pathA szMount;
	lf_pathA szOSPath;
	
	//Get the full path to the specified directory...
	BDIO_GetFullPathNameA(szOSPath, szOSPathToMount);
	//Insure that the mountine path does not have a / or \.
	size_t nLast=strlen(szOSPath)-1;
	if(szOSPath[nLast]=='/' || szOSPath[nLast]=='\\')
		szOSPath[nLast]=0;
	
	if(szMountToPath[0]!='/')
	{
		LF_ErrPrintW(
			L"The path must be mounted at least to the base.  Try mount \"%s\" \"/%s\" instead.",
			ERR_LEVEL_WARNING,
			szOSPathToMount,
			szMountToPath);
			
		return 0;
	}
	
	//Insure that the mount path has a / at the end of it
	if(szMountToPath[strlen(szMountToPath)-1]!=L'/')
		_snprintf(szMount, LF_MAX_PATH, "%s/", szMountToPath);
	else
		_snprintf(szMount, LF_MAX_PATH, "%s", szMountToPath);
		
	lf_pathW szOSPathW, szMountW;
	mbstowcs(szOSPathW, szOSPath, LF_MAX_PATH);
	mbstowcs(szMountW, szMount, LF_MAX_PATH);
	LF_ErrPrintW(L"Mounting \"%s\" to \"%s\"...", ERR_LEVEL_NOTICE, szOSPathW, szMountW);
	
	return MountDirA(szOSPath, szMount, Flags);
}


lf_bool CLFileSystem::UnMountAll()
{
	LF_ErrPrintW(L"Clearing partition table...", ERR_LEVEL_NOTICE);
	m_PrtTable.Clear();
	LF_ErrPrintW(L"Unmounting the base...", ERR_LEVEL_NOTICE);
	m_bBaseMounted=LF_FALSE;
	m_szBasePath[0]=0;
	return LF_TRUE;
}

lf_bool CLFileSystem::MountFile(MOUNT_FILE* pFile, lf_dword dwFlags)
{
	LF_ErrPrintW(L"Mounting \"%s\"", ERR_LEVEL_SUPERDETAIL, pFile->szMountFileW);
	return m_PrtTable.MountFile(pFile, dwFlags);
}

/*********************************************************
	File opening code, note that files are opened and
	closed using the file system, but they are accessed
	(read and written to) using the CLFile class.
*********************************************************/
CLFile* CLFileSystem::OpenFileW(lf_cwstr szFilename, lf_dword Access, lf_dword CreateMode)
{
	lf_bool bOpenExisting=LF_TRUE;
	lf_bool bNew=LF_FALSE;
	
	//Check the flags to make sure they are compatible.
	if(LF_CHECK_FLAG(Access, LF_ACCESS_WRITE) && LF_CHECK_FLAG(Access, LF_ACCESS_MEMORY))
	{
		LF_ErrPrintW(L"OpenFile Error: Cannot open a file with memory access and write access.", ERR_LEVEL_ERROR);
		return LF_NULL;
	}
	//Note that if creating a new file, then we shouldn't end here.
	const MOUNT_FILE* pMountInfo=GetFileInfoW(szFilename);
	if(CreateMode==LF_OPEN_EXISTING)
	{
		if(!pMountInfo)
		{
			LF_ErrPrintW(L"OpenFile Error: \"%s\" could not be found in the file system.", ERR_LEVEL_ERROR, szFilename);
			return LF_NULL;
		}
		bOpenExisting=LF_TRUE;
		bNew=LF_FALSE;
	}
	else if(CreateMode==LF_OPEN_ALWAYS)
	{
		bOpenExisting=pMountInfo?LF_TRUE:LF_FALSE;
		bNew=LF_FALSE;
	}
	else if(CreateMode==LF_CREATE_NEW)
	{
		if(pMountInfo)
		{
			LF_ErrPrintW(L"OpenFile Error: Cannot create \"%s\", it already exists.", ERR_LEVEL_ERROR, szFilename);
			return LF_NULL;
		}
		bOpenExisting=LF_FALSE;
		bNew=LF_TRUE;
	}
	else if(CreateMode==LF_CREATE_ALWAYS)
	{
		if(pMountInfo)
		{
			bOpenExisting=LF_TRUE;
			bNew=LF_TRUE;
		}
		else
		{
			bOpenExisting=LF_FALSE;
			bNew=LF_TRUE;
		}
	}
	else
	{
		LF_ErrPrintW(L"OpenFile Error: Invalid creation mode specifed.", ERR_LEVEL_ERROR);
		return LF_NULL;
	}
	
	//If the file is mounted then we are opening an existing file.
	if(bOpenExisting)
	{
		if(LF_CHECK_FLAG(pMountInfo->Flags, MOUNT_FILE::MOUNT_FILE_DIRECTORY))
		{
			LF_ErrPrintW(L"OpenFile Error: \"%s\" is a directory, not a file.", ERR_LEVEL_ERROR, pMountInfo->szMountFileW);
			return LF_NULL;
		}
		//If we're trying to create a new file, and the existing file is read
		//only then we can't.
		if(LF_CHECK_FLAG(pMountInfo->Flags, MOUNT_FILE::MOUNT_FILE_READONLY) && bNew)
		{
			LF_ErrPrintW(L"OpenFile Error: \"%s\" cannot be created, because an existing file is read only.",
				ERR_LEVEL_ERROR,
				szFilename);
		
			return LF_NULL;
		}
		BDIO_FILE bdioFile=BDIO_OpenW(pMountInfo->szOSFileW, bNew?BDIO_CREATE_ALWAYS:BDIO_OPEN_EXISTING, Access);
		if(!bdioFile)
		{
			LF_ErrPrintW(L"Could not open the OS file: \"%s\".", ERR_LEVEL_ERROR, pMountInfo->szOSFileW);
			return LF_NULL;
		}
		return OpenExistingFile(pMountInfo, bdioFile, Access, bNew);
	}
	else
	{
		//TODO: Should also check to see if it is likely that the
		//file exists (because some OSs are not case sensitive
		//in which case the file may exist, but the path specified
		//may be a mounted file.
		
		//If it is necessary to create a new file, then we need to prepare
		//the mount info for the new file, as well as create an OS file.
		MOUNT_FILE mountInfo;
		//Get the specifie directory for the file,
		//then find the information about that directory in
		//the partition table.
		lf_pathW szDir;
		LF_GetDirFromPathW(szDir, szFilename);
		//The mount filename will be the same
		wcsncpy(mountInfo.szMountFileW, szFilename, LF_MAX_PATH);
		const MOUNT_FILE* pDirInfo=GetFileInfoW(szDir);
		if(!pDirInfo || !LF_CHECK_FLAG(pDirInfo->Flags, MOUNT_FILE::MOUNT_FILE_DIRECTORY))
		{
			LF_ErrPrintW(
				L"OpenFile Error: Cannot create the specified file, the directory \"%s\" does not exist.",
				ERR_LEVEL_ERROR,
				szDir);
				
			return LF_NULL;
		}
		//Get just the filename for the file,
		//this will be used to create the OS file.
		lf_pathW szFile;
		LF_GetFileNameFromPathW(szFile, szFilename);
		//The OS filename is the information from the dir + the filename
		//Note that the OS separator must be inserted.
		_snwprintf(mountInfo.szOSFileW, LF_MAX_PATH, L"%s\\%s", pDirInfo->szOSFileW, szFile);
		
		//The initial file information is all zeroes.
		mountInfo.Flags=0;
		mountInfo.nCmpSize=0;
		mountInfo.nSize=0;
		mountInfo.nOffset=0;
		
		//Create the OS file.
		BDIO_FILE bdioFile=BDIO_OpenW(mountInfo.szOSFileW, BDIO_CREATE_NEW, Access);
		if(!bdioFile)
		{
			LF_ErrPrintW(
				L"OpenFile Error: Could not create new file: \"%s\".",
				ERR_LEVEL_ERROR,
				mountInfo.szMountFileW);
				
			return LF_NULL;
		}
		//If the OS file was created, then we can mount the new file
		//and open the existing file.
		MountFile(&mountInfo, MOUNT_FILE_OVERWRITE);
		return OpenExistingFile(&mountInfo, bdioFile, Access, LF_TRUE);
	}
}

CLFile* CLFileSystem::OpenFileA(lf_cstr szFilename, lf_dword Access, lf_dword CreateMode)
{
	lf_bool bOpenExisting=LF_TRUE;
	lf_bool bNew=LF_FALSE;
	
	//lf_pathW szFilenameW;
	//mbstowcs(szFilenameW, szFilename, LF_MAX_PATH);
	
	//Check the flags to make sure they are compatible.
	if(LF_CHECK_FLAG(Access, LF_ACCESS_WRITE) && LF_CHECK_FLAG(Access, LF_ACCESS_MEMORY))
	{
		LF_ErrPrintA("OpenFile Error: Cannot open a file with memory access and write access.", ERR_LEVEL_ERROR);
		return LF_NULL;
	}
	//Note that if creating a new file, then we shouldn't end here.
	const MOUNT_FILE* pMountInfo=GetFileInfoA(szFilename);

	if(CreateMode==LF_OPEN_EXISTING)
	{
		if(!pMountInfo)
		{
			LF_ErrPrintA("OpenFile Error: \"%s\" could not be found in the file system.", ERR_LEVEL_ERROR, szFilename);
			return LF_NULL;
		}
		bOpenExisting=LF_TRUE;
		bNew=LF_FALSE;
	}
	else if(CreateMode==LF_OPEN_ALWAYS)
	{
		bOpenExisting=pMountInfo?LF_TRUE:LF_FALSE;
		bNew=LF_FALSE;
	}
	else if(CreateMode==LF_CREATE_NEW)
	{
		if(pMountInfo)
		{
			LF_ErrPrintA("OpenFile Error: Cannot create \"%s\", it already exists.", ERR_LEVEL_ERROR, szFilename);
			return LF_NULL;
		}
		bOpenExisting=LF_FALSE;
		bNew=LF_TRUE;
	}
	else if(CreateMode==LF_CREATE_ALWAYS)
	{
		if(pMountInfo)
		{
			bOpenExisting=LF_TRUE;
			bNew=LF_TRUE;
		}
		else
		{
			bOpenExisting=LF_FALSE;
			bNew=LF_TRUE;
		}
	}
	else
	{
		LF_ErrPrintA("OpenFile Error: Invalid creation mode specifed.", ERR_LEVEL_ERROR);
		return LF_NULL;
	}
	
	//If the file is mounted then we are opening an existing file.
	if(bOpenExisting)
	{
		if(LF_CHECK_FLAG(pMountInfo->Flags, MOUNT_FILE::MOUNT_FILE_DIRECTORY))
		{
			LF_ErrPrintW(L"OpenFile Error: \"%s\" is a directory, not a file.", ERR_LEVEL_ERROR, pMountInfo->szMountFileW);
			return LF_NULL;
		}
		//If we're trying to create a new file, and the existing file is read
		//only then we can't.
		if(LF_CHECK_FLAG(pMountInfo->Flags, MOUNT_FILE::MOUNT_FILE_READONLY) && bNew)
		{
			LF_ErrPrintA("OpenFile Error: \"%s\" cannot be created, because an existing file is read only.",
				ERR_LEVEL_ERROR,
				szFilename);
		
			return LF_NULL;
		}
		lf_pathA szOSFileA;
		wcstombs(szOSFileA, pMountInfo->szOSFileW, LF_MAX_PATH);
		BDIO_FILE bdioFile=BDIO_OpenA(szOSFileA, bNew?BDIO_CREATE_ALWAYS:BDIO_OPEN_EXISTING, Access);
		if(!bdioFile)
		{
			LF_ErrPrintW(L"Could not open the OS file: \"%s\".", ERR_LEVEL_ERROR, pMountInfo->szOSFileW);
			return LF_NULL;
		}
		return OpenExistingFile(pMountInfo, bdioFile, Access, bNew);
	}
	else
	{
		//Should also check to see if it is likely that the
		//file exists (because some OSs are not case sensitive
		//in which case the file may exist, but the path specified
		//may be a mounted file.
		
		//If it is necessary to create a new file, then we need to prepare
		//the mount info for the new file, as well as create an OS file.
		MOUNT_FILE mountInfo;
		//Get the specifie directory for the file,
		//then find the information about that directory in
		//the partition table.
		lf_pathA szDir;
		LF_GetDirFromPathA(szDir, szFilename);
		//The mount filename will be the same
		mbstowcs(mountInfo.szMountFileW, szFilename, LF_MAX_PATH);
		const MOUNT_FILE* pDirInfo=GetFileInfoA(szDir);
		if(!pDirInfo || !LF_CHECK_FLAG(pDirInfo->Flags, MOUNT_FILE::MOUNT_FILE_DIRECTORY))
		{
			LF_ErrPrintA(
				"OpenFile Error: Cannot create the specified file, the directory \"%s\" does not exist.",
				ERR_LEVEL_ERROR,
				szDir);
				
			return LF_NULL;
		}
		//Get just the filename for the file,
		//this will be used to create the OS file.
		lf_pathA szFile;
		lf_pathW szFileW;
		LF_GetFileNameFromPathA(szFile, szFilename);
		mbstowcs(szFileW, szFile, LF_MAX_PATH);
		//The OS filename is the information from the dir + the filename
		//Note that the OS separator must be inserted.
		_snwprintf(mountInfo.szOSFileW, LF_MAX_PATH, L"%s\\%s", pDirInfo->szOSFileW, szFileW);
		
		//The initial file information is all zeroes.
		mountInfo.Flags=0;
		mountInfo.nCmpSize=0;
		mountInfo.nSize=0;
		mountInfo.nOffset=0;
		
		//Create the OS file.
		lf_pathA szOSFileA;
		wcstombs(szOSFileA, mountInfo.szOSFileW, LF_MAX_PATH);
		BDIO_FILE bdioFile=BDIO_OpenA(szOSFileA, BDIO_CREATE_NEW, Access);
		if(!bdioFile)
		{
			LF_ErrPrintW(
				L"OpenFile Error: Could not create new file: \"%s\".",
				ERR_LEVEL_ERROR,
				mountInfo.szMountFileW);
				
			return LF_NULL;
		}
		//If the OS file was created, then we can mount the new file
		//and open the existing file.
		MountFile(&mountInfo, MOUNT_FILE_OVERWRITE);
		return OpenExistingFile(&mountInfo, bdioFile, Access, LF_TRUE);
	}
}

CLFile* CLFileSystem::OpenExistingFile(const MOUNT_FILE* pMountInfo, BDIO_FILE File, lf_dword Access, lf_bool bNew)
{
	//First thing's first, allocate memory for the file.
	CLFile* pFile=new CLFile;
	if(!pFile)
	{
		LF_ErrPrintW(L"OpenFile Error: Could not allocate memory for file.", ERR_LEVEL_ERROR);
		return LF_NULL;
	}
	
	//Initialize the file.
	pFile->m_BaseFile=File;
	pFile->m_bEOF=LF_FALSE;
	pFile->m_nAccess=Access;
	pFile->m_nBaseFileBegin=pMountInfo->nOffset;
	pFile->m_nFilePointer=0;
	//If creating a new file, the size needs to be set to 0.
	pFile->m_nSize=bNew?0:pMountInfo->nSize;
	pFile->m_pData=LF_NULL;
	wcsncpy(pFile->m_szPathW, pMountInfo->szMountFileW, LF_MAX_PATH);
	
	if(LF_CHECK_FLAG(pMountInfo->Flags, MOUNT_FILE::MOUNT_FILE_READONLY) && bNew)
	{
		LF_ErrPrintW(L"OpenFile Error: \"%s\" cannot be created, because an existing file is read only.",
			ERR_LEVEL_ERROR,
			pFile->m_szPathW);
		
		BDIO_Close(pFile->m_BaseFile);
		delete pFile;
		return LF_NULL;
	}
	
	//Check infor flags again.
	if(LF_CHECK_FLAG(pFile->m_nAccess, LF_ACCESS_WRITE) 
		&& LF_CHECK_FLAG(pMountInfo->Flags, MOUNT_FILE::MOUNT_FILE_READONLY))
	{
		LF_ErrPrintW(L"OpenFile Error: \"%s\" is read only, cannot open for writing.", ERR_LEVEL_ERROR, pFile->m_szPathW);
		BDIO_Close(pFile->m_BaseFile);
		delete pFile;
		return LF_NULL;
	}
	
	LF_ErrPrintW(L"Opening \"%s\"...", ERR_LEVEL_DETAIL, pFile->m_szPathW);
	//If the file is in an archive and is compressed it needs to be opened as a
	//memory file.
	if(LF_CHECK_FLAG(pMountInfo->Flags, MOUNT_FILE::MOUNT_FILE_ARCHIVED)
		&& LF_CHECK_FLAG(pMountInfo->Flags, MOUNT_FILE::MOUNT_FILE_ZLIBCMP))
	{
		pFile->m_nAccess|=LF_ACCESS_MEMORY;
	}
	
	//If the file is a memory file then we just open it,
	//allocate memory for it, and copy the file data into
	//the buffer.
	if(LF_CHECK_FLAG(pFile->m_nAccess, LF_ACCESS_MEMORY))
	{
		//Allocate memory...
		pFile->m_pData=new lf_byte[pFile->m_nSize];
		if(!pFile->m_pData)
		{
			LF_ErrPrintW(L"OpenFile Error: Could not allocte memory for \"%s\".", ERR_LEVEL_ERROR, pFile->m_szPathW);
			BDIO_Close(pFile->m_BaseFile);
			delete pFile;
			return LF_NULL;
		}
		//Read the file data...
		lf_dword nRead=0;
		//Seek to the beginning of the file...
		BDIO_Seek(pFile->m_BaseFile, pFile->m_nBaseFileBegin, BDIO_SEEK_BEGIN);
		//Then read either the compress data, or the uncompressed data.
		if(LF_CHECK_FLAG(pMountInfo->Flags, MOUNT_FILE::MOUNT_FILE_ZLIBCMP))
		{
			LF_ErrPrintW(
				L"Reading and uncompressing \"%s\"...",
				ERR_LEVEL_DETAIL,
				pFile->m_szPathW);
			nRead=BDIO_ReadCompressed(pFile->m_BaseFile, pFile->m_nSize, pFile->m_pData);
		}
		else
		{
			LF_ErrPrintW(
				L"Reading \"%s\"...",
				ERR_LEVEL_DETAIL,
				pFile->m_szPathW);
			nRead=BDIO_Read(pFile->m_BaseFile, pFile->m_nSize, pFile->m_pData);
		}
		
		if(nRead!=pFile->m_nSize)
		{
			LF_ErrPrintW(
				L"OpenFile Warning: Only read %d out of %d bytes in \"%s\".", 
				ERR_LEVEL_WARNING, 
				nRead, 
				pFile->m_nSize, 
				pFile->m_szPathW);
				
			
			pFile->m_nSize=nRead;	
		}
		
		//We have the file open in memory so we no longer need to keep the
		//BDIO file open.
		BDIO_Close(pFile->m_BaseFile);
		pFile->m_BaseFile=LF_NULL;
	}
	else
	{
		//The file is not being opened as a memory file.
		//So we just leave it as it is and all file
		//reading/seeking functions will take care of everything.
	}
	pFile->m_bEOF=pFile->m_nFilePointer>=pFile->m_nSize;
	return pFile;
}

lf_bool CLFileSystem::CloseFile(CLFile* pFile)
{
	lf_dword nSize=0;
	
	if(!pFile)
		return LF_FALSE;
	LF_ErrPrintW(L"Closing \"%s\"...", ERR_LEVEL_DETAIL, pFile->m_szPathW);
	if(pFile->m_BaseFile)
	{
		nSize=BDIO_GetSize(pFile->m_BaseFile);
		BDIO_Close(pFile->m_BaseFile);
	}	
	LF_SAFE_DELETE_ARRAY(pFile->m_pData);
	//If the file was writeable we need to update the data in the
	//partition table.
	if(LF_CHECK_FLAG(pFile->m_nAccess, LF_ACCESS_WRITE))
	{
		const MOUNT_FILE* pMountInfo=GetFileInfoW(pFile->m_szPathW);
		if(!pMountInfo)
		{
			LF_ErrPrintW(L"CloseFile Error: Could not update partition table.", ERR_LEVEL_ERROR);
		}
		else
		{
			MOUNT_FILE mountInfo=*pMountInfo;
			mountInfo.nSize=nSize;
			mountInfo.nCmpSize=nSize;
			m_PrtTable.MountFile(&mountInfo, MOUNT_FILE_OVERWRITE);
		}
	}
	LF_SAFE_DELETE(pFile);
	return LF_TRUE;
}

/********************************************************
*** The Partition Table Code
*** A partition table keeps track of mounted files
*** the table stores information about each file.
*** 
*** The partition table stores all the file information
*** in a hash table with 1024 entries.  If a filename
*** has a duplicate hash value then the table points
*** to a linked list with all the filenames.
********************************************************/
CLFileSystem::CPartitionTable::CPartitionTable(lf_dword nMaxFiles):
	m_pHashList(LF_NULL),
	m_nMaxFiles(nMaxFiles),
	m_pMasterList(LF_NULL)
{
	//The hash table is always 1024 in size, if any dupicate
	//hashes are generated, then there will be a linked list at the hash entry.
	m_pHashList=new MOUNT_FILE_EX*[1024];
	if(!m_pHashList)
		return;
		
	//Create an initialize the master list
	m_pMasterList=new MOUNT_FILE_EX[nMaxFiles];
	if(!m_pMasterList)
	{
		LF_SAFE_DELETE_ARRAY(m_pHashList);
		return;
	}
	m_UnusedFiles.Init(m_pMasterList, nMaxFiles, sizeof(MOUNT_FILE_EX));
		
	
	for(lf_dword i=0; i<1024; i++)
	{
		/*
		m_pList2[i].Flags=CLFileSystem::MOUNT_FILE::MOUNT_FILE_EMPTY;
		m_pList2[i].pNext=LF_NULL;
		//There really isn't any reason to save the hash value,
		//but we are doing so anyway.
		m_pList2[i].nHashValue=i;
		*/
		m_pHashList[i]=LF_NULL;
	}
}

CLFileSystem::CPartitionTable::~CPartitionTable()
{
	Clear();
	//Delete the hash table (note that the Clear method has already
	//deleted any linked lists that may exist, so the array can
	//safely be deleted without memory leaks.
	LF_SAFE_DELETE_ARRAY(m_pHashList);
	LF_SAFE_DELETE_ARRAY(m_pMasterList);
}

void CLFileSystem::CPartitionTable::Clear()
{
	#if 1
	m_UnusedFiles.Init(m_pMasterList, m_nMaxFiles, sizeof(MOUNT_FILE_EX));
	for(lf_dword i=0; i<1024; i++)
	{
		m_pHashList[i]=LF_NULL;
	}
	#else
	for(lf_dword i=0; i<1024; i++)
	{
		if(m_pHashList[i])
		{
			//If there was more than one file with the same hash
			//proceed to move all files from the linked list
			//to the unused stack.
			for(MOUNT_FILE_EX* pItem=m_pHashList[i].pNext; pItem; )
			{
				MOUNT_FILE_EX* pNext=pItem->pNext;
				//LF_SAFE_DELETE(pItem);
				m_UnusedList.Push(pItem);
				pItem=pNext;
			}
			//Push the actuall item into the unused stack
			m_UnusedList.Push(m_pHashList[i]);
			m_pHashList[i]=LF_NULL;
		}
		/*
		//Set the file to empty.
		m_pList2[i].Flags=MOUNT_FILE::MOUNT_FILE_EMPTY;
		//If there was more than one file with the same hash
		//proceed to delete the linked list.
		for(MOUNT_FILE_EX* pItem=m_pList2[i].pNext; pItem; )
		{
			MOUNT_FILE_EX* pNext=pItem->pNext;
			LF_SAFE_DELETE(pItem);
			pItem=pNext;
		}
		//Set the next item to null.
		m_pList2[i].pNext=LF_NULL;
		*/
	}
	#endif
}

//FindFile finds a file in the partition table with the specified path.
//If not file exists it returns NULL, also note that
//this is CASE SENSITIVE so /base1/Tex.tga and /base1/tex.tga are not
//the same file.

const CLFileSystem::MOUNT_FILE* CLFileSystem::CPartitionTable::FindFile(lf_cwstr szFile)
{
	//To find a file just get the hash value for the specified filename
	//and check to see if a file is at that hash entry, not that
	//the linked list should be checked at that hash entry.
	lf_dword nHash=GetHashValue1024(szFile);
	//Loop through the linked list (usually 1-3 items long) and find
	//the filename.  
	for(MOUNT_FILE_EX* pItem=m_pHashList[nHash]; pItem; pItem=pItem->pNext)
	{
		//Note that if a file is empty we don't need to worry
		//about it (we actually NEED to continue, becuase if a file
		//was mounted in that spot and was then unmounted all the
		//data is still there, only the Flags value has been changed.)
		if(pItem->Flags==MOUNT_FILE::MOUNT_FILE_EMPTY)
			continue;
			
		//If the string matches then return the file info.
		if(pItem->Flags!=MOUNT_FILE::MOUNT_FILE_ARCHIVED 
			&& (wcscmp(pItem->szMountFileW, szFile)==0))
			return pItem;
	}
	return LF_NULL;
}

lf_bool CLFileSystem::CPartitionTable::MountFile(CLFileSystem::MOUNT_FILE* pFile, lf_dword Flags)
{
	//Get the hash value for the mounted filename
	//note that we always use the mounted filename for the
	//hash table and not the OS filename.
	lf_dword nHash=GetHashValue1024(pFile->szMountFileW);
	
	
	//If the file at that hash entry is empty and there is no
	//linked list attached we can simply add the file to that spot.
	if( (m_pHashList[nHash]==LF_NULL))
	{
		//Create a new files;
		MOUNT_FILE_EX* pNewFile=(MOUNT_FILE_EX*)m_UnusedFiles.Pop();
		if(!pNewFile)
			return LF_FALSE;
			
		pNewFile->pNext=LF_NULL;
		pNewFile->nHashValue=nHash;
		
		//Copy over the information for the file:
		memcpy(pNewFile, pFile, sizeof(MOUNT_FILE));
		////Copy just the MOUNT_FILE data, that way we don't overwrite the pNext value.
		//memcpy(m_pHashList[nHash], pFile, sizeof(MOUNT_FILE));
		m_pHashList[nHash]=pNewFile;
		return LF_TRUE;
	}
	else
	{
		//The same filename may already exist.  So loop through the table and find
		//out if it does, if it does either skip the file or overwrite it based on
		//the flag, if the file doesn't exist add it on the end.
		for(MOUNT_FILE_EX* pItem=m_pHashList[nHash]; pItem; pItem=pItem->pNext)
		{
			if(wcscmp(pItem->szMountFileW, pFile->szMountFileW)==0)
			{
				//The same file already exists, check flags and act appropriately.
				//If we need to overwrite an old file then we just copy the info
				if(LF_CHECK_FLAG(Flags, MOUNT_FILE_OVERWRITELPKONLY))
				{
					if(LF_CHECK_FLAG(pItem->Flags, MOUNT_FILE::MOUNT_FILE_ARCHIVED))
					{
						LF_ErrPrintW(L"Copying %s over old archive file.", ERR_LEVEL_DETAIL, pFile->szMountFileW);
						memcpy(pItem, pFile, sizeof(MOUNT_FILE));
						return LF_TRUE;
					}
					else
					{
						LF_ErrPrintW(L"Cannot mount %s.  A file with the same name has already been mounted.", ERR_LEVEL_DETAIL, pFile->szMountFileW);
						return LF_FALSE;
					}	
				}
				else if(LF_CHECK_FLAG(Flags, MOUNT_FILE_OVERWRITE))
				{
					LF_ErrPrintW(L"Copying %s over old file.", ERR_LEVEL_DETAIL, pFile->szMountFileW);
					memcpy(pItem, pFile, sizeof(MOUNT_FILE));
					return LF_TRUE;
				}
				else
				{
					LF_ErrPrintW(L"Cannot mount %s.  A file with the same name has already been mounted.", ERR_LEVEL_DETAIL, pFile->szMountFileW);
					return LF_FALSE;
				}
			}
			
			if(pItem->pNext==LF_NULL)
			{
				//We got to the end of the list and the file wasn't found,
				//so we proceed to add the new file onto the end of the list.
				MOUNT_FILE_EX* pNew=(MOUNT_FILE_EX*)m_UnusedFiles.Pop();//new MOUNT_FILE_EX;
				if(!pNew)
					return LF_FALSE;
					
				memcpy(pNew, pFile, sizeof(MOUNT_FILE));
				pNew->nHashValue=nHash;
				pNew->pNext=LF_NULL;
				pItem->pNext=pNew;
				return LF_TRUE;
			}
			
			//^^^...Loop to the next item in the list...^^^
		}
		
	}
	//Shoul never actually get here.
	return LF_FALSE;
}


lf_dword CLFileSystem::CPartitionTable::GetHashValue1024(lf_cwstr szString)
{
	//This is basically Bob Jenkins' One-At-A-Time-Hash.
	//http://www.burtleburtle.net/bob/hash/doobs.html
	lf_dword nLen=wcslen(szString);
	lf_dword nHash=0;
	lf_dword i=0;
	
	for(i=0; i<nLen; i++)
	{
		nHash+=szString[i];
		nHash+=(nHash<<10);
		nHash^=(nHash>>6);
	}
	nHash+=(nHash<<3);
	nHash^=(nHash>>11);
	nHash+=(nHash<<15);
	//We'll limit our hash value from 0 to 1023.
	return nHash&0x000003FF; //Same as nHash%1024 but should be faster.
}

lf_dword CLFileSystem::CPartitionTable::GetHashValue1024_Short(lf_cwstr szString)
{
	//This is basically Bob Jenkins' One-At-A-Time-Hash.
	//http://www.burtleburtle.net/bob/hash/doobs.html
	lf_pathW szKey;
	lf_dword nLen=0;//wcslen(szKey);
	lf_dword nHash=0;
	lf_dword i=0;
	
	LF_GetFileNameFromPathW(szKey, szString);
	nLen=wcslen(szKey);
	
	for(i=0; i<nLen; i++)
	{
		nHash+=szKey[i];
		nHash+=(nHash<<10);
		nHash^=(nHash>>6);
	}
	nHash+=(nHash<<3);
	nHash^=(nHash>>11);
	nHash+=(nHash<<15);
	//We'll limit our hash value from 0 to 1023.
	return nHash&0x000003FF; //Same as nHash%1024 but should be faster.
}

void CLFileSystem::CPartitionTable::PrintMountInfo()
{
	//Loop through all hash indexes and if the file exists
	//then print out the information about it.
	LF_ErrPrint("FLAGS  SIZE       PACKED    PATH", ERR_LEVEL_ALWAYS);
	LF_ErrPrint("-----  ----       ------    ----", ERR_LEVEL_ALWAYS);
	lf_dword nFiles=0;
	for(lf_dword i=0; i<1024; i++)
	{
		for(MOUNT_FILE_EX* pItem=m_pHashList[i]; pItem; pItem=pItem->pNext)
		{
			nFiles++;
			PrintFileInfo(pItem);
		}
	}
	LF_ErrPrint("Totals: %d files", ERR_LEVEL_ALWAYS, nFiles);
}

void CLFileSystem::CPartitionTable::PrintFileInfo(MOUNT_FILE_EX* pFile)
{
	//If there was not file in that table position we'll simply return.
	if(pFile->Flags==MOUNT_FILE::MOUNT_FILE_EMPTY)
		return;
		
	lf_wchar_t szOutput[29+LF_MAX_PATH];
	szOutput[5]=' ';
	
	//First thing print all the file flags.
	if(LF_CHECK_FLAG(pFile->Flags, MOUNT_FILE::MOUNT_FILE_DIRECTORY))
		szOutput[0]=('d');
	else
		szOutput[0]=('-');
	if(LF_CHECK_FLAG(pFile->Flags, MOUNT_FILE::MOUNT_FILE_ARCHIVED))
		szOutput[1]=('a');
	else
		szOutput[1]=('-');
				
	if(LF_CHECK_FLAG(pFile->Flags, MOUNT_FILE::MOUNT_FILE_READONLY))
	{
		szOutput[2]=('r');
		szOutput[3]=('-');
	}
	else
	{
		szOutput[2]=('r');
		szOutput[3]=('w');
	}
			
	if(LF_CHECK_FLAG(pFile->Flags, MOUNT_FILE::MOUNT_FILE_ZLIBCMP))
		szOutput[4]=('z');
	else
		szOutput[4]=('-');
	
	_snwprintf(&szOutput[6], 10, L"%10u", pFile->nSize);
	szOutput[16]=' ';
	_snwprintf(&szOutput[17], 10, L"%10u", pFile->nCmpSize);
	szOutput[27]=' ';
	_snwprintf(&szOutput[28], LF_MAX_PATH, L"%s", pFile->szMountFileW);
	szOutput[28+LF_MAX_PATH]=0;
	LF_ErrPrintW(szOutput, ERR_LEVEL_ALWAYS);
	
	_snwprintf(szOutput, LF_MAX_PATH+27, L"%4u: %10u           (%s)", pFile->nHashValue, pFile->nOffset, pFile->szOSFileW);
	LF_ErrPrintW(szOutput, ERR_LEVEL_DETAIL);
	#if 0
		
	LF_ErrPrintW(L"%s", ERR_LEVEL_ALWAYS, pFile->szMountFileW);
	LF_ErrPrintW(L"(%s).%d", ERR_LEVEL_ALWAYS, pFile->szOSFileW, pFile->nOffset);
	LF_ErrPrintW(L"Size: %d Compressed Size: %d Hash Value: %d", ERR_LEVEL_ALWAYS,  pFile->nSize, pFile->nCmpSize, pFile->nHashValue);
	LF_ErrPrintW(L"Flags: ", ERR_LEVEL_ALWAYS);
	
	lf_wchar_t szFlags[6];
	szFlags[5]=0;
	if(LF_CHECK_FLAG(pFile->Flags, MOUNT_FILE::MOUNT_FILE_DIRECTORY))
		szFlags[0]=('D');
	else
		szFlags[0]=('-');
	if(LF_CHECK_FLAG(pFile->Flags, MOUNT_FILE::MOUNT_FILE_ARCHIVED))
		szFlags[1]=('A');
	else
		szFlags[1]=('-');
				
	if(LF_CHECK_FLAG(pFile->Flags, MOUNT_FILE::MOUNT_FILE_READONLY))
	{
		szFlags[2]=('R');
		szFlags[3]=('-');
	}
	else
	{
		szFlags[2]=('R');
		szFlags[3]=('W');
	}
			
	if(LF_CHECK_FLAG(pFile->Flags, MOUNT_FILE::MOUNT_FILE_ZLIBCMP))
		szFlags[4]=('Z');
	else
		szFlags[4]=('-');
			
	LF_ErrPrintW(szFlags, ERR_LEVEL_ALWAYS);
	#endif
}


