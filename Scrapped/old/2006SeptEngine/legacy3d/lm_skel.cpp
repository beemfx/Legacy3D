#include <stdio.h>
#include "lm_sys.h"


//Alright here is the skeleton class.
//Further this needs to support some kind of
//mechanism for pointing out different animations
//based on a range of animations.

//Also need to change the way the model gets rendered.
//Either CLegacyMesh needs to have a RenderWithSkel funciton.
//Or CLegacySkeleton needs to have a RenderModel functions.

CLegacySkeleton::CLegacySkeleton():
m_nID(0),
m_nVersion(0),
m_nNumJoints(0),
m_nNumKeyFrames(0),
m_pBaseSkeleton(L_null),
m_pKeyFrames(L_null),
m_nTotalSize(0),
m_bLoaded(L_false)
{
	
}

CLegacySkeleton::~CLegacySkeleton()
{
	Unload();
}

L_bool CLegacySkeleton::Unload()
{
	this->DeallocateMemory();

	m_nID=0;
	m_nVersion=0;
	m_nNumJoints=0;
	m_nNumKeyFrames=0;
	m_nTotalSize=0;
	m_bLoaded=L_false;
	return L_true;
}

L_bool CLegacySkeleton::Load(void* file, LMESH_CALLBACKS* pcb)
{
	L_dword i=0, j=0;
	LMESH_CALLBACKS cb;

	cb.close=(int (__cdecl*)(void*))fclose;
	cb.read=(unsigned int (__cdecl *)(void *,unsigned int,unsigned int,void *))fread;
	cb.seek=(int (__cdecl *)(void *,long,int))fseek;
	cb.tell=(int (__cdecl *)(void *))ftell;

	if(!pcb)
	{
		pcb=&cb;
	}

	//Unload in case a skeleton is already loaded.
	Unload();

	//Read the header.
	pcb->read(&m_nID, 4, 1, file);
	pcb->read(&m_nVersion, 4, 1, file);
	pcb->read(&m_nNumJoints, 4, 1, file);
	pcb->read(&m_nNumKeyFrames, 4, 1, file);

	if(m_nID!=LSKEL_ID || m_nVersion!=LSKEL_VERSION)
	{
		pcb->close(file);
		return L_false;
	}

	//Allocate memory for the frames and base skeleton.
	if(!AllocateMemory())
		return L_false;

	//Read the base skeleton.
	for(i=0; i<m_nNumJoints; i++)
	{
		//Note that we only read the size of LMESH_JOINT because that
		//is all that is stored.  The rest of the information (namely
		//the rotatoin matrix) is built afterwards when CalcExData is
		//called.
		pcb->read(&m_pBaseSkeleton[i], sizeof(LMESH_JOINT), 1, file);
	}

	//Read the key frames.
	
	for(i=0; i<m_nNumKeyFrames; i++)
	{
		for(j=0; j<m_nNumJoints; j++)
		{
			//We only read the size of LMESH_JOINTPOS cause that is all that is
			//stored.  The rest of the structure is built when CalcExData is called.
			pcb->read(&this->m_pKeyFrames[i].pJointPos[j], sizeof(LMESH_JOINTPOS), 1, file);
		}
	}
	
	pcb->close(file);
	m_bLoaded=L_true;
	//We will now get all the calculations for the extra data (the local
	//and final matrices for the base skeleton and keyframes).
	//this->CalcExData();
	this->CalcExData();
	return m_bLoaded;
}

L_bool CLegacySkeleton::Save(char* szFilename, L_bool bAppend)
{
	FILE* fout=L_null;
	char mode[4];
	L_dword i=0;

	if(!bAppend)
		L_strncpy(mode, "wb", 4);
	else
		L_strncpy(mode, "rb+", 4);

	if(!m_bLoaded)
		return L_false;

	if(!m_pBaseSkeleton || !m_pKeyFrames)
		return L_false;

	fout=fopen(szFilename, mode);
	if(!fout)
		return L_false;

	//Seek to the end of the file (in case we are appending).
	fseek(fout, 0, SEEK_END);
	
	//Write the header
	fwrite(&m_nID, 4, 1, fout);
	fwrite(&m_nVersion, 4, 1, fout);
	fwrite(&m_nNumJoints, 4, 1, fout);
	fwrite(&m_nNumKeyFrames, 4, 1, fout);
	


	//Write the base skeleton.
	for(i=0; i<m_nNumJoints; i++)
	{
		//We only write the size of LMESH_JOINT because the other
		//data is built when we load the model.
		fwrite(&this->m_pBaseSkeleton[i], sizeof(LMESH_JOINT), 1, fout);
	}
	//Write the key frames.
	L_dword j=0;
	for(i=0; i<m_nNumKeyFrames; i++)
	{
		for(j=0; j<m_nNumJoints; j++)
		{
			fwrite(&this->m_pKeyFrames[i].pJointPos[j], sizeof(LMESH_JOINTPOS), 1, fout);
		}
	}

	//Write the footer (which is the total size of the skeleton then the ID again).
	m_nTotalSize=16+m_nNumJoints*sizeof(LMESH_JOINT)+m_nNumJoints*m_nNumKeyFrames*sizeof(LMESH_JOINTPOS)+8;
	fwrite(&m_nTotalSize, 4, 1, fout);
	fwrite(&m_nID, 4, 1, fout);

	fclose(fout);
	m_bLoaded=L_true;

	return L_true;
}

