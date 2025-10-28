;ML_shared.nasm - Various functions and data for ML_lib library.
;Copyright (c) 2006, Blaine Myers
;
;Optimized functions (SSE, 3DNOW, etc) are based upon SIMDx86 
;(www.sourceforge.net/projects/simdx86)

[BITS 32]
[SECTION .code]


%macro DECLARE_CB_FUNC 1
global _ML_%1
	_ML_%1:
		jmp [_ML_%1CB] ;When we jump to the function, it takes over and it will return.
%endmacro

DECLARE_CB_FUNC Vec3Add
DECLARE_CB_FUNC Vec3Cross
DECLARE_CB_FUNC Vec3Dot
DECLARE_CB_FUNC Vec3Length
DECLARE_CB_FUNC Vec3LengthSq
DECLARE_CB_FUNC Vec3Normalize
DECLARE_CB_FUNC Vec3Scale
DECLARE_CB_FUNC Vec3Subtract
DECLARE_CB_FUNC Vec3Distance
DECLARE_CB_FUNC Vec3Transform
DECLARE_CB_FUNC Vec3TransformCoord
DECLARE_CB_FUNC Vec3TransformNormal

DECLARE_CB_FUNC MatMultiply
DECLARE_CB_FUNC MatInverse

[SECTION .data]
%macro DECLARE_CB 1
	global _ML_%1CB
	_ML_%1CB dd 0x00000000
%endmacro

;The masking data is used in varios SIMD functions.
global FLOAT_SSE_NO_W_MASK
FLOAT_SSE_NO_W_MASK dd 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x00000000
global FLOAT_3DNOW_NO_W_MASK
FLOAT_3DNOW_NO_W_MASK dd 0xFFFFFFFF, 0x00000000

DECLARE_CB Vec3Add
DECLARE_CB Vec3Cross
DECLARE_CB Vec3Dot
DECLARE_CB Vec3Length
DECLARE_CB Vec3LengthSq
DECLARE_CB Vec3Normalize
DECLARE_CB Vec3Scale
DECLARE_CB Vec3Subtract
DECLARE_CB Vec3Distance
DECLARE_CB Vec3Transform
DECLARE_CB Vec3TransformCoord
DECLARE_CB Vec3TransformNormal

DECLARE_CB MatMultiply
DECLARE_CB MatInverse




