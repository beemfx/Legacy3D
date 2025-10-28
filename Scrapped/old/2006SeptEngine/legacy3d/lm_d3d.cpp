#include "lm_d3d.h"
#include "lv_tex.h"
#include <lf_sys.h>
#include "lm_math.h"

#include <d3dx9.h>

int __cdecl mesh_close(void* stream)
{
	return File_Close(stream);
}
int __cdecl mesh_seek(void* stream, long offset, int origin)
{
	return File_Seek(stream, offset, (LF_MOVETYPE) origin);
}
int __cdecl mesh_tell(void* stream)
{
	return File_Tell(stream);
}
unsigned int __cdecl mesh_read(void* buffer, unsigned int size, unsigned int count, void* stream)
{
	return File_Read(stream, size*count, buffer);
}

CLegacyMeshD3D::CLegacyMeshD3D()
{
	m_lpDevice=L_null;
	m_lpVB=L_null;
	m_lppTexs=L_null;
	m_bD3DValid=L_false;
	m_pSkel=L_null;
	m_pAnimMats=L_null;
}

L_bool CLegacyMeshD3D::Create(char* szFilename, IDirect3DDevice9* lpDevice)
{
	Unload();
	LF_FILE2 model=File_Open(szFilename, 0, LF_ACCESS_READ, LFCREATE_OPEN_EXISTING);
	if(!model)
		return L_false;


	LMESH_CALLBACKS cb;
	char szPath[LMESH_MAX_PATH]={0};
	cb.close=mesh_close;
	cb.read=mesh_read;
	cb.seek=mesh_seek;
	cb.tell=mesh_tell;

	if(Load(model, &cb, L_getfilepath(szPath, szFilename)))
	{
		if(!CreateD3DComponents(lpDevice))
		{
			Unload();
			return L_false;
		}

		return L_true;
	}
	else
		return L_false;
}

L_bool CLegacyMeshD3D::CreateD3DComponents(IDirect3DDevice9* lpDevice)
{
	m_lpDevice=lpDevice;
	m_lpDevice->AddRef();
	//Allocate memory for textures.
	m_lppTexs=new IDirect3DTexture9*[m_nNumMaterials];

	m_pSkel=new JOINTVERTEX[m_cSkel.m_nNumJoints];
	//We need the animation matrices to be the number
	//of joints*2 so we can store the matrices for
	//the first and second frame.
	m_pAnimMats=new D3DXMATRIX[m_cSkel.m_nNumJoints*2];
	this->SetupSkeleton(0, 0, 0.0f);
	this->Validate();

	return L_true;
}


L_bool CLegacyMeshD3D::Unload()
{
	if(!m_bLoaded)
		return L_true;
	L_dword i=0;
	Invalidate();
	L_safe_delete_array(m_lppTexs);
	L_safe_delete_array(m_pSkel);
	L_safe_delete_array(m_pAnimMats);
	L_safe_release(m_lpDevice);
	this->CLegacyMesh::Unload();
	
	m_lpDevice=L_null;
	m_lpVB=L_null;
	m_lppTexs=L_null;
	m_bD3DValid=L_false;
	m_pSkel=L_null;

	return L_true;
}

L_bool CLegacyMeshD3D::Validate()
{
	//If the device has not been set, then we can't validate.
	if(!m_lpDevice)
		return L_false;
	//If we are already valid we don't need to revalidate the object.
	if(m_bD3DValid)
		return L_true;
	//Create the vertex buffer and fill it with the default information.
	L_result nResult=0;
	nResult=m_lpDevice->CreateVertexBuffer(
		m_nNumVertices*sizeof(LMESH_VERTEX),
		D3DUSAGE_WRITEONLY,
		LMESH_VERTEX_FORMAT,
		D3DPOOL_DEFAULT,
		&m_lpVB,
		L_null);

	if(L_failed(nResult))
	{
		return L_false;
	}
	// Now fill the vertex buffer with the default vertex data.
	// Note that animation data will be created and filled into
	// the vertex buffer on the fly.
	void* lpBuffer=L_null;

	nResult=m_lpVB->Lock(0, m_nNumVertices*sizeof(LMESH_VERTEX), &lpBuffer, 0);
	if(L_failed(nResult))
	{
		L_safe_release(m_lpVB);
		return L_false;
	}

	memcpy(lpBuffer, m_pVertices, m_nNumVertices*sizeof(LMESH_VERTEX));

	m_lpVB->Unlock();


	//Load the textures.
	L_dword i=0;
	for(i=0; i<m_nNumMaterials; i++)
	{
		Tex_Load(m_lpDevice, m_pMaterials[i].szTexture, D3DPOOL_DEFAULT, 0xFFFFFFFF, L_false, &m_lppTexs[i]);
	}

	m_bD3DValid=L_true;
	return L_true;
}

L_bool CLegacyMeshD3D::Invalidate()
{
	if(!m_bD3DValid)
		return L_false;

	L_dword i=0;
	L_safe_release(m_lpVB);
	for(i=0; i<m_nNumMaterials; i++)
	{
		L_safe_release(m_lppTexs[i]);
	}
	m_bD3DValid=L_false;
	return L_true;
}


