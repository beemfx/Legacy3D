#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include "lm_ms.h"
#include "msLib\\msLib.h"

extern "C" void ErrorBox(char* format, ...)
{
	char szOutput[1024];
	va_list arglist=L_null;
	int nResult=0;

	va_start(arglist, format);
	_vsnprintf(szOutput, 1024-1, format, arglist);
	MessageBox(0, szOutput, "Error Message", MB_OK);
	va_end(arglist);
}

float vertex_length(float* pv)
{
	return sqrt(pv[0]*pv[0]+pv[1]*pv[1]+pv[2]*pv[2]);
}

CLegacyMeshMS::CLegacyMeshMS(void* pModel)
{
	this->LoadFromMS(pModel);
}

L_bool CLegacyMeshMS::LoadFromMS(void* pModelParam)
{
	//Make sure that nothing is currently loaded.
	L_dword i=0, j=0;

	msModel* pModel=(msModel*)pModelParam;

	this->Unload();
	m_ID=LMESH_ID;
	m_nVersion=LMESH_VERSION;
	m_nNumMeshes=msModel_GetMeshCount(pModel);
	m_nNumMaterials=msModel_GetMaterialCount(pModel);
	//m_nNumJoints=msModel_GetBoneCount(pModel);
	//m_nNumKeyFrames=msModel_GetTotalFrames(pModel);
	
	//To get the total number of vertices, we have to find
	//out how many vertices are in each mesh, to optomize
	//things just a bit we also find out some of the material
	//information.

	m_pMeshes=new LMESH_SET[m_nNumMeshes];
	if(!m_pMeshes)
		return L_false;
	/*
	//Allocate memory for the frames.
	m_pKeyFrames=new LMESH_KEYFRAME[m_nNumKeyFrames];
	if(!m_pKeyFrames)
	{
		L_safe_delete_array(m_pMeshes);
		return L_false;
	}
	for(i=0; i<m_nNumKeyFrames; i++)
	{
		m_pKeyFrames[i].pJointPos=new LMESH_JOINTPOSEX[m_nNumJoints];

		if(!m_pKeyFrames[i].pJointPos)
		{
			for(j=i-1; j>0; j--)
			{
				L_safe_delete_array(m_pKeyFrames[j].pJointPos);
			}
			L_safe_delete_array(m_pKeyFrames);
			L_safe_delete_array(m_pMeshes);
			return L_false;
		}
	}
	*/

	for(i=0, m_nNumVertices=0; i<m_nNumMeshes; i++)
	{
		msMesh* pMesh=msModel_GetMeshAt(pModel, i);
		//The location of the current number of vertices is
		//the index of the first vertex in the mesh.
		m_pMeshes[i].nFirstVertex=m_nNumVertices;
		//We have to multiply the number of triangles by three to get
		//the nubmer of vertices.
		m_pMeshes[i].nNumTriangles=msMesh_GetTriangleCount(pMesh);
		//Finally add the triangle count to get the total.
		m_nNumVertices+=(msMesh_GetTriangleCount(pMesh)*3);
	}

	//Now lets allocate memory for everything else.
	m_pVertices=new LMESH_VERTEX[m_nNumVertices];
	m_pVertexBoneList=new L_dword[m_nNumVertices];
	m_pMaterials=new LMESH_MATERIAL[m_nNumMaterials];
	//m_pBaseSkeleton=new LMESH_JOINTEX[m_nNumJoints];
	//m_pBones=new LMESH_BONE[m_nNumJoints];
	if(!m_pVertices || !m_pMaterials || /*!m_pBaseSkeleton ||*/ !m_pVertexBoneList || !m_pMeshes)
	{
		L_safe_delete_array(m_pVertexBoneList);
		L_safe_delete_array(m_pMeshes);
		L_safe_delete_array(m_pVertices);
		L_safe_delete_array(m_pMaterials);
		/*
		L_safe_delete_array(m_pBaseSkeleton);
		for(i=0; i<m_nNumKeyFrames; i++)
		{
			L_safe_delete_array(m_pKeyFrames[i].pJointPos);
		}
		L_safe_delete_array(m_pKeyFrames);
	//	L_safe_delete_array(m_pBones);
		*/
		return 0;
	}

	//Now load the vertices.
	L_dword v=0;
	for(i=0, v=0; i<m_nNumMeshes; i++)
	{
		msMesh* pMesh=msModel_GetMeshAt(pModel, i);
		L_dword nTrianlgeCount=(L_dword)msMesh_GetTriangleCount(pMesh);
		for(j=0; j<(L_dword)msMesh_GetTriangleCount(pMesh); j++, v+=3)
		{
			msTriangle* pTriangle=msMesh_GetTriangleAt(pMesh, j);
			L_word nIndices[3];
			L_word nNormalIndices[3];
			msTriangle_GetVertexIndices(pTriangle, nIndices);
			msTriangle_GetNormalIndices(pTriangle, nNormalIndices);

			for(int k=0; k<3; k++)
			{
						
				//We use 2-k to get the vertex, cause we want to reverse
				//the vertext order, because the Legacy engine's standard
				//mode for culling is counter-clockwise and Milkshapes is
				//clockwise.
				msVertex* pVertex=msMesh_GetVertexAt(pMesh, nIndices[2-k]);
				msVec3 pNormal;
				msMesh_GetVertexNormalAt(pMesh, nNormalIndices[2-k], pNormal);

				//Copy coordinates.
				m_pVertices[v+k].x=pVertex->Vertex[0];
				m_pVertices[v+k].y=pVertex->Vertex[1];
				//Note that z is negative.
				m_pVertices[v+k].z=-pVertex->Vertex[2];

				
				//Copy normals.
				m_pVertices[v+k].nx=pNormal[0]+pVertex->Vertex[0];
				m_pVertices[v+k].ny=pNormal[1]+pVertex->Vertex[1];
				//Note that z is negative.
				m_pVertices[v+k].nz=-(pNormal[2]+pVertex->Vertex[2]);
				

				m_pVertices[v+k].tu=pVertex->u;
				m_pVertices[v+k].tv=pVertex->v;

				//Save the bone index.
				m_pVertexBoneList[v+k]=pVertex->nBoneIndex;

			
				/*
				static L_bool bFirst=6;
				if(bFirst)
				{
					bFirst--;
					float v3[3];
					v3[0]=pNormal[0]-pVertex->Vertex[0];
					v3[1]=pNormal[1]-pVertex->Vertex[1];
					v3[2]=pNormal[2]-pVertex->Vertex[2];
					//ErrorBox("Normal Index: %i", nIndices[2-k]);
					
					ErrorBox(
						"Vertex <%f, %f, %f> len: %f Normal: <%f, %f, %f> len: %f index: %i difference: %f", 
						pVertex->Vertex[0],
						pVertex->Vertex[1],
						pVertex->Vertex[2],
						vertex_length(pVertex->Vertex),
						pNormal[0],
						pNormal[1],
						pNormal[2],
						vertex_length(pNormal),
						nNormalIndices[2-k],
						vertex_length(v3));
				}
				
				*/
				
			}
		}
	}

	//Load the meshes.
	for(i=0; i<m_nNumMeshes; i++)
	{
		//We've already loaded the start vertex, and
		//number of vertexes from above.
		msMesh* pMesh=msModel_GetMeshAt(pModel, i);
		L_strncpy(m_pMeshes[i].szName, pMesh->szName, LMESH_MAX_NAME_LENGTH);
		m_pMeshes[i].nMaterialIndex=pMesh->nMaterialIndex;
	}

	//Load the material.
	for(i=0; i<m_nNumMaterials; i++)
	{
		msMaterial* pMaterial=msModel_GetMaterialAt(pModel, i);
		L_strncpy(m_pMaterials[i].szMaterialName, pMaterial->szName, LMESH_MAX_NAME_LENGTH);
		L_strncpy(m_pMaterials[i].szTexture, pMaterial->szDiffuseTexture, LMESH_MAX_PATH);
	}

	m_bLoaded=L_true;
	return L_true;
}


