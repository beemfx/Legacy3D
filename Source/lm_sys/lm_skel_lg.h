#ifndef __LM_SKEL_LG_H__
#define __LM_SKEL_LG_H__

#include "lm_skel.h"

class CLSkelLG: public CLSkel2
{
public:
	virtual lg_bool Load(LMPath szFile);
	virtual lg_bool Save(LMPath szFile);
};

#endif __LM_SKEL_LG_H__