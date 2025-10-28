#include "var_var.h"
#include "var_mgr.h"

namespace var_sys {

void CVar::CopyString(var_char* out, const var_char* in)
{
	_tcsncpy_s(out, VAR_STR_LEN, in, VAR_STR_LEN);
	out[VAR_STR_LEN]=0;
}

var_word CVar::HexToDword(const var_char* in)
{
	var_char  c=0;
	var_word nHexValue=0;

	/* The idea behind this is keep adding to value, until
		we reach an invalid character, or the end of the string. */
	for(var_word i=2; in[i]!=0; i++)
	{
		c=in[i];
		
		if(c>='0' && c<='9')
			nHexValue=(nHexValue<<4)|(c-'0');
		else if(c>='a' && c<='f')
			nHexValue=(nHexValue<<4)|(c-'a'+10);
		else if(c>='A' && c<='F')
			nHexValue=(nHexValue<<4)|(c-'A'+10);
		else
			break;
	}
	
	return nHexValue;
}

void CVar::StringToNumber(const var_char* in, var_float * fValue, var_long * nValue)
{
	if(in[0]=='0' && (in[1]=='x' || in[1]=='X'))
	{
		*nValue = HexToDword(in);
		*fValue = (var_float)*nValue;
	}
	else
	{
		//Get definiton.
		if(!CVarMgr::DefToValues(in, fValue, nValue))
		{
			//If no definition.
			*nValue = _ttol(in);
			*fValue = (var_float)_tstof(in);
		}
	}
}

void CVar::Init(const var_char* name)
{
	m_nFlags=0;
	CopyString(m_strName, name);
	Set(_T(""));
}

CVar::CVar(void)
: m_nFlags(0)
{
	//_tprintf(_T("No arg constructor\n"));
	CopyString(m_strName, _T("(Not Initialized)"));
	Set(_T(""));
}

CVar::~CVar(void)
{
}

void CVar::SetFlags(var_word flags)
{
	m_nFlags=flags;
}

var_float CVar::Float()const
{
	return m_fValue;
}

var_word CVar::Word()const
{
	return (var_word)m_nValue;
}

var_long CVar::Long()const
{
	return (var_long)m_nValue;
}

const var_char* CVar::String()const
{
	return &m_strValue[0];
}

const var_char* CVar::Name()const
{
	return &m_strName[0];
}

void CVar::Set(var_float value)
{
	if(m_nFlags&F_ROM)
		return;
		
	m_fValue=value;
	m_nValue=(var_word)(var_long)value;
	_sntprintf_s(m_strValue, VAR_STR_LEN, VAR_STR_LEN, _T("%f"), value);

}


void CVar::Set(var_word value)
{
	if(m_nFlags&F_ROM)
		return;
		
	m_fValue=(var_float)value;
	m_nValue=value;
	_sntprintf_s(m_strValue, VAR_STR_LEN, VAR_STR_LEN, _T("%u"), value);
}


void CVar::Set(var_long value)
{
	if(m_nFlags&F_ROM)
		return;
		
	m_fValue=(var_float)value;
	m_nValue=value;
	
	_sntprintf_s(m_strValue, VAR_STR_LEN, VAR_STR_LEN, _T("%i"), value);
}

void CVar::Set(const var_char* value)
{
	if(m_nFlags&F_ROM)
		return;
		
	CopyString(m_strValue, value);
	StringToNumber(value, &m_fValue, (var_long*)&m_nValue);	
}

void CVar::Set(const CVar& rhs)
{
	if(m_nFlags&F_ROM)
		return;
		
	CopyString(this->m_strValue, rhs.m_strValue);
	this->m_nValue=rhs.m_nValue;
	this->m_fValue=rhs.m_fValue;
}

void CVar::operator= (var_float value)
{
	Set(value);
}


void CVar::operator= (var_word value)
{
	Set(value);
}

void CVar::operator= (var_long value)
{
	Set(value);
}

void CVar::operator= (const var_char* value)
{
	Set(value);
}

void CVar::operator= (CVar & rhs)
{
	//_tprintf(_T("= operator\n"));
	Set(rhs);
}

} //namespace var_sys
