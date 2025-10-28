// lmsh3DWS.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "lmsh3DWS.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//
//TODO: If this DLL is dynamically linked against the MFC DLLs,
//		any functions exported from this DLL which call into
//		MFC must have the AFX_MANAGE_STATE macro added at the
//		very beginning of the function.
//
//		For example:
//
//		extern "C" BOOL PASCAL EXPORT ExportedFunction()
//		{
//			AFX_MANAGE_STATE(AfxGetStaticModuleState());
//			// normal function body here
//		}
//
//		It is very important that this macro appear in each
//		function, prior to any calls into MFC.  This means that
//		it must appear as the first statement within the 
//		function, even before any object variable declarations
//		as their constructors may generate calls into the MFC
//		DLL.
//
//		Please see MFC Technical Notes 33 and 58 for additional
//		details.
//


// Clmsh3DWSApp

BEGIN_MESSAGE_MAP(Clmsh3DWSApp, CWinApp)
END_MESSAGE_MAP()


// Clmsh3DWSApp construction

Clmsh3DWSApp::Clmsh3DWSApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only Clmsh3DWSApp object

Clmsh3DWSApp theApp;


// Clmsh3DWSApp initialization

BOOL Clmsh3DWSApp::InitInstance()
{
	CWinApp::InitInstance();

	return TRUE;
}


#include <stdio.h>
#include "lm_xexp.h"


void WS_FUNC PluginMeshLoad(wsByte* pIn, wsDword nInSize, wsByte*  pOut, wsDword nOutSize)
{
	wsString szFilename=(wsString)pIn;
	
	//We are going to convert the file to the x format,
	//then use the x plugin to load the file, so
	//we need to first load the x library.
	HMODULE hSMF=LoadLibrary(_T("plugins\\x.dll"));
	if(!hSMF)
	{
		AfxMessageBox(_T("Could not load X plugin."), MB_ICONERROR);
		return;	
	}
	WS_PLUGIN_FUNC pPluginMeshLoadX=(WS_PLUGIN_FUNC)GetProcAddress(hSMF, _T("PluginMeshLoad"));
	if(!pPluginMeshLoadX)
	{
		AfxMessageBox(_T("Could not get address of PluginMeshLoad."), MB_ICONERROR);
		return;
	}
	
	CLMeshXExp cMesh;
	
	#if 0
	void* file=L_fopen(szFilename, LOPEN_OPEN_EXISTING, LOPEN_READ);
	if(!file)
		return;
	
	if(!cMesh.Load(file, L_fread, ""))
	{
		L_fclose(file);
		return;	
	}
	L_fclose(file);
	#endif
	FILE* file=fopen(szFilename, "rb");
	if(!file)
		return;
	lg_bool bResult=cMesh.Load(file, (LM_RW_FN)fread, "");
	fclose(file);
	if(!bResult)
		return;
	
	//Create a temp file to save to.
	CString szPath, szTempFilename;
	GetTempPath(MAX_PATH, szPath.GetBuffer(MAX_PATH+1));
	GetTempFileName(szPath, _T("xcn"), 0, szTempFilename.GetBuffer(MAX_PATH));
	
	bResult=cMesh.SaveAsX(szTempFilename.GetBuffer());
	
	cMesh.Unload();
	
	if(!bResult)
		return;
		
	pPluginMeshLoadX((wsByte*)szTempFilename.GetBuffer(), szTempFilename.GetLength()+1, pOut, nOutSize);
	//Delete the temporary file.
	DeleteFile(szTempFilename);
	//Free the x plugin library.
	FreeLibrary(hSMF);
	return;
}

void WS_FUNC PluginClass(wsByte* pIn, wsDword nInSize, wsByte* pOut, wsDword nOutSize)
{
	wsDword nType=PLUGIN_MESHLOAD;
	memcpy(pOut, &nType, 4);
}


void WS_FUNC PluginDescription(wsByte* pIn, wsDword nInSize, wsByte* pOut, wsDword nOutSize)
{
	wsChar szTemp[]="Load Legacy meshes (*.lmsh)"" ("__DATE__" "__TIME__")";
	memcpy(pOut, szTemp, sizeof(szTemp));
}


void WS_FUNC PluginFileExtension(wsByte* pIn, wsDword nInSize, wsByte* pOut, wsDword nOutSize)
{
	wsChar szTemp[]="lmsh";
	memcpy(pOut, szTemp, sizeof(szTemp));
}



/*
void PluginExport(unsigned char* inbuffer,
          int insize,
          unsigned char* outbuffer,
          int outsize)
{
  char buffer[1024];
  unsigned char* MapData=(unsigned char*)*((unsigned int *)inbuffer);
  unsigned short* mapversion = (unsigned short*) MapData;
  unsigned char*  mapflags = (unsigned char*) (MapData+2);
  int* name_count = (int*) (MapData+3);
  int* name_offset = (int*) (MapData+7);
  int* object_count = (int*) (MapData+11);
  int* object_offset = (int*) (MapData+15);
  char** NameTable=new char*[*name_count];
  MapData=(unsigned char*)*((unsigned int *)inbuffer)+*name_offset;
  for(int i=0;i<*name_count;++i)
    {
      NameTable[i]=(char*)MapData;
      MapData+=strlen(NameTable[i])+1;
      sprintf(buffer,"%s",NameTable[i]);
      MessageBox(NULL,buffer,"PluginExport",MB_OK);
    }
  MapData=(unsigned char*)*((unsigned int *)inbuffer)+*object_offset;
  for(int i=0;i<*object_count;++i)
    {
      sprintf(buffer,"Object %d %s size %d",i,
          NameTable[*((unsigned int*)MapData)-1],
          *((int*)MapData+1));
      MessageBox(NULL,buffer,"PluginExport",MB_OK);
      MapData+=(*((int*)MapData+1))+8;
    }
  FILE* exp = fopen((const char*)inbuffer+4,"wb");
  fwrite((unsigned char*)*((unsigned int *)inbuffer)+*object_offset,sizeof(unsigned char),512,exp);
  fclose(exp);
  delete[] NameTable;
}
*/
