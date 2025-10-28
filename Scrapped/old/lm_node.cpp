#include "lm_node.h"

/*****************************************************************
*** The Mesh Node System is a method of frame hierarchy for
*** rendering multi-part meshes.  Note that this class doesn't
*** allocate memory at all and any child nodes need to be kept
*** track of by the user application.  
*** The usual way to do this is to create an array of nodes, and
*** use the first node in the array as the topmost parent, then
*** setup all the other nodes as necessary.  See example.
*****************************************************************/


CLMeshNode::CLMeshNode():
	m_pMesh(LG_NULL),
	m_pDefSkel(LG_NULL),
	m_pNext(LG_NULL),
	m_pChildren(LG_NULL),
	m_pParent(LG_NULL),
	m_nParentJoint(0xFFFFFFFF),
	m_pSkel(LG_NULL),
	m_nAnim(0),
	m_nAnimSpeed(0),
	m_nTime(0),
	m_nLastUpdate(0),
	m_fTime(0.0f),
	m_pPrevSkel(LG_NULL),
	m_nPrevAnim(0),
	m_fPrevTime(0.0f),
	m_nTransTime(0)
{

}
CLMeshNode::~CLMeshNode()
{
	Unload();
}

void CLMeshNode::SetCompatibleWith(CLSkelLG* pSkel)
{
	if(m_pMesh && pSkel)
		m_pMesh->SetCompatibleWith((CLSkel2*)pSkel);
}

void CLMeshNode::UpdateTime(lg_dword nTime)
{
	m_nTime=nTime;
	lg_long nMSPerAnim=m_pPrevSkel?m_nTransTime:m_nAnimSpeed;
	lg_dword nElapsed=nTime-m_nLastUpdate;
	
	if(nElapsed>(lg_dword)ML_absl(nMSPerAnim))
	{
		m_nLastUpdate=nTime;
		m_pPrevSkel=LG_NULL;
		nElapsed=0;
	}
	
	m_fTime=(float)nElapsed/(float)nMSPerAnim*99.0f;
	
	//If the animation speed is negative we
	//reverse the animation by subtracting the time
	//only we add because m_fTime will be negative.
	//In this way the animation is backwards...
	if(nMSPerAnim<0)
		m_fTime=100.0f+m_fTime;
		
	//Now update all children
	for(CLMeshNode* pNode=m_pChildren; pNode; pNode=pNode->m_pNext)
	{
		pNode->UpdateTime(nTime);
	}
}

void CLMeshNode::SetAnimation(CLSkelLG* pSkel, lg_dword nAnim, lg_long nSpeed, lg_dword nTransTime)
{
	//If we're setting the same animation, there is no need to transition.
	if((m_pSkel==pSkel) && (nAnim==m_nAnim) && (m_nAnimSpeed==nSpeed))
		return;
	m_pPrevSkel=m_pSkel;
	m_nPrevAnim=m_nAnim;
	m_fPrevTime=m_fTime;
	
	m_pSkel=pSkel;
	m_nAnim=nAnim;
	m_nAnimSpeed=nSpeed;
	m_nTransTime=nTransTime;
	m_nLastUpdate=m_nTime;
	m_fTime=0.0f;
	UpdateTime(m_nTime);
	return;
}

void CLMeshNode::Load(CLMeshLG* pMesh)
{
	//Clear currently loaded information.
	Unload();
	m_pMesh=pMesh;
}

void CLMeshNode::AddChild(CLMeshNode* pNode, lg_cstr szJoint)
{
	//Add child adds the node to a linked list child.
	pNode->m_pParent=this;
	if(this->m_pMesh)
		pNode->m_nParentJoint=this->m_pMesh->GetJointRef(szJoint);
	pNode->m_pNext=this->m_pChildren;
	this->m_pChildren=pNode;
}

