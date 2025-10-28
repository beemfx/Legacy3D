#include "stdafx.h"
#include "lm_skel_edit.h"

CLSkelEdit::CLSkelEdit()
: CLSkel2()
, m_nColor(0xFFFF0000)
, m_pSkel(LG_NULL)
{
}

CLSkelEdit::~CLSkelEdit()
{
	Unload();
}

lg_bool CLSkelEdit::SetAnims(lg_dword nCount, SkelAnim* pAnims)
{
	LG_SafeDeleteArray(m_pAnims);
	m_nNumAnims=0;
	
	m_nNumAnims=nCount;
	m_pAnims=new SkelAnim[m_nNumAnims];
	if(!m_pAnims)
	{
		m_nNumAnims=0;
		return LG_FALSE;
	}
	lg_dword i=0;
	for(i=0; i<nCount; i++)
	{
		m_pAnims[i]=pAnims[i];
	}
	return LG_TRUE;
}

//CalculateAABBs simply finds the minimum and maximum
//joint locations for each frame, the fExtent perameter
//will extend the AABB that far (to account for the
//fact that models are typically a little bigger than the
//skeleton.
lg_bool CLSkelEdit::CalculateAABBs(float fExtent)
{
	//To calculate the AABBs we need to calculate the extremes
	//of each frame.
	lg_dword i,j;
	
	for(i=0; i<m_nNumKeyFrames; i++)
	{
		SetupFrame(i+1, i+1, 0.0f);
		m_pFrames[i].aabbBox.v3Min.x=m_pSkel[0].x;
		m_pFrames[i].aabbBox.v3Min.y=m_pSkel[0].y;
		m_pFrames[i].aabbBox.v3Min.z=m_pSkel[0].z;
		
		m_pFrames[i].aabbBox.v3Max.x=m_pSkel[0].x;
		m_pFrames[i].aabbBox.v3Max.y=m_pSkel[0].y;
		m_pFrames[i].aabbBox.v3Max.z=m_pSkel[0].z;
		for(j=0; j<m_nNumJoints; j++)
		{
			m_pFrames[i].aabbBox.v3Min.x=LG_Min(m_pSkel[j].x, m_pFrames[i].aabbBox.v3Min.x);
			m_pFrames[i].aabbBox.v3Min.y=LG_Min(m_pSkel[j].y, m_pFrames[i].aabbBox.v3Min.y);
			m_pFrames[i].aabbBox.v3Min.z=LG_Min(m_pSkel[j].z, m_pFrames[i].aabbBox.v3Min.z);
			
			m_pFrames[i].aabbBox.v3Max.x=LG_Max(m_pSkel[j].x, m_pFrames[i].aabbBox.v3Max.x);
			m_pFrames[i].aabbBox.v3Max.y=LG_Max(m_pSkel[j].y, m_pFrames[i].aabbBox.v3Max.y);
			m_pFrames[i].aabbBox.v3Max.z=LG_Max(m_pSkel[j].z, m_pFrames[i].aabbBox.v3Max.z);
		}
		
		//Adjust the AABB by the extent.
		m_pFrames[i].aabbBox.v3Min.x-=fExtent;
		m_pFrames[i].aabbBox.v3Min.y-=fExtent;
		m_pFrames[i].aabbBox.v3Min.z-=fExtent;
		
		m_pFrames[i].aabbBox.v3Max.x+=fExtent;
		m_pFrames[i].aabbBox.v3Max.y+=fExtent;
		m_pFrames[i].aabbBox.v3Max.z+=fExtent;
	}
	return LG_TRUE;
}

