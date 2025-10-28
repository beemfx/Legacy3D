;ML_mat.nasm - Matrix operations.
;Copyright (c) 2006, Blaine Myers
;
;Optimized functions (SSE, 3DNOW, etc) are based upon SIMDx86 
;(www.sourceforge.net/projects/simdx86)

[BITS 32]
[SECTION .code]

%macro L_matmul_element2 2 ;%1=row %2=column
	fld   dword [ecx+16*%1+0]
	fld   dword [edx+4*%2+0]
	fmulp st1  

	fld   dword [ecx+16*%1+4]
	fld   dword [edx+4*%2+16]
	fmulp st1
	faddp st1

	fld   dword [ecx+16*%1+8]
	fld   dword [edx+4*%2+32]
	fmulp st1
	faddp st1

	fld   dword [ecx+16*%1+12]
	fld   dword [edx+4*%2+48]
	fmulp st1
	faddp st1
	fstp  dword [esp+16*%1+4*%2]
%endmacro


%macro L_matmul_element 2; %1=row %2=column;
	fld   dword [ecx+16*%1+0]
	fmul  dword [edx+4*%2+0]

	fld   dword [ecx+16*%1+4]
	fmul  dword [edx+4*%2+16]
	faddp st1

	fld   dword [ecx+16*%1+8]
	fmul  dword [edx+4*%2+32]
	faddp st1

	fld   dword [ecx+16*%1+12]
	fmul  dword [edx+4*%2+48]
	faddp st1
	fstp  dword [esp+16*%1+4*%2]
%endmacro

global _ML_MatMultiply_F
_ML_MatMultiply_F:
	mov eax, [esp+4]  ;Get pOut.
	mov ecx, [esp+8]  ;Get pM1.
	mov edx, [esp+12] ;Get pM2.

	sub esp, 64   ;Make room for output matrix on the stack (64 bytes).
	;Multiply matrix to get result for each element of output matrix.
	L_matmul_element 0,0
	L_matmul_element 0,1
	L_matmul_element 0,2
	L_matmul_element 0,3
	L_matmul_element 1,0
	L_matmul_element 1,1
	L_matmul_element 1,2
	L_matmul_element 1,3
	L_matmul_element 2,0
	L_matmul_element 2,1
	L_matmul_element 2,2
	L_matmul_element 2,3
	L_matmul_element 3,0
	L_matmul_element 3,1
	L_matmul_element 3,2
	L_matmul_element 3,3
	
	;Move matrix from stack to pOut.
	push edi
	push esi
	mov ecx, 16
	lea edi, [eax]
	lea esi, [esp+8]
	rep movsd
	pop esi
	pop edi
	add esp, 64

	ret ;Return to caller, pOut is in eax.
	
global _ML_MatMultiply_SSE
_ML_MatMultiply_SSE:
	mov eax, [esp+4]  ;Get pOut.
	mov ecx, [esp+8]  ;Get pM1.
	mov edx, [esp+12] ;Get pM2.
	
	movups xmm0, [edx]
	movups xmm1, [edx+16]
	movups xmm2, [edx+32]
	movups xmm3, [edx+48]
	
	;Processes 1/2 of the matrix at a time (2x4), unrolled loop
	movss xmm4, [ecx]
	movss xmm6, [ecx+4]
	movss xmm5, [ecx+16]
	movss xmm7, [ecx+20]
	shufps xmm4, xmm4, 0x00
	shufps xmm5, xmm5, 0x00
	shufps xmm6, xmm6, 0x00
	shufps xmm7, xmm7, 0x00
	mulps xmm4, xmm0
	mulps xmm5, xmm0
	mulps xmm6, xmm1
	mulps xmm7, xmm1
	addps xmm5, xmm7
	addps xmm4, xmm6
	
	movss xmm6, [ecx+8]
	movss xmm7, [ecx+24]
	shufps xmm6, xmm6, 0x00
	shufps xmm7, xmm7, 0x00
	mulps xmm6, xmm2
	mulps xmm7, xmm2
	addps xmm4, xmm6
	addps xmm5, xmm7
	
	movss xmm6, [ecx+12]
	movss xmm7, [ecx+28]
	shufps xmm6, xmm6, 0x00
	shufps xmm7, xmm7, 0x00
	mulps xmm6, xmm3
	mulps xmm7, xmm3
	addps xmm4, xmm6
	addps xmm5, xmm7
	
	movups [eax], xmm4
	movups [eax+16], xmm5
	
	; second half of the matrix
	movss xmm4, [ecx+32]
	movss xmm6, [ecx+36]
	movss xmm5, [ecx+48]
	movss xmm7, [ecx+52]
	shufps xmm4, xmm4, 0x00
	shufps xmm5, xmm5, 0x00
	mulps xmm4, xmm0
	mulps xmm5, xmm0
	
	shufps xmm6, xmm6, 0x00
	shufps xmm7, xmm7, 0x00
	mulps xmm6, xmm1
	mulps xmm7, xmm1
	addps xmm4, xmm6
	addps xmm5, xmm7
	
	movss xmm6, [ecx+40]
	movss xmm7, [ecx+56]
	shufps xmm6, xmm6, 0x00
	shufps xmm7, xmm7, 0x00
	mulps xmm6, xmm2
	mulps xmm7, xmm2
	addps xmm4, xmm6
	addps xmm5, xmm7
	
	movss xmm6, [ecx+44]
	movss xmm7, [ecx+60]
	shufps xmm6, xmm6, 0x00
	shufps xmm7, xmm7, 0x00
	mulps xmm6, xmm3
	mulps xmm7, xmm3
	addps xmm4, xmm6
	addps xmm5, xmm7
	
	movups [eax+32], xmm4
	movups [eax+48], xmm5
	ret
	