void CLMeshNode::Unload()
{
	m_pSkin=LG_NULL;
	m_pMesh=LG_NULL;
	m_pNext=LG_NULL;
	m_pChildren=LG_NULL;
	m_pParent=LG_NULL;
	m_nParentJoint=0xFFFFFFFF;
	
	m_pSkel=LG_NULL;
	m_nAnim=0;
	m_nAnimSpeed=0;
	m_nTime=0;
	m_nLastUpdate=0;
	m_fTime=0.0f;
	m_pPrevSkel=LG_NULL;
	m_nPrevAnim=0;
	m_fPrevTime=0.0f;
}

void CLMeshNode::Render(const ML_MAT* matTrans)
{
	if(m_pMesh)
	{
		CLMeshLG::s_pDevice->SetTransform(D3DTS_WORLD, (D3DMATRIX*)matTrans);
		
		if(m_pPrevSkel)
		{
			m_pMesh->SetupFrame(m_nPrevAnim, m_fPrevTime, (CLSkel2*)m_pPrevSkel);
		}
		
		if(m_pPrevSkel && m_pSkel)
		{
			m_pMesh->TransitionTo(m_nAnim, m_fTime*0.01f, (CLSkel2*)m_pSkel);
		}
		else if(m_pSkel && !m_pPrevSkel)
		{
			m_pMesh->SetupFrame(m_nAnim, m_fTime, (CLSkel2*)m_pSkel);
		}
		else
		{
			m_pMesh->SetupDefFrame();
		}
		
		m_pMesh->DoTransform();
		
		
		m_pMesh->Render(m_pSkin);
	}
	
	#if 1
	//Now Render all the children... If the children have any children
	//they will get rendered as well.
	for(CLMeshNode* pNode=m_pChildren; pNode; pNode=pNode->m_pNext)
	{
		ML_MAT matTransChild;
		if(pNode->m_nParentJoint!=0xFFFFFFFF)
		{
			ML_MatMultiply(
				&matTransChild, 
				this->m_pMesh->GetJointAttachTransform(pNode->m_nParentJoint), 
				matTrans);
		}
		else
		{
			matTransChild=*matTrans;
		}
		pNode->Render(&matTransChild);
	}
	#endif
}

const CLMeshNode* CLMeshNode::GetParent()
{
	return m_pParent;
}


#include "lg_err.h"
#include "lg_mmgr.h"
#include "lg_xml.h"


//Right now for the XML mesh script parsing there
//are two parts, the first part counts how many meshes
//there are, the second part, actually loads the mesh.
//If the mesh count attribute is specified in the base
//mesh then that number is used for the mesh count.

typedef struct _XML_Mesh_Data{
	lg_dword nMesh;
	lg_dword nMaxMesh;
	CLMeshNode* pNodes;
	CLMeshNode* pParent;
	lg_str szXMLFilename;
}XML_Mesh_Data;


lg_dword LM_CountNumMeshes(lg_path szXMLScriptFile);
void XML_Mesh_Start(void* userData, const XML_Char* name, const XML_Char** atts);
void XML_Mesh_End(void* userData, const XML_Char* name);

//LE_LoadMeshNodes loads a set of mesh nodes from based on an XML script,
//the return value is the said array of nodes, node 0 being the parent
//node.  The nNumMeshes pointer will contain the number of nodes loaded.
//LE_DeleteMeshNodes is used to delete the array of mesh nodes, note that
//LE_DeleteMeshNodes should be used as opposed to delete[] because it
//insures that compatible memory freeing methods are used.  These functions
//are used for loading mesh sets for CLEntities.
#include "lf_sys2.h"

