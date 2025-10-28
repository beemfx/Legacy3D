#include "lm_d3d.h"
#include "ML_lib.h"
#include <stdio.h>
#include "lg_func.h"



/**************************************************************
	The Renderable Skeleton Code...
	(Really only used in LMEdit and not the Legacy Engine)
**************************************************************/
CLSkelD3D::CLSkelD3D():
CLSkel(),
m_pDevice(LG_NULL),
m_bD3DValid(LG_FALSE),
m_nColor(0xFFFF0000),
m_pSkel(LG_NULL)
{

}

CLSkelD3D::~CLSkelD3D()
{
	Unload();
}

lg_bool CLSkelD3D::CreateD3DComponents(IDirect3DDevice9* lpDevice)
{
	if(!m_bLoaded || !lpDevice)
		return LG_FALSE;

	L_safe_release(m_pDevice);
	m_pDevice=lpDevice;	
	m_pDevice->AddRef();
	
	m_pSkel=new JOINTVERTEX[m_nNumJoints];
	if(!m_pSkel)
	{
		m_pDevice->Release();
		return LG_FALSE;
	}
	
	//We need the animation matrices to be the number
	//of joints*2 so we can store the matrices for
	//the first and second frame.
	if(!Validate())
	{
		L_safe_delete_array(m_pSkel);
		L_safe_release(m_pDevice);
		return LG_FALSE;
	}
	PrepareFrame(0, 0, 0.0f);
	return LG_TRUE;
}

lg_bool CLSkelD3D::Validate()
{
	m_bD3DValid=LG_TRUE;
	return m_bD3DValid;
}

lg_bool CLSkelD3D::Invalidate()
{
	m_bD3DValid=LG_FALSE;
	return LG_TRUE;
}

lg_bool CLSkelD3D::Unload()
{
	if(!m_bLoaded)
		return LG_TRUE;
	lg_dword i=0;
	Invalidate();
	L_safe_delete_array(m_pSkel);
	L_safe_release(m_pDevice);

	this->CLSkel::Unload();
	
	m_pDevice=LG_NULL;
	m_bD3DValid=LG_FALSE;
	m_pSkel=LG_NULL;
	return LG_TRUE;
}

lg_bool CLSkelD3D::PrepareFrame(lg_dword nAnim, float fTime)
{
	// The range could easily be changed by modifying this
	//value, 100.0f seemed to be a good range to me.
	#define PREP_MAX_RANGE 100.0f
	
	if(m_nNumAnims<1)
		return LG_FALSE;
	
	nAnim=LG_Clamp(nAnim, 0, m_nNumAnims-1);
	fTime=LG_Clamp(fTime, 0.0f, PREP_MAX_RANGE);
	
	float fFrame;
	lg_dword nFrame1=0, nFrame2=0;
	//Get the animation we are dealing with.
	const LSKEL_ANIM* pAnim=GetAnim(nAnim);
	if(L_CHECK_FLAG(pAnim->m_nFlags, LSKEL_ANIM_LOOPBACK))
	{
		if(fTime>=50.0f)
			fTime=100.0f-fTime;
			
		fFrame=pAnim->m_nFirstFrame+((float)pAnim->m_nNumFrames-1-0.000001f)*fTime/(PREP_MAX_RANGE*0.5f);
	}
	else
	{
		fFrame=pAnim->m_nFirstFrame+((float)pAnim->m_nNumFrames-0.000001f)*fTime/PREP_MAX_RANGE;
	}
	nFrame1=(lg_dword)fFrame;
	nFrame2=nFrame1>=(pAnim->m_nFirstFrame+pAnim->m_nNumFrames-1)?pAnim->m_nFirstFrame:nFrame1+1;

	//To get the interpolation value we need only find the fraction portion of fFrame fFrame-nFrame1.
	return PrepareFrame(nFrame1, nFrame2, fFrame-nFrame1);
}

lg_bool CLSkelD3D::PrepareFrame(lg_dword nFrame1, lg_dword nFrame2, float t)
{
	lg_dword i=0;
	
	if(!m_bLoaded || !m_bD3DValid)
		return LG_FALSE;

	lg_dword nNumJoints=this->m_nNumJoints;

	for(i=0; i<nNumJoints; i++)
	{
		ML_MAT Trans;
		
		GenerateJointTransform(&Trans, i, nFrame1, nFrame2, t);
		
		ML_MatMultiply(
			&Trans,
			GetBaseTransform(i),
			&Trans);
		//To find the final location of a joint we need to (1) find the
		//relative location of the joint. (2) multiply that by all of
		//it parent/grandparent's/etc transformation matrices.
		//(1)
		m_pSkel[i].x=0.0f;
		m_pSkel[i].y=0.0f;
		m_pSkel[i].z=0.0f;
		m_pSkel[i].psize=4.0f;
		m_pSkel[i].color=m_nColor;
		//(2)
		ML_Vec3TransformCoord((ML_VEC3*)&m_pSkel[i].x, (ML_VEC3*)&m_pSkel[i].x, &Trans);//pJointTrans[i]);		
	}
	
	//ML_AABB aabb;
	GenerateAABB(&m_aabb, nFrame1, nFrame2, t);

	return LG_TRUE;
}

