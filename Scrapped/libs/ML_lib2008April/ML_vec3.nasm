;ML_vec3.nasm - Vector operations for 3D vectors.
;Copyright (c) 2006, Blaine Myers
;
;Optimized functions (SSE, 3DNOW, etc) are based upon SIMDx86 
;(www.sourceforge.net/projects/simdx86)

;TODO: A few more functions ML_LengthSq, etc. and 3DNOW! implimentations.

;If LOW_PREC_SQRT is defined the code will compile to use
;a faster square root, but not as accurate.
;%define LOW_PREC_SQRT

[BITS 32]
[SECTION .code]

;Note that when returning a vector result from
;the xmm registers those quantities are 32 bytes
;wide and the ML_vec3 is 24 bytes so to return
;the result we either need to make the vector
;32 bytes, or copy only 24 bytes.

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



%macro ML_ReturnVec33DNOW 2
	movq [eax], %1
	movd [eax+8], %2
	femms
%endmacro


;ML_VEC3* ML_Vec3Add(ML_VEC3* pOut, ML_VEC3* pV1, ML_VEC3* pV2);
;Adds two vectors together and puts the result in pOut.

;Floating Point:
global _ML_Vec3Add_F
_ML_Vec3Add_F:
	mov  eax,  [esp+4] ;Get pointers to the parameters.
	mov  ecx,  [esp+8]
	mov  edx,  [esp+12]
	fld  dword [ecx]   ;Then add the x, y, and z component respectively.
   fadd dword [edx]
	fstp dword [eax]
	fld  dword [ecx+4]
	fadd dword [edx+4]
	fstp dword [eax+4]
	fld  dword [ecx+8]
	fadd dword [edx+8]
	fstp dword [eax+8] 
	ret                ;The return value of the output vector is already in eax.

;SSE:
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
	
;3DNOW:
global _ML_Vec3Add_3DNOW
_ML_Vec3Add_3DNOW:
	mov eax, [esp+4]
	mov ecx, [esp+8]
	mov edx, [esp+12]
	
	;Load registers then add.
	movq mm0, [edx]
	movd mm1, [edx+8]
	pfadd mm0, [ecx]
	pfadd mm1, [ecx+8]
	;Return result.
	ML_ReturnVec33DNOW mm0, mm1
	ret


;ML_VEC3* ML_Vec3Cross(ML_VEC3* pOut, ML_VEC3* pV1, ML_VEC3* pV2)
;Finds cross product.

;Floating Point:
global _ML_Vec3Cross_F
_ML_Vec3Cross_F:
	mov eax, [esp+4]   ;Save pointers to parameters,
	mov ecx, [esp+8]   ;and make room on stack for output.
	mov edx, [esp+12]
	sub esp, 12

	fld  dword [ecx+4]
	fmul dword [edx+8]
	fld  dword [ecx+8] ;Calculate the cross value for x,
	fmul dword [edx+4] ;store at [esp].
	fsubp st1
	fstp dword [esp]

	fld  dword [ecx+8]
	fmul dword [edx]
	fld  dword [ecx]   ;Calculate the cross value for y,
	fmul dword [edx+8] ;and store t [esp+4].
	fsubp st1
	fstp dword [esp+4]

	fld  dword [ecx]
	fmul dword [edx+4]
	fld  dword [ecx+4] ;Calculate teh cross value for z,
	fmul dword [edx]   ;and store at [esp+8].
	fsubp st1
	fstp dword [esp+8]

	pop dword [eax]    ;Pop output values off the stack, into pOut.
	pop dword [eax+4]
	pop dword [eax+8]  ;pOut is already set to eax as the return value.
	ret
	
;SSE:
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
	
;3DNOW:
global _ML_Vec3Cross_3DNOW
_ML_Vec3Cross_3DNOW:
	mov eax, [esp+4]   ;Save pointers to parameters,
	mov ecx, [esp+8]
	mov edx, [esp+12]
	
	movq mm0, [edx+4]
	movd mm1, [edx]
	movd mm5, [edx+4]
	movd mm4, [edx+8]
	punpckldq mm4, [edx]
	movd mm3, [ecx+4]
	movd mm2, [ecx+8]
	punpckldq mm2, [ecx]
	movq mm6, [ecx+4]
	movd mm7, [ecx]

	;Multiply column 1 & 2
	pfmul mm6, mm4
	pfmul mm7, mm5
	
	;3 & 4
	pfmul mm2, mm0
	pfmul mm3, mm1
	
	;1*2 - 3*4
	pfsub mm6, mm2
	pfsub mm7, mm3
	
	ML_ReturnVec33DNOW mm6, mm7
	ret
	
