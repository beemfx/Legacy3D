;ML_misc.nasm - Miscelanious functions for ML_lib library.
;Copyright (c) 2006, Blaine Myers
;
;Optimized functions (SSE, 3DNOW, etc) are based upon SIMDx86 
;(www.sourceforge.net/projects/simdx86)

[BITS 32]
[SECTION .code]

;ml_dword ML_NextPow2(ml_dword n)
;
;Finds the next highest power of 2 relative to n.
;Primarily used to find texture sizes that would be
;power 2 compatible (in case the original texture was
;not power 2 compatible.  This isn't necisarily the
;fastest available code, but it was built solid, as not
;to result in errors no matter what value is passed to
;n (even if the value is completely unrealistic).
global _ML_NextPow2
_ML_NextPow2:
	mov eax, 80000000h  ;First of all we are going to make sure the value passed
	mov ecx, [esp+4]    ;is greater than the highest value allowed (0x80000000).
	cmp ecx, eax        ;We do this by moving the high value into eax, then comparing
	jae L_nextpow2Exit  ;it with the parameter (which we move into ecx), and if the value
	mov eax, 00000001h  ;Is greater we jump to the exit, leaving the high value in eax.
	L_nextpow2Loop:     ;A label for the loop.
	cmp eax, ecx        ;Compare the parameter with the current value in eax.
	jae L_nextpow2Exit  ;If (eax>=ecx) (result>=n) goto L_nexpow2b to return;
	shl eax, 1          ;If not, shift eax left one (in effect going to the next highest power of 2).
	jmp L_nextpow2Loop  ;and redo the loop.
	L_nextpow2Exit:     ;The label for the loop exit.
	ret                 ;Return with appropriate value in eax.

; L_nextpow2_ASM_P2()
;
; Prototype: lg_dword __cdecl L_nextpow2(lg_dword n);
; Raises a value to the next nearest power of 2
; primarily designed to find texture sizes that would
; be power 2 compatible.  This code was taken from a
; forum on www.gamedev.net and was modified for NASM
; assembly.  Does not work on Pentium 1's.
global _ML_NextPow2_P2;
_ML_NextPow2_P2: ;Function is __cdecl
	mov ecx, [esp+4] ;Get the value we want to test from the stack (n).
	dec ecx          ;Decrease ecx (n) by 1, this way, when we find the next highest power of 2 if the value was already a power of 2 it will be the same value.
	bsr ecx, ecx     ;Perfrom a bsr on ecx, which gets the first bit that is set to 1, this is will be the power we need to increase by minus 1.
	inc ecx          ;Increase that number by 1 (this give us the power we want to increase to).
	mov eax, 1       ;Set eax to one,
	shl eax, cl      ;then left shift it to set it to the power we need.
	ret              ;Return control to the caller.

;ml_dword __cdecl ML_Pow2(lg_byte n);
;Finds (2^n), used for getting valid dimensions
;for textures and the like.
global _ML_Pow2;
_ML_Pow2:
	mov cl, [esp+4]
	mov eax, 0x00000001
	shl eax, cl
	ret
