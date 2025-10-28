#ifndef __LM_BASE_H__
#define __LM_BASE_H__

#include "ML_lib.h"
#include "lg_types.h"

class CLMBase{
//Types and constants:
public:
	static const lg_dword LM_MAX_NAME=31;
	typedef lg_char LMName[LM_MAX_NAME+1];
	static const lg_dword LM_MAX_PATH=259;
	typedef lg_char LMPath[LM_MAX_PATH+1];
protected:
	enum RW_MODE{
		RW_UNKNOWN=0,
		RW_READ,
		RW_WRITE
	};
	typedef lg_uint (__cdecl * ReadWriteFn)(lg_void* file, lg_void* buffer, lg_uint size);
	
	static const lg_dword LM_FLAG_LOADED=0x00000001;
	
//Private data:
protected:
	lg_dword m_nFlags;
	
//Private required methods:
protected:
	CLMBase(): m_nFlags(0){}
	~CLMBase(){}
	
	virtual lg_bool Serialize(
		lg_void* file, 
		ReadWriteFn read_or_write,
		RW_MODE mode)=0;
		
public:
	lg_bool IsLoaded(){return m_nFlags&LM_FLAG_LOADED;}
};

#endif __LM_BASE_H__