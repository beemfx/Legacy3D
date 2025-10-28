/* lg_hash_mgr.h - The Hash Item Manager.
	Copyright (c) 2008 B. Everett Myers.
	
	The Legacy engine uses quite a few managers to manage
	resources.  Hashing seems to be one of the fastest
	ways to store and locate resources.  I had developed
	three hash managers already, and it occured to me that
	I'd be developing more, so I decided to create a base
	template class, that could be used for multiple hash
	managers, that way I didn't have to duplicate code, if
	I found that there was a problem with a manager's hashing
	then I'd only have to change it in one class and so forth.
	
	The idea behind this class is that it is inherited by
	a parent class, at which time the parent indicates the
	type of item stored in the hash.  This class is primarily
	designed to store resources with an associated filename,
	and the filename can be used to locate the resource
	needed.  When overriding it is necessary to realize that
	The base class will allocate and deallocate memory, but
	any resources that have been loaded need to be deleted,
	or released, before the destructor of this base class
	is called.  Further DoLoad and DoDestroy must be overloaded
	as they specify how a resource should be loaded and
	destroyed (or released).  All items are obtained by
	reference.  That is the manager returns an unsigned long
	(hm_item) that can then be used to reference the item
	obtained.  Individual classes may have a method such
	as GetInterface that might retrun and actual interface
	or class, or they may have methods such as SetTexture
	which works only internally to Set the current rendering
	texture, for instance (See the CLTMgr class).  See the
	comments before each method, for more specific usage, the
	user important methods are located at the bottom of the
	class.
	
	Currently this is used for:
	CLTMgr
	CLFxMgr
	
	Should be used for:
	CLSkelMgr
	CLMeshMgr
	
	
*/
#ifndef __LG_HASH_MGR_H__
#define __LG_HASH_MGR_H__

#include "lg_func.h"
#include "lg_types.h"
#include "lg_err.h"
#include "lg_err_ex.h"

//All items are represented by hm_item, in this way
//they are used by reference rather than by the actual
//item.  Only the hash manager itself should do
//anything with the items.
typedef unsigned long hm_item;

template<class T>
class CLHashMgr
{
//Constants and flags:
protected:
	//Internally Used Values:
	//Empty node is used to specify a hash location
	//that currently does not have an item associated
	//with it.
	const static lg_dword HM_EMPTY_NODE=0xFFFFFFFF;
	//A memory node is one in which an item exists at 
	//that hash location, but it doesn't have a hash 
	//code associated with it, can be used in classes
	//that which too, but it is not used in the base
	//class.
	const static lg_dword HM_MEMORY_NODE=0xFFFFFFD;
	
