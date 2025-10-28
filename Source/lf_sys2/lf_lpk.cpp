#include <string.h>
#include <stdlib.h>
#include "lf_lpk.h"
#include "stdio.h"

//#include <windows.h>
CLArchive::CLArchive():
	m_bOpen(LF_FALSE),
	m_pFileList(LF_NULL),
	m_nType(0),
	m_nVersion(0),
	m_nCount(0),
	m_nInfoOffset(0),
	m_File(LF_NULL),
	m_TempFile(LF_NULL),
	m_nMainFileWritePos(0),
	m_bHasChanged(LF_FALSE),
	m_nCmpThreshold(20)
{

}

CLArchive::~CLArchive()
{
	Close();
}

lf_dword CLArchive::GetNumFiles()
{
	return m_nCount;
}

lf_bool CLArchive::IsOpen()
{
	return m_bOpen;
}

const LPK_FILE_INFO* CLArchive::GetFileInfo(lf_dword n)
{
	if(n>=m_nCount)
		return LF_NULL;
		
	return &m_pFileList[n];
}

lf_bool CLArchive::GetFileInfo(lf_dword nRef, LPK_FILE_INFO* pInfo)
{
	if(nRef>=m_nCount)
		return LF_FALSE;
		
	*pInfo=m_pFileList[nRef];
	return LF_TRUE;
}

lf_bool CLArchive::CreateNewW(lf_cwstr szFilename)
{
	Close();
	
	m_File=BDIO_OpenW(szFilename, BDIO_CREATE_ALWAYS, BDIO_ACCESS_READ|BDIO_ACCESS_WRITE);
	if(!m_File)
		return LF_FALSE;
		
	m_bWriteable=LF_TRUE;
	m_TempFile=BDIO_OpenTempFileW(BDIO_CREATE_ALWAYS, BDIO_ACCESS_READ|BDIO_ACCESS_WRITE|BDIO_ACCESS_TEMP);
	if(!m_TempFile)
	{
		BDIO_Close(m_File);
		m_File=NULL;
		m_TempFile=NULL;
		return LF_FALSE;
	}
	
	m_nType=LPK_TYPE;
	m_nVersion=LPK_VERSION;
	m_nCount=0;
	m_nInfoOffset=0;
	m_pFileList=LF_NULL;
	m_bOpen=LF_TRUE;
	m_bHasChanged=LF_TRUE;
	return m_bOpen;
}

lf_bool CLArchive::CreateNewA(lf_cstr szFilename)
{
	Close();
	
	m_File=BDIO_OpenA(szFilename, BDIO_CREATE_ALWAYS, BDIO_ACCESS_READ|BDIO_ACCESS_WRITE);
	if(!m_File)
		return LF_FALSE;
		
	m_bWriteable=LF_TRUE;
	m_TempFile=BDIO_OpenTempFileA(BDIO_CREATE_ALWAYS, BDIO_ACCESS_READ|BDIO_ACCESS_WRITE|BDIO_ACCESS_TEMP);
	if(!m_TempFile)
	{
		BDIO_Close(m_File);
		m_File=NULL;
		m_TempFile=NULL;
		return LF_FALSE;
	}
	
	m_nType=LPK_TYPE;
	m_nVersion=LPK_VERSION;
	m_nCount=0;
	m_nInfoOffset=0;
	m_pFileList=LF_NULL;
	m_bOpen=LF_TRUE;
	m_bHasChanged=LF_TRUE;
	return m_bOpen;
}

lf_bool CLArchive::OpenW(lf_cwstr szFilename, lf_dword Flags)
{
	Close();
	
	lf_dword dwAccess=BDIO_ACCESS_READ;
	
	if(LF_CHECK_FLAG(Flags, LPK_OPEN_ENABLEWRITE))
	{
		m_bWriteable=LF_TRUE;
		dwAccess|=BDIO_ACCESS_WRITE;
	}
	
	m_File=BDIO_OpenW(szFilename, BDIO_OPEN_ALWAYS, dwAccess);
	if(!m_File)
	{
		Close();
		return LF_FALSE;
	}
	
	//If the archive is going to be writable then we open a temp file.
	//if a temp file can't be openened then the archive is set to read
	//only.
	if(m_bWriteable)
	{
		m_TempFile=BDIO_OpenTempFileW(BDIO_CREATE_ALWAYS, BDIO_ACCESS_READ|BDIO_ACCESS_WRITE|BDIO_ACCESS_TEMP);
		if(!m_TempFile)
			m_bWriteable=LF_FALSE;
	}	
	else
		m_TempFile=LF_NULL;

		
	if(!ReadArcInfo())
	{
		if(!OpenOld())
		{
			Close();
			return LF_FALSE;
		}
	}	
	m_bHasChanged=LF_FALSE;
	m_bOpen=LF_TRUE;
	return LF_TRUE;
}

