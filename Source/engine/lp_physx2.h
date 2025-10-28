#if 0
#ifndef __LP_PHYSX2_H__
#define __LP_PHYSX2_H__

#include <NxPhysics.h>
#include <NxStream.h>
#include <NxControllerManager.h>
#include "lp_sys2.h"
#include "lf_sys2.h"
#include "lg_mem_file.h"
#include "lw_entity.h"

class CLPhysPhysX: public CLPhys
{
//Internally used classes:
private:
	/* The following is the output stream, it allows output from the PhysX SDK to be
		sent to the console.
	*/
	class CXOut: public NxUserOutputStream
	{
		virtual void reportError(NxErrorCode code, const char* message, const char* file, int line);
		virtual NxAssertResponse reportAssertViolation(const char* message, const char* file, int line);
		virtual void print(const char* message);
	};
	
	class CLNxStream: public NxStream
	{
	private:
		LF_FILE3 m_File;
	public:
		CLNxStream(const lg_char filename[], lg_bool bLoad);
		virtual ~CLNxStream();
		
		virtual NxU8 readByte()const;
		virtual NxU16 readWord()const;
		virtual NxU32 readDword()const;
		virtual NxReal readFloat()const;
		virtual NxF64 readDouble()const;
		virtual void readBuffer(void* buffer, NxU32 size)const;
		
		virtual NxStream& storeByte(NxU8 n);
		virtual NxStream& storeWord(NxU16 w);
		virtual NxStream& storeDword(NxU32 d);
		virtual NxStream& storeFloat(NxReal f);
		virtual NxStream& storeDouble(NxF64 f);
		virtual NxStream& storeBuffer(const void* buffer, NxU32 size);
		
		void Open(const lg_char filename[], lg_bool bLoad);
		void Close();
	};
	
	class CLNxMemStream: public NxStream
	{
	private:
		CLMemFile m_File;
	public:
		CLNxMemStream(lg_dword nSize);
		virtual ~CLNxMemStream();
		
		virtual NxU8 readByte()const;
		virtual NxU16 readWord()const;
		virtual NxU32 readDword()const;
		virtual NxReal readFloat()const;
		virtual NxF64 readDouble()const;
		virtual void readBuffer(void* buffer, NxU32 size)const;
		
		virtual NxStream& storeByte(NxU8 n);
		virtual NxStream& storeWord(NxU16 w);
		virtual NxStream& storeDword(NxU32 d);
		virtual NxStream& storeFloat(NxReal f);
		virtual NxStream& storeDouble(NxF64 f);
		virtual NxStream& storeBuffer(const void* buffer, NxU32 size);
		
		void Open(lg_dword nSize);
		void Close();
		void Reset();
	};
	
	class CXAlloc : public NxUserAllocator
	{
	public:        
		 void * malloc(NxU32 size);
		 void * mallocDEBUG(NxU32 size,const char *fileName, int line);
		 void * realloc(void * memory, NxU32 size);
		 void free(void * memory);
	};
	
//Member variables:
private:
	CXOut         m_Out;
	CXAlloc       m_Alloc;
	NxPhysicsSDK* m_pSDK;
	NxScene*      m_pScene;
	NxActor*      m_pActrMap;
	//Character controller manager:
	NxControllerManager* m_pCtrlrMgr;
	
	//Cooking library:
	NxCookingInterface* m_pCooking;
	
	//Active transform vars
	NxU32 m_nATs;
	NxActiveTransform* m_pATs;
	
	//Some materials
	NxMaterial* m_pMtrInt; //Material for intelligent objects.
	NxMaterial* m_pMtrMap; //Material for the map.
//Overridden member methods:
public:
	virtual void Init(lg_dword nMaxBodies);
	virtual void Shutdown();
	virtual lg_void* AddBody(lg_void* pEnt, lp_body_info* pInfo);
	virtual void RemoveBody(lg_void* pBody);
	virtual void SetupWorld(lg_void* pWorldMap);
	virtual void SetGravity(ML_VEC3* pGrav);
	virtual void Simulate(lg_float fTimeStepSec);
	
	virtual void SetBodyFlag(lg_void* pBody, lg_dword nFlagID, lg_dword nValue);
	virtual void SetBodyVector(lg_void* pBody, lg_dword nVecID, ML_VEC3* pVec);
	virtual void SetBodyFloat(lg_void* pBody, lg_dword nFloatID, lg_float fValue);
	virtual void SetBodyPosition(lg_void* pBody, ML_MAT* pMat);
	
	virtual lg_void* GetBodySaveInfo(lg_void* pBody, lg_dword* pSize);
	virtual lg_void* LoadBodySaveInfo(lg_void* pData, lg_dword nSize);
private:
	lg_void* AddIntelligentBody(lg_void* pEnt, lp_body_info* pInfo);
	lg_void* AddDynamicBody(lg_void* pEnt, lp_body_info* pInfo);
private:
	__inline static void UpdateToSrv(NxActor* pSrc, lg_srv_ent* pEnt, NxMat34* pPos);
	__inline static void UpdateFromSrv(NxActor* pSrc);
};

#endif __LP_PHYSX2_H__
#endif