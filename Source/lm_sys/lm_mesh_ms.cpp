#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include <math.h>
#include "lm_ms.h"
#include "lg_func.h"
#include "msLib\\msLib.h"

lg_uint __cdecl MSWrite(lg_void *file, lg_void *buffer, lg_uint size)
{
	return fwrite(buffer, 1, size, (FILE*)file);
}

extern "C" void ErrorBox(char* format, ...)
{
	char szOutput[1024];
	va_list arglist=LG_NULL;
	int nResult=0;

	va_start(arglist, format);
	_vsnprintf(szOutput, 1024-1, format, arglist);
	MessageBoxA(0, szOutput, "Error Message", MB_OK);
	va_end(arglist);
}

float vertex_length(float* pv)
{
	return sqrt(pv[0]*pv[0]+pv[1]*pv[1]+pv[2]*pv[2]);
}

CLMeshMS::CLMeshMS(void* pModel, char* szName):
CLMesh2()
{
	this->LoadFromMS(pModel, szName);
}

lg_bool CLMeshMS::LoadFromMS(void* pModelParam, char* szName)
{
	Unload();
	
	//Make sure that nothing is currently loaded.
	lg_dword i=0, j=0;

	msModel* pModel=(msModel*)pModelParam;

	this->Unload();
	m_nID=LM_MESH_ID;
	m_nVer=LM_MESH_VER;
	LG_strncpy(m_szMeshName, szName, LM_MAX_NAME);
	m_nNumSubMesh=msModel_GetMeshCount(pModel);
	m_nNumMtrs=msModel_GetMaterialCount(pModel);
	m_nNumBones=msModel_GetBoneCount(pModel);
	//To get the total number of vertices, we have to find
	//out how many vertices are in each mesh, to optomize
	//things just a bit we also find out some of the material
	//information.
	
	//Milkshape to L3DM steps:
	//Step 1: Set all the variables and allocate memory:
	m_nID=LM_MESH_ID;
	m_nVer=LM_MESH_VER;
	m_nNumSubMesh=msModel_GetMeshCount(pModel);
	m_nNumMtrs=msModel_GetMaterialCount(pModel);
	
	for(i=0, m_nNumVerts=0, m_nNumTris=0; i<m_nNumSubMesh; i++)
	{
		msMesh* pMesh=msModel_GetMeshAt(pModel, i);
		m_nNumVerts+=msMesh_GetVertexCount(pMesh);
		m_nNumTris+=msMesh_GetTriangleCount(pMesh);
	}
	//char szTemp[1024];
	//sprintf(szTemp, "There are %d triangles", m_nNumTris);
	//MessageBox(0, szTemp, 0, 0);
	
	//Allocate the memory:
	if(!AllocateMemory())
		return LG_FALSE;

	//Now load the vertices.
	
	//Step 2: Load the vertices and indexes:
	lg_word v=0, indexAt=0;
	for(i=0, v=0, indexAt=0; i<m_nNumSubMesh; i++)
	{
		msMesh* pMesh=msModel_GetMeshAt(pModel, i);
		//Some temp stuff.
		m_pSubMesh[i].nFirstIndex=indexAt;
		m_pSubMesh[i].nNumTri=msMesh_GetTriangleCount(pMesh);
		
		for(j=0; j<(lg_dword)msMesh_GetVertexCount(pMesh); j++, v++)
		{
			msVec3 Normal;
			msVertex* pVertex=msMesh_GetVertexAt(pMesh, j);
			msMesh_GetVertexNormalAt(pMesh, j, Normal); //?
			//Copy vertex (note that z must be made negative)
			m_pVerts[v].x=pVertex->Vertex[0];
			m_pVerts[v].y=pVertex->Vertex[1];
			m_pVerts[v].z=-pVertex->Vertex[2];
			//Copy normals(again z is negative and the base vertex is added).
			//The normals will get set later.
			m_pVerts[v].nx=0.0f;
			m_pVerts[v].ny=0.0f;
			m_pVerts[v].nz=0.0f;
			//Copy tex coordinates.
			m_pVerts[v].tu=pVertex->u;
			m_pVerts[v].tv=pVertex->v;
			
			//For each vertex we save the bone index in the bone index list:
			m_pVertBoneList[v]=pVertex->nBoneIndex;
		}

		lg_word nOffset=v-msMesh_GetVertexCount(pMesh);
		
		for(j=0; j<(lg_dword)msMesh_GetTriangleCount(pMesh); j++, indexAt+=3)
		{
			lg_word nIndices[3], nNormalIndices[3];
			msVec3 Normal;
			
			msTriangle* pTriangle=msMesh_GetTriangleAt(pMesh, j);
			msTriangle_GetVertexIndices(pTriangle, nIndices);
			msTriangle_GetNormalIndices(pTriangle, nNormalIndices);
			
			//Note that we reverse the order of the indices, so that
			//we can cull the way we want to.
			for(int k=0; k<3; k++)
			{	
				m_pIndexes[indexAt+k]=nIndices[2-k]+nOffset;
				
				//Here is where we set the normals, note that
				//the actual normals may be affected multiply
				//times, but in the end a valid set of normals
				//should be available.
				lg_word nIndex=m_pIndexes[indexAt+k];
				msMesh_GetVertexNormalAt(pMesh, nNormalIndices[2-k], Normal);
				m_pVerts[nIndex].nx=Normal[0];
				m_pVerts[nIndex].ny=Normal[1];
				m_pVerts[nIndex].nz=-Normal[2];
			}			
		}
	}
	
	//Step 2.5: Save all the bone names:
	for(i=0; i<m_nNumBones; i++)
	{
		msBone* pBone=msModel_GetBoneAt(pModel, i);
		LG_strncpy(m_pBoneNameList[i], pBone->szName, LM_MAX_NAME);
	}
	
	//Step 3: Load the meshes.
	for(i=0; i<m_nNumSubMesh; i++)
	{
		//We've already loaded the start vertex, and
		//number of vertexes from above.
		msMesh* pMesh=msModel_GetMeshAt(pModel, i);
		LG_strncpy(m_pSubMesh[i].szName, pMesh->szName, LM_MAX_NAME);
		m_pSubMesh[i].nMtrIndex=pMesh->nMaterialIndex;
	}

	//Step 4: Load the material.
	for(i=0; i<m_nNumMtrs; i++)
	{
		msMaterial* pMaterial=msModel_GetMaterialAt(pModel, i);
		LG_strncpy(m_pMtrs[i].szName, pMaterial->szName, LM_MAX_NAME);
		//We want to get rid of the path information:
		int nPos=0;
		char* szPtr=pMaterial->szDiffuseTexture;
		nPos=strlen(szPtr);
		for( ; nPos>0; --nPos)
		{
			if(szPtr[nPos]=='/' || szPtr[nPos]=='\\')
			{
				nPos++;
				break;
			}
		}
		
		LG_strncpy(m_pMtrs[i].szFile, &szPtr[nPos], LM_MAX_PATH);
		//ErrorBox("The file is: \"%s\" from \"%s\" %d", m_pMtrs[i].szFile, szPtr, nPos);
	}
	
	//Calculate the AABB
	
	m_AABB.v3Min.x=m_pVerts[0].x;
	m_AABB.v3Min.y=m_pVerts[0].y;
	m_AABB.v3Min.z=m_pVerts[0].z;
	m_AABB.v3Max.x=m_pVerts[0].x;
	m_AABB.v3Max.y=m_pVerts[0].y;
	m_AABB.v3Max.z=m_pVerts[0].z;
	
	for(i=0; i<m_nNumVerts; i++)
	{
		m_AABB.v3Min.x=LG_Min(m_AABB.v3Min.x, m_pVerts[i].x);
		m_AABB.v3Min.y=LG_Min(m_AABB.v3Min.y, m_pVerts[i].y);
		m_AABB.v3Min.z=LG_Min(m_AABB.v3Min.z, m_pVerts[i].z);
		
		m_AABB.v3Max.x=LG_Max(m_AABB.v3Max.x, m_pVerts[i].x);
		m_AABB.v3Max.y=LG_Max(m_AABB.v3Max.y, m_pVerts[i].y);
		m_AABB.v3Max.z=LG_Max(m_AABB.v3Max.z, m_pVerts[i].z);	
	}
	
	m_nFlags=LM_FLAG_LOADED;
	return LG_TRUE;
}