lg_bool CLSkelEdit::ImportFrames(CLSkelEdit* pSkel)
{
	CString szTemp;
	lg_dword i=0, j=0, nNumJoints=0;
	//Start by allocating enough memory to temporarility store joint information.
	
	#ifdef IMPORT_ENTIRESKEL
	//Generate a list of names with each of the bones
	LMESH_NAME szList1[1024]; //List of all the joints.
	lg_dword nIndex[1024]; //Holds a list of joint reference to any imported joints (either points to an old joint, or points to a new one).
	
	
	for(i=0; i<m_nNumJoints; i++)
	{
		L_strncpy(szList1[i], m_pBaseSkeleton[i].szName, LMESH_MAX_NAME_LENGTH);
	}
	
	//Generate a list with all the unique joints.
	for(i=0, nNumJoints=this->m_nNumJoints; i<pSkel->m_nNumJoints; i++)
	{
		lg_bool bFound=LG_FALSE;
		for(j=0; j<this->m_nNumJoints; j++)
		{
			if(L_strnicmp(szList1[j], pSkel->m_pBaseSkeleton[i].szName, 0))
			{
				bFound=LG_TRUE;
				nIndex[i]=j;
				break;
			}
		}
		if(!bFound)
		{
			L_strncpy(szList1[nNumJoints], pSkel->m_pBaseSkeleton[i].szName, LMESH_MAX_NAME_LENGTH);
			nIndex[i]=nNumJoints;
			nNumJoints++;
		}
	}
	
	szTemp="The joints:\n";
	for(i=0; i<nNumJoints; i++)
	{
		szTemp.AppendFormat("%s, ", szList1[i]);
	}
	AfxMessageBox(szTemp);
	//Allocate some memory for the joints.
	
	LMESH_JOINTEX* pNewSkel=new LMESH_JOINTEX[nNumJoints];
	if(!pNewSkel)
		return LG_FALSE;
		
	/*
	for(i=0; i<nNumJoints; i++)
	{
		memset(&pNewSkel[i], 0, sizeof(LMESH_JOINTEX));
		szTemp.Format("Bone %i", i);
		L_strncpy(pNewSkel[i].szName, szTemp, LMESH_MAX_NAME_LENGTH);
		pNewSkel[i].szParentBone[0]=0;
		pNewSkel[i].nParentBone=0;
		pNewSkel[i].position[0]=0.0f;
		pNewSkel[i].position[1]=0.0f;
		pNewSkel[i].position[2]=0.0f;
		pNewSkel[i].rotation[0]=0.0f;
		pNewSkel[i].rotation[1]=0.0f;
		pNewSkel[i].rotation[2]=0.0f;
		
		if(i<m_nNumJoints)
			pNewSkel[i]=m_pBaseSkeleton[i];
	}
	*/
	
	//Copy original skeleton stuff:
	for(i=0; i<nNumJoints; i++)
	{
		//Find the bone with the same name.
		lg_bool bFound=LG_FALSE;
		pNewSkel[i]=m_pBaseSkeleton[0];
		for(j=0; j<m_nNumJoints; j++)
		{
			if(L_strnicmp(szList1[i], m_pBaseSkeleton[j].szName, 0))
			{
				bFound=LG_TRUE;
				pNewSkel[i]=m_pBaseSkeleton[j];
				break;
			}
		}
		
		if(!bFound)
		{
			for(j=0; j<pSkel->m_nNumJoints; j++)
			{
				if(L_strnicmp(szList1[i+m_nNumJoints], pSkel->m_pBaseSkeleton[j].szName, 0))
				{
					bFound=LG_TRUE;
					pNewSkel[i]=pSkel->m_pBaseSkeleton[j];
					pNewSkel[i].nJointRef=i;
					break;
				}
			}
		}
	}
	
	//Now make sure all the parent bone refs are correct.
	for(i=0; i<nNumJoints; i++)
	{
		for(j=0; j<nNumJoints; j++)
		{
			
			if(L_strnicmp(pNewSkel[i].szParentBone, pNewSkel[j].szName, 0))
			{
				pNewSkel[i].nParentBone=j+1;
			}
		}
		
	}
	
	m_nNumJoints=nNumJoints;
	delete[]m_pBaseSkeleton;
	m_pBaseSkeleton=pNewSkel;
	#endif IMPORT_ENTIRESKEL
	//Copy the new joints
	//Allocate some memory for those frames
	
	//The process of importing frames, means that we import any
	//keyframe information for any joints that have the same
	//name, probably should check to make sure that there
	//is at least one matching frame before importing.
	lg_dword nKeyFrames=m_nNumKeyFrames+pSkel->GetNumKeyFrames();
	SkelFrame* pTempPositions=new SkelFrame[nKeyFrames];
	if(!pTempPositions)
		return LG_FALSE;
		
	for(i=0; i<nKeyFrames; i++)
		pTempPositions[i].Initialize(m_nNumJoints);
		
	//Copy the original positions:
	for(i=0; i<m_nNumJoints; i++)
	{
		for(j=0; j<nKeyFrames; j++)
		{
			if(j<m_nNumKeyFrames)
			{
				pTempPositions[j].LocalPos[i]=m_pFrames[j].LocalPos[i];
			}
			else
			{
				//Set non original frames to identity.
				memset(&pTempPositions[j].LocalPos[i], 0, sizeof(SkelJointPos));
			}
		}
	}
	//Copy the original AABBs
	for(i=0; i<m_nNumKeyFrames; i++)
	{
		pTempPositions[i].aabbBox=m_pFrames[i].aabbBox;
	}
	//Copy the imported AABBs
	for(i=0; i<pSkel->GetNumFrames(); i++)
	{
		pTempPositions[i+m_nNumKeyFrames].aabbBox=pSkel->m_pFrames[i].aabbBox;		
	}
	
	//For the skeleton we are importing we find out if there are
	//compatible bones and for each compatible bone we copy the
	//keyframe information.
	for(i=0; i<pSkel->GetNumJoints(); i++)
	{
		//Compare the joint with each of the frames.
		for(j=0; j<m_nNumJoints; j++)
		{
			if(stricmp(pSkel->m_pBaseSkeleton[i].szName, m_pBaseSkeleton[j].szName)==0)
			{
				//We found a compatible bone...
				//Now copy in the animation stuff.
				lg_dword nCmpFrame=j;
				for(j=0; j<pSkel->GetNumFrames(); j++)
				{
					pTempPositions[j+m_nNumKeyFrames].LocalPos[nCmpFrame]=pSkel->m_pFrames[j].LocalPos[nCmpFrame];
				}
				break;
			}
		}
	}
	
	lg_dword nOffset=m_nNumKeyFrames;
	m_nNumKeyFrames=nKeyFrames;
	delete[]m_pFrames;
	m_pFrames=pTempPositions;
	
	//We also want to import the animation information,
	//we need to set off the start frame.
	lg_dword nNumAnims=m_nNumAnims+pSkel->m_nNumAnims;
	
	SkelAnim* pAnims=new SkelAnim[nNumAnims];
	if(!pAnims)
	{
		AfxMessageBox("Could not import animation information!");
		CalcExData();
		return LG_TRUE;
	}
	
	//Copy the original anims, and copy the new anims;
	for(i=0; i<nNumAnims; i++)
	{
		if(i<m_nNumAnims)
			pAnims[i]=m_pAnims[i];
		else
		{
			pAnims[i]=pSkel->m_pAnims[i-m_nNumAnims];
			pAnims[i].nFirstFrame+=nOffset;
		}
	}
	
	m_nNumAnims=nNumAnims;
	delete[]m_pAnims;
	m_pAnims=pAnims;
	
	//Since the skeleton has significantly changed we need to
	//calculate the extra data again.
	CalcExData();
	return LG_TRUE;
}


