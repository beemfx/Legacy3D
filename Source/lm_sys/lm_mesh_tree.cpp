#include "lm_mesh_tree.h"
#include "lx_sys.h"
#include "lg_mmgr.h"
#include "lg_skin_mgr.h"

CLMeshTree::MeshNode::MeshNode()
/*
: m_pMesh(LG_NULL)
, m_pSkin(LG_NULL)
, m_pDefSkel(LG_NULL)
, m_pSkel(LG_NULL)
, m_nAnim(0)
, m_fAnimProgress(0.0f)
, m_nAnimSpeed(0)
, m_pPrevSkel(LG_NULL)
, m_nPrevAnim(0)
, m_fPrevAnimProgress(0.0f)
, m_fTransitionTime(0.0f)
, m_nParentJoint(0)
*/
{
	Clear();
}

CLMeshTree::MeshNode::~MeshNode()
{
	//Don't need to do anything as no
	//memory was allocated.
}

lg_void CLMeshTree::MeshNode::Load(CLMeshLG* pMesh, CLSkin* pSkin, CLSkelLG* pSkel)
{
	Clear();
	m_pMesh=pMesh;
	m_pSkin=pSkin;
	m_pDefSkel=pSkel;	
	m_pSkel=m_pDefSkel;
}

lg_void CLMeshTree::MeshNode::Clear()
{
	m_pMesh=LG_NULL;
	m_pSkin=LG_NULL;
	m_pDefSkel=LG_NULL;
	m_pSkel=LG_NULL;
	m_pNextSkel=LG_NULL;
	m_nAnim=0;
	m_fAnimProgress=0.0f;
	m_fAnimSpeed=1.0f;
	m_fElapsed=0.0f;
	m_pPrevSkel=LG_NULL;
	m_nPrevAnim=0;
	m_fPrevAnimProgress=0.0f;
	m_fPostTransSpeed=1.0f;
	
	m_nNumChildren=0;
	m_nParentJoint=0xFFFFFFFF;
	
	for(lg_dword i=0; i<MT_MAX_MESH_NODES; i++)
	{
		m_pChildren[i]=LG_NULL;
	}
}

lg_void CLMeshTree::MeshNode::Render(ml_mat* pMatWorldTrans)
{
	m_pMesh->s_pDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX*)pMatWorldTrans);
	
	if(m_pMesh)
	{
		m_pMesh->DoTransform();
		m_pMesh->Render(m_pSkin);
	}
		
	//Now Render all the children... If the children have any children
	//they will get rendered as well.
	for(lg_dword i=0; i<m_nNumChildren; i++)
	{
		ML_MAT matTrans;
		matTrans=*pMatWorldTrans;
	
		//Note that if the parent joint reference is invalid (0xFFFFFFFF)
		//then we'll end up multiplying by the identiy matrix
		//so we are alright.
		ML_MatMultiply(
			&matTrans,
			m_pMesh->GetJointAttachTransform(m_pChildren[i]->m_nParentJoint),
			pMatWorldTrans);
		
		m_pChildren[i]->Render(&matTrans);
	}
}

lg_void CLMeshTree::MeshNode::AddChild(MeshNode* pChild, lg_str szJoint)
{
	if(m_nNumChildren>=CLMeshTree::MT_MAX_MESH_NODES)
		return;
		
	m_pChildren[m_nNumChildren]=pChild;
	m_nNumChildren++;
	pChild->m_nParentJoint=m_pMesh->GetJointRef(szJoint);
}

CLMeshTree::CLMeshTree()
: CLMBase()
{
	//Unload will clear all the data:
	Unload();
}

CLMeshTree::~CLMeshTree()
{
}

lg_void CLMeshTree::GetAttachTransform(ml_mat* pOut, lg_dword nChild)
{
	//Not really quite right as this gets the mesh attached to the
	//base node.
	if(m_MeshNodes[0].m_nNumChildren!=0)
	{
		nChild=LG_Clamp(nChild, 0, m_MeshNodes[0].m_nNumChildren-1);
		*pOut=*(m_MeshNodes[0].m_pMesh->GetJointAttachTransform(m_MeshNodes[0].m_pChildren[nChild]->m_nParentJoint));
		ML_MatMultiply(pOut, pOut, &m_matBaseTrans);
	}
	else
	{
		ML_MatIdentity(pOut);
	}
}

