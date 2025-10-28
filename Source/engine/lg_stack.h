#ifndef __LG_STACK_H__
#define __LG_STACK_H__

#include "lg_types.h"
#include "memory.h"

template <class T>
class CLStack{
private:
	lg_dword m_nSize;
	lg_dword m_nPointer;
	
	T*       m_pStack;
	T        m_objDummy;
public:
	CLStack():m_nSize(10), m_nPointer(0)
	{
		m_pStack = new T[m_nSize];
		memset(&m_objDummy, 0, sizeof(T));
	}
	
	CLStack(lg_dword nSize):m_nSize(nSize), m_nPointer(0)
	{
		m_pStack = new T[m_nSize];
		memset(&m_objDummy, 0, sizeof(T));
	}
	
	~CLStack()
	{
		if(m_pStack) delete[]m_pStack;
	}
	
	void Push(T obj)
	{
		if(m_nPointer<m_nSize)
			m_pStack[m_nPointer++]=obj;
	}
	
	T Peek()
	{
		if(m_nPointer>0)
			return m_pStack[m_nPointer-1];
		else
			return m_objDummy;
	}
	
	T Pop()
	{
		if(m_nPointer>0)
			return m_pStack[--m_nPointer];
		else
			return m_objDummy;
	}
	
	lg_bool IsEmpty()
	{
		return m_nPointer>0?LG_FALSE:LG_TRUE;
	}
};

#endif //__LG_STACK__