lf_bool CLArchive::OpenA(lf_cstr szFilename, lf_dword Flags)
{
	Close();
	
	lf_dword dwAccess=BDIO_ACCESS_READ;
	
	if(LF_CHECK_FLAG(Flags, LPK_OPEN_ENABLEWRITE))
	{
		m_bWriteable=LF_TRUE;
		dwAccess|=BDIO_ACCESS_WRITE;
	}
	
	m_File=BDIO_OpenA(szFilename, BDIO_OPEN_ALWAYS, dwAccess);
	if(!m_File)
	{
		Close();
		return LF_FALSE;
	}
	
	//If the archive is going to be writable then we open a temp file.
	//if a temp file can't be openened then the archive is set to read
	//only.
	if(m_bWriteable)
	{
		m_TempFile=BDIO_OpenTempFileA(BDIO_CREATE_ALWAYS, BDIO_ACCESS_READ|BDIO_ACCESS_WRITE|BDIO_ACCESS_TEMP);
		if(!m_TempFile)
			m_bWriteable=LF_FALSE;
	}	
	else
		m_TempFile=LF_NULL;

		
	if(!ReadArcInfo())
	{
		if(!OpenOld())
		{
			Close();
			return LF_FALSE;
		}
	}	
	m_bHasChanged=LF_FALSE;
	m_bOpen=LF_TRUE;
	return LF_TRUE;
}

lf_bool CLArchive::OpenOld()
{
	BDIO_Seek(m_File, 0, BDIO_SEEK_BEGIN);
	BDIO_Read(m_File, 4, &m_nType);
	BDIO_Read(m_File, 4, &m_nVersion);
	BDIO_Read(m_File, 4, &m_nCount);
	m_bWriteable=LF_FALSE;
	
	if(m_nType!=LPK_TYPE || m_nVersion!=101)
	{
		m_bOpen=LF_FALSE;
		return LF_FALSE;
	}
	m_pFileList=new LPK_FILE_INFO_EX[m_nCount];
	for(lf_dword i=0; i<m_nCount; i++)
	{
		lf_char szFilename[260];
		lf_dword nType=0;
		lf_dword nInfoOffset=0;
		BDIO_Read(m_File, 260, szFilename);
		BDIO_Read(m_File, 4, &nType);
		BDIO_Read(m_File, 4, &m_pFileList[i].nOffset);
		BDIO_Read(m_File, 4, &nInfoOffset);
		BDIO_Read(m_File, 4, &m_pFileList[i].nCmpSize);
		BDIO_Read(m_File, 4, &m_pFileList[i].nSize);
		
		if(nType==1)
			m_pFileList[i].nType=LPK_FILE_TYPE_ZLIBCMP;
		else
			m_pFileList[i].nType=LPK_FILE_TYPE_NORMAL;
			
		m_pFileList[i].nInternalPosition=LPK_FILE_POS_MAIN;
		
		mbstowcs(m_pFileList[i].szFilename, &szFilename[2], LF_MAX_PATH);
		for(lf_dword j=0; j<wcslen(m_pFileList[i].szFilename); j++)
			if(m_pFileList[i].szFilename[j]=='\\')m_pFileList[i].szFilename[j]='/';
		
	}
	
	m_bOpen=LF_TRUE;	
	return LF_TRUE;
}

