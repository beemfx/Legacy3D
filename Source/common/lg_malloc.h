#ifndef __LG_MALLOC_H__
#define __LG_MALLOC_H__

typedef unsigned int lgm_size_t;

#ifdef __cplusplus
extern "C" void* LG_Malloc(lgm_size_t size);
extern "C" void LG_Free(void* p);
extern "C" void LG_MemDbgOutput();
#else !__cplusplus
void* LG_Malloc(lgm_size_t size);
void LG_Free(void* p);
void LG_MemDbOutput();
#endif __cplusplus

#endif __LG_MALLOC_H__