lg_bool CLSkelD3D::Render()
{
	//We now want to render the skelton for testing purposes.
	lg_dword i=0;
	//If there are no joints just return true cause there is nothing
	//to render.
	if(!m_nNumJoints)
		return LG_TRUE;
	BONEVERTEX   Line[2];

	//We want to render the bones first, this
	//is simply a line from the bone to it's parent.
	//If it has no parent nothing gets rendered.
	m_pDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
	m_pDevice->SetTexture(0, LG_NULL);
	m_pDevice->SetFVF(BONEVERTEX_FVF);
	for(i=0; i<GetNumJoints(); i++)
	{
		memcpy(&Line[0].x, &m_pSkel[i].x, 12);
		Line[0].color=m_pSkel[i].color;
		lg_dword nBone=GetParentBoneRef(i);
		if(nBone)
		{
			memcpy(&Line[1].x, &m_pSkel[nBone-1].x, 12);
			Line[1].color=m_pSkel[nBone-1].color;
			m_pDevice->DrawPrimitiveUP(D3DPT_LINELIST, 1, &Line, sizeof(BONEVERTEX));
		}
	}
	
	//We can render all the joints at once since they are just points.
	m_pDevice->SetFVF(JOINTVERTEX_FVF);
	m_pDevice->DrawPrimitiveUP(D3DPT_POINTLIST, GetNumJoints(), m_pSkel, sizeof(JOINTVERTEX));
	
	//Render the AABB.
	m_pDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
	RenderAABB();
	return LG_TRUE;
}