lg_bool CLMeshTree::Load(lg_path szFile)
{
	Unload();
	
	lx_mesh_tree_data mtd;
	lg_bool bRes=LX_LoadMeshTree(szFile, &mtd);
	
	if(bRes)
	{
		//Now we must construct the mesh tree:
		m_nNumNodes=mtd.nNodeCount;
		m_nNumSkels=mtd.nSkelCount;
		
		static ml_mat matTemp;
		//Set up the base transform (should include scale):
		if(mtd.bYaw180)
			ML_MatRotationY(&matTemp, ML_PI);
		else
			ML_MatIdentity(&matTemp);
			
		ML_MatMultiply(&m_matBaseTrans, &matTemp, &m_matBaseTrans);
		
		if(mtd.fScale!=1.0f)
			ML_MatScaling(&matTemp, mtd.fScale, mtd.fScale, mtd.fScale);
		else
			ML_MatIdentity(&matTemp);
			
		ML_MatMultiply(&m_matBaseTrans, &matTemp, &m_matBaseTrans);
		//First we'll load all the resources:
		
		//Setup the nodes:
		for(lg_dword i=0; i<m_nNumNodes; i++)
		{
			CLMeshLG* pMesh=CLMMgr::MM_LoadMesh(mtd.Nodes[i].szMeshFile, 0);
			CLSkelLG* pDefSkel=CLMMgr::MM_LoadSkel(mtd.Nodes[i].szDefSkelFile, 0);
			CLSkin*   pSkin=CLSkinMgr::SM_Load(mtd.Nodes[i].szSkinFile, 0);
			pMesh->SetCompatibleWith(pDefSkel);
			pSkin->MakeCompatibleWithMesh(pMesh);
			
			m_MeshNodes[i].Load(pMesh, pSkin, pDefSkel);
		}
		
		//Load the skels:
		for(lg_dword i=0; i<m_nNumSkels; i++)
		{
			m_pSkels[i]=CLMMgr::MM_LoadSkel(mtd.szSkels[i], 0);
		}
		
		//Now we should set up the heirarchy.
		//We start at 1 because 0 is the base node,
		//so even if a parent was specified it doesn't
		//matter.
		for(lg_dword i=1; i<m_nNumNodes; i++)
		{
			m_MeshNodes[mtd.Nodes[i].nParentNode].AddChild(
				&m_MeshNodes[i],
				mtd.Nodes[i].szParentJoint);
		}
	}
	
	return bRes;
}

lg_void CLMeshTree::Unload()
{
	for(lg_dword i=0; i<MT_MAX_MESH_NODES; i++)
	{
		m_MeshNodes[i].Clear();
	}
	
	for(lg_dword i=0; i<MT_MAX_SKELS; i++)
	{
		m_pSkels[i]=0;
	}
	
	m_nNumNodes=0;
	m_nNumSkels=0;
	m_nFlags=0;
	
	ML_MatIdentity(&m_matBaseTrans);
}

lg_void CLMeshTree::Render(ml_mat *pMatWorldTrans)
{
	static ml_mat matTrans;
	//We only need to render the base node, all other
	//notes should get render subsequentiall.
	ML_MatMultiply(&matTrans, &m_matBaseTrans, pMatWorldTrans);
	m_MeshNodes[0].Render(&matTrans);
}

lg_void CLMeshTree::Update(lg_float fDeltaTimeSec)
{
	for(lg_dword i=0; i<m_nNumNodes; i++)
	{
		static MeshNode* pCur;
		pCur=&m_MeshNodes[i];
		if(!pCur->m_pSkel)
			continue;
			
		//This is just here as a test.
		pCur->m_fElapsed+=fDeltaTimeSec;
		if(pCur->m_fElapsed>pCur->m_fAnimSpeed)
		{
			//Since it is possible that a transition completed
			//we'll set the speed to teh post trans speed, which
			//should contain the desired speed.
			pCur->m_pPrevSkel=LG_NULL;
			
			//In the case that we have the next animation in que,
			//we'll need to transition to that.
			if(pCur->m_pNextSkel)
			{
				pCur->m_pPrevSkel=pCur->m_pSkel;
				pCur->m_nPrevAnim=pCur->m_nAnim;
				pCur->m_fPrevAnimProgress=1.0f;
				pCur->m_pSkel=pCur->m_pNextSkel;
				pCur->m_nAnim=pCur->m_nNextAnim;
				pCur->m_pNextSkel=LG_NULL;
			}
			else
			{
				pCur->m_fAnimSpeed=pCur->m_fPostTransSpeed;
			}
			pCur->m_fElapsed=0.0f;
		}
		
		//There are two options, either we are transitioning,
		//or we are doing a regular animation.  If pCur->m_pPrevSkel
		//is not null then we are transitioning.
		if(pCur->m_pPrevSkel)
		{
			pCur->m_pMesh->SetupFrame(pCur->m_nPrevAnim, pCur->m_fPrevAnimProgress, pCur->m_pPrevSkel);
			pCur->m_fAnimProgress=(pCur->m_fElapsed/pCur->m_fAnimSpeed)*100.0f;
			pCur->m_pMesh->TransitionTo(pCur->m_nAnim, pCur->m_fAnimProgress*0.01f, pCur->m_pSkel);
		}
		else
		{
			pCur->m_fAnimProgress=(pCur->m_fElapsed/pCur->m_fAnimSpeed)*100.0f;
			pCur->m_pMesh->SetupFrame(pCur->m_nAnim, pCur->m_fAnimProgress, pCur->m_pSkel);
		}
		
		/*
		if(i==0)
			Err_MsgPrintf("Time: %.2f Progress: %.2f", pCur->m_fElapsed, pCur->m_fAnimProgress);
		*/
	}
}