lg_bool CLMeshMS::Load(LMPath szFile)
{
	//We only load using the converter.
	return LG_FALSE;
}

lg_bool CLMeshMS::Save(LMPath szFile)
{
	FILE* fout=fopen(szFile, "wb");
	if(!fout)
		return LG_FALSE;
		
	lg_bool bRes=Serialize(fout, MSWrite, RW_WRITE);
	
	fclose(fout);
	
	return bRes;
}






////////////////////////////////////////////////////////////////
/// Legacy Skeleton Milkshape loading. /////////////////////////
////////////////////////////////////////////////////////////////


CLSkelMS::CLSkelMS(void* pModel, char* szName):
CLSkel2()
{
	this->LoadFromMS(pModel, szName);
}

lg_bool CLSkelMS::LoadFromMS(void* pModelParam, char* szName)
{
	msModel* pModel=(msModel*)pModelParam;
	lg_dword i=0, j=0;

	m_nID=LM_SKEL_ID;
	m_nVersion=LM_SKEL_VER;
	LG_strncpy(m_szSkelName, szName, LM_MAX_NAME);
	m_nNumJoints=msModel_GetBoneCount(pModel);
	m_nNumKeyFrames=msModel_GetTotalFrames(pModel);
	m_nNumAnims=0; //Animations have to be added using a different utility.
	
	if(m_nNumJoints<1)
	{
		//No skeleton.
		m_nID=0;
		m_nVersion=0;
		m_nNumJoints=0;
		m_nNumKeyFrames=0;
		m_nFlags=0;
		return LG_FALSE;
	}
	if(!AllocateMemory())
		return LG_FALSE;
		
	#define MINUS(a) (-a)
	#define MINUSR(a) (-a)

	//Load the base skeleton.
	for(i=0; i<m_nNumJoints; i++)
	{
		msBone* pBone=msModel_GetBoneAt(pModel, i);
		LG_strncpy(m_pBaseSkeleton[i].szName, pBone->szName, LM_MAX_NAME);
		LG_strncpy(m_pBaseSkeleton[i].szParent, pBone->szParentName, LM_MAX_NAME);
		m_pBaseSkeleton[i].fPos[0]=pBone->Position[0];
		m_pBaseSkeleton[i].fPos[1]=pBone->Position[1];
		m_pBaseSkeleton[i].fPos[2]=MINUS(pBone->Position[2]);

		m_pBaseSkeleton[i].fRot[0]=pBone->Rotation[0];
		m_pBaseSkeleton[i].fRot[1]=pBone->Rotation[1];
		m_pBaseSkeleton[i].fRot[2]=MINUSR(pBone->Rotation[2]);
	}
	//We now need to find all the indexes for the parent bones.
	for(i=0; i<m_nNumJoints; i++)
	{
		m_pBaseSkeleton[i].nParent=0;
		//If there is no parent the index for the parent bone is 0,
		//note that the index for bones is not zero based.  In other
		//words bone 1 would be m_pBaseSkeleton[0].
		if(strlen(m_pBaseSkeleton[i].szParent)<1)
		{		
			m_pBaseSkeleton[i].nParent=0;
			continue;
		}
		for(j=1; j<=m_nNumJoints; j++)
		{
			if(stricmp(m_pBaseSkeleton[i].szParent, m_pBaseSkeleton[j-1].szName)==0)
			{
				m_pBaseSkeleton[i].nParent=j;
				break;
			}
		}
	}

	//Now we will write the keyframe information,
	//in the MS3D format only joints that are actually operated on
	//are saved, but in the legacy format we want every rotation
	//for every joint every frame.  This can be calculated by finding
	//the most previous joint position.
	//Find the total number of keyframes.
	
	float fPosition[3];
	float fRotation[3];

	for(i=0; i<m_nNumJoints; i++)
	{
		msBone* pBone=msModel_GetBoneAt(pModel, i);
		//Set the position and rotation values of the
		//first frame.
		msPositionKey* pPos=msBone_GetPositionKeyAt(pBone, 0);
		msRotationKey* pRot=msBone_GetRotationKeyAt(pBone, 0);
		if(pPos)
		{
			fPosition[0]=pPos->Position[0];
			fPosition[1]=pPos->Position[1];
			fPosition[2]=MINUS(pPos->Position[2]);
		}
		else
		{
			fPosition[0]=0.0f;
			fPosition[1]=0.0f;
			fPosition[2]=0.0f;
		}	
		if(pRot)
		{	
			fRotation[0]=pRot->Rotation[0];
			fRotation[1]=pRot->Rotation[1];
			fRotation[2]=MINUSR(pRot->Rotation[2]);
		}
		else
		{
			fRotation[0]=0.0f;
			fRotation[1]=0.0f;
			fRotation[2]=0.0f;
		}
		m_pFrames[0].SetLocalMat(i, fPosition, fRotation);
		for(j=0; j<m_nNumKeyFrames; j++)
		{
			//We'll zero out the bounding boxes,
			//they will need to be set using LMEdit or
			//some other method.
			ML_VEC3 Zero={0.0f, 0.0f, 0.0f};
			m_pFrames[j].aabbBox.v3Min=Zero;
			m_pFrames[j].aabbBox.v3Max=Zero;
			/*
			m_pKeyFrames[j].pJointPos[i].position[0]=0.0f;
			m_pKeyFrames[j].pJointPos[i].position[1]=0.0f;
			m_pKeyFrames[j].pJointPos[i].position[2]=0.0f;
			m_pKeyFrames[j].pJointPos[i].rotation[0]=0.0f;
			m_pKeyFrames[j].pJointPos[i].rotation[1]=0.0f;
			m_pKeyFrames[j].pJointPos[i].rotation[2]=0.0f;
			*/
			//If we are dealing with the first frame set it
			//to zero in case there is no first frame, for
			//any other frames we simply set it to the previous
			//frame if there is no frame.
			/*
			if(j==0)
			{
				fPosition[0]=0.0f;
				fPosition[1]=0.0f;
				fPosition[2]=0.0f;
				
				fRotation[0]=0.0f;
				fRotation[1]=0.0f;
				fRotation[2]=0.0f;
				
				m_pFrames[j].SetLocalMat(i, fPosition, fRotation);	
			}
			*/
		
			lg_word k=0;
			lg_bool bFound=LG_FALSE;
			for(k=0; k<pBone->nNumPositionKeys; k++)
			{
				msPositionKey* pPos=msBone_GetPositionKeyAt(pBone, k);
				msRotationKey* pRot=msBone_GetRotationKeyAt(pBone, k);
				if((lg_dword)pPos->fTime==(j+0))
				{			
					fPosition[0]=pPos->Position[0];
					fPosition[1]=pPos->Position[1];
					fPosition[2]=MINUS(pPos->Position[2]);
					
					fRotation[0]=pRot->Rotation[0];
					fRotation[1]=pRot->Rotation[1];
					fRotation[2]=MINUSR(pRot->Rotation[2]);
					
					m_pFrames[j].SetLocalMat(i, &fPosition[0], &fRotation[0]);
					
					bFound=LG_TRUE;
					break;
				}
			}
			
			if(!bFound && j!=0)
			{		
				fPosition[0]=m_pFrames[j-1].LocalPos[i].fPos[0];
				fPosition[1]=m_pFrames[j-1].LocalPos[i].fPos[1];
				fPosition[2]=m_pFrames[j-1].LocalPos[i].fPos[2];
				
				fRotation[0]=m_pFrames[j-1].LocalPos[i].fRot[0];
				fRotation[1]=m_pFrames[j-1].LocalPos[i].fRot[1];
				fRotation[2]=m_pFrames[j-1].LocalPos[i].fRot[2];
				
				m_pFrames[j].SetLocalMat(i, &fPosition[0], &fRotation[0]);
				
			}
				
			
		}
		
	}

	m_nFlags=LM_FLAG_LOADED;
	return LG_TRUE;
}


lg_bool CLSkelMS::Load(LMPath szFile)
{
	return LG_FALSE;
}

lg_bool CLSkelMS::Save(CLMBase::LMPath szFile)
{
	FILE* fout=fopen(szFile, "wb");
	if(!fout)
		return LG_FALSE;
		
	lg_bool bRes=Serialize(fout, MSWrite, RW_WRITE);
	
	fclose(fout);
	
	return bRes;
}
