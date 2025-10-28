#include <memory.h>
#include "lf_file.h"

CLFile::CLFile()
{

}

CLFile::~CLFile()
{

}

lf_dword CLFile::Read(lf_void* pOutBuffer, lf_dword nSize)
{
	if(!LF_CHECK_FLAG(m_nAccess, LF_ACCESS_READ))
	{
		LF_ErrPrintW(
			L"File Read Error: \"%s\" doesn't have read permission.", 
			ERR_LEVEL_ERROR, 
			m_szPathW);
		
		return 0;
	}
	//We don't need to read anything if the size to read
	//was zero or if we are at the end of the file.
	if(!nSize || m_bEOF)
		return 0;
	
	//If somehow the file pointer has gone past the end of the file,
	//which it shouldn't, then we can't read anything.	
	if(m_nFilePointer>m_nSize)
	{
		LF_ErrPrintW(
			L"File Read Error: Imposible situation the file pointer has gone pas the end of the file.",
			ERR_LEVEL_ERROR);
			
		return 0;
	}
		
	//If the bytes we want to read goes past the end of
	//the file we need to adjust the reading size.
	if(m_nSize<(m_nFilePointer+nSize))
	{
		nSize=m_nSize-m_nFilePointer;
	}
	
	//There are two types of read, either a disk read or
	//a memory read, the memory read is performed if the
	//access mode is LF_ACCESS_MEMORY.
	if(LF_CHECK_FLAG(m_nAccess, LF_ACCESS_MEMORY))
	{
		//Doing a memory read...
		LF_ErrPrint("Performing memory read...", ERR_LEVEL_SUPERDETAIL);
		memcpy(pOutBuffer, (lf_pvoid)((lf_uint)m_pData+m_nFilePointer), nSize);
		m_nFilePointer+=nSize;
	}
	else
	{
		//Doing a disk read...
		LF_ErrPrint("Performing disk read...", ERR_LEVEL_SUPERDETAIL);
		BDIO_Seek(m_BaseFile, m_nBaseFileBegin+m_nFilePointer, BDIO_SEEK_BEGIN);
		nSize=BDIO_Read(m_BaseFile, nSize, pOutBuffer);
		m_nFilePointer+=nSize;
	}
	
	//If we've reached the end of the file, set the EOF flag.
	m_bEOF=m_nFilePointer>=m_nSize;
	//LF_ErrPrintW(L"Read %d bytes.", ERR_LEVEL_ERROR, nSize);
	return nSize;
}

lf_dword CLFile::Write(lf_void* pInBuffer, lf_dword nSize)
{	
	if(!LF_CHECK_FLAG(m_nAccess, LF_ACCESS_WRITE) 
		|| LF_CHECK_FLAG(m_nAccess, LF_ACCESS_MEMORY))
	{
		LF_ErrPrintW(
			L"File Write Error: \"%s\" doesn't have write permission.", 
			ERR_LEVEL_ERROR, 
			m_szPathW);
		
		return 0;
	}
	//Seek to the file pointer of the file.
	BDIO_Seek(m_BaseFile, m_nBaseFileBegin+m_nFilePointer, BDIO_SEEK_BEGIN);
	nSize=BDIO_Write(m_BaseFile, nSize, pInBuffer);
	m_nFilePointer+=nSize;
	//Update the size of the file.
	#if 1
	if(m_nFilePointer>m_nSize)
	{
		m_nSize=m_nFilePointer;
	}
	#else !1
	m_nSize=BDIO_GetSize(m_BaseFile);
	#endif
	//Set the flag if we're at the end of the file.
	m_bEOF=m_nFilePointer>=m_nSize;
	return nSize;
}
lf_dword CLFile::Tell()
{
	return m_nFilePointer;
}
lf_dword CLFile::Seek(LF_SEEK_TYPE nMode, lf_long nOffset)
{
	switch(nMode)
	{
	case LF_SEEK_BEGIN:
		m_nFilePointer=nOffset;
		break;
	case LF_SEEK_CURRENT:
		m_nFilePointer+=nOffset;
		break;
	case LF_SEEK_END:
		m_nFilePointer=m_nSize+nOffset;
		break;
	}
	
	//Clamp the new file pointer if seeking in the positive direction
	//we'll limit it to the size of the file,
	//if seeking negative, 0.
	if(m_nFilePointer>m_nSize)
	{
		m_nFilePointer=nOffset>=0?m_nSize:0;
	}
	
	LF_ErrPrint("Seeked to %d out of %d", ERR_LEVEL_SUPERDETAIL, m_nFilePointer, m_nSize);
	//Set the EOF flag if necessary.
	m_bEOF=m_nFilePointer>=m_nSize;
	return m_nFilePointer;
}
lf_dword CLFile::GetSize()
{
	return m_nSize;
}
const lf_pvoid CLFile::GetMemPointer()
{
	if(LF_CHECK_FLAG(m_nAccess, LF_ACCESS_MEMORY))
		return m_pData;
	else
		return LF_NULL;
}

lf_bool CLFile::IsEOF()
{
	return m_bEOF;
}