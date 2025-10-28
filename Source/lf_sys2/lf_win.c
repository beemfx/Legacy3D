#if 0
#ifdef _DEBUG
#include <windows.h>
#include <crtdbg.h>


#define L_BEGIN_D_DUMP {_CrtMemState s1, s2, s3;_CrtMemCheckpoint(&s1);
#define L_END_D_DUMP(txt) _CrtMemCheckpoint(&s2);\
							     _CrtMemDifference(&s3, &s1, &s2);\
							     OutputDebugString("MEMORY USAGE FOR: \""txt"\":\n");\
	                       _CrtMemDumpStatistics(&s3);}
	                                         

BOOL WINAPI DllMain(HINSTANCE hInst, DWORD fdwReason, LPVOID lpvReserved)
{
	static _CrtMemState s1, s2, s3;
	static BOOL bFirst=TRUE;
	switch(fdwReason)
	{
	case DLL_PROCESS_ATTACH:
	//case DLL_THREAD_ATTACH:
		if(bFirst)
		{
			OutputDebugString("Starting lf_sys2.dll\n");
			_CrtMemCheckpoint(&s1);
			bFirst=FALSE;
		}
		break;
	case DLL_PROCESS_DETACH:
	//case DLL_THREAD_DETACH:
		OutputDebugString("Stopping lf_sys2.dll\n");
		_CrtMemCheckpoint(&s2);
		_CrtMemDifference(&s3, &s1, &s2);
		OutputDebugString("MEMORY USAGE FOR: \"lf_sy2.dll\":\n");
	   _CrtMemDumpStatistics(&s3);
	   {
			//char szTemp[10];
			//sprintf(szTemp, "%u ", g_nBlocks);
			//OutputDebugString(szTemp);
			//OutputDebugString("allocations left.\n");
	   }
		break;
	}
	return TRUE;
}
#endif
#endif