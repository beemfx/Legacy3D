;ML_math.nasm - Basic math operations
;Copyright (c) 2006, Blaine Myers
;
;Optimized functions (SSE, 3DNOW, etc) are based upon SIMDx86 
;(www.sourceforge.net/projects/simdx86)

[BITS 32]
[SECTION .code]

global _ML_absl
_ML_absl:
	mov eax, [esp+4]
	cdq
	xor eax, edx
	sub eax, edx
	ret;
	
global _ML_absf
_ML_absf:
	fld dword [esp+4]
	fabs
	ret;

global _ML_sqrtf
_ML_sqrtf:
	fld dword [esp+4]
	fsqrt
	ret ;result will be in st0.
	
global _ML_cosf
_ML_cosf:
	fld dword [esp+4]
	fcos
	ret
	
global _ML_sinf
_ML_sinf:
	fld dword [esp+4]
	fsin
	ret
	
global _ML_sincosf
_ML_sincosf:
	mov ecx, [esp+8]
	fld dword [esp+4]
	fsincos
	fstp dword [ecx]
	ret
	
global _ML_tanf
_ML_tanf:
	fld dword [esp+4]
	;dividing the sin and cos is more effective than ftan
	fsincos
	fdivp st1
	ret ;result is in st0
	
global _ML_acosf
_ML_acosf: 
	fld dword [esp+4] ;X
   fld st0           ;X X
   fmul st1          ;X^2 X
   fld1              ;1 X^2 X
   fsubrp st1, st0   ;(1-X^2) X
   fsqrt             ;sqrt(1-X^2) X
   fxch st1          ;X sqrt(1-X^2)
   fpatan            ;arctan(sqrt(1-X^2)/X)
	ret
	
global _ML_asinf
_ML_asinf:
	fld dword [esp+4] ;X
   fld st0           ;X X
   fmul st1          ;X^2 X
   fld1              ;1 X^2 X
   fsubrp st1, st0   ;(1-X^2) X
   fsqrt             ;sqrt(1-X^2) X
   fpatan            ;arctan(X/sqrt(1-X^2))	
	ret
	
	
;function ArcSin2(x : extended) : extended ;
;assembler ; asm
;    fld [x]           { X }
;    fld st(0)         { X   X }
;    fmul st,st(0)     { X^2 X }
;    fld1              { 1 X^2 X }
;    fsubrp st(1),st   { (1-X^2) X }
;    FSQRT             { sqrt(1-X^2) X }
;    fpatan            { Uses both arguments }
;    end {ArcSin2} ;
