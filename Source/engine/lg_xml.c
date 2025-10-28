#include "lg_xml.h"
#include "lg_malloc.h"

void* XML_Alloc(size_t size)
{
	return LG_Malloc((lgm_size_t)size);
}

void* XML_Realloc(void* ptr, size_t size)
{
	LG_Free(ptr);
	return LG_Malloc((lgm_size_t)size);
}

void XML_Free(void* ptr)
{
	if(ptr)
		LG_Free(ptr);
}

XML_Parser XML_ParserCreate_LG(const XML_Char *encoding)
{
	XML_Memory_Handling_Suite ms;
	ms.malloc_fcn=XML_Alloc;
	ms.realloc_fcn=XML_Realloc;
	ms.free_fcn=XML_Free;
	
	return XML_ParserCreate_MM(0, &ms, 0);
}