lg_bool CLSkelEdit::Load(LMPath szFile)
{
	Unload();
	CFile fin;
	
	if(!fin.Open(szFile, fin.modeRead))
		return LG_FALSE;
	
	lg_bool bResult=Serialize((lg_void*)&fin, CLMeshEdit::ReadFn, RW_READ);
	
	fin.Close();
	
	m_nFlags=bResult?LM_FLAG_LOADED:0;
	
	if(bResult)
	{
		//Since information gets edited we're going to have
		//to allocate memory in a special way, we can't
		//use the information supplied by Serialize.
		
		
		//Need direct3d stuff
		m_pSkel=new JointVertex[m_nNumJoints];
		SetupFrame(0, 0, 0.0f);
		
		//We also need the ex data.
		CalcExData();
	}
	
	return bResult;
}
lg_bool CLSkelEdit::Save(LMPath szFile)
{
	if(!IsLoaded())
		return LG_FALSE;
	
	CFile fin;
	
	if(!fin.Open(szFile, fin.modeWrite|fin.modeCreate))
		return LG_FALSE;
	
	lg_bool bResult=Serialize((lg_void*)&fin, CLMeshEdit::WriteFn, RW_WRITE);
	
	fin.Close();
	
	return bResult;
}

lg_void CLSkelEdit::Unload()
{
	//Do the actual delete:
	//To come...
	
	//Base unload:
	CLSkel2::Unload();
	
	//Get rid of any extra data.
	LG_SafeDeleteArray(m_pSkel);
}


