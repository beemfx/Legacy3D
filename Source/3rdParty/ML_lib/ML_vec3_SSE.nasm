;ML_vec3.nasm - Vector operations for 3D vectors with SSE.
;Copyright (c) 2006, Blaine Myers
;
;Optimized functions (SSE, 3DNOW, etc) are based upon SIMDx86 
;(www.sourceforge.net/projects/simdx86)

;If LOW_PREC_SQRT is defined the code will compile to use
;a faster square root, but not as accurate.
;%define LOW_PREC_SQRT

[BITS 32]
[SECTION .code]

;/** ML_VEC3 SSE FUNCTIONS  ***
;***                        ***
;*** ML_Vec3Add             ***
;*** ML_Vec3Subtract        ***
;*** ML_Vec3Cross           ***
;*** ML_Vec3Dot             ***
;*** ML_Vec3Length          ***
;*** ML_Vec3LengthSq        ***
;*** ML_Vec3Normalize       ***
;*** ML_Vec3Scale           ***
;*** ML_Vec3Distance        ***
;*** ML_Vec3DistanceSq      ***
;*** ML_Vec3Transform       ***
;*** ML_Vec3TransformCoord  ***
;*** ML_Vec3TransformNormal ***
;******************************/

;Note that when returning a vector result from
;the xmm registers those quantities are 4 dwords
;wide and the ML_vec3 is 3 dwords so to return
;the result we either need to make the vector
;4 dwords, or copy only 3 dwords.

%macro ML_ReturnVec3SSE 1
	ML_ReturnVec3SSE_UNALIGNED %1
%endmacro

%macro ML_ReturnVec3SSE_ALIGNED 1
	movups [eax], %1
%endmacro

%macro ML_ReturnVec3SSE_UNALIGNED 1
	movlps [eax], %1      ;Move x and y into the result (first 16 bits).
	shufps %1, %1, 0x06   ;Move z to the x position.
	movss  [eax+8], %1    ;Move z (now at x position) into the result.
%endmacro

;;;;;
;Add;
;;;;;
;SSE
global _ML_Vec3Add_SSE
_ML_Vec3Add_SSE:
	mov  eax,  [esp+4] ;Get pointers to the parameters.
	mov  ecx,  [esp+8]
	mov  edx,  [esp+12]
	
	movups xmm0, [ecx]
	movups xmm1, [edx]
	addps  xmm1, xmm0
	
	ML_ReturnVec3SSE xmm1
	ret
	
;;;;;;;;;;	
;Subtract;
;;;;;;;;;;	
;SSE
global _ML_Vec3Subtract_SSE
_ML_Vec3Subtract_SSE:
	mov  eax,  [esp+4] ;Get pointers to the parameters.
	mov  ecx,  [esp+8]
	mov  edx,  [esp+12]
	
	movups xmm0, [ecx]
	movups xmm1, [edx]
	subps xmm0, xmm1
	ML_ReturnVec3SSE xmm0
	ret                ;The return value of the output vector is already in eax.
	
;;;;;;;	
;Cross;
;;;;;;;
;SSE
global _ML_Vec3Cross_SSE
_ML_Vec3Cross_SSE:
	mov eax, [esp+4]   ;Save pointers to parameters,
	mov ecx, [esp+8]
	mov edx, [esp+12]
	
	movups xmm0, [ecx]
	movups xmm1, [edx]
	movaps xmm2, xmm0
	movaps xmm3, xmm1
	
	shufps xmm0, xmm0, 0xC9
	shufps xmm1, xmm1, 0xD2
	shufps xmm2, xmm2, 0xD2
	shufps xmm3, xmm3, 0xC9
	
	mulps xmm1, xmm0
	mulps xmm3, xmm2
	
	subps xmm1, xmm3
	
	ML_ReturnVec3SSE xmm1
	ret
	