	//Null is an item with a zero reference.  This
	//is never returned by the Load method, as eve
	//if an item could not be laoded, HM_DEFAULT_ITEM
	//is returned.
	const static hm_item HM_NULL_ITEM=0x00000000;
	//The default item, note that it is item 1,
	//in that way it is the first item in the list.
	//Default items must be specified in the parent
	//classes constructor.  By default it is just
	//a NULL item.
	const static hm_item HM_DEFAULT_ITEM=0x00000001;
	
//Internal structures:
protected:
	//A list of item node structures is where all the
	//data is stored.
	LG_CACHE_ALIGN struct ItemNode{
		T* pItem;            //The pointer to the item stored.
		lg_void* pExtra;     //Any extra data that the manager may want to use.
		lg_path szName;      //The name of the item, usually the filename, but can be anything.
		lg_dword nFlags;     //Flags associated with the item, passed to the DoLoad function.
		lg_dword nHashCode;  //The hash code associated with this item.
		lg_dword nHashCount; //The hash count associated with the spot in the list (may want to change this to nNextMatchingHash instead).
	};
//Internal data:
protected:
	lg_string m_szMgrName; //Stores the name of the manager, for debugging.
	const lg_dword m_nMaxItems; //Maximum number of items allowed in the list, unchangeable.
	ItemNode* m_pItemList;     //The hash list of items, note that m_pItemList[0] is reserved for the default item.
//Internal methods:
protected:
	/* PRE: Constructor: Should be called from the 
		inherited class's constructor.
		POST: Memory is allocated for the items, all items set to empty.
	*/
	CLHashMgr(lg_dword nMaxItem, lg_string szMgrName)
		: m_nMaxItems(nMaxItem)
	{
		//Allocate memory for the hash list, and throw a memory
		//exception if we are out.
		m_pItemList=new ItemNode[m_nMaxItems];
		if(!m_pItemList)
		{
			throw LG_ERROR(LG_ERR_OUTOFMEMORY, "Ran out of memory!");
		}
		
		//Start by setting all the items to empty,
		//and setting the hashcount to 0 for all of them.
		for(lg_dword i=0; i<m_nMaxItems; i++)
		{
			m_pItemList[i].nHashCode=HM_EMPTY_NODE;
			m_pItemList[i].nHashCount=0;
		}
		
		//Save the name of the manager, this is used in error messages.
		LG_strncpy(m_szMgrName, szMgrName, LG_MAX_STRING);
		
		//Setup some stuff for the default item.
		//Note that it's hash code is 0, and no other
		//functions should ever be hashed as zero.
		m_pItemList[0].nHashCode=0;
		m_pItemList[0].nHashCount=1;
		m_pItemList[0].nFlags=0;
		m_pItemList[0].pItem=LG_NULL;
		m_pItemList[0].pExtra=LG_NULL;
		m_pItemList[0].szName[0]=0;
	}
	
	/* PRE: N/A
		POST: Destroys everything, unloads all items, and
		deallocates memory associated.
	*/
	~CLHashMgr()
	{
		LG_SafeDeleteArray(m_pItemList);
	}
	
//Internal use only methods:
private:
	/* PRE: Called only from LoadItem.
		POST: Basically a check to see if an item
		is already loaded in the manager.  If it is, it is
		returned and LoadItem method ends quickly.  If it isn't
		The manager knows it must load the item.  0 means the
		item does not exist.
	*/
	hm_item GetItem(lg_path szFilename, lg_dword* pHash)
	{
		//First get the hash for the item, then see if it is loaded.
		//The hash should limit the items to be from 1 to m_nMaxItems-1.
		//This way location 0 is always reserved for the default itme.
		*pHash=LG_HashFilename(szFilename)%(m_nMaxItems-1)+1;
		//If the hash count for the hash is 0 then there is no
		//matching hash, and we know the item isn't loaded.
		if(m_pItemList[*pHash].nHashCount==0)
		{
			return HM_NULL_ITEM;
		}
		
		//Since the hash index is not empty we'll search until we either
		//find a matching node, or exhaust the search.
		
		//For each index in the hash list we keep a count of how
		//many nodes use that hash, this speeds up the search, so
		//we don't have to loop through the entire list to find
		//our node, we just need to search through till we find
		//all the nodes that have the mathcing hash.
		lg_dword nSearchCount=m_pItemList[*pHash].nHashCount;
		//We keep track of the location we are currently searching,
		//naturally we start with the current hash, hopefully this
		//is where the item we want is actually stored, then our
		//method will return quickly.
		lg_dword nSearchLoc=*pHash;
		
		//Now we do a loop, where we continue to search for a mathcing
		//node, or until we've searched and found all the nodes that
		//matched the hash, but didn't find the node we wanted.
		while(nSearchCount>0) //If the search count drops to zero we've found all matching hashes, and didn't find the node.
		{
			//If the item we're searching at has a matching hash
			//we need to do a specific check.
			if(m_pItemList[nSearchLoc].nHashCode==*pHash)
			{
				//POTENTIAL EXIT POINT:
				if(strncmp(m_pItemList[nSearchLoc].szName, szFilename, LG_MAX_PATH)==0)
					return nSearchLoc+1; //Add one so we don't have a zero reference.
				else
					nSearchCount--; //If the hash didn't match the node, we have one less node to search for.
			}
			
			//We now move our search to the next index, note that
			//our indexes wrap around, so if we get to the maximum hash
			//we simply wrap around, that is why our increment is not simply
			//nSearchLoc++, this does mean that if we loop we'll
			//end up going through the default item, but the search should
			//still be quick.
			nSearchLoc=(nSearchLoc+1)%m_nMaxItems;
			//The following is just a check to make sure, our code is correct.
			//If we loop all the way around to the initial node, then it means
			//that the number stored in m_ItemList[*pHash].nHashCount was
			//incorrect, and that means that there was an error somewhere in
			//our code.
			if(nSearchLoc==*pHash)
			{
				throw LG_ERROR(LG_ERR_UNKNOWN,
					LG_TEXT("HashMgr ERROR: Looped through list and didn't exhaust search.  Check Code."));
			}
		}
		
		//If we exhausted the search, and didn't find anything we
		//return 0 for empty.
		return HM_NULL_ITEM;	
	}
	
	
/* User methods, these are the methods that are used in
	an individual manager, or that need to be overridden.
	The user also has access to the list, the maximum items
	avaailabe, and the default item.  These variables can
	be used, but should generally not be altered.
*/
//Private Ones:
private:
	/* PRE: Called only from the LoadItem method.
		POST: Loads the item, flags can be used to specify
		how the item is loaded.
		
		Notes: Must be overridden, typically should use the
		filename to laod some kind of resource, and flags
		are specific to the class.  This is called when
		the Load method returns a new resource, but it can
		be called for any reason the user may see fit, such
		as for validating D3D objects, loading the default
		item, etc.
	*/
	virtual T* DoLoad(lg_path szFilename, lg_dword nFlags)=0;
	
