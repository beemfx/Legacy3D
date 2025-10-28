/* lv_test.c - Some test stuff. */
#include "common.h"
#include "lv_test.h"
#include <d3d9.h>
#include "lv_tex.h"
#include "lg_err.h"
#include <lf_sys.h>

#include "lm_d3d.h"

typedef struct _LVTVERTEX{
	float x, y, z;
	L_dword diffuse;
	float tu, tv;
	float tcu, tcv;
}LVTVERTEX;

CLegacyMeshD3D g_Mesh;



#define LVTVERTEX_TYPE (D3DFVF_XYZ|D3DFVF_DIFFUSE|D3DFVF_TEX2)

//int LVT_ValidateInvalidate(LVT_OBJ* lpObj, IDirect3DDevice9* lpDevice, L_bool bValidate);

void LVT_LoadModel(char* szFilename)
{
	g_Mesh.Create(szFilename, LG_GetGame()->v.m_lpDevice);
}

LVT_OBJ* LVT_CreateTest(IDirect3DDevice9* lpDevice)
{
	LVT_OBJ* lpObj=L_null;


	lpObj=(LVT_OBJ*)malloc(sizeof(LVT_OBJ));
	if(!lpObj)
		return 0;


	//Stuff for the test model.
	g_Mesh.Create("Models\\maxpayne\\payne_walk.l3dm", lpDevice);

	if(!LVT_ValidateInvalidate(lpObj, lpDevice, L_true))
	{
		free(lpObj);
		return 0;
	}
	return lpObj;
}

int LVT_ValidateInvalidate(LVT_OBJ* lpObj, IDirect3DDevice9* lpDevice, L_bool bValidate)
{
	if(bValidate)
	{
		// Stuff for the test wall.
		LVTVERTEX lpVerts[600];
		int x=0, y=0, i=0;
		L_result nResult=0;
		void* lpBuffer=L_null;

		//Code for test wall.
		if(!Tex_Load(
			lpDevice, 
			"textures\\graybrick.tga", 
			D3DPOOL_DEFAULT, 0x00000000, L_false, &lpObj->lpTex))
		{
			return 0;
		}
		
		if(!Tex_Load(lpDevice, "test\\spotdown01lm.tga", D3DPOOL_DEFAULT, 0x00000000, L_false, &lpObj->lpLM))
		{
			L_safe_release(lpObj->lpTex);

			return 0;
		}
		
		
		for(x=0, i=0; x<10; x++)
		{
			for(y=0; y<10; y++, i+=6)
			{
				L_bool bExtY=L_false;
				L_bool bExtX=L_false;

				float fTop=0.0f;
				float fBottom=1.0f;
				float fLeft=0.0f;
				float fRight=1.0f;

				if(!(x%2))
				{
					fLeft=0.0f;
					fRight=0.5f;
				}
				else
				{
					fLeft=0.5f;
					fRight=1.0f;
				}

				if(!(y%2))
				{
					fTop=0.0f;
					fBottom=0.5f;
				}
				else
				{
					fTop=0.5f;
					fBottom=1.0f;
				}

				lpVerts[i].x=x*10.0f-50.0f;
				lpVerts[i].z=0.0f;
				lpVerts[i].y=y*10.0f-50.0f;
				lpVerts[i].diffuse=0xFFFFFFFF;
				lpVerts[i].tu=0.0f;
				lpVerts[i].tv=1.0f;
				lpVerts[i].tcu=fLeft;
				lpVerts[i].tcv=fBottom;

				lpVerts[i+1].x=x*10.0f+10.0f-50.0f;
				lpVerts[i+1].z=0.0f;
				lpVerts[i+1].y=y*10.0f+10.0f-50.0f;
				lpVerts[i+1].diffuse=0xFFFFFFFF;
				lpVerts[i+1].tu=1.0f;
				lpVerts[i+1].tv=0.0f;
				lpVerts[i+1].tcu=fRight;
				lpVerts[i+1].tcv=fTop;;

				lpVerts[i+2].x=x*10.0f+10.0f-50.0f;
				lpVerts[i+2].z=0.0f;
				lpVerts[i+2].y=y*10.0f-50.0f;
				lpVerts[i+2].diffuse=0xFFFFFFFF;
				lpVerts[i+2].tu=1.0f;
				lpVerts[i+2].tv=1.0f;
				lpVerts[i+2].tcu=fRight;
				lpVerts[i+2].tcv=fBottom;

				memcpy(&lpVerts[i+3], &lpVerts[i+1], sizeof(lpVerts[0]));

				memcpy(&lpVerts[i+4], &lpVerts[i], sizeof(lpVerts[0]));

				lpVerts[i+5].x=x*10.0f-50.0f;
				lpVerts[i+5].z=0.0f;
				lpVerts[i+5].y=y*10.0f+10.0f-50.0f;
				lpVerts[i+5].diffuse=0x00FFFFFF;
				lpVerts[i+5].tu=0.0f;
				lpVerts[i+5].tv=0.0f;
				lpVerts[i+5].tcu=fLeft;
				lpVerts[i+5].tcv=fTop;
			}
		}

		nResult=IDirect3DDevice9_CreateVertexBuffer(
			lpDevice,
			sizeof(LVTVERTEX)*600, 
			0, 
			LVTVERTEX_TYPE, 
			D3DPOOL_MANAGED, 
			&lpObj->lpVB, 
			L_null);

		if(L_failed(nResult))
		{
			Err_PrintDX("IDirect3DDevice9::CreateVertexBuffer", nResult);
			L_safe_release(lpObj->lpLM);
			L_safe_release(lpObj->lpTex);;
			return 0;
		}

		nResult=IDirect3DVertexBuffer9_Lock(lpObj->lpVB, 0, sizeof(LVTVERTEX)*600, &lpBuffer, 0);
		if(lpBuffer)
			memcpy(lpBuffer, &lpVerts, sizeof(LVTVERTEX)*600);
		IDirect3DVertexBuffer9_Unlock(lpObj->lpVB);


		
		//The mesh stuff.
		g_Mesh.Validate();
		return 1;
	}
	else
	{
		g_Mesh.Invalidate();
		L_safe_release(lpObj->lpLM);
		L_safe_release(lpObj->lpTex);
		L_safe_release(lpObj->lpVB);
		return 1;
	}
	return 1;
}