lf_dword CLArchive::DoAddFile(BDIO_FILE fin, lf_cwstr szNameInArc, lf_dword Flags)
{	
	LPK_FILE_INFO_EX Entry;
	Entry.nSize=BDIO_GetSize(fin);
	Entry.nCmpSize=Entry.nSize;
	//Entry.nOffset=BDIO_Tell(m_File);
	Entry.nType=LF_CHECK_FLAG(Flags, LPK_ADD_ZLIBCMP)?LPK_FILE_TYPE_ZLIBCMP:LPK_FILE_TYPE_NORMAL;
	Entry.nInternalPosition=LPK_FILE_POS_MAIN;
	
	//Just in case a file with the same name already exists, go ahead
	//and add something onto the end of it (we'll add an X).
	wcsncpy(Entry.szFilename, szNameInArc, LF_MAX_PATH);
	while(GetFileRef(Entry.szFilename)!=0xFFFFFFFF)
	{
		wcscat(Entry.szFilename, L"X");
	}
	
	lf_byte* pTemp=new lf_byte[Entry.nSize];
	if(!pTemp)
	{
		return 0;
	}
	if(BDIO_Read(fin, Entry.nSize, pTemp)!=Entry.nSize)
	{
		delete[]pTemp;
		return 0;
	}
	
	//Write the file to the end of the main file.
	BDIO_Seek(m_File, m_nMainFileWritePos, BDIO_SEEK_BEGIN);
	Entry.nOffset=BDIO_Tell(m_File);
	if(Entry.nType==LPK_FILE_TYPE_ZLIBCMP)
	{
		Entry.nCmpSize=BDIO_WriteCompressed(m_File, Entry.nSize, pTemp);
		lf_dword nCmp=(lf_dword)(100.0f-(float)Entry.nCmpSize/(float)Entry.nSize*100.0f);
		//If the compression did not occur, or if the compression
		//percentage was too low, we'll just write the file data.
		//By default the compression threshold is 20.
		if( (Entry.nCmpSize==0) || (nCmp<m_nCmpThreshold) )
		{
			BDIO_Seek(m_File, Entry.nOffset, BDIO_SEEK_BEGIN);
			Entry.nCmpSize=BDIO_Write(m_File, Entry.nSize, pTemp);
			Entry.nType=LPK_FILE_TYPE_NORMAL;
		}
	}
	else
	{
		Entry.nCmpSize=BDIO_Write(m_File, Entry.nSize, pTemp);
	}
	m_nMainFileWritePos=BDIO_Tell(m_File);
	
	delete[]pTemp;

	LPK_FILE_INFO_EX* pNew=new LPK_FILE_INFO_EX[m_nCount+1];
	for(lf_dword i=0; i<m_nCount; i++)
	{
		pNew[i]=m_pFileList[i];
	}
	pNew[m_nCount]=Entry;
	m_nCount++;
	if(m_pFileList){delete[]m_pFileList;}
	m_pFileList=pNew;
	
	m_bHasChanged=LF_TRUE;
	return Entry.nCmpSize;
	
}

lf_dword CLArchive::AddFileW(lf_cwstr szFilename, lf_cwstr szNameInArc, lf_dword Flags)
{
	if(!m_bOpen || !m_bWriteable)
		return 0;
		
	BDIO_FILE fin=BDIO_OpenW(szFilename, BDIO_OPEN_EXISTING, BDIO_ACCESS_READ);
	if(!fin)
		return 0;
	
	lf_dword nSize=DoAddFile(fin, szNameInArc, Flags);
	BDIO_Close(fin);
	return nSize;	
}

lf_dword CLArchive::AddFileA(lf_cstr szFilename, lf_cstr szNameInArc, lf_dword Flags)
{
	if(!m_bOpen || !m_bWriteable)
		return 0;
		
	BDIO_FILE fin=BDIO_OpenA(szFilename, BDIO_OPEN_EXISTING, BDIO_ACCESS_READ);
	if(!fin)
		return 0;
		
	lf_pathW szWideName;
	mbstowcs(szWideName, szNameInArc, LF_MAX_PATH);
	
	lf_dword nSize=DoAddFile(fin, szWideName, Flags);
	BDIO_Close(fin);
	return nSize;	
}

lf_bool CLArchive::ReadArcInfo()
{
	//Read the header info.  It should be the last 16 bytes of
	//the file.
	BDIO_Seek(m_File, -16, BDIO_SEEK_END);
	BDIO_Read(m_File, 4, &m_nType);
	BDIO_Read(m_File, 4, &m_nVersion);
	BDIO_Read(m_File, 4, &m_nCount);
	BDIO_Read(m_File, 4, &m_nInfoOffset);
	
	if((m_nType!=LPK_TYPE) || (m_nVersion!=LPK_VERSION))
		return LF_FALSE;
	
	//Seek to the beginning of the file data.
	BDIO_Seek(m_File, m_nInfoOffset, BDIO_SEEK_BEGIN);
	if(BDIO_Tell(m_File)!=m_nInfoOffset)
		return LF_FALSE;
		
	//Save the info offset as the main file write position,
	//as this will be where new files are written to.
	m_nMainFileWritePos=m_nInfoOffset;
	
	//Allocate memory for file data.
	m_pFileList=new LPK_FILE_INFO_EX[m_nCount];
	if(!m_pFileList)
		return LF_FALSE;
		
	//Now read the data.
	for(lf_dword i=0; i<m_nCount; i++)
	{
		BDIO_Read(m_File, 256, &m_pFileList[i].szFilename);
		BDIO_Read(m_File, 4, &m_pFileList[i].nType);
		BDIO_Read(m_File, 4, &m_pFileList[i].nOffset);
		BDIO_Read(m_File, 4, &m_pFileList[i].nSize);
		BDIO_Read(m_File, 4, &m_pFileList[i].nCmpSize);
		m_pFileList[i].nInternalPosition=LPK_FILE_POS_MAIN;
	}	
	return LF_TRUE;
}