CLMeshNode* LM_LoadMeshNodes(lg_path szXMLScriptFile, lg_dword* nNumMeshes)
{
	//To load a mesh we first need to open the script file,
	//we'll open it with LF_ACCESS_MEMORY so we can pass the buffer
	//to the parser.
	LF_FILE3 fileScript=LF_Open(szXMLScriptFile, LF_ACCESS_READ|LF_ACCESS_MEMORY, LF_OPEN_EXISTING);
	if(!fileScript)
	{
		Err_Printf("LoadMesh ERROR: Could not open \"%s\" for parsing.", szXMLScriptFile);
		return LG_NULL;
	}
	
	//Now we'll create the parser.
	XML_Parser parser=XML_ParserCreate_LG(LG_NULL);
	if(!parser)
	{
		Err_Printf("LoadMesh ERROR: Could not create XML parser. (%s)", szXMLScriptFile);
		LF_Close(fileScript);
		return LG_NULL;
	}
	
	XML_Mesh_Data data;
	
	data.nMaxMesh=0;
	data.nMesh=0;
	data.pParent=LG_NULL;
	data.pNodes=LG_NULL;
	data.szXMLFilename=szXMLScriptFile;
	
	XML_ParserReset(parser, LG_NULL);
	XML_SetUserData(parser, &data);
	XML_SetElementHandler(parser, XML_Mesh_Start, XML_Mesh_End);
	
	if(!XML_Parse(parser, (const char*)LF_GetMemPointer(fileScript), LF_GetSize(fileScript), LG_TRUE))
	{
		Err_Printf("LoadMesh ERROR: Parse error at line %d: \"%s\"", 
			XML_GetCurrentLineNumber(parser),
			XML_ErrorString(XML_GetErrorCode(parser)));
				
		data.nMesh=0;
		LG_SafeDeleteArray(data.pNodes);
	}
	
	LF_Close(fileScript);
	XML_ParserFree(parser);
	if(nNumMeshes)
		*nNumMeshes=data.nMesh;
	return data.pNodes;
}

void LM_DeleteMeshNodes(CLMeshNode* pNodes)
{
	LG_SafeDeleteArray(pNodes);
}

void XML_Mesh_Start(void* userData, const XML_Char* name, const XML_Char** atts)
{
	//If this isn't a mesh tag there is no reason to parse it.
	if(strcmp(name, "mesh")!=0)
	{
		Err_Printf("MeshLoad WARNING: <%s> is not a valid mesh loading tag.", name);
		return;
	}
		
	XML_Mesh_Data* pData=(XML_Mesh_Data*)userData;
	const XML_Char* szName=NULL;
	const XML_Char* szFile=NULL;
	const XML_Char* szSkinFile=NULL;
	const XML_Char* szParentJoint=NULL;
	const XML_Char* szDefSkel=LG_NULL;
	lg_dword nMeshCount=0;
	
	for(lg_dword i=0; atts[i]; i+=2)
	{
		if(strcmp(atts[i], "name")==0)
			szName=atts[i+1];
		else if(strcmp(atts[i], "mesh_file")==0)
			szFile=atts[i+1];
		else if(strcmp(atts[i], "skin_file")==0)
			szSkinFile=atts[i+1];
		else if(strcmp(atts[i], "parent_joint")==0)
			szParentJoint=atts[i+1];
		else if(strcmp(atts[i], "default_skel")==0)
			szDefSkel=atts[i+1];
		else if(strcmp(atts[i], "mesh_count")==0)
			nMeshCount=(lg_dword)atol((char*)atts[i+1]);
	}
	
	if(pData->nMesh==0)
	{
		//If we are handling the first mesh, we need to allocate
		//memory for the mesh.
		if(pData->nMaxMesh || nMeshCount)
		{
			pData->nMaxMesh=LG_Max(pData->nMaxMesh, (lg_dword)nMeshCount);
		}
		else
		{
			Err_Printf("MeshLoad WARNING: Mesh count not specified, counting...");
			//pData->bNeedToCount=LG_TRUE;
			pData->nMaxMesh=LM_CountNumMeshes(pData->szXMLFilename);
		}
		
		if(pData->nMaxMesh)
			pData->pNodes=new CLMeshNode[pData->nMaxMesh];
			
		if(!pData->pNodes)
		{
			pData->nMaxMesh=0;
			Err_Printf("MeshLoad ERROR: Could not allocate memory for mesh nodes.");
			return;
		}
	}
	
	if((pData->nMesh>=pData->nMaxMesh))
	{
		Err_Printf("MeshLoad ERROR: Exceeded the maximum number of meshes.");
		return;
	}
	
	//Load the mesh based off the filename.
	pData->pNodes[pData->nMesh].Load(CLMMgr::MM_LoadMesh((lf_char*)szFile, 0));
	
	//If a default skeleton was specified for this node,
	//then set it.
	if(szDefSkel)
	{
		pData->pNodes[pData->nMesh].m_pDefSkel=CLMMgr::MM_LoadSkel((lf_char*)szDefSkel, 0);
		pData->pNodes[pData->nMesh].SetCompatibleWith(pData->pNodes[pData->nMesh].m_pDefSkel);
	}
	//Load the skin for the mesh
	if(szSkinFile)
	{	
		pData->pNodes[pData->nMesh].m_pSkin=CLSkinMgr::SM_Load((lg_char*)&szSkinFile[0], 0);
		if(pData->pNodes[pData->nMesh].m_pSkin)
		{
			pData->pNodes[pData->nMesh].m_pSkin->MakeCompatibleWithMesh(
				pData->pNodes[pData->nMesh].m_pMesh);
		}
		
		
	}
	
	if(pData->pParent)
		pData->pParent->AddChild(&pData->pNodes[pData->nMesh], szParentJoint);
		
	pData->pParent=&pData->pNodes[pData->nMesh];
	pData->nMesh++;
}

