#ifndef __LG_MEM_FILE_H__
#define __LG_MEM_FILE_H__

#include "lg_types.h"

class CLMemFile
{
private:
	lg_byte* m_pMem;
	lg_dword m_nSize;
	lg_dword m_nPos;
public:
	enum MEM_SEEK_TYPE
	{
		MEM_SEEK_CUR=0,
		MEM_SEEK_BEGIN,
		MEM_SEEK_END
	};
public:
	CLMemFile(lg_dword nSize);
	~CLMemFile();
	
	lg_dword Read(lg_void* pOut, const lg_dword nSize);
	lg_dword Write(const lg_void* const pIn, const lg_dword nSize);
		
	lg_dword Tell()const;
	lg_dword Size()const;
	void Open(lg_dword nSize);
	void Resize(lg_dword nSize);
	void Close();
	void Seek(MEM_SEEK_TYPE type, lg_long distance);
};

#endif  __LG_MEM_FILE_H__