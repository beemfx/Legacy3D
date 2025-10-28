/* lv_test.c - Some test stuff. */
#include "common.h"
#include "lv_test.h"
#include <d3d9.h>
#include <d3dx9.h>
#include "lv_tex.h"
#include "lg_err.h"
//#include <lf_sys.h>

#include "lm_d3d.h"
#include "ML_lib.h"
#include "lg_meshmgr.h"

#include "lv_img2d.h"

#include "lg_sys.h"


void CLVTObj::Init(IDirect3DDevice9* pDevice, void* pGame)
{
	m_Camera.SetDevice(pDevice);
	CLGame* pGame1=(CLGame*)pGame;
	
	m_pDevice=pDevice;
	m_pDevice->AddRef();
	
	m_Img.CreateFromFile(m_pDevice,"/dbase/textures/misc/test_face.tga", LG_NULL, 128, 128, 0x00000000);
	
	ML_VEC3 v3Pos={2.0f, 0.0f, -2.0f};
	this->m_pEnts[0]=new CLJack();
	m_pEnts[0]->Initialize(pGame, &v3Pos);
	//m_Player.Initialize(pGame, &v3Pos);
	v3Pos.x=20.0f;
	v3Pos.z=1.2f;
	v3Pos.y=0.1f;
	m_pEnts[1]=new CLMonaEnt();
	m_pEnts[1]->Initialize(pGame, &v3Pos);
	//m_Mona.Initialize(pGame, &v3Pos);
	v3Pos.x=-10.0f;
	v3Pos.z=0.0f;
	m_pEnts[2]=new CLBarrelEnt();
	m_pEnts[2]->Initialize(pGame, &v3Pos);
	//m_Barrel.Initialize(pGame, &v3Pos);
	ML_VEC3 offset={0.0f, 1.5f, 0.0f};
	//ML_VEC3 offset={0.0f, 1.5f*30.0f, 0.0f};
	//m_Camera.AttachToObject(&m_Player, CLCamera::CLCAMERA_ATTACH_FOLLOW, &offset, 2.0f);
	m_Camera.AttachToObject(
		m_pEnts[0], 
		CLCamera::CLCAMERA_ATTACH_FOLLOW, 
		//CLCamera::CLCAMERA_ATTACH_EYE, 
		&offset, 2.0f);//*30.0f);
	m_nNumEnts=3;
	ValidateInvalidate(LG_TRUE);
	
	//m_SkyBox.Initialize(m_pDevice);
	//m_SkyBox.Load("textures\\set 16\\samp%d.bmp", LG_TRUE);
	m_SkyBox.Load("/dbase/meshes/skybox/skybox.lmsh");
	
	//lg_char szMap[]="/dbase/maps/solids_test.3dw";
	//lg_char szMap[]="/dbase/maps/tri_test.3dw";
	//lg_char szMap[]="/dbase/maps/thing.3dw";
	lg_char szMap[]="/dbase/maps/room_test.3dw";
	//lg_char szMap[]="/dbase/maps/block.3dw";
	//lg_char szMap[]="/dbase/maps/house.3dw";
	//lg_char szMap[]="/dbase/maps/simple_example.3dw";
	
	m_World.LoadFromWS(szMap, m_pDevice);
}


void CLVTObj::Render()
{
	lg_result nResult=0;
	if(!m_pDevice)
		return;
	/*	
	D3DVIEWPORT9 vp, oldvp;
	
	vp.X=10;
	vp.Y=10;
	vp.Width=640;
	vp.Height=480;
	vp.MinZ=0.0f;
	vp.MaxZ=1.0f;
	m_pDevice->GetViewport(&oldvp);
	m_pDevice->SetViewport(&vp);
	*/
	//First things first is to process all the entitie's AI (processing
	//AI doesn't move the object, but it generates a proposoed movement
	//vector stored int m_v3Vel.
	lg_dword i, j;
	for(i=0; i<m_nNumEnts; i++)
	{
		m_pEnts[i]->ProcessAI();
		m_pEnts[i]->WorldCollision(&m_World);
	}
	
	
	//Now check to see if the proposed movements will cause any collisions
	//to occur.
	for(i=0; i<m_nNumEnts; i++)
	{
		for(j=0; j<m_nNumEnts; j++)
		{
			//Don't bother checking with itself.
			if(i==j)
				continue;
				
			if(m_pEnts[i]->Collision(m_pEnts[j]))
			{
				;//j=0;
			}
		}
	}

	for(i=0; i<m_nNumEnts; i++)
		m_pEnts[i]->Update();

	//The camera should be the last thing updated.
	m_Camera.Update();
	
	//The camera should be the first thing rendered.
	m_Camera.RenderForSkybox();
	CLSkybox2::SetSkyboxRenderMode(m_pDevice);
	m_SkyBox.Render();	
	m_Camera.Render();
	//The level should be the second thing rendered.
	m_pDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
	m_pDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);
	
	//RenderTestWall();
	
	m_pDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
	
	//m_Map.Render();
	
	m_World.Render();
	
	//Then all the entities should be rendered
	//note that in the call to Update for
	//each entity they should be flagged whether or
	//not they are visible (in a room that is visible)
	//and in the call to render they should be checked
	//to see if they are in the view frustrum.
	CLMeshD3D::SetLMeshRenderStates(m_pDevice);
	m_pDevice->SetRenderState(D3DRS_LIGHTING, TRUE);
	for(i=0; i<m_nNumEnts; i++)
		m_pEnts[i]->Render();
	
	//Any 2D stuff should be rendered last, the 2d drawing
	//code is not very good right now.
	CLImg2D::StartStopDrawing(m_pDevice, 640, 480, LG_TRUE);
	m_Img.Render(10, 10);
	CLImg2D::StartStopDrawing(m_pDevice, 640, 480, LG_FALSE);
	
	//m_pDevice->SetViewport(&oldvp);
}
void CLVTObj::UnInit()
{
	for(lg_dword i=0; i<m_nNumEnts; i++)
		delete m_pEnts[i];
	m_Img.Delete();
	m_SkyBox.Unload();
	m_World.Unload();
	ValidateInvalidate(LG_FALSE);
	m_pDevice->Release();
	m_pDevice=NULL;
}
void CLVTObj::ValidateInvalidate(lg_bool bValidate)
{
	if(bValidate)
	{
		m_Img.Validate(LG_NULL);
		m_World.Validate();
	}
	else
	{
		m_Img.Invalidate();
		m_World.Invalidate();
	}
}
