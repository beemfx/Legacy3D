#define ML_LIB_MAIN
#include "ML_lib.h"
#include "ML_vec3.h"
#include "ML_mat.h"

ml_dword FLOAT_SSE_NO_W_MASK[]   = {0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000};
ml_dword FLOAT_3DNOW_NO_W_MASK[] = {0xFFFFFFFF, 0x00000000};

const ml_mat ML_matIdentity = {1.0f, 0.0f, 0.0f, 0.0f,
                               0.0f, 1.0f, 0.0f, 0.0f,
                               0.0f, 0.0f, 1.0f, 0.0f,
                               0.0f, 0.0f, 0.0f, 1.0f};
const ml_vec3 ML_v3Zero = {0.0f, 0.0f, 0.0f};


ml_bool ML_FUNC ML_Init(ML_INSTR nInstr)
{
	return ML_TRUE;
}