lg_void CLMeshTree::SetAnimation(
	lg_dword nNode, 
	lg_dword nSkel, 
	lg_dword nAnim, 
	lg_float fSpeed, 
	lg_float fTransitionTime)
{
	//We'll clamp some values, just so we don't go out of bounds.
	nNode=LG_Clamp(nNode, 0, m_nNumNodes-1);
	nSkel=LG_Clamp(nSkel, 0, m_nNumSkels-1);
	
	static MeshNode* pCur;
	pCur=&m_MeshNodes[nNode];
	
	if((pCur->m_pSkel==m_pSkels[nSkel]) && (pCur->m_nAnim==nAnim))
	{
		//If we set the animation to the current animation,
		//then we need to insure that we aren't queing a new animation
		//and further, that we don't reset the animation, but we will
		//changed the speed (which won't take effect until the current
		//animation completes).
		pCur->m_pNextSkel=LG_NULL;
		pCur->m_fPostTransSpeed=fSpeed;
	}
	else if(pCur->m_pPrevSkel)
	{
		//If we are currently transitioning, then we'll que up
		//the new animation (that is we wait for the current
		//transition to complete, then we transition to the 
		//new animation (note that if the new animation is the
		//same animation we are transitioning to then the
		//case above will occur.
		pCur->m_pNextSkel=m_pSkels[nSkel];
		pCur->m_nNextAnim=nAnim;
		pCur->m_fPostTransSpeed=fSpeed;
		#if 0
		//We won't use the fTransitionTime value because
		//it could cause some jumping, when we change the
		//m_fAnimSpeed.
		pCur->m_fAnimSpeed=fTransitionTime;
		#endif
	}
	else
	{
		//If this is just a regular transition then we
		//need only set the new values, and save the old values
		//for the transition:
		
		//Set the current values, to the old values:
		pCur->m_fPrevAnimProgress=pCur->m_fAnimProgress;
		pCur->m_pPrevSkel=pCur->m_pSkel;
		pCur->m_nPrevAnim=pCur->m_nAnim;
		
		//Set the new values
		pCur->m_fAnimProgress=0.0f;
		//We save the transition time as teh speed, because it will
		//be the speed during the transition.
		pCur->m_fAnimSpeed=fTransitionTime;
		pCur->m_fElapsed=0.0f;
		//We'll save the future speed, the anim speed will change
		//to this after the transition is complete.
		pCur->m_fPostTransSpeed=fSpeed;
		pCur->m_nAnim=nAnim;
		pCur->m_pSkel=m_pSkels[nSkel];
	}
}

lg_bool CLMeshTree::Serialize(lg_void*, ReadWriteFn, RW_MODE)
{
	return LG_FALSE;
}


#include "lw_ai_sdk.h"
lg_void CLMeshTree::SetAnimation(lg_dword nFlags)
{
	if(!LG_CheckFlag(nFlags, LWAI_ANIM_CHANGE))
		return;
	
	SetAnimation(
		LWAI_GetAnimNode(nFlags),
		LWAI_GetAnimSkel(nFlags),
		LWAI_GetAnimAnim(nFlags),
		LWAI_GetAnimSpeed(nFlags),
		LWAI_GetAnimTrans(nFlags));
}
