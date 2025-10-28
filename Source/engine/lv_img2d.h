#ifndef __LV_2DIMG_H__
#define __LV_2DIMG_H__
#include <d3d9.h>
#include "common.h"
#include "lg_tmgr.h"

class CLImg2D
{
private:
	//The vertex type for the 2D image, and
	//the structure.
	#define LV2DIMGVERTEX_TYPE \
	(                          \
		D3DFVF_XYZ|             \
		D3DFVF_DIFFUSE|         \
		D3DFVF_TEX1             \
	)

	typedef struct _LV2DIMGVERTEX{
		float x, y, z;
		lg_dword Diffuse;
		float tu, tv;
	}LV2DIMGVERTEX, *LPLV2DIMGVERTEX;
private:
	IDirect3DDevice9*       m_pDevice;
	tm_tex      m_pTexture; //Image texture.
	IDirect3DVertexBuffer9* m_pVB; //Vertex buffer for image.
	LV2DIMGVERTEX           m_Vertices[4]; //Vertices for image.
	
	lg_dword m_dwWidth; //Width of image.
	lg_dword m_dwHeight; //Height of image.
	lg_bool  m_bIsColor; //Color of image, if there is a color.
	lg_bool  m_bFromTexture; //If the image is from a texture.
	
	struct _LV2DIMAGE* m_pCreate;//A pointer to the image that created this one.
	
	char m_szFilename[LG_MAX_PATH+1]; //The name of the texture for reloading.
	
	lg_dword m_dwTransparent;
	lg_rect  m_rcSrc;
public:
	CLImg2D();
	~CLImg2D();

	lg_bool CreateFromFile(
		IDirect3DDevice9* lpDevice,
		char* szFilename,
		lg_rect* rcSrc,
		lg_dword dwWidth,
		lg_dword dwHeight,
		lg_dword dwTransparent);

	lg_bool CreateFromColor(
		IDirect3DDevice9* lpDevice,
		lg_dword dwWidth,
		lg_dword dwHeight,
		lg_dword dwColor);

	void Invalidate();
	lg_bool Validate(void* pExtra);
	void Delete();
	lg_bool Render(float x, float y);
	
private:
	void CLImg2D::SetVertices(
		lg_rect* rcSrc,
		float fTexWidth,
		float fTexHeight,
		float fWidth, 
		float fHeight);
public:
	static lg_bool StartStopDrawing(
		IDirect3DDevice9* lpDevice, 
		lg_dword ViewWidth, 
		lg_dword ViewHeight, 
		lg_bool bStart);
};
#endif /* __LV_2DIMG_H__ */