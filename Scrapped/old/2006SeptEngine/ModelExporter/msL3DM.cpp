//Milkshape Exporter for the Legacy3D Model Formal (L3DM).
#include <windows.h>
#include "msLib\\msPlugIn.h"
#include "lm_ms.h"
#include "msLib\\msLib.h"
#pragma comment(lib, "msLib\\lib6\\msModelLib.lib")

#pragma comment(lib, "d3dx9.lib")


// ----------------------------------------------------------------------------------------------
class CLegacy3DExp:public cMsPlugIn
{
private:

public:
	CLegacy3DExp(){}
	virtual ~CLegacy3DExp(){};
	int GetType(){return cMsPlugIn::eTypeExport/*|cMsPlugIn::eNormalsAndTexCoordsPerTriangleVertex*/;}
	const char* GetTitle(){return "Legacy 3D Format";}
	int Execute(msModel* pModel);
};

int CLegacy3DExp::Execute(msModel* pModel)
{
	//Get a filename to export our data to.
	char szFile[MAX_PATH];
	szFile[0]=0;
	OPENFILENAME sf;
	memset(&sf, 0, sizeof(OPENFILENAME));
	sf.lStructSize=sizeof(OPENFILENAME);
	sf.lpstrFilter="Legacy 3D Model Format (L3DM)\0*l3dm\0All Files\0*.*\0";
	sf.lpstrFile=szFile;
	sf.nMaxFile=MAX_PATH;
	sf.lpstrTitle="Legacy 3D Model Exporter";
	sf.lpstrDefExt="l3dm";
	if(!GetSaveFileName(&sf))
		return -1;



	//All we need to to is load the mesh
	//from the milkshape model then call
	//the save function.
	CLegacyMeshMS Mesh(pModel);
	Mesh.Save(szFile);
	CLegacySkelMS Skel(pModel);
	Skel.Save(szFile, L_true);
	msModel_Destroy(pModel);
	MessageBox(
		NULL, 
		"Legacy 3D Model Exported", 
		"Legacy 3D Model Exporter", 
		MB_OK|MB_ICONINFORMATION);
	return 0;
}

//Our export function so milkshape can perform
//the operations.
cMsPlugIn *CreatePlugIn()
{
	return new CLegacy3DExp;
}