;;;;;	
;Dot;
;;;;;	
;SSE3:
global _ML_Vec3Dot_SSE3
_ML_Vec3Dot_SSE3:
	;Get points to vectors from stack and load them.
	mov ecx, [esp+4]
	mov edx, [esp+8]
	movups xmm0, [ecx]
	movups xmm1, [edx]

	;Remove w component from one vector with AND mask.
	andps xmm0, [_FLOAT_SSE_NO_W_MASK]
	mulps xmm1, xmm0
	
	;Sum components.
	haddps xmm1, xmm1
	haddps xmm1, xmm1
	
	;Allocate four bytes of stack space, and put
	;the result there, then copy the value, onto
	;the floating point stack and return.
	sub esp, 4
	movss [esp], xmm1
	fld dword [esp]
	add esp, 4
	ret
	
;SSE:
global _ML_Vec3Dot_SSE
_ML_Vec3Dot_SSE:
	;Get pointers to vectors from stack and load them.
	mov ecx, [esp+4]
	mov edx, [esp+8]
	movups xmm0, [ecx]
	movups xmm1, [edx]
	;Multiply across.
	andps xmm0, [_FLOAT_SSE_NO_W_MASK]
	mulps xmm1, xmm0
	
	;Shift data around in the registers.
	movaps xmm2, xmm1
	movaps xmm3, xmm1
	shufps xmm2, xmm2, 0x55
	shufps xmm3, xmm3, 0xAA
	addss xmm2, xmm1
	addss xmm2, xmm3
	
	;Allocate space, store on stack, and return with value on fp stack
	sub esp, 4
	movss [esp], xmm2
	fld dword [esp]
	add esp, 4
	ret
	
;;;;;;;;	
;Length;
;;;;;;;;
;SSE3:
global _ML_Vec3Length_SSE3
_ML_Vec3Length_SSE3:
	mov ecx, [esp+4]
	movups xmm0, [ecx]
	andps xmm0, [_FLOAT_SSE_NO_W_MASK]
	mulps xmm0, xmm0
	haddps xmm0, xmm0
	haddps xmm0, xmm0
	
	%ifndef LOW_PREC_SQRT
	;High precession
	sqrtss xmm0, xmm0
	%else ;LOW_PREC_SQRT
	;Low precission
	rsqrtss xmm0, xmm0
	rcpss xmm0, xmm0
	%endif ;LOW_PREC_SQRT
	
	sub esp, 4
	movss [esp], xmm0
	fld dword [esp]
	add esp, 4
	ret

;SSE:
;Similar to SSE3 implimentation but haddps is not available.
global _ML_Vec3Length_SSE
_ML_Vec3Length_SSE:
	mov ecx, [esp+4]
	movups xmm0, [ecx]
	andps xmm0, [_FLOAT_SSE_NO_W_MASK]
	mulps xmm0, xmm0
	
	movaps xmm1, xmm0
	movaps xmm2, xmm0
	shufps xmm1, xmm1, 0x55
	shufps xmm2, xmm2, 0xAA
	addss xmm1, xmm0
	addss xmm1, xmm2
	
	%ifndef LOW_PREC_SQRT
	;High precession
	sqrtss xmm1, xmm1
	%else ;LOW_PREC_SQRT
	;Low precission
	rsqrtss xmm1, xmm1
	rcpss xmm1, xmm1
	%endif ;LOW_PREC_SQRT
	
	sub esp, 4
	movss [esp], xmm1
	fld dword [esp]
	add esp, 4
	ret

;;;;;;;;;;	
;LengthSq;
;;;;;;;;;;
;SSE3:
global _ML_Vec3LengthSq_SSE3
_ML_Vec3LengthSq_SSE3:
	mov ecx, [esp+4]
	movups xmm0, [ecx]
	andps xmm0, [_FLOAT_SSE_NO_W_MASK]
	mulps xmm0, xmm0
	haddps xmm0, xmm0
	haddps xmm0, xmm0
	
	sub esp, 4
	movss [esp], xmm0
	fld dword [esp]
	add esp, 4
	ret

