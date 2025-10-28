//See header file for instructions.
#include "lg_list_stack.h"

//We keep track of a ID identifier which
//is assigned to each list, that way we can
//distinguish lists, so that we can identify
//to what list a node belongs.
lg_dword CLListStack::s_nNextID=811983;  


CLListStack::CLListStack():
	m_nID(s_nNextID++),
	m_pFirst(LG_NULL),
	m_pLast(LG_NULL),
	m_nCount(0)
{}

CLListStack::~CLListStack()
{}

CLListStack::LSItem* CLListStack::Pop()
{
	if(!m_pFirst)
		return LG_NULL;
	
	CLListStack::LSItem* pRes=m_pFirst;
		
	//Removing the first node is a special case.
	m_pFirst=m_pFirst->m_pNext;
	//If the first node was also the last node then we also
	//need to set the last node to null.
	if(m_pFirst==LG_NULL)
	{
		m_pLast=LG_NULL;
	}
	else
	{
		//The first node's previous must be set to null.
		m_pFirst->m_pPrev=LG_NULL;
	}
	
	m_nCount--;
	pRes->m_nListStackID=0;
	return pRes;
}


CLListStack::LSItem* CLListStack::Peek()
{
	return m_pFirst;
}


void CLListStack::Push(CLListStack::LSItem* pNode)
{
	#if 0
	if(pNode->m_nListStackID!=0)
	{
		//Warning the node was still in a list.
	}
	#endif
	pNode->m_pPrev=LG_NULL;
	pNode->m_pNext=m_pFirst;
	pNode->m_nListStackID=m_nID;
	if(m_pFirst)
	{
		m_pFirst->m_pPrev=pNode;
	}
	else
	{
		m_pLast=pNode;
	}
	m_pFirst=pNode;
	m_nCount++;
}


lg_bool CLListStack::IsEmpty()
{		
	return m_pFirst?LG_FALSE:LG_TRUE;
}


void CLListStack::Clear()
{
	for(LSItem* pNode=m_pFirst; pNode; pNode=pNode->m_pNext)
	{
		pNode->m_nListStackID=0;
	}
	m_pFirst=LG_NULL;
	m_pLast=LG_NULL;
	m_nCount=0;
}
void CLListStack::Remove(CLListStack::LSItem* pNode)
{
	//If this node wasn't even in this list, we just
	//return.
	if(pNode->m_nListStackID!=m_nID)
	{
		return;
	}
		
	if(pNode==m_pFirst)
	{
		//Removing the first node is a special case.
		m_pFirst=m_pFirst->m_pNext;
		//If the first node was also the last node then we also
		//need to set the last node to null.
		if(m_pFirst==LG_NULL)
		{
			m_pLast=LG_NULL;
		}
		else
		{
			//The first node's previous must be set to null.
			m_pFirst->m_pPrev=LG_NULL;
		}
	}
	else if(pNode==m_pLast)
	{
		//The last node is also a special case, but we know it isn't the only
		//node in the list because that would have been checked above.
		m_pLast=m_pLast->m_pPrev;
		m_pLast->m_pNext=LG_NULL;
	}
	else
	{
		//One problem here is that it isn't gauranteed that pNode
		//was actually in pList, but it is a quick way to remove
		//a node, the id check above should prevent weird removals.
		if(pNode->m_pPrev && pNode->m_pNext)
		{
			pNode->m_pPrev->m_pNext=pNode->m_pNext;
			pNode->m_pNext->m_pPrev=pNode->m_pPrev;
		}
	}
	m_nCount--;
}

void CLListStack::Init(CLListStack::LSItem* pList, lg_dword nCount, lg_dword nItemSize)
{
	this->Clear();
	for(lg_dword i=0; i<nCount; i++)
	{
		LSItem* pCurItem=(LSItem*)((lg_byte*)pList+i*nItemSize);
		pCurItem->m_nListStackID=0;
		pCurItem->m_nItemID=i;
		Push(pCurItem);
		/*
		pList[i].m_nListStackID=0;
		Push(&pList[i]);
		*/
	}
}