L_bool CLegacyMeshD3D::PrepareFrame(L_dword dwFrame1, L_dword dwFrame2, float t)
{
	if(!m_bD3DValid)
		return L_false;
	//Make sure that all of the function parameters are
	//within a valid range.
	dwFrame1=L_clamp(dwFrame1, 0, m_cSkel.m_nNumKeyFrames);
	dwFrame2=L_clamp(dwFrame2, 0, m_cSkel.m_nNumKeyFrames);
	t=L_clamp(t, 0.0f, 1.0f);

	//Lock the vertex buffer to prepare to write the data.
	LMESH_VERTEX* pVerts=L_null;
	if(L_failed(m_lpVB->Lock(0, m_nNumVertices*sizeof(LMESH_VERTEX), (void**)&pVerts, 0)))
		return L_false;

	L_dword i=0;
	//If the 0th frame is specified that is just the standard
	//default positioin.  Should probably actually just set up
	//some normalized matrices.
	if(dwFrame1==0)
	{
		for(i=0; i<m_nNumVertices; i++)
			memcpy(&pVerts[i], &m_pVertices[i], sizeof(LMESH_VERTEX));	
	}
	else
	{
		D3DXMATRIX* pJointTrans=m_pAnimMats;
		//Prepare each joints final transformation matrix.
		for(i=0; i<m_cSkel.m_nNumJoints; i++)
		{
			//We already calculated the final transformation
			//matrices for each frame for each joint so we need
			//only interpolate the the matrices for the two frames.

			//Interpolate transformation matrices.
			L_matslerp(
				&pJointTrans[i], 
				&m_cSkel.m_pKeyFrames[dwFrame1-1].pJointPos[i].Final,
				&m_cSkel.m_pKeyFrames[dwFrame2-1].pJointPos[i].Final, 
				t);
		}

		//Now that the transformations are all set up,
		//all we have to do is multiply each vertex by the 
		//transformation matrix for it's joint.
		for(i=0; i<m_nNumVertices; i++)
		{
			//Copy the vertex we want.
			memcpy(&pVerts[i], &m_pVertices[i], sizeof(LMESH_VERTEX));
			
			//If the vertex's bone is -1 it means that it is
			//not attached to a joint so we don't need to transform it.
			if(m_pVertexBoneList[i]!=-1)
			{
				//Multiply each vertex by it's joints transform.
				D3DXVec3TransformCoord((D3DXVECTOR3*)&pVerts[i].x, (D3DXVECTOR3*)&pVerts[i].x, &pJointTrans[m_pVertexBoneList[i]]);	
				//Do the same for each vertex's normal.
				D3DXVec3TransformCoord((D3DXVECTOR3*)&pVerts[i].nx, (D3DXVECTOR3*)&pVerts[i].nx, &pJointTrans[m_pVertexBoneList[i]]);
			}
		}
	}
	//We've written all the vertices so lets unlock the VB.
	m_lpVB->Unlock();
	return L_true;
}





L_bool CLegacyMeshD3D::SetupSkeleton(L_dword nFrame1, L_dword nFrame2, float t)
{
	L_dword i=0;
	
	if(!m_pSkel || !m_cSkel.m_pBaseSkeleton)
		return L_false;
	

	nFrame1=L_clamp(nFrame1, 0, m_cSkel.m_nNumKeyFrames);
	nFrame2=L_clamp(nFrame2, 0, m_cSkel.m_nNumKeyFrames);
	t=L_clamp(t, 0.0f, 1.0f);

	//If frame 0 is specifed that is the
	//default skeleton.
	if(nFrame1==0)
	{
		for(i=0; i<m_cSkel.m_nNumJoints; i++)
		{
			//To find the final location of a joint we need to (1) find the
			//relative location of the joint. (2) multiply that by all of
			//it parent/grandparent's/etc transformation matrices.
			//(1)
			m_pSkel[i].x=0.0f;
			m_pSkel[i].y=0.0f;
			m_pSkel[i].z=0.0f;
			m_pSkel[i].psize=4.0f;
			m_pSkel[i].color=0xFFFF0000;
			//(2)
			D3DXVec3TransformCoord((D3DXVECTOR3*)&m_pSkel[i].x, (D3DXVECTOR3*)&m_pSkel[i].x, &m_cSkel.m_pBaseSkeleton[i].Final);
		}
	}
	else
	{
		D3DXMATRIX* pJointTrans=m_pAnimMats;
		for(i=0; i<m_cSkel.m_nNumJoints; i++)
		{
			//Interpolate the matrices.
			//L_matslerp(&pJointTrans[i], &pJointTrans[i], &pJointTrans[i+m_nNumJoints], t);
			L_matslerp(&pJointTrans[i], &m_cSkel.m_pKeyFrames[nFrame1-1].pJointPos[i].Final, &m_cSkel.m_pKeyFrames[nFrame2-1].pJointPos[i].Final, t);
			//We have to multiply by the base skelton final because we already multipled
			//the final key frame by the inverse, and that only needs to be done for transforming
			//the vertices of the model, not for transforming the vertices for the skeleton
			//joints.
			pJointTrans[i]=m_cSkel.m_pBaseSkeleton[i].Final*pJointTrans[i];
		}
			
		for(i=0; i<m_cSkel.m_nNumJoints; i++)
		{
			//To find the final location of a joint we need to (1) find the
			//relative location of the joint. (2) multiply that by all of
			//it parent/grandparent's/etc transformation matrices.
			//(1)
			m_pSkel[i].x=0.0f;
			m_pSkel[i].y=0.0f;
			m_pSkel[i].z=0.0f;
			m_pSkel[i].psize=4.0f;
			m_pSkel[i].color=0xFFFF0000;
			//(2)
			D3DXVec3TransformCoord((D3DXVECTOR3*)&m_pSkel[i].x, (D3DXVECTOR3*)&m_pSkel[i].x, &pJointTrans[i]);		
		}
	}
	return L_true;
}


