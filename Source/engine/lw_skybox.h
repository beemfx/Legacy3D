#ifndef __LV_SKYBOX_H__
#define __LV_SKYBOX_H__

#include "lm_mesh_lg.h"
#include "lf_sys2.h"
#include "lm_skin.h"

//The skybox is simply a Legacy Mesh that gets rendered as if it
//were over the head of the camera.  Note that the CLCamera::RenderForSkybox()
//method should be called before rendering the skybox.  The static method
//of SetSkyboxRenderMode should also be called before rendering.

class CLSkybox3
{
private:
	CLMeshLG* m_pSkyMesh;
	CLSkin    m_Skin;
public:
	CLSkybox3();
	~CLSkybox3();
	void Load(lg_path szMeshFile, lg_path szSkinFile);
	void Unload();
	void Render();
};

#endif __LV_SKYBOX_H__