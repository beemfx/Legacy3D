#ifndef __LE_3DBASE_H__
#define __LE_3DBASE_H__

#include "le_sys.h"
#include "lm_d3d.h"
//#include "li_sys.h"
//#include "lt_sys.h"
#include "lg_meshmgr.h"
#include "lw_map.h"

#define LOADMESH_BB90 0x00000001
#define LOADMESH_BB45 0x00000002

//#define PHYSICS_ENGINE CLPhysXEntity
//#define PHYSICS_ENGINE CLNewtonEntity
#define PHYSICS_ENGINE CLEntity


class CLBase3DEntity: public PHYSICS_ENGINE
{
friend class CLBase3DEntity;
protected:	
	CLSkel*    m_pSkel;
	
	CLMeshNode* m_pMeshNodes;
	lg_dword    m_nMeshCount;
	lg_float    m_fScale;
	
public:
	CLBase3DEntity();
	~CLBase3DEntity();
	virtual void Initialize(ML_VEC3* v3Pos);
	
	virtual void Render();
	
protected:
	void LoadMesh(lf_path szXMLScriptFile, lg_float fScale, lg_dword Flags);
};

#endif __LE_3DBASE_H__