;float ML_Vec3Dot(ML_VEC3* pV1, ML_VEC3* pV2)
;Dot Product

;Floating Point:
global _ML_Vec3Dot_F
_ML_Vec3Dot_F:
	mov ecx, [esp+4]
	mov edx, [esp+8]

	fld   dword [ecx]   ;x1*x2+
	fmul  dword [edx]
	fld   dword [ecx+4] ;y1*y2+
	fmul  dword [edx+4]
	faddp st1
	fld   dword [ecx+8] ;z1*z2
	fmul  dword [edx+8]
	faddp st1
	ret                 ;result is in st0.
	
;SSE3:
global _ML_Vec3Dot_SSE3
_ML_Vec3Dot_SSE3:
	;Get points to vectors from stack and load them.
	mov ecx, [esp+4]
	mov edx, [esp+8]
	movups xmm0, [ecx]
	movups xmm1, [edx]

	;Remove w component from one vector with AND mask.
	andps xmm0, [FLOAT_SSE_NO_W_MASK]
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
	andps xmm0, [FLOAT_SSE_NO_W_MASK]
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
	
;Floating Point:
global _ML_Vec3Dot_3DNOW
_ML_Vec3Dot_3DNOW:
	mov ecx, [esp+4]
	mov edx, [esp+8]

	movq mm0, [ecx]
	movd mm1, [ecx+8]
	
	pfmul mm0, [edx]
	pand mm1, [FLOAT_3DNOW_NO_W_MASK]
	pfmul mm1, [edx+8]
	
	pfadd mm1, mm0
	pfacc mm1, mm1
	
	;Return the result
	sub esp, 4
	movd [esp], mm1
	femms
	fld dword [esp]
	add esp, 4
	ret
		
;float ML_Vec3Length(ML_VEC3* pV)
;Vector Length

;Floating Point
global _ML_Vec3Length_F
_ML_Vec3Length_F:
	mov ecx, [esp+4]
	ML_Vec3GetLength:
	fld   dword [ecx]
	fmul  dword [ecx]
	fld   dword [ecx+4]
	fmul  dword [ecx+4]
	faddp st1
	fld   dword [ecx+8]
	fmul  dword [ecx+8]
	faddp st1
	fsqrt
	ret
	
;SSE3:
global _ML_Vec3Length_SSE3
_ML_Vec3Length_SSE3:
	mov ecx, [esp+4]
	movups xmm0, [ecx]
	andps xmm0, [FLOAT_SSE_NO_W_MASK]
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
	andps xmm0, [FLOAT_SSE_NO_W_MASK]
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
	
;3DNOW
global _ML_Vec3Length_3DNOW
_ML_Vec3Length_3DNOW:
	mov ecx, [esp+4]
	
	movq mm0, [ecx]
	movd mm1, [ecx+8]
	
	;Square components
	pfmul mm0, mm0
	pfmul mm1, mm1
	
	;Sum componenets
	pfacc mm0, mm0
	pfadd mm0, mm1
	
	%ifndef LOW_PREC_SQRT
	;High Prec sqrt
	movq mm3, mm0
	pfrsqrt mm1, mm0
	movq mm2, mm1
	pfmul mm1, mm1
	pfrsqit1 mm0, mm1
	pfrcpit2 mm0, mm2
	pfmul mm3, mm0
	;return result.
	sub esp, 4
	movd [esp], mm3
	femms
	fld dword [esp]
	add esp, 4
	%else ;LOW_PREC_SQRT
	;Low prec sqrt
	pfrsqrt mm1, mm0
	pfmul mm1, mm0
	;Return result
	sub esp, 4
	movd [esp], mm1
	femms
	fld dword [esp]
	add esp, 4
	%endif ;LOW_PREC_SQRT
	
	ret
	
;ML_VEC3* ML_Vec3Normalize(ML_VEC3* pOut, ML_VEC3* pV);
;Normalize (unit 1 length) vector.

