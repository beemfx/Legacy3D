#ifndef __LE_TEST_H__
#define __LE_TEST_H__

#include <dinput.h>
#include <al/al.h>
#include "le_3dbase.h"
/*
#include "le_camera.h"
#include "le_sys.h"
#include "lm_d3d.h"
#include "lt_sys.h"
#include "lg_sys.h"
*/
class CLJack: public CLBase3DEntity
{
private:
	//const DIMOUSESTATE2* m_pMouse;
	
	//CLICommand* m_pCmds;
	
	CLSkel* m_pWalk;
	CLSkel* m_pStand;
	CLSkel* m_pJump;
	
	NewtonJoint* m_UpVec;
	
public:
	virtual void Initialize(ML_VEC3* v3Pos);
	virtual void ProcessAI();
	//virtual void Render();
};

class CLBlaineEnt: public CLBase3DEntity
{
private:
	CLSkel* m_pWalk;
	CLSkel* m_pStand;
	lg_dword m_nLastAIUpdate;
	lg_float m_fRotation;
	ALuint   m_Snd;
	ALuint   m_SndBuffer;
	
	NewtonJoint* m_UpVec;
public:
	virtual void Initialize(ML_VEC3* v3Pos);
	virtual void ProcessAI();
};


class CLMonaEnt: public CLBase3DEntity
{
public:
	virtual void Initialize(ML_VEC3* v3Pos);
	virtual void ProcessAI();
};

class CLBarrelEnt: public CLBase3DEntity
{
public:
	virtual void Initialize(ML_VEC3* v3Pos);
	virtual void ProcessAI();
};

#endif __LE_TEST_H__