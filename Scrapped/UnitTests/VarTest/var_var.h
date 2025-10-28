#pragma once

#include "var_types.h"

namespace var_sys{

class CVar;

typedef CVar& lg_var;

class CVar
{
friend class CVarMgr;
public:
	//This is a blank var, whose hash value hasn't come up.
	static const var_word F_EMPTY=0x00000001; 
	//var can't be changed.
	static const var_word F_ROM=0x00000002;
	//var should be saved upon exit.
	static const var_word F_SAVE=0x00000004;
	//something should update immediately on change of var.
	static const var_word F_UPDATE=0x00000008;
private:
	var_string m_strName;
	var_word  m_nFlags;
	
	
	var_float  m_fValue;
	var_long   m_nValue;
	var_string m_strValue;
private:
	__inline static void CopyString(var_char* out, const var_char* in);
	__inline static var_word HexToDword(const var_char* in);
	__inline static void StringToNumber(const var_char* in, var_float * fValue, var_long * nValue);
private:
	void SetFlags(var_word flags);
	void Init(const var_char* name);
private:
	CVar(void);
	~CVar(void);
public:
	var_float        Float()const;
	var_word         Word()const;
	var_long         Long()const;
	const var_char*  String()const;
	
	const var_char* Name()const;
	
	void Set(var_float value);
	void Set(var_word value);
	void Set(var_long value);
	void Set(const var_char* value);
	void Set(const CVar& rhs);
	
	void operator= (var_float value);
	void operator= (var_word value);
	void operator= (var_long value);
	void operator= (const var_char* value);
	void operator= (CVar& rhs);
};

} //namespace var_sys
