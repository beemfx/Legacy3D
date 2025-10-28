#include "lw_entity.h"
#ifdef OLD_METHOD
/* PRE: pList must already be allocated, the EntList_ 
	functions do not allocate or deallocate memory, they
	just change pointers.
	POST: The list will be ready to use.
*/
_LEntList::_LEntList()
{
	Clear();
}

_LEntList::~_LEntList()
{

}

/* PRE: N/A
	POST: If there was a ent available it is returned
		and removed from the list.
*/
LEntityBase* _LEntList::Pop()
{
	if(!m_pFirst)
		return LG_NULL;
	
	LEntityBase* pRes=m_pFirst;
		
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
	
	return pRes;
}

/* PRE: N/A
	POST: Returns the first available item in the list,
	or null if there is no first item.
*/
LEntityBase* _LEntList::Peek()
{
	return m_pFirst;
}

/* PRE: pEnt should not be in pList, and it should be
	removed from any other list that it might be in.
	POST: pEnt will be added to the beginning of the list.
*/
void _LEntList::Push(LEntityBase* pEnt)
{
	pEnt->m_pPrev=LG_NULL;
	pEnt->m_pNext=m_pFirst;
	if(m_pFirst)
	{
		m_pFirst->m_pPrev=pEnt;
	}
	m_pFirst=pEnt;
	m_nCount++;
}

	
/* PRE: N/A
	POST: Returns true if the list is empty.
*/
lg_bool _LEntList::IsEmpty()
{		
	return m_pFirst?LG_FALSE:LG_TRUE;
}

/* PRE: N/A
	POST: Clears the list.
*/
void _LEntList::Clear()
{
	m_pFirst=LG_NULL;
	m_pLast=LG_NULL;
	m_nCount=0;
}

/* PRE: pEnt should be in this list.
	POST: pEnt is removed.
*/
void _LEntList::Remove(LEntityBase* pEnt)
{
	if(pEnt==m_pFirst)
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
	else if(pEnt==m_pLast)
	{
		//The last node is also a special case, but we know it isn't the only
		//node in the list because that would have been checked above.
		m_pLast=m_pLast->m_pPrev;
		m_pLast->m_pNext=LG_NULL;
	}
	else
	{
		//One problem here is that it isn't gauranteed that pEnt
		//was actually in pList, but it is a quick way to remove
		//a node.
		if(pEnt->m_pPrev && pEnt->m_pNext)
		{
			pEnt->m_pPrev->m_pNext=pEnt->m_pNext;
			pEnt->m_pNext->m_pPrev=pEnt->m_pPrev;
		}
	}
	m_nCount--;
}

#endif OLD_METHOD

