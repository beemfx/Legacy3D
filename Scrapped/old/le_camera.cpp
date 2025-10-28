#include "le_camera.h"
#include <d3dx9.h>
#include "common.h"
#include <al/al.h>


CLCamera::CLCamera():
	CLEntity(),
	m_pAttachedEnt(LG_NULL),
	m_nMode(0),
	m_fDistance(1.0f)
{
	m_v3Up.x=m_v3Up.z=0.0f;
	m_v3Up.y=1.0f;
	m_v3LookAt.x=m_v3LookAt.y=m_v3LookAt.z=0.0f;
	m_v3AttachedOffset.x=m_v3AttachedOffset.y=m_v3AttachedOffset.z=0.0f;
}


CLCamera::~CLCamera()
{

}

void CLCamera::AttachToObject(CLEntity* pEntity, lg_dword mode, ML_VEC3* pv3Offset, float fDist)
{
	m_pAttachedEnt=pEntity;
	m_nMode=mode;
	m_v3AttachedOffset=*pv3Offset;
	m_fDistance=fDist;
}

void CLCamera::SetYawPitchRoll(float yaw, float pitch, float roll)
{
	m_fYaw=yaw;
	m_fPitch=pitch;
	m_fRoll=roll;
}

void CLCamera::Update()
{
	static const ML_VEC3 v3At={0.0f, 0.0f, 1.0f}, v3Up={0.0f, 1.0f, 0.0f};
	
	if(m_pAttachedEnt)
	{
		m_v3Pos=m_pAttachedEnt->m_v3Pos;
		
		m_fYaw=m_pAttachedEnt->m_fYaw;
		m_fPitch=-m_pAttachedEnt->m_fPitch;
		m_fRoll=m_pAttachedEnt->m_fRoll;
		
		switch(m_nMode)
		{
		case CLCAMERA_ATTACH_FOLLOW:
			CLEntity::Update();
			ML_Vec3Add(&m_v3LookAt, &m_v3Pos, &m_v3AttachedOffset);
			m_v3Pos.x=0.0f;
			m_v3Pos.y=0.0f;
			m_v3Pos.z=m_fDistance;
			ML_MatRotationYawPitchRoll(&m_matView, m_fYaw, m_fPitch, m_fRoll);
			ML_Vec3TransformCoord(&m_v3Pos, &m_v3Pos, &m_matView);
			ML_Vec3Subtract(&m_v3Pos, &m_v3LookAt, &m_v3Pos);
			break;
		case CLCAMERA_ATTACH_EYE:
			ML_Vec3Add(&m_v3Pos, &m_v3AttachedOffset, &m_v3Pos);
			CLEntity::Update();
			ML_Vec3TransformCoord(&m_v3LookAt, &v3At, &m_matOrient);
			ML_Vec3TransformNormal(&m_v3Up, &v3Up, &m_matOrient);
			break;
		}
		return;
	}
}

void CLCamera::Render()
{
	if(!s_pDevice)
		return;
		
	ML_MatLookAtLH(
		&m_matView,
		&m_v3Pos,
		&m_v3LookAt,
		&m_v3Up);
		
	s_pDevice->SetTransform(D3DTS_VIEW, (D3DMATRIX*)&m_matView);
	
	alListener3f(AL_POSITION, m_v3Pos.x, m_v3Pos.y, -m_v3Pos.z);
	//alListener3f(AL_VELOCITY, 0, 0, 0);
	#if 1
	ML_VEC3 v3[2]={
		m_v3LookAt.x-m_v3Pos.x, m_v3LookAt.y-m_v3Pos.y, -(m_v3LookAt.z-m_v3Pos.z),
		m_v3Up.x, m_v3Up.y, -m_v3Up.z};
	#else	
	ML_VEC3 v3[2]={
		ML_sinf(m_fYaw), 0.0f, -ML_cosf(m_fYaw),
		0, 1.0f, 0.0f};
	#endif
	alListenerfv(AL_ORIENTATION, (ALfloat*)v3);
}

void CLCamera::RenderForSkybox()
{
	if(!s_pDevice)
		return;
		
	ML_VEC3 v3Pos={0.0f, 0.0f, 0.0f};
	ML_VEC3 v3LookAt;
	ML_Vec3Subtract(&v3LookAt, &m_v3LookAt, &m_v3Pos);
	ML_MatLookAtLH(
		&m_matView,
		&v3Pos,
		&v3LookAt,
		&m_v3Up);
		
	s_pDevice->SetTransform(D3DTS_VIEW, (D3DMATRIX*)&m_matView);
}

void CLCamera::SetPosition(ML_VEC3* pVec)
{
	m_v3Pos=*pVec;
}

void CLCamera::SetLookAt(ML_VEC3* pVec)
{
	m_v3LookAt=*pVec;
}

void CLCamera::SetTransform(ML_MAT* pM)
{
	m_matView=*pM;
}