global _ML_MatMultiply_3DNOW
_ML_MatMultiply_3DNOW:
	push ebx
	mov ebx, [esp+12]  ;Get pM1.
	mov edx, [esp+16] ;Get pM2.
	
	;Make room for output matrix on the stack.
	sub esp, 64
	mov eax, esp
	
	mov ecx, 4
	ML_MatMultiply_3DNOW_Loop:
	movd mm0, [ebx]
	movd mm2, [ebx+4]
	movd mm4, [ebx+8]
	movd mm6, [ebx+12]
	
	;prefetch [ebx+16]
	;prefetchw [eax+16]
	
	punpckldq mm0, mm0
	punpckldq mm2, mm2
	punpckldq mm4, mm4
	punpckldq mm6, mm6
	
	movq mm1, mm0
	movq mm3, mm2
	movq mm5, mm4
	movq mm7, mm6
	
	pfmul mm0, [edx]
	pfmul mm1, [edx+8]
	pfmul mm2, [edx+16]
	pfmul mm3, [edx+24]
	pfmul mm4, [edx+32]
	pfmul mm5, [edx+40]
	pfmul mm6, [edx+48]
	pfmul mm7, [edx+56]
	
	pfadd mm2, mm0
	pfadd mm3, mm1
	pfadd mm6, mm4
	pfadd mm7, mm5
	pfadd mm6, mm2
	pfadd mm7, mm3
	
	movq [eax], mm6
	movq [eax+8], mm7
	
	add ebx, 16
	add eax, 16
	
	loop ML_MatMultiply_3DNOW_Loop
	
	;Move matrix from stack to pOut.
	mov eax, [esp+8+64] ;Get pOut in eax.
	push edi
	push esi
	mov ecx, 16
	lea edi, [eax]
	lea esi, [esp+8]
	rep movsd
	pop esi
	pop edi
	add esp, 64
	
	pop ebx
	femms
	ret


global _ML_MatAdd_SSE
_ML_MatAdd_SSE:
	mov eax, [esp+4]  ;Get pOut.
	mov ecx, [esp+8]  ;Get pM1.
	mov edx, [esp+12] ;Get pM2.
	
	movups xmm0, [edx]
	movups xmm1, [edx+16]
	movups xmm2, [edx+32]
	movups xmm3, [edx+48]
	movups xmm4, [ecx]
	movups xmm5, [ecx+16]
	movups xmm6, [ecx+32]
	movups xmm7, [ecx+48]
	
	addps xmm4, xmm0
	addps xmm5, xmm1
	addps xmm6, xmm2
	addps xmm7, xmm3
	
	movups [eax], xmm4
	movups [eax+16], xmm5
	movups [eax+32], xmm6
	movups [eax+48], xmm7
	ret
	
;ML_MAT* ML_MatIdentity(ML_MAT* pOut);
;Sets the matrix passed as a parameter to identity matrix.
global _ML_MatIdentity
_ML_MatIdentity:                  ;L_matrix* __cdecl L_matident(L_matrix* pOut);
	mov eax, [esp+4]              ;Save a pointer to the matrix pointer in eax.
	mov [eax],    dword 3F800000H ;(_11) The following lines set up the identity matrix
	mov [eax+4],  dword 0         ;(_12) according to the L_matrix type.
	mov [eax+8],  dword 0         ;(_13)
	mov [eax+12], dword 0         ;(_14) Note that 3F800000H is 1.0f.
	
	mov [eax+16], dword 0         ;(_21)
	mov [eax+20], dword 3F800000H ;(_22)
	mov [eax+24], dword 0         ;(_23)
	mov [eax+28], dword 0         ;(_24)

	mov [eax+32], dword 0         ;(_31)
	mov [eax+36], dword 0         ;(_32)
	mov [eax+40], dword 3F800000H ;(_33)
	mov [eax+44], dword 0         ;(_34)

	mov [eax+48], dword 0         ;(_41)
	mov [eax+52], dword 0         ;(_42)
	mov [eax+56], dword 0         ;(_43)
	mov [eax+60], dword 3F800000H ;(_44)
	ret                           ;Return to the caller, pointer to matrix in eax.
	