lf_bool CLArchive::Save()
{
	if(!m_bWriteable || !m_bOpen || !m_bHasChanged)
		return LF_FALSE;
		
	BDIO_Seek(m_File, m_nMainFileWritePos, BDIO_SEEK_BEGIN);
	BDIO_Seek(m_TempFile, 0, BDIO_SEEK_BEGIN);
	
	for(lf_dword i=0; i<m_nCount; i++)
	{
		if(m_pFileList[i].nInternalPosition==LPK_FILE_POS_TEMP)
		{
			BDIO_Seek(m_TempFile, m_pFileList[i].nOffset, BDIO_SEEK_BEGIN);
			m_pFileList[i].nOffset=BDIO_Tell(m_File);
			BDIO_CopyData(m_File, m_TempFile, m_pFileList[i].nCmpSize);
			
			m_pFileList[i].nInternalPosition=LPK_FILE_POS_MAIN;
		}
		else
		{
			//We don't need to do anything.
		}
	}
	m_nInfoOffset=BDIO_Tell(m_File);
	//Write the file info data.
	for(lf_dword i=0; i<m_nCount; i++)
	{
		BDIO_Write(m_File, 256, &m_pFileList[i].szFilename);
		BDIO_Write(m_File, 4, &m_pFileList[i].nType);
		BDIO_Write(m_File, 4, &m_pFileList[i].nOffset);
		BDIO_Write(m_File, 4, &m_pFileList[i].nSize);
		BDIO_Write(m_File, 4, &m_pFileList[i].nCmpSize);
	}
	
	BDIO_Seek(m_File, 0, BDIO_SEEK_END);
	//Write the header...
	BDIO_Write(m_File, 4, &m_nType);
	BDIO_Write(m_File, 4, &m_nVersion);
	BDIO_Write(m_File, 4, &m_nCount);
	BDIO_Write(m_File, 4, &m_nInfoOffset);
	
	BDIO_Seek(m_TempFile, 0, BDIO_SEEK_BEGIN);
	m_bHasChanged=LF_FALSE;
	return LF_TRUE;
}

void CLArchive::Close()
{		
	//If the file was writeable then everything will be written from the temp file.
	if(m_bWriteable && m_bOpen)
	{
		Save();
	}
	
	if(m_File)
		BDIO_Close(m_File);
		
	if(m_TempFile)
		BDIO_Close(m_TempFile);
		
	if(m_pFileList)
		delete[]m_pFileList;
		
	m_File=m_TempFile=m_pFileList=LF_NULL;
		
	m_bOpen=LF_FALSE;
	m_nType=0;
	m_nVersion=0;
	m_nCount=0;
	m_nInfoOffset=0;
	m_bHasChanged=LF_FALSE;
}

lf_dword CLArchive::GetFileRef(const lf_pathW szName)
{
	for(lf_dword i=0; i<this->m_nCount; i++)
	{
		if(wcscmp(szName, m_pFileList[i].szFilename)==0)
			return i;
	}
	return 0xFFFFFFFF;
}
lf_byte* CLArchive::ExtractFile(lf_byte* pOut, lf_dword nRef)
{	
	if(nRef>=m_nCount)
		return LF_NULL;
		
	BDIO_FILE fin=LF_NULL;
	if(m_pFileList[nRef].nInternalPosition==LPK_FILE_POS_TEMP)
		fin=m_TempFile;
	else
		fin=m_File;
		
	BDIO_Seek(fin, m_pFileList[nRef].nOffset, BDIO_SEEK_BEGIN);
	
	lf_dword nRead=0;
	if(m_pFileList[nRef].nType==LPK_FILE_TYPE_ZLIBCMP)
	{
		nRead=BDIO_ReadCompressed(fin, m_pFileList[nRef].nSize, pOut);
	}
	else
	{
		nRead=BDIO_Read(fin, m_pFileList[nRef].nSize, pOut);
	}
	
	if(nRead!=m_pFileList[nRef].nSize)
	{
		//An error occured while reading.
	}
	return pOut;
}