L_bool CLegacySkeleton::AllocateMemory()
{
	L_dword i=0, j=0;
	//Allocate memory for the frames and base skeleton.
	m_pKeyFrames=new LMESH_KEYFRAME[m_nNumKeyFrames];
	
	if(!m_pKeyFrames)
		return L_false;
	
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
			return L_false;
		}
	}

	m_pBaseSkeleton=new LMESH_JOINTEX[m_nNumJoints];
	if(!m_pBaseSkeleton)
	{
		for(i=0; i<m_nNumKeyFrames; i++)
		{
			L_safe_delete_array(m_pKeyFrames[i].pJointPos);
		}
		L_safe_delete_array(m_pKeyFrames);
		return L_false;
	}

	return L_true;
}

void CLegacySkeleton::DeallocateMemory()
{
	L_dword i=0;

	L_safe_delete_array(this->m_pBaseSkeleton);
	for(i=0; i<m_nNumKeyFrames; i++)
	{
		L_safe_delete_array(this->m_pKeyFrames[i].pJointPos);
	}
	L_safe_delete_array(this->m_pKeyFrames);
}


L_dword CLegacySkeleton::GetNumFrames()
{
	return m_nNumKeyFrames;
}


L_bool CLegacySkeleton::CalcExData()
{
	L_dword i=0, j=0;

	if(!m_bLoaded)
		return L_false;

	//Firstly we must convert Euler angles for the base skeleton and
	//keyframes to matrices.
	//Read the base skeleton.
	for(i=0; i<m_nNumJoints; i++)
	{

		//Now create the rotation matrixes (in the final format the rotation
		//matrices will probably be stored instead of the euler angles.
		LM_EulerToMatrix(&m_pBaseSkeleton[i].Local, (float*)&m_pBaseSkeleton[i].rotation);
		//Now stick the translation into the matrix.
		m_pBaseSkeleton[i].Local._41=m_pBaseSkeleton[i].position[0];
		m_pBaseSkeleton[i].Local._42=m_pBaseSkeleton[i].position[1];
		m_pBaseSkeleton[i].Local._43=m_pBaseSkeleton[i].position[2];

		m_pBaseSkeleton[i].nJointRef=i;
	}

	//Read the key frames.
	
	for(i=0; i<m_nNumKeyFrames; i++)
	{
		for(j=0; j<m_nNumJoints; j++)
		{
			//pcb->read(&this->m_pKeyFrames[i].pJointPos[j], sizeof(LMESH_JOINTPOS), 1, file);
			LM_EulerToMatrix(&m_pKeyFrames[i].pJointPos[j].Local, (float*)&m_pKeyFrames[i].pJointPos[j].rotation);
			m_pKeyFrames[i].pJointPos[j].Local._41=m_pKeyFrames[i].pJointPos[j].position[0];
			m_pKeyFrames[i].pJointPos[j].Local._42=m_pKeyFrames[i].pJointPos[j].position[1];
			m_pKeyFrames[i].pJointPos[j].Local._43=m_pKeyFrames[i].pJointPos[j].position[2];
		}
	}

	//Calculate the final matrices for the base skeleton.
	//This is simply a matter of multiplying each joint by
	//all of it's parent's matrices.
	for(i=0; i<m_nNumJoints; i++)
	{
		LMESH_JOINTEX* pTemp=&m_pBaseSkeleton[i];
		m_pBaseSkeleton[i].Final=pTemp->Local;
		while(pTemp->nParentBone)
		{
			pTemp=&m_pBaseSkeleton[pTemp->nParentBone-1];
			m_pBaseSkeleton[i].Final*=m_pBaseSkeleton[pTemp->nJointRef].Local;
		}
	}

	//We calculate the final transformation matrix for each joint for each
	//frame now, that way we don't have to calculate them on the fly.  It
	//takes more memory this way, but the real time rendering is faster because
	//it isn't necessary to calculate each frames matrix every frame, it is only
	//necessary to interploate the joint matrixes for the given frame.
	
	//For each joint...
	for(i=0; i<m_nNumJoints; i++)
	{
		//For each frame for each joint...
		for(j=0; j<m_nNumKeyFrames; j++)
		{
			//1. Obtain the base joint for that joint.
			LMESH_JOINTEX* pTemp=&m_pBaseSkeleton[i];
			//2. Start out by making the final translation for the frame the local frame joint
			//location multiplyed the by the local base joint.
			m_pKeyFrames[j].pJointPos[i].Final=m_pKeyFrames[j].pJointPos[i].Local*pTemp->Local;
			//3. Then if the joint has a parent...
			while(pTemp->nParentBone)
			{
				//3 (cont'd). It is necessary to multiply the final frame matrix
				//by the same calculation in step 2 (frame pos for the frame * local pos for the frame).
				pTemp=&m_pBaseSkeleton[pTemp->nParentBone-1];
				m_pKeyFrames[j].pJointPos[i].Final*=m_pKeyFrames[j].pJointPos[pTemp->nJointRef].Local*pTemp->Local;
			}

			//4. Fianlly the final position needs to be multiplied by the
			//final base position's inverse so that the transformation is
			//relative to the joint's location and not to 0,0,0.
			D3DXMATRIX MI;
			D3DXMatrixInverse(&MI, L_null, &m_pBaseSkeleton[i].Final);
			m_pKeyFrames[j].pJointPos[i].Final=MI*m_pKeyFrames[j].pJointPos[i].Final;
		}
		
	}
	return L_true;
}


