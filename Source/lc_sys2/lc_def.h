#ifndef __LC_DEF_H__
#define __LC_DEF_H__

#include "lg_types.h"

#define LC_MAX_DEF_LEN   16
#define LC_DEF_HASH_SIZE 128

#define LC_DEF_NOREDEFINE 0x00000001

lg_bool LC_CheckName(const lg_char* szName);

class CDefs
{
friend class CConsole;
private:
public:
	struct DEF_VALUE{
		lg_float f;
		lg_int   u;
	};
private:	
	struct DEF{
		lg_char   szDef[LC_MAX_DEF_LEN+1];
		DEF_VALUE nValue;
		DEF*      pHashNext;
		lg_bool   bDefined;
	};
private:
	DEF* m_pHashList;
	lg_dword m_nDefCount;
	
	lg_dword GenDefHash(const lg_char* def);
	lg_bool AddDef(const lg_char* szDef, DEF_VALUE* pValue, lg_dword Flags=0);
public:
	DEF_VALUE* GetDef(const lg_char* szDef);
public:
	CDefs();
	~CDefs();
	lg_bool AddDef(const lg_char* szDef, lg_float value, lg_dword Flags=0);
	lg_bool AddDef(const lg_char* szDef, lg_dword value, lg_dword Flags=0);
	lg_bool AddDef(const lg_char* szDef, lg_long value, lg_dword Flags=0);
	void Clear();
	lg_dword GetDefUnsigned(const lg_char* def, lg_bool* bGot);
	lg_long  GetDefSigned(const lg_char* def, lg_bool* bGot);
	lg_float GetDefFloat(const lg_char* def, lg_bool* bGot);
};


#endif __LC_DEF_H__