#include <lf_sys.h>
#include <stdio.h>
#include <string.h>
#include <process.h>

int main(int argc, char* argv[])
{
	char szCommand[MAX_F_PATH];
	char* szFunc=L_null;
	
	system("cls");
	printf("Beem Software Command Line Explorer [version 1.00]\n(C) Copyright 2006 Blaine Myers.\n\n");
	LF_Init("C:\\", 0);
	
	do
	{
		printf("%s>", LF_GetDir(L_null, MAX_F_PATH));
		gets(szCommand);
	
		szFunc=L_strtok(szCommand, " \"", '"');
		if(L_strnicmp(szFunc, "CD", 0))
		{
			LF_ChangeDir(L_strtok(L_null, L_null, 0));
		}
		
	}while (!L_strnicmp(szCommand, "QUIT", 0) && !L_strnicmp(szCommand, "EXIT", 0));
	
	return 0;
}

