#include "common.h"
#include "ws_sys.h"

#include <stdio.h>
#include <tchar.h>

CWSFile::CWSFile():
	m_nMapVersion(0),
	m_nMapFlags(0),
	m_nNameCount(0),
	m_nNameOffset(0),
	m_nObjectCount(0),
	m_nObjectOffset(0),
	m_pNameTable(0),
	m_bOpen(WS_FALSE)
{

}

CWSFile::~CWSFile()
{
	Close();
}

wsBool CWSFile::TempOpen(wspString szFilename)
{
	Close();
	
	fio_callbacks cb;
	cb.close=L_fclose;
	cb.read=L_fread;
	cb.seek=L_fseek;
	cb.tell=L_ftell;
	cb.write=L_fwrite;
	
	void* fin=L_fopen(szFilename, LOPEN_OPEN_EXISTING, LOPEN_READ);
	if(!fin)
		return m_bOpen;
		
	//Read the header...
	cb.read(&m_nMapVersion, sizeof(wsWord), 1, fin);
	if(m_nMapVersion!=WS_MAP_VERSION)
	{
		cb.close(fin);
		return m_bOpen;
	}
	cb.read(&m_nMapFlags, sizeof(wsByte), 1, fin);
	cb.read(&m_nNameCount, sizeof(wsLong), 1, fin);
	cb.read(&m_nNameOffset, sizeof(wsLong), 1, fin);
	cb.read(&m_nObjectCount, sizeof(wsLong), 1, fin);
	cb.read(&m_nObjectOffset, sizeof(wsLong), 1, fin);
	
	#if 1
	_tprintf(_T("Map Version: %d\n"), m_nMapVersion);
	_tprintf(_T("Name Count: %d\n"), m_nNameCount);
	_tprintf(_T("Name Offset: %d\n"), m_nNameOffset);
	_tprintf(_T("Object Count: %d\n"), m_nObjectCount);
	_tprintf(_T("Object Offset: %d\n"), m_nObjectOffset);
	#endif
	
	//Read the string table:
	//Step 1: Allocate memory for the string table...
	m_pNameTable=new wsString[m_nNameCount];
	if(!m_pNameTable)
	{
		cb.close(fin);
		return m_bOpen;
	}
	//Step 2: Seek to the beginning of the name table...
	cb.seek(fin, m_nNameOffset, LFSEEK_SET);
	//Stemp 3: Read each name of of the table,
	//we have a helper function ReadString to read
	//one string out of the table.
	for(wsLong i=0; i<m_nNameCount; i++)
	{
		ReadString(fin, &cb, m_pNameTable[i]);
		#if 1
		printf("%s\n", m_pNameTable[i]);
		#endif
	}
	m_bOpen=WS_TRUE;
	return m_bOpen;
}

void CWSFile::Close()
{
	if(!m_bOpen)
		return;
		
	WS_SAFE_DELETE_ARRAY(m_pNameTable);
	m_bOpen=WS_FALSE;
}

wsDword CWSFile::ReadString(void* fin, fio_callbacks* pCB, wsString szOut)
{
	wsChar c;
	wsDword i=0;
	do
	{
		pCB->read(&c, sizeof(wsChar), 1, fin);
		//Won't read pas the maximum string length,
		//but will continue reading the file, so
		//it will get to the next string.  The size of
		//wsString should be large enough however.
		if(i>=WS_MAX_STRING_LENGTH)
		{
			szOut[WS_MAX_STRING_LENGTH-1]=0;
			continue;
		}
		szOut[i]=c;
		i++;
	}while(c!=0);
	return i;
}