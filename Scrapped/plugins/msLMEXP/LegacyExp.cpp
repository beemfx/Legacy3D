//Milkshape Exporter for the Legacy Mesh and Skeleton Formats (lmsh, lskl).
#include <windows.h>

#include "stdafx.h"
#include "ExportDialog.h"

#pragma comment(lib, "msLib\\lib\\msModelLib.lib")
#pragma comment(lib, "ML_lib.lib")


// --------------------------------------------------------
class CLegacyExp:public cMsPlugIn
{
private:

public:
	CLegacyExp(){}
	virtual ~CLegacyExp(){};
	int GetType(){return cMsPlugIn::eTypeExport;}
	const char* GetTitle(){return "Legacy Engine Format...";}
	int Execute(msModel* pModel);
};

int CLegacyExp::Execute(msModel* pModel)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
	//Get a filename to export our data to.
	//Initialize the math library.
	ML_Init(ML_INSTR_F);
	
	CExportDialog dlgExport;
	
	if(msModel_GetMeshCount(pModel)<1)
		dlgExport.m_bMeshExport=FALSE;
	if(msModel_GetBoneCount(pModel)<1)
		dlgExport.m_bSkelExport=FALSE;
	
	do
	{
		if(dlgExport.DoModal()==IDCANCEL)
		{
			msModel_Destroy(pModel);
			return 0;
		}
		
		BOOL bError=FALSE;
		if(dlgExport.m_bMeshExport)
		{
			bError=bError||dlgExport.m_szMeshName.IsEmpty();
			bError=bError||dlgExport.m_szMeshPath.IsEmpty();
		}
		if(dlgExport.m_bSkelExport)
		{
			bError=bError||dlgExport.m_szSkelName.IsEmpty();
			bError=bError||dlgExport.m_szSkelPath.IsEmpty();
		}
		
		if(bError)
		{
			AfxMessageBox(_T("A required parameter was not specified!"));
			continue;
		}		
		break;
	}while(TRUE);
	
	//All we need to to is load the mesh
	//from the milkshape model then call
	//the save function.
	if(dlgExport.m_bMeshExport)
	{
		CLMeshMS Mesh(pModel, dlgExport.m_szMeshName.GetBuffer());
		if(Mesh.IsLoaded())
		{
			Mesh.Save(dlgExport.m_szMeshPath.GetBuffer());
		}
		Mesh.Unload();
	}

	if(dlgExport.m_bSkelExport)
	{
		CLSkelMS Skel(pModel, dlgExport.m_szSkelName.GetBuffer());
		if(Skel.IsLoaded())
		{
			Skel.Save(dlgExport.m_szSkelPath.GetBuffer());
		}
		Skel.Unload();
	}
	MessageBox(
		NULL, 
		_T("Legacy Object(s) Exported"), 
		_T("Legacy Engine Exporter"), 
		MB_OK|MB_ICONINFORMATION);
	
	msModel_Destroy(pModel);
	return 0;
}

//Our export function so milkshape can perform
//the operations.
cMsPlugIn *CreatePlugIn()
{
	return new CLegacyExp;
}

