/* lf_list_stack.h - The CLfListStack is a special type of
	list that works as both a stack and a linked list.  It
	is designed to be used with anything, but all classes
	it contains must have the base type of LSItem.  This is
	designed to work both as a stack and as a linked list.
	In that way it has a broad range of uses.  It is was
	primarily designed to hold various lists of entities.
*/
#ifndef __LF_LIST_STACK_H__
#define __LF_LIST_STACK_H__

#include "lf_sys2.h"

#define LF_CACHE_ALIGN __declspec(align(4))

struct LF_SYS2_EXPORTS CLfListStack
{
public:
	//All items to be stored in the list stack must inherit from
	//CLfListStack::LSItem.  If they don't they can't be stored in
	//the list.
	LF_CACHE_ALIGN struct LSItem
	{
		LSItem* m_pNext;
		LSItem* m_pPrev;
		lf_dword m_nItemID;
		lf_dword m_nListStackID;
	};
private:
	//We keep track of the ID of this list, this ID is generated
	//to be unique from any other lists.
	const lf_dword m_nID;
public:
	//It is necessary to keep track of a few items in the list.
	//The list is designed to be transversed either way.
	//The members are public, so they can be accessed directly, for
	//transversals.
	LSItem* m_pFirst;
	LSItem* m_pLast;
	lf_dword m_nCount;
public:
	/* PRE: Before usage there must be a master list of nodes.
		The CLfListStack only manages nodes, it does not create or destroy them.
		POST: The list will be ready to use.
	*/
	CLfListStack();
	/* PRE: N/A
		POST: No longer usable.
	*/
	~CLfListStack();
	/* PRE: N/A
		POST: If there was a node available it is returned
		and removed from the list.  If there was no node, then
		NULL is returned.
	*/
	LSItem* Pop();
	/* PRE: N/A
		POST: Returns the first available item in the list,
		or null if there is no first item, does not remove
		it from the list.
	*/
	LSItem* Peek();
	/* PRE: pNode should not be in pList, and it should be
		removed from any other list that it might be in.
		POST: pNode will be added to the beginning of the list.
	*/
	void Push(LSItem* pNode);
	/* PRE: pNode should be in this list.
		POST: pNode is removed.  Note that if pNode was
		in a CLfListStack, but not the one on which Remove
		was called, nothing will happen.
	*/
	void Remove(LSItem* pNode);
	/* PRE: N/A
		POST: Returns TRUE if the list is empty.
	*/
	lf_bool IsEmpty();
	/* PRE: N/A
		POST: Clears the list.
	*/
	void Clear();
	
	/* PRE: N/A
		POST: Puts all the items in the list into
		this list stack, usefull when first creating a list.
	*/
	void Init(LSItem* pList, lf_dword nCount, lf_dword nItemSize);
	
private:
	//We keep track of the next id to be used when another list is created,
	//this is incremented each time we create a new list.
	static lf_dword s_nNextID;
};

#endif __LF_LIST_STACK_H__