	/* PRE: Called only from the UnloadItems method.
		POST: Should destroy the item, deallocate, or release it.
		
		Nost: Must be overridden, should destroy the item obtained,
		by DoLoad, deallocate any memory associated, or release
		the item, etc.  Called whenever an item is unloaded, as in
		UnloadItems, but may be called for other reasons as well
		(such as destroying the default item).
	*/
	virtual void DoDestroy(T* pItem)=0;
	
//Public ones:
public:
	/* PRE: Used to load an item, calls DoLoad() which must be
		overridden.  Flags are pass to DoLoad and are used there,
		they can also be used in the UnloadItems.
		POST: If the filename was not a valid item, then the default
		item will be returned.
		
		Notes: Called only to load an item, should not be overriden,
		and should not be modified.
	*/
	hm_item Load(lg_path szFilename, lg_dword nFlags)
	{
		//We first attempt to get the item, if we find it
		//we can return it.
		lg_dword nHash;
		hm_item itemOut=GetItem(szFilename, &nHash);
		if(itemOut)
		{
			Err_Printf(LG_TEXT("%s: Obtained \"%s\"."), m_szMgrName, szFilename);
			return itemOut;
		}
		
		//If not we need to load it:
		//So first we need to find an empty spot in our list:
		lg_dword nLoc=nHash;
		while(m_pItemList[nLoc].nHashCode!=HM_EMPTY_NODE)
		{
			//Search through the list in a circular manner.
			nLoc=(nLoc+1)%m_nMaxItems;
			//If wee looped through the entire list then we don't
			//have any more room.
			if(nLoc==nHash)
			{
				//POTENTIAL EXIT POINT:
				Err_Printf(LG_TEXT("%s ERROR: Max textures laoded. Can't load \"%s\"."), m_szMgrName, szFilename);
				Err_Printf(LG_TEXT("%s: Obtained default texure instead."), m_szMgrName);
				return HM_DEFAULT_ITEM;
			}
		}
		
		//We found a spot so let's do the load procedure:
		T* pOut=DoLoad(szFilename, nFlags);
		//If we ended up obtaining the default item
		//then we'll update that data.
		if(!pOut)
		{
			Err_Printf(LG_TEXT("%s: Obtained default item."), m_szMgrName);
			return HM_DEFAULT_ITEM;
		}
		else
		{
			//Increase the item count under the correct hash
			m_pItemList[nHash].nHashCount++;
			//Everything else is updated under the location
			m_pItemList[nLoc].nFlags=nFlags;
			m_pItemList[nLoc].nHashCode=nHash;
			LG_strncpy(m_pItemList[nLoc].szName, szFilename, LG_MAX_PATH);
			m_pItemList[nLoc].pItem=pOut;
			Err_Printf(LG_TEXT("%s: Loaded \"%s\" %i@%i"), m_szMgrName, szFilename, nHash, nLoc);
		}
		
		return nLoc+1; //We add 1 so the reference so 0 can be null.	
	}
	
