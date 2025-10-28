#ifndef __LM_XEXP_H__
#define __LM_XEXP_H__
#include "lm_sys.h"

#include <pshpack1.h>

typedef struct _XMESH_INDEX
{
	lg_dword nNumVert;
	lg_dword v[3];
}XMESH_INDEX;

#include <poppack.h>

class CLMeshXExp: public CLMesh
{
public:
	lg_bool SaveAsX(char* szFilename);
private:
	lg_bool AddMesh(void *pFileSaveRoot);
};


#endif __LM_XEXP_H__