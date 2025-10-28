/* Memory allocation. */
#include <malloc.h>
#include "lf_sys2.h"

LF_ALLOC_FUNC g_pMalloc=malloc;
LF_FREE_FUNC  g_pFree=free;

extern "C" lf_void LF_SetMemFuncs(LF_ALLOC_FUNC pAlloc, LF_FREE_FUNC pFree)
{
	g_pMalloc=pAlloc;
	g_pFree=pFree;
}

extern "C" lf_void* LF_Malloc(lf_size_t size)
{
	if(g_pMalloc)
		return g_pMalloc(size);
	else
		return LF_NULL;
}

extern "C" lf_void LF_Free(lf_void* p)
{
	if(g_pFree)
		return g_pFree(p);
}

#if 0
void* operator new (lf_size_t size)
{
	return LF_Malloc(size);
}

void operator delete (void* p)
{
	LF_Free(p);
}
#endif