void LVT_Render(LVT_OBJ* lpObj, IDirect3DDevice9* lpDevice)
{
	L_result nResult=0;
	if(!lpObj)
		return;

	//Test render the model.
	/*
	IDirect3DDevice9_SetFVF(lpDevice, L3DMVERTEX_TYPE);
	IDirect3DDevice9_SetStreamSource(lpDevice,0,lpObj->l3dmTest.lpVB,0,sizeof(L3DMVERTEX));
	IDirect3DDevice9_SetTexture(lpDevice,0,lpObj->l3dmTest.lpSkin);
	//For the model rendering we set it to alpha blend based on the texture,
	//in this way say we wanted a holographic model we could use a partially
	//transparent texture.
	*/
	IDirect3DDevice9_SetRenderState(lpDevice, D3DRS_ALPHABLENDENABLE, TRUE);
	IDirect3DDevice9_SetRenderState(lpDevice, D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	IDirect3DDevice9_SetRenderState(lpDevice, D3DRS_DESTBLEND, D3DBLEND_ZERO);
	//IDirect3DDevice9_DrawPrimitive(lpDevice,D3DPT_TRIANGLELIST, 0, lpObj->l3dmTest.dwTriangleCount);
	
	L_dword i=0;
	
	{
	static L_dword frame1=1, frame2=2;
	static L_dword time=timeGetTime();
	static float fMSPerFrame=30.0f;


	if((timeGetTime()-time)>fMSPerFrame)
	{
		frame1++;
		frame2++;
		//frame2=frame1+1;
		time=timeGetTime();

		//We set the frames to 2 for the max
		//payne model because the first frame
		//is the same as the last.
		frame2=frame1+1;
		if(frame1==g_Mesh.GetNumFrames())
		{
			frame2=1;
		}
		else if(frame1>g_Mesh.GetNumFrames())
		{
			frame1=1;
			frame2=frame1+1;
		}
	}
	g_Mesh.PrepareFrame(
		frame1, 
		frame2, 
		(float)(timeGetTime()-time)/fMSPerFrame);
	g_Mesh.SetupSkeleton(frame1, frame2, (float)(timeGetTime()-time)/fMSPerFrame);
	}
	D3DXMATRIX matTrans;
	D3DXMatrixIdentity(&matTrans);
	matTrans._43=250.0f;
	matTrans._43=2.5f;
	matTrans._42=-0.9f;
	lpDevice->SetTransform(D3DTS_VIEW, &matTrans);
	D3DXMatrixRotationY(&matTrans, timeGetTime()/3000.0f);
	lpDevice->SetTransform(D3DTS_WORLD, &matTrans);

	g_Mesh.Render();
	g_Mesh.RenderSkeleton();
	IDirect3DDevice9_SetFVF(lpDevice, LVTVERTEX_TYPE);
	IDirect3DDevice9_SetStreamSource(lpDevice,0,lpObj->lpVB,0,sizeof(LVTVERTEX));

	//#define RENDERTESTWALL
	#ifdef RENDERTESTWALL
	D3DXMatrixIdentity(&matTrans);
	matTrans._43=50.0f;
	matTrans._42=0.0f;
	lpDevice->SetTransform(D3DTS_VIEW, &matTrans);
	lpDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	#define MULTIPASS
	#ifdef MULTIPASS
	IDirect3DDevice9_SetTexture(lpDevice,0,lpObj->lpTex);
	IDirect3DDevice9_SetRenderState(lpDevice, D3DRS_ALPHABLENDENABLE, FALSE);
	nResult=IDirect3DDevice9_DrawPrimitive(lpDevice,D3DPT_TRIANGLELIST, 0, 200);
	IDirect3DDevice9_SetRenderState(lpDevice, D3DRS_ALPHABLENDENABLE, TRUE);
	IDirect3DDevice9_SetTexture(lpDevice,0,lpObj->lpLM);
	IDirect3DDevice9_SetTextureStageState(lpDevice, 0, D3DTSS_TEXCOORDINDEX, 1);
	IDirect3DDevice9_SetRenderState(lpDevice, D3DRS_SRCBLEND, D3DBLEND_ZERO);
	IDirect3DDevice9_SetRenderState(lpDevice, D3DRS_DESTBLEND, D3DBLEND_SRCCOLOR);
	nResult=IDirect3DDevice9_DrawPrimitive(lpDevice,D3DPT_TRIANGLELIST, 0, 200);
	IDirect3DDevice9_SetTextureStageState(lpDevice, 0, D3DTSS_TEXCOORDINDEX, 0);
	#else
	lpDevice->lpVtbl->SetTexture(lpDevice,0,lpObj->lpTex);
	lpDevice->lpVtbl->SetTexture(lpDevice,1,lpObj->lpLM);
	lpDevice->lpVtbl->SetTextureStageState(lpDevice, 0, D3DTSS_TEXCOORDINDEX, 0);
	lpDevice->lpVtbl->SetTextureStageState(lpDevice, 0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	lpDevice->lpVtbl->SetTextureStageState(lpDevice, 0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
	lpDevice->lpVtbl->SetTextureStageState(lpDevice, 1, D3DTSS_TEXCOORDINDEX, 1);
	lpDevice->lpVtbl->SetTextureStageState(lpDevice, 1, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	lpDevice->lpVtbl->SetTextureStageState(lpDevice, 1, D3DTSS_COLORARG2, D3DTA_CURRENT);
	lpDevice->lpVtbl->SetTextureStageState(lpDevice, 1, D3DTSS_COLOROP, D3DTOP_MODULATE);
	nResult=lpDevice->lpVtbl->DrawPrimitive(lpDevice,D3DPT_TRIANGLELIST, 0, 200);
	lpDevice->lpVtbl->SetTexture(lpDevice, 1, L_null);
	#endif
	#endif
	
	
	//A little test.
	if(0)
	{
	POINT p={20,20};
	IDirect3DSurface9* lpSurface=L_null, *lpBackBuffer=L_null;
	IDirect3DTexture9_GetSurfaceLevel(lpObj->lpTex, 0, &lpSurface);
	IDirect3DDevice9_SetRenderState(lpDevice, D3DRS_ALPHABLENDENABLE, TRUE);	
	IDirect3DDevice9_SetRenderState(lpDevice, D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
	IDirect3DDevice9_SetRenderState(lpDevice, D3DRS_DESTBLEND, D3DBLEND_ZERO);
	IDirect3DDevice9_GetBackBuffer(lpDevice, 0, 0, D3DBACKBUFFER_TYPE_MONO, &lpBackBuffer);
	IDirect3DDevice9_StretchRect(lpDevice, lpSurface, L_null, lpBackBuffer, L_null, D3DTEXF_POINT);

	L_safe_release(lpBackBuffer);
	L_safe_release(lpSurface);
	}

	return;
}

int LVT_Delete(LVT_OBJ* lpObj)
{
	if(!lpObj)
		return 0;
	
	LVT_ValidateInvalidate(lpObj, L_null, L_false);
	g_Mesh.Unload();
	free(lpObj);
	return 1;
}
