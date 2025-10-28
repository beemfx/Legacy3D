;ML_qaut.nasm - Vector operations for 3D vectors.
;Copyright (c) 2006, Blaine Myers
;
;Optimized functions (SSE, 3DNOW, etc) are based upon SIMDx86 
;(www.sourceforge.net/projects/simdx86)

[BITS 32]
[SECTION .code]

global _ML_QuatRotationMat2
_ML_QuatRotationMat2:
	mov eax, [esp+4]
	mov ecx, [esp+8]
	push 0x1
	fldz
	fld1
	fld dword [ecx+40]
	fld dword [ecx+20]
	fld dword [ecx]
	faddp st1
	faddp st1
	fadd st1
	fcomi st0, st2 ;Compare value with 0.0f;
	jng ML_QuatRotationMat_NonW
	fsqrt
	
	
	
	fstp dword [eax]
	fstp st0
	fstp st0
	ret
	
	ML_QuatRotationMat_NonW:
	ret
	
;ML_QUAT* ML_FUNC ML_QuatRotationMat2(ML_QUAT* pOut, ML_MAT* pM)
;{
;	float fTrace=pM->_11+pM->_22+pM->_33+1.0f;
;	//pOut->w=pM->_11+pM->_22+pM->_33+1.0f;
;	if(fTrace > 0.00000001f)
;	{
;		//MessageBox(0, "W", 0, 0);
;		pOut->w=sqrtf(fTrace)*0.5f;
;		fTrace=0.25f/pOut->w;
;		pOut->x=(pM->_23-pM->_32)*fTrace;
;		pOut->y=(pM->_31-pM->_13)*fTrace;
;		pOut->z=(pM->_12-pM->_21)*fTrace;
;		return pOut;
;	}
;
;	if( (pM->_11>pM->_22) && (pM->_11>pM->_33))
;	{
;		//MessageBox(0, "X", 0, 0);
;		pOut->x=sqrtf(pM->_11-pM->_22-pM->_33+1.0f)*0.5f;
;		fTrace=0.25f/pOut->x;
;		pOut->w=(pM->_23-pM->_32)*fTrace;
;		pOut->y=(pM->_12+pM->_21)*fTrace;
;		pOut->z=(pM->_31+pM->_13)*fTrace;
;		return pOut;
;	}
;	else if(pM->_22>pM->_33)
;	{
;		//MessageBox(0, "Y", 0, 0);
;		pOut->y=sqrtf(pM->_22-pM->_11-pM->_33+1.0f)*0.5f;
;		fTrace=0.25f/pOut->y;
;		pOut->w=(pM->_31-pM->_13)*fTrace;
;		pOut->x=(pM->_12+pM->_21)*fTrace;
;		pOut->z=(pM->_23+pM->_32)*fTrace;
;		return pOut;
;	}
;	else
;	{
;		//MessageBox(0, "Z", 0, 0);
;		pOut->z=sqrtf(pM->_33-pM->_11-pM->_22+1.0f)*0.5f;
;		fTrace=0.25f/pOut->z;
;		pOut->w=(pM->_12-pM->_21)*fTrace;
;		pOut->x=(pM->_31+pM->_13)*fTrace;
;		pOut->y=(pM->_23+pM->_32)*fTrace;
;		return pOut;
;	}
;}

global _ML_QuatSlerp
_ML_QuatSlerp:
	mov ecx, [esp+8]
	mov edx, [esp+12]
	
	fld dword [esp+16] ;Move t onto the fp stack
	ftst ;compare t with zero, if less than 0...
	fstsw ax
	sahf
	jbe QuatSlerpUnderZ
	;compare t with 1.0f, if great than..
	fld1
	;fxch st1, st0
	fcomp st0, st1
	fstsw ax
	sahf
	jbe QuatSlerpOverOne
	jmp QuatSlerpDo
	
	QuatSlerpEnd:
	fstp st0
	ret
	
	QuatSlerpDo:
	mov eax, [esp+4]
	;mov dword [eax], 0
	;mov dword [eax+4], 0
	;mov dword [eax+8], 0
	;mov dword [eax+12], 0
	
	;calculate the dot product of pQ2
	fld dword [edx]
	fmul st0, st0
	fld dword [edx+4]
	fmul st0, st0
	faddp st1, st0
	fld dword [edx+8]
	fmul st0, st0
	faddp st1, st0
	fld dword [edx+12]
	fmul st0, st0
	faddp st1, st0
	
	;We have the dot product
	fstp st0
	
	
	jmp QuatSlerpEnd
	
	QuatSlerpUnderZ:
	mov eax, [esp+4]
	mov edx, [ecx]
	mov [eax], edx
	mov edx, [ecx+4]
	mov [eax+4], edx
	mov edx, [ecx+8]
	mov [eax+8], edx
	mov edx, [ecx+12]
	mov [eax+12], edx
	jmp QuatSlerpEnd
	
	QuatSlerpOverOne:
	mov eax, [esp+4]
	mov ecx, [edx]
	mov [eax], ecx
	mov ecx, [edx+4]
	mov [eax+4], ecx
	mov ecx, [edx+8]
	mov [eax+8], ecx
	mov ecx, [edx+12]
	mov [eax+12], ecx
	jmp QuatSlerpEnd
