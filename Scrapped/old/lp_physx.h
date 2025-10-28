#ifndef __LP_PHYSX_H__
#define __LP_PHYSX_H__

#include <NxPhysics.h>
#include <NxCharacter.h>
#include <NxBoxController.h>
#include <NxUserOutputStream.h>
#include <NxCooking.h>
#include <NxStream.h>
#include "lg_types.h"
#include "lw_map.h"
#include "lf_sys2.h"


class CLPhysXOutputStream: public NxUserOutputStream
{
	virtual void reportError(NxErrorCode code, const char* message, const char* file, int line);
	virtual NxAssertResponse reportAssertViolation(const char* message, const char* file, int line);
	virtual void print(const char* message);
};

class CElementPhysX
{
private:
	static NxActor*      s_pWorldActor;
public:
	static NxPhysicsSDK* s_pNxPhysics;
	static NxScene*      s_pNxScene;
	static CLPhysXOutputStream s_Output;
	static void Initialize();
	static void Shutdown();
	static void SetupWorld(CLWorldMap* pMap);
	static void Simulate(lg_float fSeconds);
};

class CLNxStream: public NxStream
{
public:
	CLNxStream(const lg_char filename[], lg_bool bLoad):
		m_File(LG_NULL)
	{
		Open(filename, bLoad);
	}
	virtual ~CLNxStream()
	{
		Close();
	}
	
	virtual NxU8 readByte()const
	{
		NxU8 b;
		LF_Read(m_File, &b, 1);
		return b;
	}
	virtual NxU16 readWord()const
	{
		NxU16 b;
		LF_Read(m_File, &b, 2);
		return b;
	}
	virtual NxU32 readDword()const
	{
		NxU32 b;
		LF_Read(m_File, &b, 4);
		return b;
	}
	virtual NxReal readFloat()const
	{
		NxReal b;
		LF_Read(m_File, &b, 4);
		return b;
	}
	virtual NxF64 readDouble()const
	{
		NxF64 b;
		LF_Read(m_File, &b, 8);
		return b;
	}
	virtual void readBuffer(void* buffer, NxU32 size)const
	{
		LF_Read(m_File, buffer, size);
	}
	
	virtual NxStream& storeByte(NxU8 n)
	{
		LF_Write(m_File, &n, 1);
		return *this;
	}
	virtual NxStream& storeWord(NxU16 w)
	{
		LF_Write(m_File, &w, 2);
		return *this;
	}
	virtual NxStream& storeDword(NxU32 d)
	{
		LF_Write(m_File, &d, 4);
		return *this;
	}
	virtual NxStream& storeFloat(NxReal f)
	{
		LF_Write(m_File, &f, 4);
		return *this;
	}
	virtual NxStream& storeDouble(NxF64 f)
	{
		LF_Write(m_File, &f, 8);
		return *this;
	}
	virtual NxStream& storeBuffer(const void* buffer, NxU32 size)
	{
		LF_Write(m_File, (lf_void*)buffer, size);
		return *this;
	}
	
	void Open(const lg_char filename[], lg_bool bLoad)
	{
		Close();
		m_File=LF_Open(filename, LF_ACCESS_READ|LF_ACCESS_WRITE, bLoad?LF_OPEN_EXISTING:LF_CREATE_ALWAYS);
	}
	void Close()
	{
		if(m_File)
		{
			LF_Close(m_File);
			m_File=LG_NULL;
		}
	}
	LF_FILE3 m_File;
};

#endif __LP_PHYSX_H__