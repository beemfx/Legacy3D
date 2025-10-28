#ifndef __LM_SKEL_EDIT_H__
#define __LM_SKEL_EDIT_H__
//The skeleton class for LMEdit.exe
#include "lm_skel.h"
#include <d3d9.h>

class CLSkelEdit:public CLSkel2
{
friend class CLSkelEdit;
private:
	struct JointVertex{
		lg_float x, y, z;
		lg_float psize;
		lg_dword color;
	};
	static const lg_dword JOINT_VERTEX_FORMAT=D3DFVF_XYZ|D3DFVF_PSIZE|D3DFVF_DIFFUSE;
	
	struct BoneVertex{
		lg_float x, y, z;
		lg_dword color;
	};
	static const lg_dword BONE_VERTEX_FORMAT=D3DFVF_XYZ|D3DFVF_DIFFUSE;
	
	static const lg_dword LM_FLAG_D3D_VALID=0x00010000;
public:
	CLSkelEdit();
	~CLSkelEdit();
	lg_bool SetAnims(lg_dword nCount, SkelAnim* pAnims);
	lg_bool ImportFrames(CLSkelEdit* pSkel);
	lg_bool CalculateAABBs(float fExtent);
	
	virtual lg_bool Load(LMPath szFile);
	virtual lg_bool Save(LMPath szFile);
	virtual lg_void Unload();
	
	lg_void Render();
	lg_void RenderAABB();
	lg_void SetupFrame(lg_dword nAnim, lg_float fTime);
	lg_void SetupFrame(lg_dword nFrame1, lg_dword nFrame2, lg_float fTime);
private:
	D3DCOLOR m_nColor;
	JointVertex* m_pSkel;
	BoneVertex   m_vAABB[16];
	ml_aabb      m_aabb;
};

#endif __LM_SKEL_EDIT_H__