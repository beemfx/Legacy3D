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

;/** ML_VEC3 3DNOW FUNCTIONS***
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




%macro ML_ReturnVec33DNOW 2
	movq [eax], %1
	movd [eax+8], %2
	femms
%endmacro


;;;;;
;Add;
;;;;;
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

;;;;;;;;;;
;Subtract;
;;;;;;;;;;	
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

;;;;;;;
;Cross;
;;;;;;;	
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
	

;;;;;
;Dot;
;;;;;
;3DNOW:
global _ML_Vec3Dot_3DNOW
_ML_Vec3Dot_3DNOW:
	mov ecx, [esp+4]
	mov edx, [esp+8]

	movq mm0, [ecx]
	movd mm1, [ecx+8]
	
	pfmul mm0, [edx]
	pand mm1, [_FLOAT_3DNOW_NO_W_MASK]
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
		
;;;;;;;;
;Length;
;;;;;;;;	
;3DNOW:
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

;;;;;;;;;;
;LengthSq;
;;;;;;;;;;	
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
	

;;;;;;;;;;;
;Normalize;
;;;;;;;;;;;
;3DNOW:
global _ML_Vec3Normalize_3DNOW
_ML_Vec3Normalize_3DNOW:
	mov eax, [esp+4]
	mov ecx, [esp+8]
	
	;mm0-mm1=pVec (duplicated in mm2-mm3)
	movq mm0, [ecx]
	movq mm1, [ecx+8]
	pand mm1, [_FLOAT_3DNOW_NO_W_MASK]
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
	
;;;;;;;
;Scale;
;;;;;;;	
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
	

;;;;;;;;;;
;Distance;
;;;;;;;;;;	
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
	
;;;;;;;;;;;;
;DistanceSq;
;;;;;;;;;;;;	
;3DNOW:	
global _ML_Vec3DistanceSq_3DNOW
_ML_Vec3DistanceSq_3DNOW:	
	mov ecx, [esp+4]
	mov edx, [esp+8]
	sub esp, 16
	mov eax, esp
	push edx
	push ecx
	push eax
	call _ML_Vec3Subtract_3DNOW
	call _ML_Vec3LengthSq_3DNOW
	add esp, 28
	ret
	
;;;;;;;;;;;
;Transform;
;;;;;;;;;;;
;3DNOW:
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
	
;;;;;;;;;;;;;;;;
;TransformCoord;
;;;;;;;;;;;;;;;;
;3DNOW:
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
	
	
;;;;;;;;;;;;;;;;;
;TransformNormal;
;;;;;;;;;;;;;;;;;	
;3DNOW:
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
	
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;Vector Multiplicalion Procedure for Transforms;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;	
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
	
	
[SECTION .data]
extern _FLOAT_3DNOW_NO_W_MASK