;Floating Point:
global _ML_Vec3Normalize_F
_ML_Vec3Normalize_F:
	mov eax, [esp+4]
	mov ecx, [esp+8]
	sub esp, 4

	;Find the magnitude (length) of the vector.
	call ML_Vec3GetLength ;Get the length (note we're not calling the global function).

	;Find the ratio (1/magnitude).
	mov   dword [esp], 3F800000H
	fld   dword [esp]
	fxch  st0, st1
	fdivp st1

	;Scale the vector by the ratio we found.
	call ML_Vec3DoScale
	add  esp, 4
	ret

;SSE3:
global _ML_Vec3Normalize_SSE3
_ML_Vec3Normalize_SSE3:
	mov eax, [esp+4]
	mov ecx, [esp+8]
	movups xmm0, [ecx]
	andps xmm0, [FLOAT_SSE_NO_W_MASK]
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
	andps xmm0, [FLOAT_SSE_NO_W_MASK]
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
	
;3DNOW
global _ML_Vec3Normalize_3DNOW
_ML_Vec3Normalize_3DNOW:
	mov eax, [esp+4]
	mov ecx, [esp+8]
	
	;mm0-mm1=pVec (duplicated in mm2-mm3)
	movq mm0, [ecx]
	movq mm1, [ecx+8]
	pand mm1, [FLOAT_3DNOW_NO_W_MASK]
	movq mm6, mm0
	movq mm7, mm1
	
	;Square components
	pfmul mm0, mm0
	pfmul mm1, mm1
	
	;Sum
	pfadd mm1, mm0
	pfacc mm1, mm1
	
	;Get reciprocal, use Newton-Raphson HW to improve accuracty, then multiply by the hight prec reciprocal.
	%ifndef LOW_PREC_SQRT
	movq mm3, mm1
	pfrsqrt mm0, mm1
	movq mm2, mm0
	pfmul mm0, mm0
	pfrsqit1 mm1, mm0
	pfrcpit2 mm1, mm2
	pfmul mm6, mm1
	pfmul mm7, mm1
	%else
	;Get fast rciprocal and return
	pfrsqrt mm1, mm1
	pfmul mm6, mm1
	pfmul mm7, mm1
	%endif
	
	;movq [eax], mm6
	;movd [eax+8], mm7
	;femms
	ML_ReturnVec33DNOW mm6, mm7
	ret
	

;ML_VEC3* ML_Vec3Subtract(ML_VEC3* pOut, ML_VEC3* pV1, ML_VEC3* pV2);
;Find difference between to vectors.

;Floating Point:
global _ML_Vec3Subtract_F
_ML_Vec3Subtract_F:
	mov  eax,  [esp+4] ;Get pointers to the parameters.
	mov  ecx,  [esp+8]
	mov  edx,  [esp+12]
	fld  dword [ecx]   ;Then subtract the x, y, and z component respectively.
   fsub dword [edx]
	fstp dword [eax]
	fld  dword [ecx+4]
	fsub dword [edx+4]
	fstp dword [eax+4]
	fld  dword [ecx+8]
	fsub dword [edx+8]
	fstp dword [eax+8] 
	ret                ;The return value of the output vector is already in eax.
	
;SSE:
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
	

;3DNOW:
global _ML_Vec3Subtract_3DNOW
_ML_Vec3Subtract_3DNOW:
	mov eax, [esp+4]
	mov ecx, [esp+8]
	mov edx, [esp+12]
	
	;Load registers then add.
	movq mm0, [ecx]
	movd mm1, [ecx+8]
	pfsub mm0, [edx]
	pfsub mm1, [edx+8]
	;Return result.
	ML_ReturnVec33DNOW mm0, mm1
	ret
	

;ML_VEC3* ML_Vec3Scale(ML_VEC3* pOut, ML_VEC3* pV, float s);
;Scales vector.

;Floating Point
global _ML_Vec3Scale_F
_ML_Vec3Scale_F:
	mov  eax,  [esp+4]   ;Get pointers to the parameters.
	mov  ecx,  [esp+8]

	fld   dword [esp+12] ;Multiply x, y, and z by scaler [esp+12].
	ML_Vec3DoScale:
	fld   dword [ecx]
	fmul  st1
	fstp  dword [eax]
	fld   dword [ecx+4]
	fmul  st1
	fstp  dword [eax+4]
	fld   dword [ecx+8]
	fmulp st1           ;Pop the scaler off the stack this time.
	fstp  dword [eax+8]
	ret                 ;result is already in eax.  
	
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
	
;3DNOW:
global _ML_Vec3Scale_3DNOW
_ML_Vec3Scale_3DNOW:
	mov eax, [esp+4]
	mov ecx, [esp+8]
	
	movd mm0, [esp+12]
	punpckldq mm0, mm0
	movq mm1, mm0
	pfmul mm0, [ecx]
	pfmul mm1, [ecx+8]
	
	ML_ReturnVec33DNOW mm0, mm1
	ret
	


;float ML_Vec3Distance(L_vector3* pV1, L_vector3* pV2)
;Finds the distance between two vectors.

;Floating Point:
global _ML_Vec3Distance_F
_ML_Vec3Distance_F:
	mov   ecx, [esp+4]  ;Use the pythagoram theorem to calculate the distance between two vecotrs.
	mov   edx, [esp+8]
	fld   dword [ecx]   ;Get (v1.x-v2.x)^2
	fsub  dword [edx]
	fld   st0
	fmulp st1
	fld   dword [ecx+4] ;Get (v1.y-v1.y)^2
	fsub  dword [edx+4]
	fld   st0
	fmulp st1
	faddp st1           ;Add to x^2
	fld   dword [ecx+8] ;Get (v1.z-v2.z)^2
	fsub  dword [edx+8]
	fld   st0
	fmulp st1
	faddp st1           ;Add to x^2+y^2
	fsqrt               ;Take the sqrt, and the value is saved in st0, which is the return value.
	ret
	
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
	
;3DNOW:	
global _ML_Vec3Distance_3DNOW
_ML_Vec3Distance_3DNOW:	
	mov ecx, [esp+4]
	mov edx, [esp+8]
	sub esp, 16
	mov eax, esp
	push edx
	push ecx
	push eax
	call _ML_Vec3Subtract_3DNOW
	call _ML_Vec3Length_3DNOW
	add esp, 28
	ret
	
;ML_VEC4* ML_FUNC ML_Vec3Transform(ML_VEC4* pOut, const ML_VEC3* pV, const ML_MAT* pM);
;Transforms a 3d vector (uses w as 1.0f).

;Floating Point:
global _ML_Vec3Transform_F
_ML_Vec3Transform_F:
	mov eax, [esp+4]
	mov ecx, [esp+8]
	mov edx, [esp+12]
	
	;Move pV onto the fp stack, that way if pV and pOut are the same
	;the result will still be the same.
	fld dword [ecx+8]
	fld dword [ecx+4]
	fld dword [ecx]
	
	;Calculate x
	fld dword [edx]
	fld st1
	fmulp st1, st0
	fld dword [edx+16]
	fld st3
	fmulp st1, st0
	faddp st1, st0
	fld dword [edx+32]
	fld st4
	fmulp st1, st0
	faddp st1, st0
	fadd dword [edx+48]
	fstp dword [eax]
	
	;Calculate y
	fld dword [edx+4]
	fld st1
	fmulp st1, st0
	fld dword [edx+20]
	fld st3
	fmulp st1, st0
	faddp st1, st0
	fld dword [edx+36]
	fld st4
	fmulp st1, st0
	faddp st1, st0
	fadd dword [edx+52]
	fstp dword [eax+4]
	
	;Calculate z
	fld dword [edx+8]
	fld st1
	fmulp st1, st0
	fld dword [edx+24]
	fld st3
	fmulp st1, st0
	faddp st1, st0
	fld dword [edx+40]
	fld st4
	fmulp st1, st0
	faddp st1, st0
	fadd dword [edx+56]
	fstp dword [eax+8]

	;Calculate w            ;Note that as we
	fld dword [edx+12]      ;perform the w calculations
	fmulp st1, st0          ;we are getting rid of the
	fld dword [edx+28]      ;original vector
	fmulp st2, st0          ;that was placed on the stack
	faddp st1, st0          ;at the start of the function.
	fld dword [edx+44]
	fmulp st2, st0
	faddp st1, st0
	fadd dword [edx+60]
	fstp dword [eax+12]
	ret
	
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

;3DNOW
global _ML_Vec3Transform_3DNOW
_ML_Vec3Transform_3DNOW:
	mov eax, [esp+4]
	mov ecx, [esp+8]
	mov edx, [esp+12]
	
	call ML_Vec3MultiplyMat_3DNOW
	
	movq [eax], mm2
	movq [eax+8], mm3
	femms
	ret
	
;Procedure for multiply vextor by matrix,
;assumes [ecx] is pointer to vector and [edx] is pointer matrix.
;Result is in mm2 and mm3
ML_Vec3MultiplyMat_3DNOW:
	movd mm0, [ecx]
	movd mm2, [ecx+4]
	movd mm4, [ecx+8]
	punpckldq mm0, mm0
	punpckldq mm2, mm2
	punpckldq mm4, mm4
	
	movq mm1, mm0
	movq mm3, mm2
	movq mm5, mm4
	
	pfmul mm0, [edx]
	pfmul mm1, [edx+8]
	pfmul mm2, [edx+16]
	pfmul mm3, [edx+24]
	pfmul mm4, [edx+32]
	pfmul mm5, [edx+40]
	pfadd mm4, [edx+48]
	pfadd mm5, [edx+56]
	
	;Sum it... (not displaying register contents...)
	
	;Combine X and Y column into mm2-3
	pfadd mm2, mm0
	pfadd mm3, mm1
	
	;Combine with ZW column into mm2-3
	pfadd mm2, mm4
	pfadd mm3, mm5
	ret
	
;ML_VEC3* ML_Vec3TransformCoord(ML_VEC3* pOut, ML_VEC3* pIn, ML_VEC3* pM)

;Floating Point:
global _ML_Vec3TransformCoord_F
_ML_Vec3TransformCoord_F:
	mov eax, [esp+4]
	mov ecx, [esp+8]
	mov edx, [esp+12]
	
	;Move pV onto the fp stack, that way if pV and pOut are the same
	;the result will still be the same.  We will clear the stack when
	;we calculate w.
	fld dword [ecx+8]
	fld dword [ecx+4]
	fld dword [ecx]
	
	;Calculate x
	fld dword [edx]
	fld st1
	fmulp st1, st0
	fld dword [edx+16]
	fld st3
	fmulp st1, st0
	faddp st1, st0
	fld dword [edx+32]
	fld st4
	fmulp st1, st0
	faddp st1, st0
	fadd dword [edx+48]
	fstp dword [eax]
	
	;Calculate y
	fld dword [edx+4]
	fld st1
	fmulp st1, st0
	fld dword [edx+20]
	fld st3
	fmulp st1, st0
	faddp st1, st0
	fld dword [edx+36]
	fld st4
	fmulp st1, st0
	faddp st1, st0
	fadd dword [edx+52]
	fstp dword [eax+4]
	
	;Calculate z
	fld dword [edx+8]
	fld st1
	fmulp st1, st0
	fld dword [edx+24]
	fld st3
	fmulp st1, st0
	faddp st1, st0
	fld dword [edx+40]
	fld st4
	fmulp st1, st0
	faddp st1, st0
	fadd dword [edx+56]
	fstp dword [eax+8]

	;Calculate w            ;Note that as we
	fld dword [edx+12]      ;perform the w calculations
	fmulp st1, st0          ;we are getting rid of the
	fld dword [edx+28]      ;original vector
	fmulp st2, st0          ;that was placed on the stack
	faddp st1, st0          ;at the start of the function.
	fld dword [edx+44]
	fmulp st2, st0
	faddp st1, st0
	fadd dword [edx+60]
	;fstp dword [eax+8] (we don't save w here, instead we use it for division).
	
	;Divide x, y, and z by w.
	fld dword [eax]
	fdiv st0, st1
	fstp dword [eax]
	fld dword [eax+4]
	fdiv st0, st1
	fstp dword [eax+4]
	fld dword [eax+8]
	fxch st0, st1
	fdivp st1, st0
	fstp dword [eax+8]	
	ret
	
;SSE:
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
	
;3DNOW
global _ML_Vec3TransformCoord_3DNOW
_ML_Vec3TransformCoord_3DNOW:
	mov eax, [esp+4]
	mov ecx, [esp+8]
	mov edx, [esp+12]
	
	call ML_Vec3MultiplyMat_3DNOW
	
	;Need to dived x, y, and z by w.
	
	;Get w by itself.
	movq mm1, mm3
	punpckhdq mm1, mm1
		
	%ifndef LOW_PREC_SQRT
	pfrcp mm0, mm1
	;punpckldq mm1, mm1
	pfrcpit1 mm1, mm0
	pfrcpit2 mm1, mm0
	%else
	pfrcp mm1, mm1
	%endif
	
	pfmul mm2, mm1
	pfmul mm3, mm1
	
	ML_ReturnVec33DNOW mm2, mm3
	ret
	
	
;ML_VEC4* ML_FUNC ML_Vec3Transform(ML_VEC4* pOut, const ML_VEC3* pV, const ML_MAT* pM);
;Transforms a 3d vector (uses w as 1.0f).

;Floating Point:
global _ML_Vec3TransformNormal_F
_ML_Vec3TransformNormal_F:
	mov eax, [esp+4]
	mov ecx, [esp+8]
	mov edx, [esp+12]
	
	;Move pV onto the fp stack, that way if pV and pOut are the same
	;the result will still be the same.
	fld dword [ecx+8]
	fld dword [ecx+4]
	fld dword [ecx]
	
	;Calculate x
	fld dword [edx]
	fld st1
	fmulp st1, st0
	fld dword [edx+16]
	fld st3
	fmulp st1, st0
	faddp st1, st0
	fld dword [edx+32]
	fld st4
	fmulp st1, st0
	faddp st1, st0
	fstp dword [eax]
	
	;Calculate y
	fld dword [edx+4]
	fld st1
	fmulp st1, st0
	fld dword [edx+20]
	fld st3
	fmulp st1, st0
	faddp st1, st0
	fld dword [edx+36]
	fld st4
	fmulp st1, st0
	faddp st1, st0
	fstp dword [eax+4]
	
	;Calculate z and get original vector off of the stack.
	fld dword [edx+8]
	fmulp st1, st0
	fld dword [edx+24]
	fmulp st2, st0
	faddp st1, st0
	fld dword [edx+40]
	fmulp st2, st0
	faddp st1, st0
	fstp dword [eax+8]
	ret
	
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
	andps xmm4, [FLOAT_SSE_NO_W_MASK]
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
	
global _ML_Vec3TransformNormal_3DNOW
_ML_Vec3TransformNormal_3DNOW:
	mov eax, [esp+4]
	mov ecx, [esp+8]
	mov edx, [esp+12]
	
	movd mm0, [ecx]
	movd mm2, [ecx+4]
	movd mm4, [ecx+8]
	punpckldq mm0, mm0
	punpckldq mm2, mm2
	punpckldq mm4, mm4
	
	movq mm1, mm0
	movq mm3, mm2
	movq mm5, mm4
	
	pfmul mm0, [edx]
	pfmul mm1, [edx+8]
	pfmul mm2, [edx+16]
	pfmul mm3, [edx+24]
	pfmul mm4, [edx+32]
	pfmul mm5, [edx+40]
	;pfadd mm4, [edx+48]
	;pfadd mm5, [edx+56]
	
	;Sum it... (not displaying register contents...)
	
	;Combine X and Y column into mm2-3
	pfadd mm2, mm0
	pfadd mm3, mm1
	
	;Combine with ZW column into mm2-3
	pfadd mm2, mm4
	pfadd mm3, mm5
	
	ML_ReturnVec33DNOW mm2, mm3
	ret
	
;float ML_Vec3Length(ML_VEC3* pV)
;Vector Length

;Floating Point
global _ML_Vec3LengthSq_F
_ML_Vec3LengthSq_F:
	mov ecx, [esp+4]
	ML_Vec3GetLengthSq:
	fld   dword [ecx]
	fmul  dword [ecx]
	fld   dword [ecx+4]
	fmul  dword [ecx+4]
	faddp st1
	fld   dword [ecx+8]
	fmul  dword [ecx+8]
	faddp st1
	ret
	
;SSE3:
global _ML_Vec3LengthSq_SSE3
_ML_Vec3LengthSq_SSE3:
	mov ecx, [esp+4]
	movups xmm0, [ecx]
	andps xmm0, [FLOAT_SSE_NO_W_MASK]
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
	andps xmm0, [FLOAT_SSE_NO_W_MASK]
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
	
;3DNOW
global _ML_Vec3LengthSq_3DNOW
_ML_Vec3LengthSq_3DNOW:
	mov ecx, [esp+4]
	
	movq mm0, [ecx]
	movd mm1, [ecx+8]
	
	;Square components
	pfmul mm0, mm0
	pfmul mm1, mm1
	
	;Sum componenets
	pfacc mm0, mm0
	pfadd mm0, mm1
	
	sub esp, 4
	movd [esp], mm0
	femms
	fld dword [esp]
	add esp, 4
	ret

	
[SECTION .data]
extern FLOAT_SSE_NO_W_MASK
extern FLOAT_3DNOW_NO_W_MASK