global _ML_MatRotationX
_ML_MatRotationX:
	;First thing is to get an identity matrix.
	pop ecx
	call _ML_MatIdentity
	push ecx
	
	mov eax, [esp+4]    ;Get pointer to the matrix.
	fld dword [esp+8]   ;Put the rotation angle on the fp stack.
	fsincos             ;Get the sin and cosin of the angle
	fst dword [eax+20]  ;The cos is for _22 and _33.
	fstp dword [eax+40] 
	fst dword [eax+24]  ;_23 is the sin.
	fchs                ;_32 is 2 sin or negative sin.
	fstp dword [eax+36]
	ret

global _ML_MatRotationY
_ML_MatRotationY:
	;First thing is to get an identit matrix.
	pop ecx
	call _ML_MatIdentity
	push ecx
	
	mov eax, [esp+4]    ;Get pointer to the matrix.
	fld dword [esp+8]   ;Put the rotation angle on the fp stack.
	fsincos             ;Get the sin and cosin of the angle
	
	;Copy values into output matrix.
	fst dword [eax+0]
	fstp dword [eax+40]
	fst dword [eax+32]
	fchs
	fstp dword [eax+8]
	ret
	
global _ML_MatRotationZ
_ML_MatRotationZ:
	;First thing is to get an identit matrix.
	pop ecx
	call _ML_MatIdentity
	push ecx
	
	mov eax, [esp+4]    ;Get pointer to the matrix.
	fld dword [esp+8]   ;Put the rotation angle on the fp stack.
	fsincos             ;Get the sin and cosin of the angle
	
	;Copy values into output matrix.
	fst dword [eax+0]
	fstp dword [eax+20]
	fst dword [eax+4]
	fchs
	fstp dword [eax+16]
	ret
	
global _ML_MatRotationQuat
_ML_MatRotationQuat:
	mov eax, [esp+4]
	mov ecx, [esp+8]
	
	;Set _44 to 1.0f, we do this now because 1.0f is used
	;by a few of the calculations.
	mov dword [eax+60], 3F800000H
	
	;Put x, y, z, w on the stack.
	;If w came first we could modify the order
	;the vars are pushed onto the stack.
	fld dword [ecx+12]
	fld dword [ecx+8]
	fld dword [ecx+4]
	fld dword [ecx]
	
	;_11
	fld st2
	fadd st0, st0
	fmul st0, st3
	fld st2
	fadd st0, st0
	fmul st0, st3
	fld dword [eax+60]
	fsubrp st1, st0
	fsubrp st1, st0
	fstp dword [eax]
	
	;_12
	fld st3
	fadd st0, st0
	fmul st0, st3
	fld st1
	fadd st0, st0
	fmul st0, st3
	faddp st1, st0
	fstp dword [eax+4]
	
	;_13 not right
	fld st3
	fadd st0, st0
	fmul st0, st2
	fld st1
	fadd st0, st0
	fmul st0, st4
	fsubrp st1, st0
	fstp dword [eax+8]
	
	;_14
	mov dword [eax+12], 0x00000000
	
	;_21
	fld st3
	fadd st0, st0
	fmul st0, st3
	fld st1
	fadd st0, st0
	fmul st0, st3
	fsubrp st1, st0
	fstp dword [eax+16]
	
	;_22
	fld st2
	fadd st0, st0
	fmul st0, st3
	fld st1
	fadd st0, st0
	fmul st0, st2
	fld dword [eax+60]
	fsubrp st1, st0
	fsubrp st1, st0
	fstp dword [eax+20]
	
	;_23
	fld st3
	fadd st0, st0
	fmul st0, st1
	fld st2
	fadd st0, st0
	fmul st0, st4
	faddp st1, st0
	fstp dword [eax+24]
	
	;_24
	mov dword [eax+28], 0x00000000
	
	;_31
	fld st3
	fadd st0, st0
	fmul st0, st2
	fld st1
	fadd st0, st0
	fmul st0, st4
	faddp st1, st0
	fstp dword [eax+32]
	
	;I calculate _33 before 32 so I can get rid of x, y, z, and w on the stack
	;while caluclating _32
	
	;_33
	fld st1
	fadd st0, st0
	fmul st0, st2
	fld st1
	fadd st0, st0
	fmul st0, st2
	fld dword [eax+60]
	fsubrp st1, st0
	fsubrp st1, st0
	fstp dword [eax+40]
	
	;Should get rid of x, y, z, and w
	;for that reason _32 is calculated last
	;and slightly differently.
	;_32
	fadd st0, st0
	fmulp st3, st0
	fadd st0, st0
	fmulp st1, st0
	fsubrp st1, st0
	fstp dword [eax+36]
	
	;_34
	mov dword [eax+44], 0x00000000
	;_41
	mov dword [eax+48], 0x00000000
	;_42
	mov dword [eax+52], 0x00000000
	;_43
	mov dword [eax+56], 0x00000000
	;_44 has already been set.
	
	ret
