#include "le_3dbase.h"

#include "le_test.h"
#include "lg_err.h"
#include "lg_sys.h"
#include "ld_sys.h"
#include "lg_err_ex.h"

CLBase3DEntity::CLBase3DEntity():
	PHYSICS_ENGINE(),
	m_pMeshNodes(LG_NULL),
	m_nMeshCount(0),
	m_fScale(1.0f)
{	
	m_v3Pos.x=m_v3Pos.y=m_v3Pos.z=0.0f;
	m_v3PhysVel.x=m_v3PhysVel.x=m_v3PhysVel.z=0.0f;
	
	//The LENTITY_RENDER180 flag tells the rasterizer
	//to rotate whatever gets rendered an addition 180
	//degrees.  Most LMeshes (especially humaniods) need
	//this flag because the meshes are typically saved with
	//the front facing the negative z direction, and for
	//the legacy engine positive z is considered forward.
	m_nFlags|=LENTITY_RENDER180|LENTITY_TRANSYAWONLY;
	CLEntity::Update();
}

CLBase3DEntity::~CLBase3DEntity()
{
	LM_DeleteMeshNodes(m_pMeshNodes);
}


void CLBase3DEntity::Initialize(ML_VEC3* v3Pos)
{	
	if(!s_pWorldMap)
	{
		throw CLError(LG_ERR_DEFAULT, __FILE__, __LINE__, "Attempted to create an entity without establishing the world.");
	}
	if(v3Pos)
		m_v3Pos=*v3Pos;
		
	m_v3PhysVel.x=m_v3PhysVel.y=m_v3PhysVel.z=0.0f;
	//m_v3ExtForces.x=m_v3ExtForces.y=m_v3ExtForces.z=0.0f;
}


void CLBase3DEntity::Render()
{
	if(!m_pMeshNodes)
		return;
		
	//Should do some checking to make sure the entity should
	//actually be rendered.
	s_matTemp=m_matOrient;
	
	
	
	//To render the entity simply call Render on the base node
	//passing the orientation matrix.  All child nodes will
	//be rendered from by rendering the parent node (which should
	//be node 0).
	if(m_fScale!=1.0f)
	{
		ML_MatScaling(&s_matTemp2, m_fScale, m_fScale, m_fScale);
		ML_MatMultiply(&s_matTemp, &s_matTemp2, &m_matOrient);
	}
	
	if(L_CHECK_FLAG(m_nFlags, LENTITY_RENDER180))
	{
		ML_MatRotationY(&s_matTemp2, ML_PI);
		ML_MatMultiply(&s_matTemp, &s_matTemp2, &s_matTemp);
	}
	
	m_pMeshNodes[0].Render(&s_matTemp);
}

void CLBase3DEntity::LoadMesh(lf_path szXMLScriptFile, lg_float fScale, lg_dword Flags)
{
	m_fScale=fScale;
	
	LM_DeleteMeshNodes(m_pMeshNodes);
	m_pMeshNodes=LM_LoadMeshNodes(szXMLScriptFile, &m_nMeshCount);
	if(!m_pMeshNodes)
		return;
		
	//If we acquired a skeleton for the base mesh (m_pMeshNodes[0]),
	//then we will set a bounding box based of the first frame of
	//animation.
	if(m_pMeshNodes[0].m_pDefSkel)
	{
		m_pMeshNodes[0].m_pDefSkel->GenerateAABB(&m_aabbBase, 1, 1, 0.0f);
	}
	else if(m_pMeshNodes[0].m_pMesh)
	{
		m_aabbBase=*m_pMeshNodes[0].m_pMesh->GetBoundingBox();
	}

	ML_MAT matTrans;
	ML_MatScaling(&matTrans, m_fScale, m_fScale, m_fScale);
	ML_Vec3TransformCoordArray((ML_VEC3*)&m_aabbBase, sizeof(ML_VEC3), (ML_VEC3*)&m_aabbBase, sizeof(ML_VEC3), (ML_MAT*)&matTrans, 2);
	if(L_CHECK_FLAG(Flags, LOADMESH_BB45))
	{
		ML_MatRotationY(&matTrans, ML_PI*0.25f);
		ML_AABBTransform(&m_aabbBase, &m_aabbBase, &matTrans);
	}
	else if(L_CHECK_FLAG(Flags, LOADMESH_BB90))
	{
		ML_MatRotationY(&matTrans, ML_PI*0.5);
		ML_AABBTransform(&m_aabbBase, &m_aabbBase, &matTrans);
	}

}