////////////////////////////////////////////////////////////////
/// Legacy Skeleton Milkshape loading. /////////////////////////
////////////////////////////////////////////////////////////////


CLegacySkelMS::CLegacySkelMS(void* pModel)
{
	this->LoadFromMS(pModel);
}

L_bool CLegacySkelMS::LoadFromMS(void* pModelParam)
{
	msModel* pModel=(msModel*)pModelParam;
	L_dword i=0, j=0;

	m_nID=LSKEL_ID;
	m_nVersion=LSKEL_VERSION;
	m_nNumJoints=msModel_GetBoneCount(pModel);
	m_nNumKeyFrames=msModel_GetTotalFrames(pModel);

	if(!AllocateMemory())
		return L_false;

	//Load the base skeleton.
	for(i=0; i<m_nNumJoints; i++)
	{
		msBone* pBone=msModel_GetBoneAt(pModel, i);
		L_strncpy(m_pBaseSkeleton[i].szName, pBone->szName, LMESH_MAX_PATH);
		L_strncpy(m_pBaseSkeleton[i].szParentBone, pBone->szParentName, LMESH_MAX_PATH);
		m_pBaseSkeleton[i].position[0]=pBone->Position[0];
		m_pBaseSkeleton[i].position[1]=pBone->Position[1];
		m_pBaseSkeleton[i].position[2]=-pBone->Position[2];

		m_pBaseSkeleton[i].rotation[0]=pBone->Rotation[0];
		m_pBaseSkeleton[i].rotation[1]=pBone->Rotation[1];
		m_pBaseSkeleton[i].rotation[2]=-pBone->Rotation[2];
	}
	//We now need to find all the indexes for the parent bones.
	for(i=0; i<m_nNumJoints; i++)
	{
		m_pBaseSkeleton[i].nParentBone=0;
		//If there is no parent the index for the parent bone is 0,
		//note that the index for bones is not zero based.  In other
		//words bone 1 would be m_pBaseSkeleton[0].
		if(L_strlen(m_pBaseSkeleton[i].szParentBone)<1)
		{		
			m_pBaseSkeleton[i].nParentBone=0;
			continue;
		}
		for(j=1; j<=m_nNumJoints; j++)
		{
			if(L_strnicmp(m_pBaseSkeleton[i].szParentBone, m_pBaseSkeleton[j-1].szName, 0))
			{
				m_pBaseSkeleton[i].nParentBone=j;
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

	for(i=0; i<m_nNumJoints; i++)
	{
		msBone* pBone=msModel_GetBoneAt(pModel, i);
		for(j=0; j<m_nNumKeyFrames; j++)
		{
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
			
			if(j==0)
			{
				m_pKeyFrames[j].pJointPos[i].position[0]=0.0f;
				m_pKeyFrames[j].pJointPos[i].position[1]=0.0f;
				m_pKeyFrames[j].pJointPos[i].position[2]=0.0f;
				m_pKeyFrames[j].pJointPos[i].rotation[0]=0.0f;
				m_pKeyFrames[j].pJointPos[i].rotation[1]=0.0f;
				m_pKeyFrames[j].pJointPos[i].rotation[2]=0.0f;
			}
		
			L_word k=0;
			L_bool bFound=L_false;
			for(k=0; k<pBone->nNumPositionKeys; k++)
			{
				if((L_dword)pBone->pPositionKeys[k].fTime==(j+1))
				{
					m_pKeyFrames[j].pJointPos[i].position[0]=pBone->pPositionKeys[k].Position[0];
					m_pKeyFrames[j].pJointPos[i].position[1]=pBone->pPositionKeys[k].Position[1];
					m_pKeyFrames[j].pJointPos[i].position[2]=-pBone->pPositionKeys[k].Position[2];
					m_pKeyFrames[j].pJointPos[i].rotation[0]=pBone->pRotationKeys[k].Rotation[0];
					m_pKeyFrames[j].pJointPos[i].rotation[1]=pBone->pRotationKeys[k].Rotation[1];
					m_pKeyFrames[j].pJointPos[i].rotation[2]=-pBone->pRotationKeys[k].Rotation[2];
					bFound=L_true;
					break;
				}
			}
			
			if(!bFound && j!=0)
			{
				m_pKeyFrames[j].pJointPos[i].position[0]=m_pKeyFrames[j-1].pJointPos[i].position[0];
				m_pKeyFrames[j].pJointPos[i].position[1]=m_pKeyFrames[j-1].pJointPos[i].position[1];
				m_pKeyFrames[j].pJointPos[i].position[2]=m_pKeyFrames[j-1].pJointPos[i].position[2];
				m_pKeyFrames[j].pJointPos[i].rotation[0]=m_pKeyFrames[j-1].pJointPos[i].rotation[0];
				m_pKeyFrames[j].pJointPos[i].rotation[1]=m_pKeyFrames[j-1].pJointPos[i].rotation[1];
				m_pKeyFrames[j].pJointPos[i].rotation[2]=m_pKeyFrames[j-1].pJointPos[i].rotation[2];
			}
				
			
		}
	}

	m_bLoaded=L_true;
	return L_true;
}

