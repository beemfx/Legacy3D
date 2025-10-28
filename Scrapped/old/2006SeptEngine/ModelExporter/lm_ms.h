#include "..\\legacy3d\\lm_sys.h"


class CLegacyMeshMS: public CLegacyMesh
{
public:
	CLegacyMeshMS(void* pModel);
	L_bool LoadFromMS(void* pModelParam);
};

class CLegacySkelMS: public CLegacySkeleton{
public:
	CLegacySkelMS(void* pModel);
	L_bool LoadFromMS(void* pModelParam);
};
