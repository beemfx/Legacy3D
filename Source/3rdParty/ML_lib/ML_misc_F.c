#include "ML_lib.h"
#include <math.h>

#pragma warning(disable:4716) //Disable the "must return a value" warning.

ml_dword ML_FUNC ML_NextPow2(const ml_dword n)
{
	ml_dword nRes;
	
	if(n>0x80000000)
		return n;
		
	nRes=0x00000001;
	while(nRes<n)
	{
		nRes<<=1;
	}
	return nRes;
}

ml_dword ML_FUNC ML_Pow2(const ml_byte n)
{
	return 0x00000001<<n;
}

ml_float ML_FUNC ML_sqrtf(const ml_float f)
{
	return sqrtf( f );
}

ml_float ML_FUNC ML_cosf(const ml_float f)
{
	return cosf(f);
}

ml_float ML_FUNC ML_sinf(const ml_float f)
{
	return sinf(f);
}

ml_float ML_FUNC ML_tanf(const ml_float f)
{
	return ML_sinf(f)/ML_cosf(f);
}

ml_float ML_FUNC ML_acosf(const ml_float f)
{
	return acosf( f );
}

ml_float ML_FUNC ML_asinf(const ml_float f)
{
	return asinf( f );
}

ml_float ML_FUNC ML_sincosf(const ml_float f, ml_float *cos)
{
	*cos = cosf( f );
	return sinf( f );
}

ml_long ML_FUNC ML_absl(ml_long l)
{
	return abs( l );
}

ml_float ML_FUNC ML_absf(ml_float f)
{
	return fabsf( f );
}

