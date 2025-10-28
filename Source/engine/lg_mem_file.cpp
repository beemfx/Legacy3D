#include "lg_mem_file.h"
#include "lg_func.h"
#include "lg_err.h"
#include "lg_err_ex.h"
#include <memory.h>

CLMemFile::CLMemFile(lg_dword nSize)
: m_pMem(LG_NULL)
, m_nSize(0)
, m_nPos(0)
{
	if(nSize)
		Open(nSize);
}
CLMemFile::~CLMemFile()
{
	Close();
}

lg_dword CLMemFile::Tell()const
{
	return m_nPos;
}

lg_dword CLMemFile::Size()const
{
	return m_nSize;
}
	
lg_dword CLMemFile::Read(lg_void* pOut, const lg_dword nSize)
{
	lg_dword nReadSize=LG_Min(nSize, m_nSize-m_nPos);
	memcpy(pOut, &m_pMem[m_nPos], nReadSize);
	m_nPos+=nReadSize;
	return nReadSize;
}

lg_dword CLMemFile::Write(const lg_void* const pIn, const lg_dword nSize)
{
	lg_dword nWriteSize=LG_Min(nSize, m_nSize-m_nPos);
	memcpy(&m_pMem[m_nPos], pIn, nWriteSize);
	m_nPos+=nWriteSize;
	return nWriteSize;
}
		
void CLMemFile::Open(lg_dword nSize)
{
	Close();
	m_pMem=new lg_byte[nSize];
	LG_ASSERT(m_pMem, LG_NULL);
	m_nPos=0;
	m_nSize=nSize;
}
void CLMemFile::Resize(lg_dword nSize)
{
	lg_byte* pNew=new lg_byte[nSize];
	LG_ASSERT(pNew, LG_NULL);
		
	memcpy(pNew, m_pMem, LG_Min(m_nSize, nSize));
	LG_SafeDeleteArray(m_pMem);
	m_pMem=pNew;
	m_nSize=nSize;
	m_nPos=LG_Clamp(m_nPos, 0, m_nSize);
}
void CLMemFile::Close()
{
	LG_SafeDeleteArray(m_pMem);
	m_nSize=0;
	m_nPos=0;
}

void CLMemFile::Seek(MEM_SEEK_TYPE type, lg_long distance)
{
	lg_dword nNewPos=0;
	if(type==MEM_SEEK_CUR)
	{
		nNewPos+=distance;
	}
	else if(type==MEM_SEEK_BEGIN)
	{
		nNewPos=distance;
	}
	else if(type==MEM_SEEK_END)
	{
		nNewPos=m_nSize+distance;
	}
	
	m_nPos=LG_Clamp(nNewPos, 0, m_nSize);
}