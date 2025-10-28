#ifndef __LM_SKIN_H__
#define __LM_SKIN_H__

#include "lm_mesh.h"
#include "lg_types.h"
#include "lg_mtr_mgr.h"

class CLSkin: public CLMBase
{
friend class CLMeshD3D;
friend class CLMeshLG;
private:
	LG_CACHE_ALIGN struct MtrGrp{
		lg_dword   nMtr;   //Material associated with this group.
		LMName szName; //Name of this group (should match mesh group names).
	};
	
	lg_dword m_nMtrCount; //The number of materials in the skin.
	lg_dword m_nGrpCount; //The number of groups assigned in the skin.
	
	lg_material* m_pMaterials; //List of materials (index 0 is material 1)
	MtrGrp*      m_pGrps; //List of the groups and associated materials.
	lg_material* m_pCmpRefs; //References to translate mesh group ids to skin ids.

	lg_byte* m_pMem;
public:
	CLSkin();
	~CLSkin();
	
	/* PRE: N/A
		POST: Loads the skin, if the skin could not be
		loaded false is returned, otherwise true is returned.
	*/
	lg_bool Load(lg_path szFilename);
	/* PRE: N/A
		POST: Unloads the skin (freeing memory) if one
		was loaded.
	*/
	void Unload();
	/* PRE: pMesh should be loaded.
		POST: The skin is compatible with the specified
		mesh.  Note that skins should usually only be used
		with one mesh, so this call should be made with only
		one mesh.
	*/
	void MakeCompatibleWithMesh(CLMesh2* pMesh);
	
	//The loader method.
	static void LX_SkinStart(void* userData, const char* name, const char** atts);
	
	//Serialize method;
	lg_bool Serialize(lg_void* file, ReadWriteFn read_or_write, RW_MODE mode);
	
};

#endif __LM_SKIN_H__