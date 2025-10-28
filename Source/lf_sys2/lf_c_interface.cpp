//lf_c_interface.cpp - Code for the C interface for the file system.
#include "lf_fs2.h"

//#define FS_ON_STACK
#ifdef FS_ON_STACK
CLFileSystem g_FS;
CLFileSystem* g_pFS=&g_FS;
#else !FS_ON_STACK
CLFileSystem* g_pFS=LF_NULL;
#endif FS_ON_STACK

//A testing function, this will not be the final directory listing function
extern "C" void FS_PrintMountInfo()
{
	g_pFS->PrintMountInfo();
}


//File system function (c-interface).

extern "C" lf_bool FS_Initialize(lf_dword nMaxFiles, LF_ALLOC_FUNC pAlloc, LF_FREE_FUNC pFree)
{
	if(pAlloc && pFree)
		LF_SetMemFuncs(pAlloc, pFree);
		
	#ifndef FS_ON_STACK
	g_pFS=new CLFileSystem(nMaxFiles);
	if(!g_pFS)
	{
		LF_ErrPrintW(L"FS_Initalize Error: Not enough memory.", ERR_LEVEL_ERROR);
		return LF_FALSE;
	}
	#endif
	return LF_TRUE;
}

extern "C" lf_bool FS_Shutdown()
{
	lf_bool bResult=g_pFS->UnMountAll();
	#ifndef FS_ON_STACK
	delete g_pFS;
	#endif FS_ON_STACK
	return bResult;
}

extern "C" lf_dword FS_MountBaseA(lf_cstr szOSPath)
{
	return g_pFS->MountBaseA(szOSPath);
}
extern "C" lf_dword FS_MountBaseW(lf_cwstr szOSPath)
{
	return g_pFS->MountBaseW(szOSPath);
}

extern "C" lf_dword FS_MountA(lf_cstr szOSPath, lf_cstr szMountPath, lf_dword Flags)
{
	return g_pFS->MountA(szOSPath, szMountPath, Flags);
}

extern "C" lf_dword FS_MountW(lf_cwstr szOSPath, lf_cwstr szMountPath, lf_dword Flags)
{
	return g_pFS->MountW(szOSPath, szMountPath, Flags);
}

	
extern "C" lf_dword FS_MountLPKA(lf_cstr szMountedFile, lf_dword Flags)
{
	return g_pFS->MountLPKA(szMountedFile, Flags);
}

extern "C" lf_dword FS_MountLPKW(lf_cwstr szMountedFile, lf_dword Flags)
{
	return g_pFS->MountLPKW(szMountedFile, Flags);
}

extern "C" lf_bool FS_UnMountAll()
{
	return g_pFS->UnMountAll();
}

//File access functions c-interface.
extern "C" LF_FILE3 LF_OpenA(lf_cstr szFilename, lf_dword Access, lf_dword CreateMode)
{
	return g_pFS->OpenFileA(szFilename, Access, CreateMode);
}

extern "C" LF_FILE3 LF_OpenW(lf_cwstr szFilename, lf_dword Access, lf_dword CreateMode)
{
	return g_pFS->OpenFileW(szFilename, Access, CreateMode);
}

extern "C" lf_bool LF_Close(LF_FILE3 File)
{
	return g_pFS->CloseFile((CLFile*)File);
}

extern "C" lf_dword LF_Read(LF_FILE3 File, lf_void* pOutBuffer, lf_dword nSize)
{
	CLFile* pFile=(CLFile*)File;
	return pFile->Read(pOutBuffer, nSize);
}
extern "C" lf_dword LF_Write(LF_FILE3 File, lf_void* pInBuffer, lf_dword nSize)
{
	CLFile* pFile=(CLFile*)File;
	return pFile->Write(pInBuffer, nSize);
}
extern "C" lf_dword LF_Tell(LF_FILE3 File)
{
	CLFile* pFile=(CLFile*)File;
	return pFile->Tell();
}
extern "C" lf_dword LF_Seek(LF_FILE3 File, LF_SEEK_TYPE nMode, lf_long nOffset)
{
	CLFile* pFile=(CLFile*)File;
	return pFile->Seek(nMode, nOffset);
}
extern "C" lf_dword LF_GetSize(LF_FILE3 File)
{
	CLFile* pFile=(CLFile*)File;
	return pFile->GetSize();
}
extern "C" lf_pcvoid LF_GetMemPointer(LF_FILE3 File)
{
	CLFile* pFile=(CLFile*)File;
	return pFile->GetMemPointer();
}
extern "C" lf_bool LF_IsEOF(LF_FILE3 File)
{
	CLFile* pFile=(CLFile*)File;
	return pFile->IsEOF();
}