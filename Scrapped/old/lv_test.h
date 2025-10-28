#ifndef __LV_TEST_H__
#define __LV_TEST_H__
#include <d3d9.h>
#include "common.h"
#include "lg_meshmgr.h"
#include "lv_img2d.h"
#include "le_test.h"
#include "le_camera.h"
#include "lv_skybox.h"
#include "lw_map.h"



class CLVTObj
{
private:
	IDirect3DDevice9* m_pDevice;	
	CLImg2D m_Img;	
	CLCamera     m_Camera;
	CLSkybox2     m_SkyBox;
	CLWorldMap m_World;
	lg_dword m_nNumEnts;
	CLBase3DEntity* m_pEnts[3];
public:
	CLVTObj():m_pDevice(LG_NULL){}
	void Init(IDirect3DDevice9* pDevice, void* pGame);
	void UnInit();
	void Render();
	void ValidateInvalidate(lg_bool bValidate);
	//void ValidateTestWall();
	void RenderMeshes();
	//void RenderTestWall();
};

#endif __LV_TEST_H__