lg_bool CLSkelD3D::RenderAABB()
{
	//ML_AABB aabb;
	//GenerateAABB(&aabb, nFrame1, nFrame2, t);
	ML_MAT matTmp;
	m_pDevice->GetTransform(D3DTS_WORLD, (D3DMATRIX*)&matTmp);
	//ML_AABB aabb=m_aabb;
	ML_AABB aabbTrans;
	ML_AABBTransform(&aabbTrans, &m_aabb, &matTmp);
	
	ML_MAT matIdent;
	ML_MatIdentity(&matIdent);
	
	for(lg_dword i=0; i<16; i++)
	{
		m_vAABB[i].color=0xFFFFFF90;
	}
	
	ML_VEC3 vTmp;
	memcpy(&m_vAABB[0], ML_AABBCorner(&vTmp, &aabbTrans, ML_AABB_BLF), 12);
	memcpy(&m_vAABB[1], ML_AABBCorner(&vTmp, &aabbTrans, ML_AABB_TLF), 12);
	m_vAABB[2].x=aabbTrans.v3Min.x;
	m_vAABB[2].y=aabbTrans.v3Max.y;
	m_vAABB[2].z=aabbTrans.v3Max.z;

	m_vAABB[3].x=aabbTrans.v3Max.x;
	m_vAABB[3].y=aabbTrans.v3Max.y;
	m_vAABB[3].z=aabbTrans.v3Max.z;

	m_vAABB[4].x=aabbTrans.v3Max.x;
	m_vAABB[4].y=aabbTrans.v3Max.y;
	m_vAABB[4].z=aabbTrans.v3Min.z;

	m_vAABB[5].x=aabbTrans.v3Max.x;
	m_vAABB[5].y=aabbTrans.v3Min.y;
	m_vAABB[5].z=aabbTrans.v3Min.z;

	m_vAABB[6].x=aabbTrans.v3Max.x;
	m_vAABB[6].y=aabbTrans.v3Min.y;
	m_vAABB[6].z=aabbTrans.v3Max.z;

	m_vAABB[7].x=aabbTrans.v3Min.x;
	m_vAABB[7].y=aabbTrans.v3Min.y;
	m_vAABB[7].z=aabbTrans.v3Max.z;

	m_vAABB[8].x=aabbTrans.v3Min.x;
	m_vAABB[8].y=aabbTrans.v3Min.y;
	m_vAABB[8].z=aabbTrans.v3Min.z;

	m_vAABB[9].x=aabbTrans.v3Max.x;
	m_vAABB[9].y=aabbTrans.v3Min.y;
	m_vAABB[9].z=aabbTrans.v3Min.z;

	m_vAABB[10].x=aabbTrans.v3Max.x;
	m_vAABB[10].y=aabbTrans.v3Min.y;
	m_vAABB[10].z=aabbTrans.v3Max.z;

	m_vAABB[11].x=aabbTrans.v3Max.x;
	m_vAABB[11].y=aabbTrans.v3Max.y;
	m_vAABB[11].z=aabbTrans.v3Max.z;

	m_vAABB[12].x=aabbTrans.v3Max.x;
	m_vAABB[12].y=aabbTrans.v3Max.y;
	m_vAABB[12].z=aabbTrans.v3Min.z;

	m_vAABB[13].x=aabbTrans.v3Min.x;
	m_vAABB[13].y=aabbTrans.v3Max.y;
	m_vAABB[13].z=aabbTrans.v3Min.z;

	m_vAABB[14].x=aabbTrans.v3Min.x;
	m_vAABB[14].y=aabbTrans.v3Max.y;
	m_vAABB[14].z=aabbTrans.v3Max.z;

	m_vAABB[15].x=aabbTrans.v3Min.x;
	m_vAABB[15].y=aabbTrans.v3Min.y;
	m_vAABB[15].z=aabbTrans.v3Max.z;
	
	for(lg_dword i=0; i<16; i++)
	{
		if(m_vAABB[i].z==aabbTrans.v3Max.z)
			m_vAABB[i].color=0xFF870178;
		else
			m_vAABB[i].color=0xffff7df1;
	}
	
	//Render the AABB.
	m_pDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX*)&matIdent);
	m_pDevice->SetFVF(BONEVERTEX_FVF);
	m_pDevice->DrawPrimitiveUP(D3DPT_LINESTRIP, 15, &m_vAABB, sizeof(BONEVERTEX)); 
	m_pDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX*)&matTmp);
	/*
	BONEVERTEX Test[16];
	ML_VEC3 Corners[8];
	ML_AABB aabbTemp;
	for(lg_dword i=0; i<8; i++)
	{
		ML_AABBCorner(&Corners[i], &m_aabb, (ML_AABB_CORNER)i);
		ML_Vec3TransformCoord(&Corners[i], &Corners[i], &matTmp);
	}
	
	ML_AABBFromVec3s(&aabbTemp, Corners, 8);
	
	for(lg_dword i=0; i<16; i++)
	{
		Test[i].color=0xFFFFFF00;
	}
	
	memcpy(&Test[0], ML_AABBCorner(&vTmp, &aabbTemp, ML_AABB_BLF), 12);
	memcpy(&Test[1], ML_AABBCorner(&vTmp, &aabbTemp, ML_AABB_TLF), 12);
	Test[2].x=aabbTemp.v3Min.x;
	Test[2].y=aabbTemp.v3Max.y;
	Test[2].z=aabbTemp.v3Max.z;

	Test[3].x=aabbTemp.v3Max.x;
	Test[3].y=aabbTemp.v3Max.y;
	Test[3].z=aabbTemp.v3Max.z;

	Test[4].x=aabbTemp.v3Max.x;
	Test[4].y=aabbTemp.v3Max.y;
	Test[4].z=aabbTemp.v3Min.z;

	Test[5].x=aabbTemp.v3Max.x;
	Test[5].y=aabbTemp.v3Min.y;
	Test[5].z=aabbTemp.v3Min.z;

	Test[6].x=aabbTemp.v3Max.x;
	Test[6].y=aabbTemp.v3Min.y;
	Test[6].z=aabbTemp.v3Max.z;

	Test[7].x=aabbTemp.v3Min.x;
	Test[7].y=aabbTemp.v3Min.y;
	Test[7].z=aabbTemp.v3Max.z;

	Test[8].x=aabbTemp.v3Min.x;
	Test[8].y=aabbTemp.v3Min.y;
	Test[8].z=aabbTemp.v3Min.z;

	Test[9].x=aabbTemp.v3Max.x;
	Test[9].y=aabbTemp.v3Min.y;
	Test[9].z=aabbTemp.v3Min.z;

	Test[10].x=aabbTemp.v3Max.x;
	Test[10].y=aabbTemp.v3Min.y;
	Test[10].z=aabbTemp.v3Max.z;

	Test[11].x=aabbTemp.v3Max.x;
	Test[11].y=aabbTemp.v3Max.y;
	Test[11].z=aabbTemp.v3Max.z;

	Test[12].x=aabbTemp.v3Max.x;
	Test[12].y=aabbTemp.v3Max.y;
	Test[12].z=aabbTemp.v3Min.z;

	Test[13].x=aabbTemp.v3Min.x;
	Test[13].y=aabbTemp.v3Max.y;
	Test[13].z=aabbTemp.v3Min.z;

	Test[14].x=aabbTemp.v3Min.x;
	Test[14].y=aabbTemp.v3Max.y;
	Test[14].z=aabbTemp.v3Max.z;

	Test[15].x=aabbTemp.v3Min.x;
	Test[15].y=aabbTemp.v3Min.y;
	Test[15].z=aabbTemp.v3Max.z;
	
	
	//Render the AABB.
	m_pDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX*)&matIdent);
	m_pDevice->SetFVF(BONEVERTEX_FVF);
	m_pDevice->DrawPrimitiveUP(D3DPT_LINESTRIP, 15, &Test, sizeof(BONEVERTEX)); 
	m_pDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX*)&matTmp);
	*/
	return LG_TRUE;
}



