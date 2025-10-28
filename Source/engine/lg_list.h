#ifndef __LG_LIST_H__
#define __LG_LIST_H__

#include "lg_types.h"
#include "lg_stack.h"

template <class T>
class CLList
{
public:
	//The node class
	class CNode
	{
	friend class CLList;
	public:
		T m_data;
		CNode* m_pPrev;
		CNode* m_pNext;
	};
	
private:
	lg_dword m_nMaxItems;
	CLStack<CNode*> m_stkUnusedNodes;
	CNode*          m_pNodeList;
public:
	CNode* m_pList;
public:
	CLList():
		m_nMaxItems(100),
		m_stkUnusedNodes(m_nMaxItems),
		m_pList(LG_NULL)
	{
		Init();
	}
	
	CLList(lg_dword nMaxItems):
		m_nMaxItems(nMaxItems),
		m_stkUnusedNodes(m_nMaxItems),
		m_pList(LG_NULL)
	{
		Init();
	}
	
	~CLList()
	{
		Destroy();
	}
	
private:
	__inline void Init()
	{
		//m_pUnusedNodeStack=new CLStack<CNode>(m_nMaxItems);
		m_pList=LG_NULL;
		m_pNodeList=new CNode[m_nMaxItems];
		for(lg_dword i=0; i<m_nMaxItems; i++)
		{
			m_stkUnusedNodes.Push(&m_pNodeList[i]);
		}
	}
	
	__inline void Destroy()
	{
		delete m_pNodeList;
	}
	
public:

	void Insert(T data)
	{
		
		if(m_stkUnusedNodes.IsEmpty())
		{
			//No memory, bad programming because
			//all lists should be of known size
			return;
		}
		
		//Get the new node and insert it at the
		//beginning of the list.
		CNode* pNodeNew = m_stkUnusedNodes.Pop();
		pNodeNew->m_data=data;
		pNodeNew->m_pPrev=LG_NULL;
		pNodeNew->m_pNext=m_pList;
		if(m_pList)
			m_pList->m_pPrev=pNodeNew;
		m_pList=pNodeNew;
	}
	
};

#endif __LG_LIST_H__