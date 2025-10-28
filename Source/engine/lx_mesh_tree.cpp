#include "lx_sys.h"

void LX_MTreeStart(void* userData, const XML_Char* name, const XML_Char** atts);
void LX_MTreeEnd(void* userData, const XML_Char* name);

lg_bool LX_LoadMeshTree(const lg_path szXMLFile, lx_mesh_tree_data* pOut)
{
	pOut->bYaw180=LG_FALSE;
	pOut->fScale=1.0f;
	return LX_Process(szXMLFile, LX_MTreeStart, LX_MTreeEnd, LG_NULL, pOut);
}


void LX_MTreeStart(void* userData, const XML_Char* name, const XML_Char** atts)
{
	lx_mesh_tree_data* pData=(lx_mesh_tree_data*)userData;
	
	LX_START_TAG
		LX_TAG(mesh_tree)
		{
			LX_START_ATTR
				LX_ATTR_DWORD(pData->nNodeCount, node_count)
				LX_ATTR_DWORD(pData->nSkelCount, skel_count)
				LX_ATTR_STRING(pData->szName, name, LG_MAX_STRING)
				LX_ATTR_DWORD(pData->bYaw180, yaw_180)
				LX_ATTR_FLOAT(pData->fScale, scale)
			LX_END_ATTR
			
			pData->nSkelCount=LG_Clamp(pData->nSkelCount, 0, CLMeshTree::MT_MAX_SKELS);
			pData->nNodeCount=LG_Clamp(pData->nNodeCount, 0, CLMeshTree::MT_MAX_MESH_NODES);
		}
		LX_TAG(node)
		{
			lg_dword nID=0;
			lg_dword nParent=0;
			lg_string szName;
			lg_string szParentJoint;
			
			lg_path szMeshFile;
			lg_path szSkinFile;
			lg_path szDefSkelFile;
			
			szName[0]=0;
			szParentJoint[0]=0;
			szMeshFile[0]=0;
			szSkinFile[0]=0;
			szDefSkelFile[0]=0;
			
			LX_START_ATTR
				LX_ATTR_DWORD(nID, id)
				LX_ATTR_STRING(szName, name, LG_MAX_STRING)
				LX_ATTR_STRING(szMeshFile, mesh_file, LG_MAX_PATH)
				LX_ATTR_DWORD(nParent, parent_node)
				LX_ATTR_STRING(szParentJoint, parent_joint, LG_MAX_STRING)
				LX_ATTR_STRING(szSkinFile, skin_file, LG_MAX_PATH)
				LX_ATTR_STRING(szDefSkelFile, default_skel, LG_MAX_PATH)
			LX_END_ATTR
			
			//Copy the data in:
			if(nID<pData->nNodeCount)
			{
				pData->Nodes[nID].nParentNode=nParent;
				LG_strncpy(pData->Nodes[nID].szName, szName, LG_MAX_STRING);
				LG_strncpy(pData->Nodes[nID].szParentJoint, szParentJoint, LG_MAX_STRING);
				LG_strncpy(pData->Nodes[nID].szMeshFile, szMeshFile, LG_MAX_PATH);
				LG_strncpy(pData->Nodes[nID].szSkinFile, szSkinFile, LG_MAX_PATH);
				LG_strncpy(pData->Nodes[nID].szDefSkelFile, szDefSkelFile, LG_MAX_PATH);
				
				//Should probably print a warning if the base node nID==0 has a parent
				//node or parent joint specified.
			}
		
		}
		LX_TAG(skel)
		{
			lg_dword nID=0;
			lg_path szFile;
			szFile[0]=0;
			
			LX_START_ATTR
				LX_ATTR_STRING(szFile, file, LG_MAX_PATH)
				LX_ATTR_DWORD(nID, id)
			LX_END_ATTR
			
			//Go ahead and insert the skel file.
			if(nID<pData->nSkelCount)
			{
				LG_strncpy(pData->szSkels[nID], szFile, LG_MAX_PATH);
			}
			else
			{
				Err_Printf(
					"LX_LoadMeshTree WARNING: Skeleton id (%d) for \"%s\" out of range.",
					nID, pData->szName);
			}
		}
	LX_END_TAG
}

void LX_MTreeEnd(void* userData, const XML_Char* name)
{

}