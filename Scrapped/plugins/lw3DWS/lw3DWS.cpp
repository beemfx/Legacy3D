// lw3DWS.cpp : Defines the initialization routines for the DLL.
//

#include "stdafx.h"
#include "lw3DWS.h"

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


// Clw3DWSApp

BEGIN_MESSAGE_MAP(Clw3DWSApp, CWinApp)
END_MESSAGE_MAP()


// Clw3DWSApp construction

Clw3DWSApp::Clw3DWSApp()
{
	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only Clw3DWSApp object

Clw3DWSApp theApp;


// Clw3DWSApp initialization

BOOL Clw3DWSApp::InitInstance()
{
	CWinApp::InitInstance();

	return TRUE;
}


void WS_FUNC PluginClass(wsByte* pIn, wsDword nInSize, wsByte* pOut, wsDword nOutSize)
{
	wsDword nType=PLUGIN_EXPORT;
	memcpy(pOut, &nType, 4);
}

void WS_FUNC PluginDescription(wsByte* pIn, wsDword nInSize, wsByte* pOut, wsDword nOutSize)
{
	wsChar szTemp[]="Export Legacy World (*.lw)"" ("__DATE__" "__TIME__")";
	memcpy(pOut, szTemp, sizeof(szTemp));
}


void WS_FUNC PluginFileExtension(wsByte* pIn, wsDword nInSize, wsByte* pOut, wsDword nOutSize)
{
	wsChar szTemp[]="lw";
	memcpy(pOut, szTemp, sizeof(szTemp));
}

void WS_FUNC PluginLabel(wsByte* pIn, wsDword nInSize, wsByte* pOut, wsDword nOutSize)
{
	wsChar szTemp[]="Legacy World (*.lw)";
	memcpy(pOut, szTemp, sizeof(szTemp));
}

void WS_FUNC PluginExport(wsByte* pIn, wsDword nInSize, wsByte* pOut, wsDword nOutSize)
{
	CString szTemp;
	
	wsChar* szFilename=(wsChar*)(pIn+4);
	wsDword nDataOffset=0;
	memcpy(&nDataOffset, pIn, 4);
	wsByte* pData=(wsByte*)nDataOffset;
	szTemp.Format("Data: 0x%08X", nDataOffset);
	AfxMessageBox(szTemp);
	//wsByte* pData=(wsByte*)(*(wsDword)pIn);
	//wsByte* pData;
	//memcpy((void*)*pData, pIn, 4);
	
	AfxMessageBox(szFilename);
	szTemp.Format("The size is: %u", nOutSize);
	AfxMessageBox(szTemp);
	CFile fout;
	if(!fout.Open(szFilename, CFile::modeWrite|CFile::modeCreate))
		return;
	
	fout.Write(pData, 249140);	
	fout.Close();
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