#include "lw_skybox.h"
#include "lg_mmgr.h"


CLSkybox3::CLSkybox3()
	: m_pSkyMesh(LG_NULL)
	, m_Skin()
{

}

CLSkybox3::~CLSkybox3()
{
	Unload();
}

void CLSkybox3::Load(lg_path szMeshFile, lg_path szSkinFile)
{
	m_pSkyMesh=CLMMgr::MM_LoadMesh(szMeshFile, 0);
	m_Skin.Load(szSkinFile);
	m_Skin.MakeCompatibleWithMesh((CLMesh2*)m_pSkyMesh);
}

void CLSkybox3::Unload()
{
	m_pSkyMesh=LG_NULL;
	m_Skin.Unload();
}

void CLSkybox3::Render()
{
	if(m_pSkyMesh)
		m_pSkyMesh->Render(&m_Skin);
}


