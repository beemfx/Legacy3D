#include "var_mgr.h"

namespace var_sys {

CVarMgr::CDefMap* CVarMgr::s_pDefMap=NULL;

CVarMgr::CVarMgr(var_word nMaxVars)
: m_nMaxVars(nMaxVars)
, m_nNextVarToCreate(0)
, m_pVarMem(NULL)
{
	m_pVarMem = new CVar[m_nMaxVars];
	s_pDefMap=&m_DefMap;
	
	//We will always register a NULL variable, and it will be returned
	//whenever we tried to get a variable that didn't exist.
	Register<var_char*>(_T("NULL"), _T("(NULL)"), CVar::F_ROM);
}

CVarMgr::~CVarMgr(void)
{
	m_DefMap.clear();
	m_VarMap.clear();
	s_pDefMap=NULL;
	delete[]m_pVarMem;
}

bool CVarMgr::DefToValues(const var_char* def, var_float * fValue, var_long * nValue)
{
	if(s_pDefMap && s_pDefMap->count(def))
	{
		*fValue=(*s_pDefMap)[def].fValue;
		*nValue=(*s_pDefMap)[def].nValue;
		return true;
	}
	
	return false;
}


CVar* CVarMgr::CreateNewVar(const var_char* name)
{
	CVar* pOut = NULL;
	
	if(m_nMaxVars>m_nNextVarToCreate)
	{
		pOut=&m_pVarMem[m_nNextVarToCreate++];
		
		pOut->Init(name);
	}
	
	return pOut;
}

lg_var CVarMgr::Get(const var_char* name)
{
	//If the requested variable has not been registered, the NULL
	//variable is returned.
	return m_VarMap.count(name)?*m_VarMap[name]:*m_VarMap[_T("NULL")];
}

lg_var CVarMgr::operator[](const var_char* name)
{
	return Get(name);
}

void CVarMgr::DebugPrintList()
{
	for(CVarMap::iterator i = m_VarMap.begin(); i!=m_VarMap.end(); i++)
	{
		if(i->second)
			_tprintf(_T("%s = %s (%f)\n"), i->second->Name(), i->second->String(), i->second->Float());
		else
			_tprintf(_T("No value at: %s\n"), i->first);
	}
	
	_tprintf(_T("The size of the map is: %i\n"), m_VarMap.size());
}

} //namespace var_sys
