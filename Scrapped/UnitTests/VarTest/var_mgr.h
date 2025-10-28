#pragma once
#include <map>
#include "var_var.h"

namespace var_sys{

class CVarMgr
{
private:
	struct MapKeyCompare
	{
	  bool operator()(const var_char* s1, const var_char* s2) const
	  {
		 return _tcsicmp(s1, s2) < 0;
	  }
	};
	
	struct DefItem{
		var_float fValue;
		var_long  nValue;
	};
	
	var_word m_nNextVarToCreate;
	const var_word m_nMaxVars;
	CVar* m_pVarMem;
	
	typedef std::map<const var_char*, CVar*, MapKeyCompare> CVarMap;
	CVarMap m_VarMap;
	
	typedef std::map<const var_char*, DefItem, MapKeyCompare> CDefMap;
	CDefMap m_DefMap;
private:
	static CDefMap* s_pDefMap;
public:
	static bool DefToValues(const var_char* def, var_float * fValue, var_long * nValue);
private:
	CVar* CreateNewVar(const var_char* name);
public:
	CVarMgr(var_word nMaxVars);
	~CVarMgr(void);
	
	template <typename T> void Define(const var_char* def, T value)
	{
		m_DefMap[def].fValue=(var_float)value;
		m_DefMap[def].nValue=(var_long)value;
	}
	
	template <typename T> lg_var Register (const var_char* name, T value, var_word flags)
	{
		//Note that there are two exit points in the register function.
		//If all goes well and the variable is registered, then the first
		//exit point will be reached, and a new variable will be returend.
		//If either the variable already existed, or it could not be
		//created because the manager is filled up, then the NULL variable
		//will be returned.
		if(!m_VarMap.count(name))
		{
			CVar* pOut = NULL;
			
			pOut=CreateNewVar(name);
			
			if(pOut)
			{
				m_VarMap[name]=pOut;
				pOut->Set((T)value);
				pOut->SetFlags(flags);
				
				//First return point, all went well.
				return *pOut;
			}
		}

		//Second return point, variable could not be created.
		return Get(_T("NULL"));
	}

	lg_var Get(const var_char* name);
	lg_var operator[](const var_char* name);
	
	void DebugPrintList();
};

} //namespace var_sys