;SSE:
;Similar to SSE3 implimentation but haddps is not available.
global _ML_Vec3LengthSq_SSE
_ML_Vec3LengthSq_SSE:
	mov ecx, [esp+4]
	movups xmm0, [ecx]
	andps xmm0, [_FLOAT_SSE_NO_W_MASK]
	mulps xmm0, xmm0
	
	movaps xmm1, xmm0
	movaps xmm2, xmm0
	shufps xmm1, xmm1, 0x55
	shufps xmm2, xmm2, 0xAA
	addss xmm1, xmm0
	addss xmm1, xmm2
	
	sub esp, 4
	movss [esp], xmm1
	fld dword [esp]
	add esp, 4
	ret
	
;;;;;;;;;;;
;Normalize;
;;;;;;;;;;;
;SSE3:
global _ML_Vec3Normalize_SSE3
_ML_Vec3Normalize_SSE3:
	mov eax, [esp+4]
	mov ecx, [esp+8]
	movups xmm0, [ecx]
	andps xmm0, [_FLOAT_SSE_NO_W_MASK]
	movaps xmm2, xmm0
	mulps xmm0, xmm0
	haddps xmm0, xmm0
	haddps xmm0, xmm0
	
	%ifndef LOW_PREC_SQRT
	;High Precision
	sqrtps xmm0, xmm0
	divps xmm2, xmm0
	%else ;LOW_PREC_SQRT
	;Low Prec
	rsqrtps xmm1, xmm0
	mulps xmm2, xmm1	
	%endif ;LOW_PREC_SQRT
	
	ML_ReturnVec3SSE xmm2
	ret

;SSE:
global _ML_Vec3Normalize_SSE
_ML_Vec3Normalize_SSE:
	mov eax, [esp+4]
	mov ecx, [esp+8]
	movups xmm0, [ecx]
	andps xmm0, [_FLOAT_SSE_NO_W_MASK]
	movaps xmm7, xmm0
	mulps xmm0, xmm0
	
	movaps xmm1, xmm0
	movaps xmm2, xmm0
	shufps xmm1, xmm1, 0x55
	shufps xmm2, xmm2, 0xAA
	addss xmm1, xmm0
	addss xmm1, xmm2
	
	%ifndef LOW_PREC_SQRT
	;High Precision
	sqrtps xmm1, xmm1
	shufps xmm1, xmm1, 0x00
	divps xmm7, xmm1
	%else ;LOW_PREC_SQRT
	;Low Prec
	rsqrtps xmm1, xmm1
	shufps xmm1, xmm1, 0x00
	mulps xmm7, xmm1	
	%endif ;LOW_PREC_SQRT
	
	ML_ReturnVec3SSE xmm7
	ret
	
;;;;;;;	
;Scale;
;;;;;;;
;SSE:
global _ML_Vec3Scale_SSE
_ML_Vec3Scale_SSE:
	mov  eax,  [esp+4]   ;Get pointers to the parameters.
	mov  ecx,  [esp+8]

	movss xmm0, [esp+12]
	movups xmm1, [ecx]
	shufps xmm0, xmm0, 0x00
	mulps xmm0, xmm1
	ML_ReturnVec3SSE xmm0
	ret   

;;;;;;;;;;
;Distance;
;;;;;;;;;;	
;SSE3:
global _ML_Vec3Distance_SSE3
_ML_Vec3Distance_SSE3:
	mov ecx, [esp+4]
	mov edx, [esp+8]
	sub esp, 16
	mov eax, esp
	push edx
	push ecx
	push eax
	call _ML_Vec3Subtract_SSE
	call _ML_Vec3Length_SSE3
	add esp, 28
	ret

;SSE:	
global _ML_Vec3Distance_SSE
_ML_Vec3Distance_SSE:	
	mov ecx, [esp+4]
	mov edx, [esp+8]
	sub esp, 16
	mov eax, esp
	push edx
	push ecx
	push eax
	call _ML_Vec3Subtract_SSE
	call _ML_Vec3Length_SSE
	add esp, 28
	ret

;;;;;;;;;;;;	
;DistanceSq;
;;;;;;;;;;;;	
;SSE3:
global _ML_Vec3DistanceSq_SSE3
_ML_Vec3DistanceSq_SSE3:
	mov ecx, [esp+4]
	mov edx, [esp+8]
	sub esp, 16
	mov eax, esp
	push edx
	push ecx
	push eax
	call _ML_Vec3Subtract_SSE
	call _ML_Vec3LengthSq_SSE3
	add esp, 28
	ret

