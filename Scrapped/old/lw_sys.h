#ifndef __LW_SYS_H__
#define __LW_SYS_H__

#include "Newton/Newton.h"

#include "lf_sys2.h"
#include "lw_map.h"
#include "lw_skybox.h"
#include "le_camera.h"
#include "le_3dbase.h"
#include "lv_sys.h"
#include "lp_sys.h"
#include "lp_newton.h"
#include "lp_physx.h"

class CLWorld: private CElementD3D, CElementTimer
{
public:
static CLPhysics s_LegacyPhysics;

private:
	friend class CLGame;
	friend class CLJack;
	CLWorldMap      m_WorldMap;
	CLSkybox2       m_SkyBox;
	CLCamera        m_Camera;
	CLBase3DEntity* m_pEntities;
	
	void SetEntityLight(CLBase3DEntity* pEnt);
public:
	CLWorld();
	~CLWorld();
	
	void Initialize();
	
	lg_bool LoadMap(lf_path szFilename);
	lg_bool LoadSky(lf_path szFilename);
	
	lg_bool LoadLevel(lf_path szXMLFilename);
	
	CLBase3DEntity* AddEntity(CLBase3DEntity* pEnt);
	void RemoveEntity(CLEntity* pEnt);
	void RemoveAllEntities();
	
	void ProcessEntities();
	void Render();
	
	lg_bool Validate();
	void Invalidate();
};

#endif __LW_SYS_H__