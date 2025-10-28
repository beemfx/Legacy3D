#include "common.h"

/* L_CheckValueName makes sure that the definition name is allowed. */
int L_CheckValueName(const char* szDef, char* szNoFirstAllow, char* szNoAllow)
{
	unsigned long len=L_strlen(szDef), lenallow=0;
	unsigned long i=0, j=0;
	/* The idea behind the default checks, is that the first
		number cannot be a number, and no special characters are
		allowed in the name, this is similar to what c language
		allows. */
	char szDefNoFirstAllow[]="0987654321";
	char szDefNoAllow[]="{}[]\'\";:.,<>?/\\`~=+)(*&$^%#@!~ ";

	if(szNoFirstAllow==LG_NULL)
		szNoFirstAllow=szDefNoFirstAllow;

	if(szNoAllow==LG_NULL)
		szNoAllow=szDefNoAllow;

	/* Check to see if the first character is a no allow. */
	for(i=0, lenallow=L_strlen(szNoFirstAllow); i<lenallow; i++)
	{
		if(szDef[0]==szNoFirstAllow[i])
			return 0;
	}

	/* Now make sure no invalid characters are in the string. */
	for(i=0, lenallow=L_strlen(szNoAllow); i<lenallow; i++)
	{
		for(j=0; j<len; j++)
		{
			if(szDef[j]==szNoAllow[i])
				return 0;
		}
	}

	return 1;
}