#ifndef __3DWSSDK_H__
#define __3DWSSDK_H__

#include <pshpack1.h>

typedef char wsChar;
typedef unsigned char wsByte;
typedef unsigned short wsWord;
typedef unsigned long wsDword;
typedef signed short wsShort;
typedef signed long wsLong;
typedef float wsFloat;
typedef char* wsString;
typedef unsigned long wsName;

typedef struct _wsColor3
{
	wsByte r, g, b;
}wsColor3;

typedef struct _wsVec2
{
	wsFloat x, y;
}wsVec2;

typedef struct _wsVec3
{
	wsFloat x, y, z;
}wsVec3;

typedef struct _wsVec4
{
	wsFloat x, y, z, w;
}wsVec4;


//Plugin constants
#define PLUGIN_EXPORT        1
#define PLUGIN_IMPORT        2
#define PLUGIN_BRUSHCREATION 4
#define PLUGIN_MESHLOAD      8

//Brush creation parameter constants
#define PLUGIN_BRUSHCREATION_SPINNER  1
#define PLUGIN_BRUSHCREATION_CHECKBOX 2
#define PLUGIN_BRUSHCREATION_COMBOBOX 3

//Map components
#define PLUGIN_BRUSHES        1
#define PLUGIN_MESHES         2
#define PLUGIN_ENTITIES       4
#define PLUGIN_TERRAIN        8
#define PLUGIN_MESHREFERENCES 16
#define PLUGIN_LIGHTMAPS      32
#define PLUGIN_COPYTEXTURES   64

#define WS_FUNC __stdcall

void WS_FUNC PluginClass(wsByte* pIn, wsDword nInSize, wsByte* pOut, wsDword nOutSize);
void WS_FUNC PluginMeshLoad(wsByte* pIn, wsDword nInSize, wsByte* pOut, wsDword nOutSize);
void WS_FUNC PluginDescription(wsByte* pIn, wsDword nInSize, wsByte* pOut, wsDword nOutSize);
void WS_FUNC PluginFileExtension(wsByte* pIn, wsDword nInSize, wsByte* pOut, wsDword nOutSize);
void WS_FUNC PluginExport(wsByte* pIn, wsDword nInSize, wsByte* pOut, wsDword nOutSize);
void WS_FUNC PluginLabel(wsByte* pIn, wsDword nInSize, wsByte* pOut, wsDword nOutSize);
typedef void (WS_FUNC * WS_PLUGIN_FUNC)(wsByte* pIn, wsDword nInSize, wsByte*  pOut, wsDword nOutSize);

#include <poppack.h>

#endif __3DWSSDK_H__