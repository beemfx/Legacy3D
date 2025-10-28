#ifndef __LV_2DIMG_H__
#define __LV_2DIMG_H__
#include <d3d9.h>
#include "common.h"

#define LV2DIMGVERTEX_TYPE \
(                          \
	D3DFVF_XYZ|             \
	D3DFVF_DIFFUSE|         \
	D3DFVF_TEX1             \
)

typedef struct _LV2DIMGVERTEX{
	float x, y, z;
	L_dword Diffuse;
	float tu, tv;
}LV2DIMGVERTEX, *LPLV2DIMGVERTEX;



#ifdef __cplusplus
extern "C"{
#endif /*__cplusplus*/
typedef struct _LV2DIMAGE{
	IDirect3DTexture9* lpTexture; //Image texture.
	IDirect3DVertexBuffer9* lpVB; //Vertex buffer for image.
	LV2DIMGVERTEX Vertices[4]; //Vertices for image.
	L_dword dwWidth; //Width of image.
	L_dword dwHeight; //Height of image.
	L_bool bIsColor; //Color of image, if there is a color.
	L_bool bFromTexture; //If the image is from a texture.
	IDirect3DDevice9* lpDevice; //Device that was used to create the image.
	struct _LV2DIMAGE* lpCreate;//A pointer to the image that created this one.
	char szFilename[260]; //The name of the texture for reloading.
	L_dword dwTransparent;
	L_rect rcSrc;
}LV2DIMAGE, *LPLV2DIMAGE;


L_bool L2DI_StartStopDrawing(
	IDirect3DDevice9* lpDevice, 
	L_dword ViewWidth, 
	L_dword ViewHeight, 
	L_bool bStart);



LV2DIMAGE* L2DI_CreateFromFile(
	IDirect3DDevice9* lpDevice,
	char* szFilename,
	L_rect* rcSrc,
	L_dword dwWidth,
	L_dword dwHeight,
	L_dword dwTransparent);

LV2DIMAGE* L2DI_CreateFromImage(
	LV2DIMAGE* lpImageSrc, 
	L_rect* rsrc, 
	L_dword dwWidth, 
	L_dword dwHeight,
	L_dword dwTransparent);

LV2DIMAGE* L2DI_CreateFromTexture(
	IDirect3DDevice9* lpDevice,
	L_rect* rcSrc,
	L_dword dwWidth,
	L_dword dwHeight,
	LPDIRECT3DTEXTURE9 lpTexture,
	L_pvoid pExtra);

LV2DIMAGE* L2DI_CreateFromColor(
	IDirect3DDevice9* lpDevice,
	L_dword dwWidth,
	L_dword dwHeight,
	L_dword dwColor);

L_bool L2DI_Invalidate(LV2DIMAGE* hImage);

L_bool L2DI_Validate(LV2DIMAGE* hImage, IDirect3DDevice9* lpDevice, void* pExtra);

L_bool L2DI_Delete(LV2DIMAGE* hImage);

L_bool L2DI_Render(
	LV2DIMAGE* hImage,
	IDirect3DDevice9* lpDevice,
	float x,
	float y);
/*
L_bool L2DI_RenderEx(
	LV2DIMAGE* hImage,
	L_rect* rcDest,
	L_rect* rcSrc);

L_bool L2DI_RenderRelativeEx(
	LV2DIMAGE* hImage,
	float fXDest,
	float fYDest,
	float fWidthDest,
	float fHeightDest,
	L_rect* rcSrc);
*/


#ifdef __cplusplus
}
#endif /*__cplusplus*/
#endif /* __LV_2DIMG_H__ */