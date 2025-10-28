#ifndef __LE_CAMERA_H__
#define __LE_CAMERA_H__

#include <d3d9.h>
#include "le_sys.h"
#include "lv_sys.h"

class CLCamera: public CLEntity, CElementD3D
{
private:
	ML_VEC3 m_v3Up;
	ML_VEC3 m_v3LookAt;
	ML_MAT m_matView;
	
	CLEntity* m_pAttachedEnt;
	ML_VEC3   m_v3AttachedOffset;
	float     m_fDistance;
	lg_dword   m_nMode;
public:
	CLCamera();
	~CLCamera();

	void SetPosition(ML_VEC3* pVec);
	void SetLookAt(ML_VEC3* pVec);
	void SetUp(ML_VEC3* pVec);
	void SetTransform(ML_MAT* pM);
	void SetYawPitchRoll(float yaw, float pitch, float roll);
	virtual void Update();
	virtual void Render();
	void RenderForSkybox();
	
	void AttachToObject(CLEntity* pEntity, lg_dword mode, ML_VEC3* pv3Offset, float fDist);
	
	static const lg_dword CLCAMERA_ATTACH_FOLLOW=0x01000001;
	static const lg_dword CLCAMERA_ATTACH_EYE=0x01000002;
};

#endif __LE_CAMERA_H__