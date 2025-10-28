/* lg_malloc.cpp - Memory management for the game. */
#include <malloc.h>
#include "lg_malloc.h"


#ifdef _DEBUG
#include <windows.h>
#include <stdio.h>
extern "C" unsigned long g_nBlocks=0;
#endif _DEBUG

extern "C" void* LG_Malloc(lgm_size_t size)
{
	#ifdef _DEBUG
	g_nBlocks++;
	
	//#define CHECK_SIZE 264
	#ifdef CHECK_SIZE
	if(size==CHECK_SIZE)
		OutputDebugString(":::BYTES ALLOCATED:::\n");
	#endif
	#endif _DEBUG
	
	return malloc(size);
}

extern "C" void LG_Free(void* p)
{
	#ifdef _DEBUG
	g_nBlocks--;
	#endif _DEBUG
	free(p);
}

extern "C" void LG_MemDbgOutput()
{
	#ifdef _DEBUG
	char szTemp[128];
	sprintf(szTemp, "%d allocations left.\n", g_nBlocks);
	OutputDebugString(szTemp);
	#endif _DEBUG
}

#ifdef USE_LG_NEW

void* operator new (lgm_size_t size)
{
	return LG_Malloc(size);
}

void operator delete (void* p)
{
	LG_Free(p);
}

#endif USE_LG_NEW

