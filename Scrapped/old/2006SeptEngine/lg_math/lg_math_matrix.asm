;lg_math_matrix.asm - 3D math routines for the Legacy 3D Engine.
;Matrix operations.
;Copyright (c) 2006, Blaine Myers
;
;All functions are __cdecl calling convention (see lm_math.h).

[BITS 32] ;Set 32 bit code generation.

[SECTION .code] ;Section containing code.

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

global _L_matmultiply
_L_matmultiply:
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
	
	pop dword [eax+0]    ;Pop the output matrix off of the stack into pOut.
	pop dword [eax+4]    ;May be a faster way to move memory around.
	pop dword [eax+8]
	pop dword [eax+12]
	pop dword [eax+16]
	pop dword [eax+20]
	pop dword [eax+24]
	pop dword [eax+28]
	pop dword [eax+32]
	pop dword [eax+36]
	pop dword [eax+40]
	pop dword [eax+44]
	pop dword [eax+48]
	pop dword [eax+52]
	pop dword [eax+56]
	pop dword [eax+60]
	;add esp, 64

	ret ;Return to caller, pOut is in eax.


;L_matrix* __cdecl L_matident(L_matrix* pOut);
;Sets the matrix passed as a parameter to identity matrix.
global _L_matident
_L_matident:                     ;L_matrix* __cdecl L_matident(L_matrix* pOut);
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