	/* PRE: item, must be an item in the list.
		POST: Removes the item from the manager, calling the
		DoDestroy method.
		
		Notes: This method isn't typically used as the point of
		the manager, is that it can find resources as needed
		without reloading them.  Typically resources would be
		unloaded and relaoding between map loads, where different
		resources are used.
	*/
	void Unload(hm_item item)
	{
		//Can't remove the default item, or a null item
		if(item<=HM_DEFAULT_ITEM)
		{
			Err_Printf(
				"%s ERROR: Attempted to either Unload the default item, or a null item.", 
				m_szMgrName);
			return;
		}
		
		//Decrease the reference of the item to match it
		//with the hash list.
		item--;
		//If the node isn't a memory object (e.g. a lightmap)
		//then we need to decrease the hash count, see above
		//for remarks concerning HM_MEMORY_NODE.
		if(m_pItemList[item].nHashCode!=HM_MEMORY_NODE)
			m_pItemList[m_pItemList[item].nHashCode].nHashCount--;
			
			
		//Zero out the information and Destroy the item.
		m_pItemList[item].nHashCode=HM_EMPTY_NODE;
		DoDestroy(m_pItemList[item].pItem);
		m_pItemList[item].pItem=LG_NULL;
		Err_Printf("%s: Unloaded \"%s\".", m_szMgrName, m_pItemList[item].szName);
		m_pItemList[item].szName[0]=0;
	}
	
	/* PRE: N/A
		POST: Unloads all items, except the default item.
		
		Notes: Will call teh DoDestroy method for every item stored
		in the manager.  Should be called in the overriding class'
		destructor.  The default item should also be destroyed in
		the destructor as well.
	*/
	void UnloadItems()
	{
		//We start at 1 in order to skip the default
		//item.
		for(lg_dword i=1; i<m_nMaxItems; i++)
		{
			if(m_pItemList[i].nHashCode!=HM_EMPTY_NODE)
			{
				if(m_pItemList[i].nHashCode!=HM_MEMORY_NODE)
					m_pItemList[m_pItemList[i].nHashCode].nHashCount--;
				m_pItemList[i].szName[0]=0;
				m_pItemList[i].nHashCode=HM_EMPTY_NODE;
				DoDestroy(m_pItemList[i].pItem);
				m_pItemList[i].pItem=LG_NULL;
			}
		}
	}
	
	/* PRE: N/A
		POST: Prints debug info about the items in the list, the
		default item is in spot 0.
	*/
	void PrintDebugInfo()
	{
		for(lg_dword i=0; i<m_nMaxItems; i++)
		{
			if(m_pItemList[i].nHashCode==CLHashMgr::HM_EMPTY_NODE)
				Err_Printf("%i Empty (%i)", i+1, m_pItemList[i].nHashCount);
			else if(m_pItemList[i].nHashCode==CLHashMgr::HM_MEMORY_NODE)
				Err_Printf("%i MEMORY (%i)", i+1, m_pItemList[i].nHashCount);
			else
				Err_Printf("%i \"%s\" %i@%i (%i)",
					i+1,
					m_pItemList[i].szName,
					m_pItemList[i].nHashCode, i,
					m_pItemList[i].nHashCount);
		}
	}
};

#endif __LG_HASH_MGR_H__