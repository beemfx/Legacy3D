#ifndef __LG_XML_H__
#define __LG_XML_H__

#include <expat.h>

#ifdef __cplusplus
extern "C" {
#endif __cplusplus

void* XML_Alloc(size_t size);
void* XML_Realloc(void* ptr, size_t size);
void XML_Free(void* ptr);

XML_Parser XML_ParserCreate_LG(const XML_Char *encoding);

#ifdef __cplusplus
}
#endif __cplusplus

#endif __LG_XML_H__