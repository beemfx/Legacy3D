#include "lm_mesh.h"
#include "lm_skel.h"

lg_uint __cdecl MSWrite(lg_void* file, lg_void* buffer, lg_uint size);

class CLMeshMS: public CLMesh2
{
friend class CLSkelMS;
public:
	CLMeshMS(void* pModel, char* szName);
	lg_bool LoadFromMS(void* pModelParam, char* szName);
	
	virtual lg_bool Load(LMPath szFile);
	virtual lg_bool Save(LMPath szFile);
};


class CLSkelMS: public CLSkel2{
public:
	CLSkelMS(void* pModel, char* szName);
	lg_bool LoadFromMS(void* pModelParam, char* szName);
	
	virtual lg_bool Load(LMPath szFile);
	virtual lg_bool Save(LMPath szFile);
};