void XML_Mesh_End(void* userData, const XML_Char* name)
{
	XML_Mesh_Data* pData=(XML_Mesh_Data*)userData;
	
	if(pData->pParent)
		pData->pParent=(CLMeshNode*)pData->pParent->GetParent();
}


//The counter parsing methods simply count how many meshes are in the XML file
//It's not really that fast to parse the XML file twice to find out how many meshes
//there are, but it is the best way (memory wise, as a linked list could be created)
//to get memory allocated for the file.
void XML_Mesh_Start_Count(void* userData, const XML_Char* name, const XML_Char** atts)
{
	//If this isn't a mesh tag there is no reason to parse it.
	if(strcmp(name, "mesh")!=0)
	{
		Err_Printf("MeshLoad WARNING: <%s> is not a valid mesh loading tag.", name);
		return;
	}
	
	XML_Mesh_Data* pData=(XML_Mesh_Data*)userData;
		
	pData->nMaxMesh++;
}

void XML_Mesh_End_Count(void* userData, const XML_Char* name){}

lg_dword LM_CountNumMeshes(lg_path szXMLScriptFile)
{
	//To load a mesh we first need to open the script file,
	//we'll open it with LF_ACCESS_MEMORY so we can pass the buffer
	//to the parser.
	LF_FILE3 fileScript=LF_Open(szXMLScriptFile, LF_ACCESS_READ|LF_ACCESS_MEMORY, LF_OPEN_EXISTING);
	if(!fileScript)
	{
		Err_Printf("LoadMesh ERROR: Could not open \"%s\" for parsing.", szXMLScriptFile);
		return 0;
	}
	
	//Now we'll create the parser.
	XML_Parser parser=XML_ParserCreate_LG(LG_NULL);
	if(!parser)
	{
		Err_Printf("LoadMesh ERROR: Could not create XML parser. (%s)", szXMLScriptFile);
		LF_Close(fileScript);
		return 0;
	}
	
	XML_Mesh_Data data;
	
	//First we count the meshes..
	data.nMaxMesh=0;
	
	XML_SetUserData(parser, &data);
	XML_SetElementHandler(parser, XML_Mesh_Start_Count, XML_Mesh_End_Count);
	if(!XML_Parse(parser, (const char*)LF_GetMemPointer(fileScript), LF_GetSize(fileScript), LG_TRUE))
	{
		Err_Printf("LoadMesh ERROR: Parse error at line %d: \"%s\"", 
			XML_GetCurrentLineNumber(parser),
			XML_ErrorString(XML_GetErrorCode(parser)));
			
		data.nMesh=0;
		data.nMaxMesh=0;
	}
	
	if(data.nMaxMesh==0)
	{
		Err_Printf("LoadMesh ERROR: Could not find any meshes in \"%s\".", szXMLScriptFile);
		LF_Close(fileScript);
		XML_ParserFree(parser);
		return 0;
	}

	LF_Close(fileScript);
	XML_ParserFree(parser);
	
	return data.nMaxMesh;
}