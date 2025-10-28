//Milkshape Exporter for the Legacy3D Model Formal (L3DM).
#include <windows.h>
#include <stdio.h>
#include "ML_lib.h"
#include "msLib\\msPlugIn.h"
#include "msLib\\msLib.h"
#include "lm_ms.h"

#pragma comment(lib, "msLib\\lib6\\msModelLib.lib")
#pragma comment(lib, "ML_lib_s.lib")


// ----------------------------------------------------------------------------------------------
class CLegacy3DExp:public cMsPlugIn
{
private:

public:
	CLegacy3DExp(){}
	virtual ~CLegacy3DExp(){};
	int GetType(){return cMsPlugIn::eTypeExport;}
	const char* GetTitle(){return "Legacy Engine Format";}
	int Execute(msModel* pModel);
};

int CLegacy3DExp::Execute(msModel* pModel)
{
	//Get a filename to export our data to.
	//Initialize the math library.
	ML_SetSIMDSupport(ML_INSTR_F);
	char szFile[MAX_PATH];
	szFile[0]=0;
	OPENFILENAME sf;
	memset(&sf, 0, sizeof(OPENFILENAME));
	sf.lStructSize=sizeof(OPENFILENAME);
	sf.lpstrFilter="Legacy Mesh Format (LMSH)\0*lmsh\0All Files\0*.*\0";
	sf.lpstrFile=szFile;
	sf.nMaxFile=MAX_PATH;
	sf.lpstrTitle="Legacy Model Exporter";
	sf.lpstrDefExt="lmsh";
	if(!GetSaveFileName(&sf))
		return -1;
		
	//Get the callbacks ready.
	fio_callbacks cb;
	cb.close=L_fclose;
	cb.read=L_fread;
	cb.seek=L_fseek;
	cb.write=L_fwrite;
	cb.tell=L_ftell;



	//All we need to to is load the mesh
	//from the milkshape model then call
	//the save function.
	
	//For now we are making the mesh name the same as
	//the filename, but without the lmsh or lskl extension
	L_char szName[MAX_PATH];
	L_GetShortNameFromPath(szName, szFile);
	
	CLegacyMeshMS Mesh(pModel, szName);
	if(Mesh.IsLoaded())
	{
		FILE* fout=fopen(szFile, "wb");
		//Calling CLMesh::Save will close the file.
		//The same is not true for CLMesh::Open wich
		//will leave the file open.
		if(fout)
			Mesh.Save((void*)fout, &cb);
	}

	CLegacySkelMS Skel(pModel, szName);
	if(Skel.IsLoaded())
	{
		//We change the filename to have a .lskl extension
		//for the skeleton.
		char szPath[MAX_PATH];
		L_GetPathFromPath(szPath, szFile);
		_snprintf(szFile, MAX_PATH-1, "%s%s.lskl", szPath, szName);
		FILE* fout=fopen(szFile, "wb");
		if(fout)
			Skel.Save((void*)fout,&cb);
	}
	msModel_Destroy(pModel);
	MessageBox(
		NULL, 
		"Legacy Model Exported", 
		"Legacy Model Exporter", 
		MB_OK|MB_ICONINFORMATION);
	return 0;
}

//Our export function so milkshape can perform
//the operations.
cMsPlugIn *CreatePlugIn()
{
	return new CLegacy3DExp;
}