;SSE:	
global _ML_Vec3DistanceSq_SSE
_ML_Vec3DistanceSq_SSE:	
	mov ecx, [esp+4]
	mov edx, [esp+8]
	sub esp, 16
	mov eax, esp
	push edx
	push ecx
	push eax
	call _ML_Vec3Subtract_SSE
	call _ML_Vec3LengthSq_SSE
	add esp, 28
	ret

;;;;;;;;;;;	
;Transform;
;;;;;;;;;;;
;SSE:
global _ML_Vec3Transform_SSE
_ML_Vec3Transform_SSE:
	mov eax, [esp+4]
	mov ecx, [esp+8]
	mov edx, [esp+12]
	
	movups xmm4, [ecx]
	movups xmm0, [edx]
	movups xmm1, [edx+16]
	movups xmm2, [edx+32]
	movups xmm3, [edx+48]
	movaps xmm5, xmm4
	movaps xmm6, xmm4
	shufps xmm4, xmm4, 0x00
	shufps xmm5, xmm5, 0x55
	shufps xmm6, xmm6, 0xAA
	
	;Multiply by each row.
	mulps xmm0, xmm4
	mulps xmm1, xmm5
	mulps xmm2, xmm6
	
	;Sum results
	addps xmm1, xmm0
	addps xmm3, xmm2
	addps xmm1, xmm3
	
	;Move result to pOut.
	movups [eax], xmm1
	ret
	
;;;;;;;;;;;;;;;;
;TransformCoord;
;;;;;;;;;;;;;;;;
;SSE:
global _ML_Vec3TransformCoord_SSE
_ML_Vec3TransformCoord_SSE:
	mov eax, [esp+4]
	mov ecx, [esp+8]
	mov edx, [esp+12]
	
	movups xmm4, [ecx]
	movups xmm0, [edx]
	movups xmm1, [edx+16]
	movups xmm2, [edx+32]
	movups xmm3, [edx+48]
	movaps xmm5, xmm4
	movaps xmm6, xmm4
	shufps xmm4, xmm4, 0x00
	shufps xmm5, xmm5, 0x55
	shufps xmm6, xmm6, 0xAA
	
	;Multiply by each row.
	mulps xmm0, xmm4
	mulps xmm1, xmm5
	mulps xmm2, xmm6
	
	;Sum results
	addps xmm1, xmm0
	addps xmm3, xmm2
	addps xmm1, xmm3
	
	;Divide x, y, and z by w.
	movaps xmm2, xmm1
	shufps xmm2, xmm2, 0xFF
	divps xmm1, xmm2

	;Move result to pOut.
	ML_ReturnVec3SSE xmm1
	ret
	
;;;;;;;;;;;;;;;;;	
;TransformNormal;
;;;;;;;;;;;;;;;;;
;SSE:
global _ML_Vec3TransformNormal_SSE
_ML_Vec3TransformNormal_SSE:
	mov eax, [esp+4]
	mov ecx, [esp+8]
	mov edx, [esp+12]
	
	movups xmm4, [ecx]
	movups xmm0, [edx]
	movups xmm1, [edx+16]
	movups xmm2, [edx+32]
	movups xmm3, [edx+48]
	andps xmm4, [_FLOAT_SSE_NO_W_MASK]
	movaps xmm5, xmm4
	movaps xmm6, xmm4
	movaps xmm7, xmm4
	shufps xmm4, xmm4, 0x00
	shufps xmm5, xmm5, 0x55
	shufps xmm6, xmm6, 0xAA
	shufps xmm7, xmm7, 0xFF
	
	;Multiply by each row.
	mulps xmm0, xmm4
	mulps xmm1, xmm5
	mulps xmm2, xmm6
	mulps xmm3, xmm7
	
	;Sum results
	addps xmm1, xmm0
	addps xmm3, xmm2
	addps xmm1, xmm3
	
	;Move result to pOut.
	ML_ReturnVec3SSE xmm1
	ret
	
	
[SECTION .data]
extern _FLOAT_SSE_NO_W_MASK
	
	