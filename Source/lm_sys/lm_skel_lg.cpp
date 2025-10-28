#include "lm_skel_lg.h"
#include "lm_mesh_lg.h"
#include "lf_sys2.h"

lg_bool CLSkelLG::Load(LMPath szFile)
{
	lg_bool bRes;
	Unload();
	
	LF_FILE3 fin=LF_Open(szFile, LF_ACCESS_READ, LF_OPEN_EXISTING);
	
	if(!fin)
		return LG_FALSE;
	
	bRes=Serialize(
		fin,
		CLMeshLG::ReadFn,
		RW_READ);
		
	LF_Close(fin);
	
	if(bRes)
	{
		m_nFlags=LM_FLAG_LOADED;
		CalcExData();
	}
	
	return bRes;
}

lg_bool CLSkelLG::Save(LMPath szFile)
{
	//We don't save ingame skels...
	return LG_FALSE;
}