L_bool CLegacyMeshD3D::RenderSkeleton()
{
	//We now want to render the skelton for testing purposes.
	L_dword i=0;
	//If there are no joints just return true cause there is nothing
	//to render.
	if(!m_cSkel.m_nNumJoints)
		return L_true;
	BONEVERTEX   Line[2];

	//We want to render the bones first, this
	//is simply a line from the bone to it's parent.
	//If it has no parent nothing gets rendered.
	m_lpDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
	m_lpDevice->SetTexture(0, L_null);
	m_lpDevice->SetFVF(BONEVERTEX_FVF);
	for(i=0; i<m_cSkel.m_nNumJoints; i++)
	{
		memcpy(&Line[0].x, &m_pSkel[i].x, 12);
		Line[0].color=m_pSkel[i].color;
		if(m_cSkel.m_pBaseSkeleton[i].nParentBone)
		{
			memcpy(&Line[1].x, &m_pSkel[m_cSkel.m_pBaseSkeleton[i].nParentBone-1].x, 12);
			Line[1].color=m_pSkel[m_cSkel.m_pBaseSkeleton[i].nParentBone-1].color;
			m_lpDevice->DrawPrimitiveUP(D3DPT_LINELIST, 1, &Line, sizeof(BONEVERTEX));
		}
	}
	
	//We can render all the joints at once since they are just points.
	m_lpDevice->SetFVF(JOINTVERTEX_FVF);
	m_lpDevice->DrawPrimitiveUP(D3DPT_POINTLIST, m_cSkel.m_nNumJoints, m_pSkel, sizeof(JOINTVERTEX));
	m_lpDevice->SetRenderState(D3DRS_ZENABLE, TRUE);
	//L_safe_delete_array(pTest);
	return L_true;
}



L_bool CLegacyMeshD3D::Render()
{
	//Test render the model.
	if(!m_bD3DValid)
		return L_false;

	//PrepareFrame should not be called here, but should be called
	//by the application prior to the call to Render(). 
	//PrepareFrame(0, 1, 0.0f);

	m_lpDevice->SetFVF(LMESH_VERTEX_FORMAT);
	m_lpDevice->SetStreamSource(0, m_lpVB, 0, sizeof(LMESH_VERTEX));
	
	L_dword i=0;
	L_bool bRenderMeshesSeparately=L_true;
	if(bRenderMeshesSeparately)
	{
		for(i=0; i<m_nNumMeshes; i++)
		{
			if(m_pMeshes[i].nMaterialIndex!=-1)
			{
				m_lpDevice->SetTexture(0, m_lppTexs[m_pMeshes[i].nMaterialIndex]);
			}
			else
			{
				m_lpDevice->SetTexture(0, L_null);
			}

			m_lpDevice->DrawPrimitive(D3DPT_TRIANGLELIST, m_pMeshes[i].nFirstVertex, m_pMeshes[i].nNumTriangles);
		}
	}
	else
	{
		//If we draw the whole thing at once we just set 
		//the texture to the first material.
		m_lpDevice->SetTexture(0, m_lppTexs[0]);
		m_lpDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, m_nNumVertices/3);	
	}
	
	/*
	//For testing purposes draw the normals.
	m_lpDevice->SetTexture(0, L_null);
	m_lpDevice->SetFVF(BONEVERTEX_FVF);
	BONEVERTEX Normal[2];
	for(i=0; i<this->m_nNumVertices; i++)
	{
		Normal[0].x=m_pVertices[i].x;
		Normal[0].y=m_pVertices[i].y;
		Normal[0].z=m_pVertices[i].z;
		Normal[0].color=0xFFFFFFFF;
		Normal[1].x=m_pVertices[i].nx;
		Normal[1].y=m_pVertices[i].ny;
		Normal[1].z=m_pVertices[i].nz;
		Normal[1].color=0xFFFFFFFF;
		m_lpDevice->DrawPrimitiveUP(D3DPT_LINELIST, 1, &Normal, sizeof(BONEVERTEX));
	}
	*/
	return L_true;
}