lg_void CLSkelEdit::SetupFrame(lg_dword nAnim, lg_float fTime)
{
	// The range could easily be changed by modifying this
	//value, 100.0f seemed to be a good range to me.
	#define PREP_MAX_RANGE 100.0f
	
	if(m_nNumAnims<1)
		return;
	
	nAnim=LG_Clamp(nAnim, 0, m_nNumAnims-1);
	fTime=LG_Clamp(fTime, 0.0f, PREP_MAX_RANGE);
	
	float fFrame;
	lg_dword nFrame1=0, nFrame2=0;
	//Get the animation we are dealing with.
	const SkelAnim* pAnim=GetAnim(nAnim);
	if(LG_CheckFlag(pAnim->nFlags, pAnim->ANIM_FLAG_LOOPBACK))
	{
		if(fTime>=50.0f)
			fTime=100.0f-fTime;
			
		fFrame=pAnim->nFirstFrame+((float)pAnim->nNumFrames-1-0.000001f)*fTime/(PREP_MAX_RANGE*0.5f);
	}
	else
	{
		fFrame=pAnim->nFirstFrame+((float)pAnim->nNumFrames-0.000001f)*fTime/PREP_MAX_RANGE;
	}
	nFrame1=(lg_dword)fFrame;
	nFrame2=nFrame1>=(pAnim->nFirstFrame+pAnim->nNumFrames-1)?pAnim->nFirstFrame:nFrame1+1;

	//To get the interpolation value we need only find the fraction portion of fFrame fFrame-nFrame1.
	SetupFrame(nFrame1, nFrame2, fFrame-nFrame1);
}

lg_void CLSkelEdit::SetupFrame(lg_dword nFrame1, lg_dword nFrame2, lg_float t)
{
	lg_dword i=0;
	
	if(!LG_CheckFlag(m_nFlags, LM_FLAG_LOADED))
		return;

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
}


lg_void CLSkelEdit::Render()
{
	//We now want to render the skelton for testing purposes.
	lg_dword i=0;
	//If there are no joints just return true cause there is nothing
	//to render.
	if(!m_nNumJoints)
		return;
		
	BoneVertex   Line[2];

	//We want to render the bones first, this
	//is simply a line from the bone to it's parent.
	//If it has no parent nothing gets rendered.
	CLMeshEdit::s_pDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
	CLMeshEdit::s_pDevice->SetTexture(0, LG_NULL);
	CLMeshEdit::s_pDevice->SetFVF(BONE_VERTEX_FORMAT);
	for(i=0; i<GetNumJoints(); i++)
	{
		memcpy(&Line[0].x, &m_pSkel[i].x, 12);
		Line[0].color=m_pSkel[i].color;
		lg_dword nBone=GetParentBoneRef(i);
		if(nBone)
		{
			memcpy(&Line[1].x, &m_pSkel[nBone-1].x, 12);
			Line[1].color=m_pSkel[nBone-1].color;
			CLMeshEdit::s_pDevice->DrawPrimitiveUP(D3DPT_LINELIST, 1, &Line, sizeof(BoneVertex));
		}
	}
	
	//We can render all the joints at once since they are just points.
	CLMeshEdit::s_pDevice->SetFVF(JOINT_VERTEX_FORMAT);
	CLMeshEdit::s_pDevice->DrawPrimitiveUP(D3DPT_POINTLIST, GetNumJoints(), m_pSkel, sizeof(JointVertex));
	
	//Render the AABB.
	CLMeshEdit::s_pDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
	RenderAABB();
}

lg_void CLSkelEdit::RenderAABB()
{
	//ML_AABB aabb;
	//GenerateAABB(&aabb, nFrame1, nFrame2, t);
	ML_MAT matTmp;
	CLMeshEdit::s_pDevice->GetTransform(D3DTS_WORLD, (D3DMATRIX*)&matTmp);
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
	CLMeshEdit::s_pDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX*)&matIdent);
	CLMeshEdit::s_pDevice->SetFVF(BONE_VERTEX_FORMAT);
	CLMeshEdit::s_pDevice->DrawPrimitiveUP(D3DPT_LINESTRIP, 15, &m_vAABB, sizeof(BoneVertex)); 
	CLMeshEdit::s_pDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX*)&matTmp);
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
	CLMeshEdit::s_pDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX*)&matIdent);
	CLMeshEdit::s_pDevice->SetFVF(BONEVERTEX_FVF);
	CLMeshEdit::s_pDevice->DrawPrimitiveUP(D3DPT_LINESTRIP, 15, &Test, sizeof(BONEVERTEX)); 
	CLMeshEdit::s_pDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX*)&matTmp);
	*/
}

